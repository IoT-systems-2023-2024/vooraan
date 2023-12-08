/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
/* STEP 3 - Include the header file of the Bluetooth LE stack */
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/drivers/i2c.h>  									
#include <dk_buttons_and_leds.h>

LOG_MODULE_REGISTER(Lesson2_Exercise1, LOG_LEVEL_INF);

#define I2C_DEV_NAME DEVICE_DT_NAME(DT_NODELABEL(i2c0))


#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

#define RUN_STATUS_LED DK_LED1
#define RUN_LED_BLINK_INTERVAL 1000

/* MPU6050 configuration */

#define MPU6050_ADDR 0x68
#define SDA_PIN 26
#define SCL_PIN 27
/*
#define SDA_PIN 20
#define SCL_PIN 21*/
#define MPU6050_I2C_ADDR (MPU6050_ADDR << 1)

/* MPU6050 accelerometer registers */
#define MPU6050_ACCEL_XOUT_H 0x3B
#define MPU6050_ACCEL_XOUT_L 0x3C
#define MPU6050_ACCEL_YOUT_H 0x3D
#define MPU6050_ACCEL_YOUT_L 0x3E
#define MPU6050_ACCEL_ZOUT_H 0x3F
#define MPU6050_ACCEL_ZOUT_L 0x40

/* STEP 4.1.1 - Declare the advertising packet */
static const struct bt_data ad[] = {
	/* STEP 4.1.2 - Set the advertising flags */
	BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR),
	/* STEP 4.1.3 - Set the advertising packet data  */
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),

};

/* STEP 4.2.2 - Declare the URL data to include in the scan response */
static unsigned char url_data[] = { 0x17, '/', '/', 'a', 'm', 'a', 'z', 'o', 'n', '.', 'c', 'o', 'm' };

/* STEP 4.2.1 - Declare the scan response packet */
static const struct bt_data sd[] = {
	/* 4.2.3 Include the URL data in the scan response packet */
	BT_DATA(BT_DATA_URI, url_data, sizeof(url_data)),
};

void main(void)
{
	int blink_status = 0;
	int err;

	LOG_INF("Start program \n");

	err = dk_leds_init();
	if (err) {
		LOG_ERR("LEDs init failed (err %d)\n", err);
		return;
	}
	/* STEP 5 - Enable the Bluetooth LE stack */
	err = bt_enable(NULL);
	if (err) {
		LOG_ERR("Bluetooth init failed (err %d)\n", err);
		return;
	}

	LOG_INF("Bluetooth initialized\n");

	/* STEP 6 - Start advertising */
	err = bt_le_adv_start(BT_LE_ADV_NCONN, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
	if (err) {
		LOG_ERR("Advertising failed to start (err %d)\n", err);
		return;
	}

	LOG_INF("Advertising successfully started\n");

	/* I2C configuration */

	const struct device *dev = device_get_binding(I2C_DEV_NAME);
	//const struct device *i2c_dev = device_get_binding("i2c0");
	if (!dev) {
		LOG_ERR("I2C initialization failed\n");
		return;
	}
	uint8_t accel_data[6];

	
	for (;;) {
		dk_set_led(RUN_STATUS_LED, (++blink_status) % 2);
		/* Read accelerometer data from MPU6050 */
		/*
		err = i2c_reg_read_byte(dev, MPU6050_ADDR, MPU6050_ACCEL_XOUT_H, &accel_data[0]);
		err += i2c_reg_read_byte(dev, MPU6050_ADDR, MPU6050_ACCEL_XOUT_L, &accel_data[1]);
		err += i2c_reg_read_byte(dev, MPU6050_ADDR, MPU6050_ACCEL_YOUT_H, &accel_data[2]);
		err += i2c_reg_read_byte(dev, MPU6050_ADDR, MPU6050_ACCEL_YOUT_L, &accel_data[3]);
		err += i2c_reg_read_byte(dev, MPU6050_ADDR, MPU6050_ACCEL_ZOUT_H, &accel_data[4]);
		err += i2c_reg_read_byte(dev, MPU6050_ADDR, MPU6050_ACCEL_ZOUT_L, &accel_data[5]);
		*/
		int ret = i2c_burst_read(dev, MPU6050_I2C_ADDR, MPU6050_ACCEL_XOUT_H, accel_data, 2);
		if (err) {
			LOG_ERR("Error reading accelerometer data (err %d)\n", err);
		} else {
			int16_t accel_x = (accel_data[0] << 8) | accel_data[1];
			int16_t accel_y = (accel_data[2] << 8) | accel_data[3];
			int16_t accel_z = (accel_data[4] << 8) | accel_data[5];
			LOG_INF("Accelerometer Data: X=%d, Y=%d, Z=%d\n", accel_x, accel_y, accel_z);
		}

		k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
	}
}
