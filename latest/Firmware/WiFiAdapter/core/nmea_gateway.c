#include "nmea_gateway.h"
#include "../bsp/bsp.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "types.h"
#include "configuration.h"
#include <esp_log.h>
#include <sys/param.h>
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>


static const char *TAG = "nmea";
static nmea_data_callback_t *__callback = NULL;

//////////////////////////////////////////////////////////////////////////////
// Private types
//////////////////////////////////////////////////////////////////////////////
#define MAX_CONNECTIONS 4

typedef bool (nmea_init_func)(void*);
typedef void (nmea_term_func)(void*);
typedef void (nmea_process_func)(void*, const char *);

typedef struct 
{
  nmea_gateway_mode_t mode;
  int fd;
  char ip[16];
  uint16_t port;
  int connections[MAX_CONNECTIONS];
  nmea_init_func *init;
  nmea_term_func *term;
  nmea_process_func *cb;
  TaskHandle_t task;
  struct sockaddr_in addr;
}
nmea_server_t;

//////////////////////////////////////////////////////////////////////////////
// Private data
//////////////////////////////////////////////////////////////////////////////

#define QUEUE_LENGTH  20

static TaskHandle_t __task_handle;
static StaticQueue_t __queue;
static QueueHandle_t __queue_handle;
static serial_message_t __queue_data[QUEUE_LENGTH];
static serial_message_t __buff = {0};
static int __pos = 0;
static nmea_server_t __nmea_server = {0};

//////////////////////////////////////////////////////////////////////////////
// Implementation
//////////////////////////////////////////////////////////////////////////////

void uart_rx_cb(char c)
{
  __buff.text[__pos++] = c;
  if ( __pos == sizeof __buff.text )
    __pos = 0;

  if ( c == '\n' )
    {
      __buff.text[__pos] = 0;
      xQueueSend(__queue_handle, &__buff, portMAX_DELAY);
      __pos = 0;
    }
}

//////////////////////////////////////////////////////////////////////////////
// TCP Server
//////////////////////////////////////////////////////////////////////////////
void tcp_server_task(void *p);

bool tcp_server_init(void *p)
{
  nmea_server_t *server = p;
  strcpy(server->ip, "0.0.0.0");
  server->port = config_get_nmea_gateway_port();
  memset(server->connections, -1, sizeof server->connections);
  xTaskCreate(tcp_server_task, "tcpserver", 2048, p, 4, &server->task);
  return true;
}

void tcp_server_shutdown(void *p)
{
  nmea_server_t *server = p;
  close(server->fd);
  sleep(1);  
}

void tcp_server_process(void *p, const char *text)
{
  nmea_server_t *server = p;
  for ( int i = 0; i < MAX_CONNECTIONS; ++i )
  {
    int fd = server->connections[i];
    if ( fd < 0 )
      continue;

    int sent = send(fd, text, strlen(text), 0);
    if ( sent < 0 )
    {
      ESP_LOGI(TAG, "Socket %d closed", fd);
      close(fd);
      server->connections[i] = -1;
    }
  }
}

void tcp_server_task(void *p)
{
  nmea_server_t *server = (nmea_server_t*)p;

  memset(&server->addr, 0, sizeof(server->addr));
  server->addr.sin_len = sizeof (server->addr);
  server->addr.sin_addr.s_addr = htonl(INADDR_ANY); //Change hostname to network byte order
  server->addr.sin_family = AF_INET;		//Define address family as Ipv4
  server->addr.sin_port = htons(config_get_nmea_gateway_port()); 	//Define PORT
  server->fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  if (server->fd < 0)
  {
    ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
  }

  if ( bind(server->fd, (struct sockaddr *)&server->addr, sizeof(server->addr)) )
  {
    ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
  }

  if ( listen(server->fd, 3) )
  {
    ESP_LOGE(TAG, "Error occured during listen: errno %d", errno);
  }

  while (true)
  {
    struct sockaddr_in sourceAddr; // Large enough for IPv4
    uint addrLen = sizeof(sourceAddr);

    int client_socket = accept(server->fd, (struct sockaddr *)&sourceAddr, &addrLen);
    if (client_socket >= 0 )
    {
      ESP_LOGI(TAG, "Client opened socket %d", client_socket);
      bool success = false;
      for ( int i = 0; i < MAX_CONNECTIONS; ++i )
      {
        if ( server->connections[i] < 0 )
        {
          int one = 1;
          server->connections[i] = client_socket;
          success = true;
          setsockopt(client_socket, IPPROTO_TCP, TCP_NODELAY, &one, sizeof one);
          break;
        }        
      }

      if ( !success )
        close(client_socket);
    }
    else
    {
      // The socket has been closed
      ESP_LOGE(TAG, "Shutting down TCP listener");
      break;
    }

  } // while

  vTaskDelete(NULL);
}


