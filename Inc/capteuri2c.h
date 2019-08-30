#include "stdint.h"
#include "stm32l0xx_hal.h"



void calcul(void);

void _Error_Handler(char *, int);

int complement(unsigned char *valeur);


// Groove barometer

int BMP085readCalibration(I2C_HandleTypeDef *I2CxHandle, uint8_t addr);

int BMP085calculateTemperature(long ut);

int BMP085calculatePressure(long up);

int BMP085readUT(I2C_HandleTypeDef *I2CxHandle);

int BMP085readUP(I2C_HandleTypeDef *I2CxHandle);
