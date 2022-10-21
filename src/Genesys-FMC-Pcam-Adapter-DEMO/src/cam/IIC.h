#ifndef IIC_H_
#define IIC_H_

#include "xparameters.h"
#include "xiic.h"
#include "xintc.h"
#include "xil_exception.h"
#include "xil_printf.h"

/************************** Function Prototypes ******************************/
int iic_init(XIic *IicInstance, uint16_t dev_id);
int iic_read(XIic *IicInstance, uint8_t addr, uint8_t* buf, size_t count);
int iic_write(XIic *IicInstance, uint8_t addr,  uint8_t *buf, size_t count);
void resetFlags();

#endif /* IIC_H_ */
