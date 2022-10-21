#include "IIC.h"
#include "TCA9546.h"

void mux_read(XIic *IicInstance, uint8_t addr, uint8_t* buf, size_t count, uint8_t channel_mask_)
{
	enable(IicInstance, channel_mask_, 0);
	iic_read(IicInstance, addr, buf, count);
	disable(IicInstance, channel_mask_, 0);
}

void mux_write(XIic *IicInstance, uint8_t addr, uint8_t* buf, size_t count, uint8_t channel_mask_)
{
	enable(IicInstance, channel_mask_, 0);
	iic_write(IicInstance, addr, buf, count);
	disable(IicInstance, channel_mask_, 0);
}

void mux_reset(XIic *IicInstance, uint8_t a_pin)
{
	uint8_t dev_address_ = (0xE0 >> 1);
	dev_address_ &= ~0x07;
	dev_address_ |=(a_pin & 0x07);
	uint8_t ctl_reg = 0;
	iic_write(IicInstance, dev_address_, &ctl_reg, 1);
}

uint8_t get_mux(XIic *IicInstance, uint8_t a_pin)
{
	uint8_t dev_address_ = (0xE0 >> 1);
	dev_address_ &= ~0x07;
	dev_address_ |=(a_pin & 0x07);
	uint8_t ctl_reg = 0;
	iic_read(IicInstance, dev_address_, &ctl_reg, 1);
	return ctl_reg;
}

void enable(XIic *IicInstance, uint8_t mask, uint8_t a_pin)
{
	uint8_t dev_address_ = (0xE0 >> 1);
	dev_address_ &= ~0x07;
	dev_address_ |=(a_pin & 0x07);
	uint8_t ctl_reg = 0;
	iic_read(IicInstance, dev_address_, &ctl_reg, 1);
	ctl_reg |= mask;
	iic_write(IicInstance, dev_address_, &ctl_reg, 1);
}

void disable(XIic *IicInstance, uint8_t mask, uint8_t a_pin)
{
	uint8_t dev_address_ = (0xE0 >> 1);
	dev_address_ &= ~0x07;
	dev_address_ |=(a_pin & 0x07);
	uint8_t ctl_reg = 0;
	iic_read(IicInstance, dev_address_, &ctl_reg, 1);
	ctl_reg &= (~mask);
	iic_write(IicInstance, dev_address_, &ctl_reg, 1);
}
