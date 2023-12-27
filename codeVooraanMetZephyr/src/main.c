/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gap.h>
#include "src\twowire\twowire.h"	


LOG_MODULE_REGISTER(Vorkheftruck, LOG_LEVEL_INF);

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)
#define INTERVAL 2000						             // #define INTERVAL 60000      (1000*60) Om de minuut waarde 
/* MPU6050 */
#define MPU6050_ADDR 0x68
#define MPU6050_WAKE_UP 0x6B							//01101011 of 107
#define MPU6050_INIT_READ 0x3B						    //00111011 of 59
/* VL53L1X */
#define VL53L1X_ADDR 0x29


const uint8_t my_service_data[] = { 0x12 };
/* Declare the advertising packet */
 static const struct bt_data ad[] = {
	/* Set the advertising flags */
	BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR),
	/* Set the advertising packet data  */
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
	BT_DATA(BT_DATA_SVC_DATA16, &my_service_data, sizeof(my_service_data)),
	BT_DATA(BT_DATA_NAME_COMPLETE, my_service_data,1),
	BT_DATA_BYTES(BT_DATA_UUID16_ALL, 0x12, 0x34),
};

/* Declare the scan response packet */
static const struct bt_data sd[] = {};

void mpu6050_setup(void) {
    twowire_write_register_byte(MPU6050_ADDR, MPU6050_WAKE_UP, 0);
}

void mpu6050_read_data(int16_t *accel_x , int16_t *accel_y, int16_t *accel_z) {
    twowire_read_register_data(MPU6050_ADDR, MPU6050_INIT_READ, (uint8_t *)accel_x);
    twowire_read_register_data(MPU6050_ADDR, MPU6050_INIT_READ + 2, (uint8_t *)accel_y);
    twowire_read_register_data(MPU6050_ADDR, MPU6050_INIT_READ + 4, (uint8_t *)accel_z);
}

void uint16ToHex(uint16_t value, char* hexString) {
    static const char hexChars[] = "0123456789ABCDEF";

    hexString[0] = hexChars[(value >> 12) & 0xF];
    hexString[1] = hexChars[(value >> 8) & 0xF];
    hexString[2] = hexChars[(value >> 4) & 0xF];
    hexString[3] = hexChars[value & 0xF];
    hexString[4] = '\0';
}

void main(void)
{
	int err;

	LOG_INF("Start program \n");
	/* STEP 5 - Enable the Bluetooth LE stack */
	err = bt_enable(NULL);
	if (err) {
		LOG_ERR("Bluetooth init failed (err %d)\n", err);
		return;
	}

	LOG_INF("Bluetooth initialized\n");

	err = bt_le_adv_start(BT_LE_ADV_NCONN, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
	if (err) {
		LOG_ERR("Advertising failed to start (err %d)\n", err);
		return;
	}
	LOG_INF("Advertising successfully started\n");

	// i2c communicatie
	twowire_write_register_byte(MPU6050_ADDR, MPU6050_WAKE_UP, 0);
	int16_t accel_x, accel_y, accel_z;
	const int16_t collisionThreshold = 20000;
	float totalAcceleration;
	uint16_t teller_acc = 0;

	uint8_t vl53l1x_data_buffer[2];
	uint16_t distance;
	uint16_t prev_distance = 0;
	uint16_t teller_tof = 0;
	char hexString[5];

    for (int i=0;i<10; i++) {					// Voor een dag: for (int i=0;i<(60*24); i++)
        mpu6050_read_data(&accel_x, &accel_y, &accel_z);
		LOG_INF("MPU6050 Acceleration - X: %d, Y: %d, Z: %d\n", accel_x, accel_y, accel_z);
		float totalAcceleration = sqrt(pow(accel_x, 2) + pow(accel_y, 2) + pow(accel_z, 2));
		if (totalAcceleration > collisionThreshold) teller_acc ++;

		vl53l1x_data_buffer[0] = 0x00;  // VL53L1X_REG_SYSRANGE_START
        vl53l1x_data_buffer[1] = 0x01;  // Value to initiate measurement
		vl53l1x_data_buffer[2] = 0x0014;
        distance = (vl53l1x_data_buffer[2] << 8) | vl53l1x_data_buffer[1];
		if (distance < 500) {
			if (prev_distance != distance) teller_tof ++;
		}
        LOG_INF("Distance: %d mm\n", distance);
        k_sleep(K_MSEC(INTERVAL));
    }
	   
	// Update the service data with the total values
	uint16ToHex(teller_acc, hexString);
	const uint8_t my_service_data_acc[] = { hexString[0], hexString[1] };

	uint16ToHex(teller_tof, hexString);
	const uint8_t my_service_data_tof[] = { hexString[0], hexString[1] };

	// Update the advertising packet
	struct bt_data ad_with_service_data[] = {
		BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR),
		BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
		BT_DATA(BT_DATA_SVC_DATA16, my_service_data_acc, sizeof(my_service_data_acc)),
		BT_DATA(BT_DATA_SVC_DATA16, my_service_data_tof, sizeof(my_service_data_tof)),
		BT_DATA_BYTES(BT_DATA_UUID16_ALL, 0x12, 0x34),
	};
	// Start advertising with updated data
	err = bt_le_adv_start(BT_LE_ADV_NCONN, ad_with_service_data, ARRAY_SIZE(ad_with_service_data), sd, ARRAY_SIZE(sd));


}