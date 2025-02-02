/*
 * @file digital_input.cpp
 *
 * @date Nov 11, 2019
 * @author Andrey Belomutskiy, (c) 2012-2019
 */

#include "digital_input.h"

void digital_input_s::setWidthCallback(VoidInt handler, void *arg) {
	widthListeners.registerCallback(handler, arg);
}

void digital_input_s::setPeridoCallback(VoidInt handler, void *arg) {
	periodListeners.registerCallback(handler, arg);
}
