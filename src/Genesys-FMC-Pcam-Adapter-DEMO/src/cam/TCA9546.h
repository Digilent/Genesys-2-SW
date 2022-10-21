#ifndef TCA9546_H_
#define TCA9546_H_
#include "xiic.h"


/************************** Function Prototypes ******************************/
//void mux_read(XIic *IicInstance, uint8_t addr, uint16_t reg_addr, uint8_t* buf, size_t count, uint8_t channel_mask);
//void mux_write(XIic *IicInstance, uint8_t addr, uint16_t reg_addr, uint8_t* buf, size_t count, uint8_t channel_mask);
void mux_read(XIic *IicInstance, uint8_t addr, uint8_t* buf, size_t count, uint8_t channel_mask);
void mux_write(XIic *IicInstance, uint8_t addr, uint8_t* buf, size_t count, uint8_t channel_mask);
void mux_reset(XIic *IicInstance, uint8_t a_pin);
uint8_t get_mux(XIic *IicInstance, uint8_t a_pin);
void enable(XIic *IicInstance, uint8_t mask, uint8_t a_pin);
void disable(XIic *IicInstance, uint8_t mask, uint8_t a_pin);

#endif /* TCA9546_H_ */
