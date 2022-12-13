#ifndef __AIS_CONFIG_H__
#define __AIS_CONFIG_H__

#include "types.h"


/**
 * @brief Initialized the AIS configuration layer
 * 
 */
void ais_config_init();

/**
 * @brief Retrieves current AIS station configuration
 * 
 * @param data 
 * @return true 
 * @return false 
 */
bool ais_config_read_station(ais_station_t *data);

/**
 * @brief Writes a new station configuration into the main board
 * 
 * @param data 
 * @return true 
 * @return false 
 */
bool ais_config_write_station(ais_station_t *data);


/**
 * @brief Reads system information
 * 
 * @param data 
 * @return true 
 * @return false 
 */
bool ais_config_read_sys(ais_system_t *data);


#endif
