/*
 * @file boards.h
 *
 * @date Nov 15, 2013
 * @author Andrey Belomutskiy, (c) 2012-2019
 */

#ifndef BOARDS_H_
#define BOARDS_H_

#include "engine_configuration.h"
#include "adc_math.h"

#define ADC_CHANNEL_VREF 0

int getAdcValue(const char *msg, adc_channel_e channel);

#endif /* BOARDS_H_ */
