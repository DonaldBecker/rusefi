/*
 * @file trigger_universal.cpp
 *
 * @date Jan 3, 2017
 * @author Andrey Belomutskiy, (c) 2012-2018
 */

#include "trigger_universal.h"

angle_t getEngineCycle(operation_mode_e operationMode) {
	return operationMode == TWO_STROKE ? 360 : FOUR_STROKE_ENGINE_CYCLE;
}

void addSkippedToothTriggerEvents(trigger_wheel_e wheel, TriggerShape *s, int totalTeethCount, int skippedCount,
		float toothWidth, float offset, float engineCycle, float filterLeft, float filterRight) {
	efiAssertVoid(CUSTOM_ERR_6586, totalTeethCount > 0, "total count");
	efiAssertVoid(CUSTOM_ERR_6587, skippedCount >= 0, "skipped count");

	for (int i = 0; i < totalTeethCount - skippedCount - 1; i++) {
		float angleDown = engineCycle / totalTeethCount * (i + (1 - toothWidth));
		float angleUp = engineCycle / totalTeethCount * (i + 1);
		s->addEventClamped(offset + angleDown, wheel, TV_RISE, filterLeft, filterRight);
		s->addEventClamped(offset + angleUp, wheel, TV_FALL, filterLeft, filterRight);
	}

	float angleDown = engineCycle / totalTeethCount * (totalTeethCount - skippedCount - 1 + (1 - toothWidth));
	s->addEventClamped(offset + angleDown, wheel, TV_RISE, filterLeft, filterRight);
	s->addEventClamped(offset + engineCycle, wheel, TV_FALL, filterLeft, filterRight);
}

void initializeSkippedToothTriggerShapeExt(TriggerShape *s, int totalTeethCount, int skippedCount,
		operation_mode_e operationMode) {
	if (totalTeethCount <= 0) {
		warning(CUSTOM_OBD_TRIGGER_SHAPE, "totalTeethCount is zero or less: %d", totalTeethCount);
		s->setShapeDefinitionError(true);
		return;
	}
	efiAssertVoid(CUSTOM_ERR_6588, s != NULL, "TriggerShape is NULL");
	s->initialize(operationMode, false);

	s->setTriggerSynchronizationGap(skippedCount + 1);
	s->isSynchronizationNeeded = (totalTeethCount > 2) && (skippedCount != 0);

	addSkippedToothTriggerEvents(T_PRIMARY, s, totalTeethCount, skippedCount, 0.5, 0, getEngineCycle(operationMode),
	NO_LEFT_FILTER, NO_RIGHT_FILTER);
}


void configureOnePlusOne(TriggerShape *s, operation_mode_e operationMode) {
	UNUSED(operationMode);
	s->initialize(FOUR_STROKE_CAM_SENSOR, true);

	s->addEvent720(180, T_PRIMARY, TV_RISE);
	s->addEvent720(360, T_PRIMARY, TV_FALL);

	s->addEvent720(540, T_SECONDARY, TV_RISE);
	s->addEvent720(720, T_SECONDARY, TV_FALL);

	s->isSynchronizationNeeded = false;
	s->useOnlyPrimaryForSync = true;
}

void configureOnePlus60_2(TriggerShape *s, operation_mode_e operationMode) {
	UNUSED(operationMode);
	s->initialize(FOUR_STROKE_CAM_SENSOR, true);

	int totalTeethCount = 60;
	int skippedCount = 2;

	s->addEvent720(2, T_PRIMARY, TV_RISE);
	addSkippedToothTriggerEvents(T_SECONDARY, s, totalTeethCount, skippedCount, 0.5, 0, 360, 2, 20);
	s->addEvent720(20, T_PRIMARY, TV_FALL);
	addSkippedToothTriggerEvents(T_SECONDARY, s, totalTeethCount, skippedCount, 0.5, 0, 360, 20, NO_RIGHT_FILTER);

	addSkippedToothTriggerEvents(T_SECONDARY, s, totalTeethCount, skippedCount, 0.5, 360, 360, NO_LEFT_FILTER,
	NO_RIGHT_FILTER);

	s->isSynchronizationNeeded = false;
	s->useOnlyPrimaryForSync = true;
}

void configure3_1_cam(TriggerShape *s, operation_mode_e operationMode) {
	UNUSED(operationMode);
	s->initialize(FOUR_STROKE_CAM_SENSOR, true);


	const float crankW = 360 / 3 / 2;


	trigger_wheel_e crank = T_SECONDARY;

	s->addEvent720(10, T_PRIMARY, TV_RISE);
	s->addEvent720(50, T_PRIMARY, TV_FALL);


	float a = 2 * crankW;

	// #1/3
	s->addEvent720(a += crankW, crank, TV_RISE);
	s->addEvent720(a += crankW, crank, TV_FALL);
	// #2/3
	s->addEvent720(a += crankW, crank, TV_RISE);
	s->addEvent720(a += crankW, crank, TV_FALL);
	// #3/3
	a += crankW;
	a += crankW;

	// 2nd #1/3
	s->addEvent720(a += crankW, crank, TV_RISE);
	s->addEvent720(a += crankW, crank, TV_FALL);

	// 2nd #2/3
	s->addEvent720(a += crankW, crank, TV_RISE);
	s->addEvent720(a += crankW, crank, TV_FALL);

	s->isSynchronizationNeeded = false;
}
