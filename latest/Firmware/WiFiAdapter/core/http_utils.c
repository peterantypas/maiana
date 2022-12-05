#include "http_utils.h"
#include <stdlib.h>
#include <sys/param.h>

int read_http_body(httpd_req_t *req, char *dest, int destLen)
{
  size_t recv_size = MIN(req->content_len, destLen);
  int bytesIn = 0;
  char *p = dest;

  while (bytesIn < recv_size)
  {
    int len = httpd_req_recv(req, p, recv_size-bytesIn);
    if (len <= 0)
    {
      return len;
    }
    else
    {
      p += len;
      bytesIn += len;
    }

  }

  return bytesIn;
}
