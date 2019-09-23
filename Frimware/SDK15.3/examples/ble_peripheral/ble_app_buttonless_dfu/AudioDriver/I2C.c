#include "nrf_twi_mngr.h"
#include "i2c.h"
#include "nrf_gpio.h"

#define PIN_I2C_SCL         7
#define PIN_I2C_SDA         8

#define TWI_INSTANCE_ID     1
#define MAX_PENDING_TRANSACTIONS        10

NRF_TWI_MNGR_DEF(m_nrf_twi_mngr, MAX_PENDING_TRANSACTIONS, TWI_INSTANCE_ID);


void I2cInit()
{
    uint32_t err_code;

    nrf_drv_twi_config_t const config = {
       .scl                = PIN_I2C_SCL,
       .sda                = PIN_I2C_SDA,
       .frequency          = NRF_DRV_TWI_FREQ_400K,
       .interrupt_priority = APP_IRQ_PRIORITY_LOWEST,
       .clear_bus_init     = false
    };

    err_code = nrf_twi_mngr_init(&m_nrf_twi_mngr, &config);
    APP_ERROR_CHECK(err_code);
}

void I2cOpBlocking(nrf_twi_mngr_transfer_t *transaction, uint8_t       number_of_transfers)
{
    APP_ERROR_CHECK(nrf_twi_mngr_perform(&m_nrf_twi_mngr, NULL, transaction, number_of_transfers, NULL));
}

