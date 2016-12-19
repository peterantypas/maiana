/*
 * LEDManager.hpp
 *
 *  Created on: Dec 25, 2015
 *      Author: peter
 */

#ifndef LEDMANAGER_HPP_
#define LEDMANAGER_HPP_

#include <inttypes.h>


class LEDManager
{
public:

    static const uint8_t GREEN_LED  = 0;
    //static const uint8_t ORANGE_LED = 1;
    //static const uint8_t RED_LED    = 2;
    //static const uint8_t BLUE_LED   = 1;

    static LEDManager &instance();
    virtual ~LEDManager();

    void clear();
    void blink(uint8_t led);
    //void blink();
    void onTimer();
private:
    LEDManager();
private:
    uint8_t mLEDState[2];
    //bool mLEDState;
};

#endif /* LEDMANAGER_HPP_ */
