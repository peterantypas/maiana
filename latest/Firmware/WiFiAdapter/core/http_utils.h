#ifndef __HTTP_UTILS_H__
#define __HTTP_UTILS_H__

#include <esp_http_server.h>


int read_http_body(httpd_req_t *req, char *dest, int destLen);

#endif
