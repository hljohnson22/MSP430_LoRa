/*
 * main.h
 *
 *  Created on: Jun 28, 2021
 *      Author: dragy
 */

#ifndef MAIN_H_
#define MAIN_H_

#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>
//
#include "spi.h"
//#include "i2c.h"

// sensors
//#include "seesaw.h"
//#include "pressure.h"
//#include "co2.h"
#include "DS18B20temp.h"

#include "wireless.h"

#include "timer.h"

uint8_t identification[8] = {0x00};        // 64-bit identification number
uint8_t temp[9] = {0x00};                  // temperature
uint8_t byte = {0x00};                     // 8-byte counter
uint8_t smsbTemp = {0x00};                 // signed most sig bit temperature
uint8_t lsbTemp = {0x00};                  // least sig bit temperature
uint8_t raw = {0x00};                      // holds concatinated values
uint8_t decimalPlaceValue = {0x00};
uint8_t finalTemp[4] = {0x00};

extern bool i2c_succeed;

#endif /* MAIN_H_ */
