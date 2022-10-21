#include "xparameters.h"
#include "xgpio.h"
#include "xil_printf.h"
#include "xil_exception.h"
#include "GPIO.h"

int gpio_init(XGpio *GpioInstance, uint16_t dev_id)
{

	XGpio_Config* config = XGpio_LookupConfig(dev_id);
	if (config == NULL) {
		return XST_FAILURE;
	}

	XStatus Status;
	//Initialize the GPIO driver
	Status = XGpio_CfgInitialize(GpioInstance, config, config->BaseAddress);
	if (Status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}

	Status = XGpio_Initialize(GpioInstance, dev_id);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	if (XGpio_SelfTest(GpioInstance) != XST_SUCCESS)
	{
		return XST_FAILURE;
	}

	XGpio_SetDataDirection(GpioInstance, CAM_GPIO0, 0); //Output
	XGpio_SetDataDirection(GpioInstance, FMC_PRSNT_L, 1); //Input

	return XST_SUCCESS;
}

void gpio_setBit(XGpio *GpioInstance, Channels channel)
{

	switch (channel)
	{
		case CAM_GPIO0: XGpio_DiscreteWrite(GpioInstance, CAM_GPIO0, 0x01); break;
		case FMC_PRSNT_L: /*do nothing, input pin*/ break;
	}

}

void gpio_clearBit(XGpio *GpioInstance, Channels channel)
{

	switch (channel)
	{
		case CAM_GPIO0: XGpio_DiscreteClear(GpioInstance, CAM_GPIO0, 0x01); break;
		case FMC_PRSNT_L: /*do nothing, input pin*/ break;
	}

}

uint8_t getBit(XGpio *GpioInstance, Channels channel)
{
	uint8_t pin_value;
	switch (channel)
	{
	case CAM_GPIO0: pin_value = XGpio_DiscreteRead(GpioInstance, CAM_GPIO0); break;
	case FMC_PRSNT_L: pin_value =  XGpio_DiscreteRead(GpioInstance, FMC_PRSNT_L); break;
	}

	return pin_value;
}

/******************************************************************************/
/**
*
* This is the interrupt handler routine for the GPIO for this example.
*
* @param	CallbackRef is the Callback reference for the handler.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void GpioHandler(void *CallbackRef)
{
	XGpio *GpioPtr = (XGpio *)CallbackRef;

	/* Clear the Interrupt */
	XGpio_InterruptClear(GpioPtr, XGPIO_IR_MASK);

}
