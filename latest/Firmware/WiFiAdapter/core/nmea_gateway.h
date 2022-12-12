#ifndef __NMEA_GATEWAY_H__
#define __NMEA_GATEWAY_H__

#include "types.h"

/**
 * @brief Starts all the NMEA gateway tasks
 * 
 */
void nmea_gateway_start();

/**
 * @brief Sets a callback to be used for consuming NMEA data. Called in the context of a dedicated RTOS task.
 * 
 * @param cb 
 */
void nmea_gateway_set_callback(nmea_data_callback_t *cb);


/**
 * @brief Sends a command to MAIANA main MCU via serial
 * 
 * @param command 
 */
void nmea_gateway_send_command(const char *command);


#endif
