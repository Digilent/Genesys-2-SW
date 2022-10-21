#ifndef GPIO_H_
#define GPIO_H_

#include "xgpio.h"
#include "xstatus.h"

typedef enum {CAM_GPIO0 = 1, FMC_PRSNT_L = 2}Channels;

/************************** Function Prototypes ******************************/
int gpio_init(XGpio *GpioInstance, uint16_t dev_id);
void gpio_setBit(XGpio *GpioInstance, Channels channel);
void gpio_clearBit(XGpio *GpioInstance, Channels channel);
uint8_t getBit(XGpio *GpioInstance, Channels channel);
void GpioHandler(void *CallbackRef);

#endif /* GPIO_H_ */
