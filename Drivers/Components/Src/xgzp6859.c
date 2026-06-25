/*
 * xgzp6859.c
 *
 *  Created on: 25-Jun-2026
 *      Author: Akhilesh
 */


#include "xgzp6859.h"
extern I2C_HandleTypeDef hi2c1;

uint8_t inst = 0X0A;
uint8_t MSB,LSB,CSB;
uint32_t pressure_ADC_value;
int32_t pressure;


uint32_t read_pressure()
{
	HAL_I2C_Mem_Write(&hi2c1, XGZP6859D_I2C_BASE_ADDR, XGZP6859D_REG_CMD, 1, &inst  , 1, 200);
	HAL_Delay(20);

	//Pressure_ADC value: = (Pressure 3rd Byte [23:16] x 65536+Pressure 2nd Byte [15:8] x 256 + Pressure1st Byte [7:0])

	HAL_I2C_Mem_Read(&hi2c1,XGZP6859D_I2C_BASE_ADDR, XGZP6859D_REG_PDATA_MSB , 1, &MSB, 1, 200);  // 3RD BYTE
	HAL_I2C_Mem_Read(&hi2c1, XGZP6859D_I2C_BASE_ADDR, XGZP6859D_REG_PDATA_LSB, 1, &LSB, 1, 200);  // 1ST BYTE
	HAL_I2C_Mem_Read(&hi2c1, XGZP6859D_I2C_BASE_ADDR, XGZP6859D_REG_PDATA_CSB, 1, &CSB, 1, 200);	 // 2ND BYTE

	pressure_ADC_value = ((MSB*65536)+(CSB*256)+LSB);

	if((pressure_ADC_value & (0x01<<23))) //Pressure=(pressure_ADC-2^24)/k;
	{
		pressure = (pressure_ADC_value - 16777216)/512;
	}
	else//Pressure = Pressure_ ADC /k;
	{
		pressure = pressure_ADC_value/512;
	}

	return pressure/133;
}
