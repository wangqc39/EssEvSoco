#ifndef __I2C_H__
#define __I2C_H__

#include "nrf_twi_mngr.h"

void I2cInit(void);
void I2cOpBlocking(nrf_twi_mngr_transfer_t *transaction, uint8_t       number_of_transfers);



#endif 

