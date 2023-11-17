#include <zephyr/kernel.h>
#include <stdint.h>
#include <stdbool.h>
#include "nrf_delay.h"
#include "nrf_twi_mngr.h"

#define MPU6050_ADDR    0x68
#define SDA_PIN         20
#define SCL_PIN         21
#define TWI_INSTANCE    0
#define I2C_DEV_NAME    DT_LABEL(DT_ALIAS(i2c_0))

int16_t AcX, AcY, AcZ;

// Function to initialize I2C communication
static void twi_init(void) {
    const struct device *i2c_dev;

    i2c_dev = device_get_binding(I2C_DEV_NAME);
    if (!i2c_dev) {
        printf("I2C: Device not found.\n");
        return;
    }

    printf("I2C: Device found: %s\n", I2C_DEV_NAME);
}

static void read_accel(void) {
    uint8_t accel_data_reg = 0x3B; // Register address for accelerometer data
    uint8_t buffer[6];
    nrf_twi_mngr_transfer_t const transfers[] = {
            NRF_TWI_MNGR_WRITE(MPU6050_ADDR, &accel_data_reg, 1, NRF_TWI_MNGR_NO_STOP),
            NRF_TWI_MNGR_READ(MPU6050_ADDR, buffer, sizeof(buffer), 0),
    };
    nrf_twi_mngr_perform(TWI_INSTANCE, NULL, transfers, sizeof(transfers) / sizeof(transfers[0]), NULL);

    AcX = (buffer[0] << 8) | buffer[1];
    AcY = (buffer[2] << 8) | buffer[3];
    AcZ = (buffer[4] << 8) | buffer[5];

    printf("Accel X: %u, Accel Y: %u, Accel Z: %u\n", AcX, AcY, AcZ);
}

int main(void) {
    twi_init(); // Initialize I2C

    while (true) {
        read_accel();
        nrf_delay_ms(1000);
    }
}