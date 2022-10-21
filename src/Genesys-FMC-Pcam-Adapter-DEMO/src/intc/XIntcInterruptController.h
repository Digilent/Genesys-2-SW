#ifndef XINTCINTERRUPTCONTROLLER_H_
#define XINTCINTERRUPTCONTROLLER_H_

#include "xintc.h"
#include "xiic.h"
#include "xgpio.h"
#include "xaxivdma.h"
#include "xil_types.h"
#include "xstatus.h"

/************************** Function Prototypes ******************************/
int intc_init(XIntc *IntcInstance, uint16_t dev_id,
		XIic *IicInst, uint32_t iic_irpt_id,
		XGpio *GpioInstance, uint32_t gpio_irpt_id,
		XAxiVdma *Vdma_Instance_a, uint32_t vdma_a_mm2s_irpt_id, uint32_t vdma_a_s2mm_irpt_id,
		XAxiVdma *Vdma_Instance_b, uint32_t vdma_b_irpt_id,
		XAxiVdma *Vdma_Instance_c, uint32_t vdma_c_irpt_id,
		XAxiVdma *Vdma_Instance_d, uint32_t vdma_d_irpt_id);
#endif /* XINTCINTERRUPTCONTROLLER_H_ */
