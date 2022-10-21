#include "xparameters.h"
#include "xiic.h"
#include "xintc.h"
#include "xil_exception.h"
#include "xil_printf.h"
#include "IIC.h"
#include "sleep.h"

volatile u8 TransmitComplete;	/* Flag to check completion of Transmission */
volatile u8 ReceiveComplete;	/* Flag to check completion of Reception */
volatile uint8_t tx_complete_flag_;	// Flag to check completion of Transmission
volatile uint8_t rx_complete_flag_;	// Flag to check completion of Reception
volatile uint8_t slave_nack_flag_;	// Flag to check completion of Reception
volatile uint8_t arb_lost_flag_;	// Flag to check completion of Reception
volatile uint8_t bus_not_busy_flag_;	// Flag to check completion of Transmission

static void StatusHandler(XIic *InstancePtr, int Event);
static void RecvHandler(XIic *InstancePtr);
static void SendHandler(XIic *InstancePtr);


int iic_init(XIic *IicInstance, uint16_t dev_id)
{

	int Status;
	XIic_Config *ConfigPtr;	/* Pointer to configuration data */

	/*
	 * Initialize the IIC driver so that it is ready to use.
	 */
	ConfigPtr = XIic_LookupConfig(dev_id);
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}

	Status = XIic_CfgInitialize(IicInstance, ConfigPtr,
			ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/*
	 * Initialize the Dynamic IIC core.
	 */
	Status = XIic_DynamicInitialize(IicInstance);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Set the Handlers for transmit and reception.
	 */
	XIic_SetSendHandler(IicInstance, IicInstance,
			(XIic_Handler) SendHandler);
	XIic_SetRecvHandler(IicInstance, IicInstance,
			(XIic_Handler) RecvHandler);
	XIic_SetStatusHandler(IicInstance, IicInstance,
			(XIic_StatusHandler) StatusHandler);

	return XST_SUCCESS;
}

int iic_read(XIic *IicInstance, uint8_t addr, uint8_t* buf, size_t count)
{

	int Status;
	/*
	 * Set the Defaults.
	 */
	ReceiveComplete = 0;

	resetFlags();

	/*
	 * Set the Slave address.
	 */
	Status = XIic_SetAddress(IicInstance, XII_ADDR_TO_SEND_TYPE,
			addr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XIic_Start(IicInstance);
	if (Status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}

	Status = XIic_DynMasterRecv(IicInstance, buf, count);
	if (Status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}

	while ((!ReceiveComplete && !slave_nack_flag_) || (XIic_IsIicBusy(IicInstance) == TRUE)) ;

	/*
	 * The ReceiveComplete and slave_nack_flag_ flags are set before the BUS transitions from BUSY to NOT BUSY
	 * so we have to wait for the BUS to NOT BE BUSY before retrying the same i2c transfer or starting a completely new i2c transfer.
	 */

	if (slave_nack_flag_)
	{
		xil_printf("Slave NACK while reading from device 0x%02x.", addr);
	}

	/*
	 * Stop the IIC device.
	 */
	Status = XIic_Stop(IicInstance);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

int iic_write(XIic *IicInstance, uint8_t addr,  uint8_t *buf, size_t count)
{

	int Status;
	int retry_count = 50;

	/*
	 * Set the defaults.
	 */
	TransmitComplete = 0;
	IicInstance->Stats.TxErrors = 0;

	resetFlags();


	/*
	 * Set the Slave address.
	 */
	Status = XIic_SetAddress(IicInstance, XII_ADDR_TO_SEND_TYPE,
			addr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Start the IIC device.
	 */
	Status = XIic_Start(IicInstance);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Send the Data.
	 */
	Status = XIic_DynMasterSend(IicInstance, buf, count);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	while ((!TransmitComplete && !slave_nack_flag_) || (XIic_IsIicBusy(IicInstance) == TRUE));

	/*
	 * The TransmitComplete and slave_nack_flag_ flags are set before the BUS transitions from BUSY to NOT BUSY
	 * so we have to wait for the BUS to NOT BE BUSY before retrying the same i2c transfer or starting a completely new i2c transfer.
	 */

	while (slave_nack_flag_ && retry_count != 0) {

		if (IicInstance->Stats.TxErrors != 0 || slave_nack_flag_) {


			/*
			 * Enable the IIC device.
			 */
			Status = XIic_Start(IicInstance);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}


			if (!XIic_IsIicBusy(IicInstance)) {
				IicInstance->Stats.TxErrors = 0;
				resetFlags();
				/*
				 * Resend the Data.
				 */
				Status = XIic_DynMasterSend(IicInstance, buf, count);
				if (Status != XST_SUCCESS) {
					return XST_FAILURE;
				}
				while ((!TransmitComplete && !slave_nack_flag_) || (XIic_IsIicBusy(IicInstance) == TRUE));

				retry_count--;

			}
		}

	}

	if (slave_nack_flag_)
	{
		xil_printf("Slave NACK while writing to device 0x%02x.", addr);
	}

	/*
	 * Stop the IIC device.
	 */
	Status = XIic_Stop(IicInstance);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;

}

void resetFlags()
	{
	tx_complete_flag_ = 0;	// Flag to check completion of Transmission
	rx_complete_flag_ = 0;	// Flag to check completion of Reception
	slave_nack_flag_ = 0;	// Flag to check for NACK error
	arb_lost_flag_ = 0; 		// Flag to check for arbitration lost error
	bus_not_busy_flag_ = 0;
	}

static void StatusHandler(XIic *InstancePtr, int Event)
{
	if (Event & XII_MASTER_WRITE_EVENT) //Master writing to slave
	{
		tx_complete_flag_ = 1;
	}
	if (Event & XII_MASTER_READ_EVENT)  //Master reading from slave
	{
		rx_complete_flag_ = 1;
	}
	if (Event & XII_SLAVE_NO_ACK_EVENT)	// Slave did not ACK (had error)
	{
		slave_nack_flag_ = 1;
	}
	if (Event & XII_ARB_LOST_EVENT) 		// Arbitration was lost
	{
		arb_lost_flag_ = 1;
	}
	if (Event & (XII_BUS_NOT_BUSY_EVENT))	// Bus transitioned to not busy
	{
		bus_not_busy_flag_ = 1;
	}
}

static void SendHandler(XIic *InstancePtr)
{
	TransmitComplete = 1;
}

static void RecvHandler(XIic *InstancePtr)
{
	ReceiveComplete = 1;
}

