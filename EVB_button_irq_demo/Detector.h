#ifndef __DETECTOR_H
#define __DETECTOR_H

#include "Platform.h"
#include "cmsis_os.h"

#define DETECTOR_GPIO							GPIOB
#define DETECTOR_LEFT_GPIO_PIN		DRVGPIO_PIN_14
#define DETECTOR_RIGHT_GPIO_PIN		DRVGPIO_PIN_15

// Detector flags.
extern BOOL detectorLeft;
extern BOOL detectorRight;

void detectorHandlerLeft(void);
void detectorHandlerRight(void);
void detectorInit(void);

#endif  // __DETECTOR_H

