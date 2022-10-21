#include "xstatus.h"
#include "../cam/GPIO.h"
#include "xil_exception.h"
#include "XIntcInterruptController.h"

int intc_init(XIntc *IntcInstance, uint16_t dev_id,
		XIic *IicInst, uint32_t iic_irpt_id,
		XGpio *GpioInstance, uint32_t gpio_irpt_id,
		XAxiVdma *Vdma_Instance_a, uint32_t vdma_a_mm2s_irpt_id, uint32_t vdma_a_s2mm_irpt_id,
		XAxiVdma *Vdma_Instance_b, uint32_t vdma_b_irpt_id,
		XAxiVdma *Vdma_Instance_c, uint32_t vdma_c_irpt_id,
		XAxiVdma *Vdma_Instance_d, uint32_t vdma_d_irpt_id)
{

	int Status;

	XIntc_Config* config = XIntc_LookupConfig(dev_id);
	if (config == NULL){
		return XST_FAILURE;
	}

	if (IntcInstance->IsStarted == XIL_COMPONENT_IS_STARTED) {
		return XST_SUCCESS;
	}

	/*
	 * Initialize the interrupt controller driver so that it's ready to use.
	 */
	Status = XIntc_Initialize(IntcInstance, dev_id);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XIntc_SelfTest(IntcInstance);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device.
	 */
	Status = XIntc_Connect(IntcInstance, iic_irpt_id,
			(XInterruptHandler) XIic_InterruptHandler,
			IicInst);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XIntc_Connect(IntcInstance, gpio_irpt_id,
			(XInterruptHandler)GpioHandler,
			GpioInstance);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XIntc_Connect(IntcInstance, vdma_a_mm2s_irpt_id,
			(XInterruptHandler) XAxiVdma_ReadIntrHandler,
			Vdma_Instance_a);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XIntc_Connect(IntcInstance, vdma_a_s2mm_irpt_id,
			(XInterruptHandler) XAxiVdma_WriteIntrHandler,
			Vdma_Instance_a);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XIntc_Connect(IntcInstance, vdma_b_irpt_id,
			(XInterruptHandler) XAxiVdma_WriteIntrHandler,
			Vdma_Instance_b);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XIntc_Connect(IntcInstance, vdma_c_irpt_id,
			(XInterruptHandler) XAxiVdma_WriteIntrHandler,
			Vdma_Instance_c);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XIntc_Connect(IntcInstance, vdma_d_irpt_id,
			(XInterruptHandler) XAxiVdma_WriteIntrHandler,
			&Vdma_Instance_d);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Start the interrupt controller so interrupts are enabled for all
	 * devices that cause interrupts.
	 */
	Status = XIntc_Start(IntcInstance, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable the interrupts for the devices.
	 */
	XIntc_Enable(IntcInstance, iic_irpt_id);
	XIntc_Enable(IntcInstance, gpio_irpt_id);
	XIntc_Enable(IntcInstance, vdma_a_mm2s_irpt_id);
	XIntc_Enable(IntcInstance, vdma_a_s2mm_irpt_id);
	XIntc_Enable(IntcInstance, vdma_b_irpt_id);
	XIntc_Enable(IntcInstance, vdma_c_irpt_id);
	XIntc_Enable(IntcInstance, vdma_d_irpt_id);

	XGpio_InterruptEnable(GpioInstance, XGPIO_IR_MASK);//enable interrupts for gpio channel 1 and 2
	XGpio_InterruptGlobalEnable(GpioInstance);

	XAxiVdma_IntrEnable(Vdma_Instance_a, XAXIVDMA_IXR_ALL_MASK, XAXIVDMA_READ);
	XAxiVdma_IntrEnable(Vdma_Instance_a , XAXIVDMA_IXR_ALL_MASK, XAXIVDMA_WRITE);
	XAxiVdma_IntrEnable(Vdma_Instance_b , XAXIVDMA_IXR_ALL_MASK, XAXIVDMA_WRITE);
	XAxiVdma_IntrEnable(Vdma_Instance_c , XAXIVDMA_IXR_ALL_MASK, XAXIVDMA_WRITE);
	XAxiVdma_IntrEnable(Vdma_Instance_d , XAXIVDMA_IXR_ALL_MASK, XAXIVDMA_WRITE);


	/*
	 * Initialize the exception table.
	 */
	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			(Xil_ExceptionHandler) XIntc_InterruptHandler,
			IntcInstance);

	/*
	 * Enable non-critical exceptions.
	 */
	Xil_ExceptionEnable();


	return XST_SUCCESS;
}
