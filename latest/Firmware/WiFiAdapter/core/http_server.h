#ifndef __HTTP_SERVER_H__
#define __HTTP_SERVER_H__

#include <stdbool.h>

void start_httpd();
void stop_httpd();
bool is_httpd_running();

#endif