//////////////////////////////////////////////////////////////////////////////
// UDP Sender
//////////////////////////////////////////////////////////////////////////////
bool udp_sender_init(void *p)
{
  nmea_server_t *server = (nmea_server_t*)p;

  server->fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
  if (server->fd < 0)
  {
    ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
    return false;
  }

  memset(&server->addr, 0, sizeof server->addr);
  server->addr.sin_len = sizeof (server->addr);
  server->addr.sin_addr.s_addr = inet_addr(config_get_nmea_gateway_ip());
  server->addr.sin_family = AF_INET;		
  server->addr.sin_port = htons(config_get_nmea_gateway_port()); 	

  return true;
}

void udp_sender_shutdown(void *p)
{
  nmea_server_t *server = (nmea_server_t*)p;
  close(server->fd);
}

void udp_sender_process(void *p, const char *text)
{
  nmea_server_t *server = (nmea_server_t*)p;
  sendto(server->fd, text, strlen(text), 0, (struct sockaddr *)&server->addr, sizeof server->addr);
}


//////////////////////////////////////////////////////////////////////////////
// Utilities
//////////////////////////////////////////////////////////////////////////////
void nmea_gateway_set_callback(nmea_data_callback_t *cb)
{
  __callback = cb;
}

void nmea_gateway_send_command(const char *command)
{
  bsp_uart_write(command);
}


void nmea_input_task(void *params)
{
  ESP_LOGI(TAG, "Started NMEA task");
  serial_message_t msg;
  while (true)
  {
    if ( xQueueReceive(__queue_handle, &msg, portMAX_DELAY) == pdTRUE )
    {
      if ( msg.text[0] == '!' || msg.text[0] == '$' )
      {
        // This is a NMEA sentence
        if ( __callback )
          (*__callback)(msg.text);

        if ( __nmea_server.cb )
          (*__nmea_server.cb)(&__nmea_server, msg.text);

      }      
      else
      {
        // It's something else
      }
    }
  }
}

void configure_network()
{
  __nmea_server.mode = config_get_nmea_gateway_mode();
  switch (__nmea_server.mode)
  {
    case NMEA_TCP_LISTENER:
      {
        __nmea_server.init = tcp_server_init;
        __nmea_server.term = tcp_server_shutdown;
        __nmea_server.cb = tcp_server_process;
        __nmea_server.init(&__nmea_server);
      }
      break;
    case NMEA_UDP_SENDER:
      {
        __nmea_server.init = udp_sender_init;
        __nmea_server.term = udp_sender_shutdown;
        __nmea_server.cb = udp_sender_process;
        __nmea_server.init(&__nmea_server);
      }
      break;
  }
}

void nmea_gateway_restart();

static void nmea_restart_handler(void *args, esp_event_base_t base, int32_t id, void *data)
{
  nmea_gateway_restart();
}

void nmea_gateway_start()
{
  configure_network();
  bsp_set_uart_rx_cb(uart_rx_cb);

  esp_event_handler_register(MAIANA_EVENT, NMEA_RESTART_EVENT, nmea_restart_handler, NULL); 

  __queue_handle = xQueueCreateStatic(QUEUE_LENGTH, sizeof(serial_message_t), (uint8_t*)__queue_data, &__queue);
  xTaskCreate(nmea_input_task, "nmea", 2048, NULL, 4, &__task_handle);
}

void nmea_gateway_restart()
{
  __nmea_server.cb = NULL;
  __nmea_server.term(&__nmea_server);
  configure_network();
}

