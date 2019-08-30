/*****************************************************************************/
/*                                                                           */
/*  Giga-Concept                            File "capteuri2c.c"              */
/*                                                                           */
/*  measurement on i2c sensors                                               */
/*                                                                           */
/*  Auteur : Giga-concept                 																	 */
/*                                                                           */
/*  (c) Giga-concept                                                         */
/*****************************************************************************/

/*---------------------------------------------------------------------------*/
/* Version |    Date   |                     Description                     */
/*---------------------------------------------------------------------------*/
/*   1.0   | 26-04-18 | Creation.  			   KALIMBA BODO Soter       			   */
/*---------------------------------------------------------------------------*/
/*

SOURCE FILE IN WHICH MEASUREMENTS ON I2C SENSOR ARE MADE

*/

#include "capteuri2c.h"
#include "stm32l0xx_hal.h"
#include "stdbool.h"

// VARIABLES FOR HUMIDITY AND TEMPERATURE SENSORS
unsigned char buffer[2];
uint16_t raw_value;
float temper;
float humid;

// VARIABLES FOR ACCELEROMETER SENSOR
unsigned char ctrl_reg1[2];
unsigned char who_am_i;
unsigned char axe_x;
unsigned char axe_y;
unsigned char axe_z;
int axe_x_bis;
int axe_y_bis;
int axe_z_bis;
char signe;
int val;
unsigned char value;		
unsigned char adresse_axe_x;
unsigned char adresse_axe_y;
unsigned char adresse_axe_z;

//GROOVE BAROMETER I2C SENSOR
uint16_t DevAdress = 0x77<<1;
int tmp;
float tmpr;
int pressure;
long press;
float atm;
/* Calibration values */
short ac1;
short ac2; 
short ac3; 
unsigned short ac4;
unsigned short ac5;
unsigned short ac6;
short b1; 
short b2;
short mb;
short mc;
short md;
short ut;
/* Temperature */
long b5; 
bool flag = false;
volatile int OSS = 3;


// I2C HANDLER VARIABLE
extern I2C_HandleTypeDef hi2c1;



// MAIN MEASUREMENT FONCTION
void calcul(void)
{
	if(flag == false)
	{
		//GROOVE INIT
		ac1 = BMP085readCalibration(&hi2c1, 0xAA);
		ac2 = BMP085readCalibration(&hi2c1, 0xAC);
		ac3 = BMP085readCalibration(&hi2c1, 0xAE);
		ac4 = BMP085readCalibration(&hi2c1, 0xB0);
		ac5 = BMP085readCalibration(&hi2c1, 0xB2);
		ac6 = BMP085readCalibration(&hi2c1, 0xB4);
		b1 = BMP085readCalibration(&hi2c1, 0xB6);
		b2 = BMP085readCalibration(&hi2c1, 0xB8);
		mb = BMP085readCalibration(&hi2c1, 0xBA);
		mc = BMP085readCalibration(&hi2c1, 0xBC);
		md = BMP085readCalibration(&hi2c1, 0xBE);
		
		//WHO AM I
		who_am_i = 0x0F;
		HAL_I2C_Master_Transmit(&hi2c1,0x19<<1,&who_am_i,1,100);
		HAL_Delay(10);
		HAL_I2C_Master_Receive(&hi2c1,0x19<<1,&who_am_i,1,100);
		
		
		// CONFIG REG 1 - ACTIVATION AXE AND MODE 100Hz
		HAL_Delay(10);
		ctrl_reg1[0] = 0x20; 
		ctrl_reg1[1] = 0x57; 
		HAL_I2C_Master_Transmit(&hi2c1,0x19<<1,ctrl_reg1,2,100);
		HAL_Delay(10);
		HAL_I2C_Master_Receive(&hi2c1,0x19<<1,ctrl_reg1,1,100);
		
		flag = true;
		
	}
			
	// TEMPERATURE
	HAL_Delay(15);				// Waiting for sensor to start (see datasheet page 10)
	buffer[0] = 0xF3;			// Control for temperature measurement (see datasheet page 10)
	HAL_I2C_Master_Transmit(&hi2c1,0x40<<1,buffer,1,100);	//Start communication and send command temperature 
	HAL_Delay(50);			// Waiting for sensor measurement (See datasheet page 5)
	HAL_I2C_Master_Receive(&hi2c1,0x40<<1,buffer,2,100);  //sends command for reading the temperature
	
	//donnee = (buffer[1] & 0x02 )>1;  // Isolation of the status bit for the measured parameter (1 for humidity , 0 for temperature)
	
	buffer[1] = buffer[1] & 0xFC;    // Reset status bits for the calculation of the real value
	
	raw_value = ((uint16_t) buffer[0] << 8) | (uint16_t) buffer[1]; // raw value of the measure
	
	temper = (raw_value * 175.72 / 65536.0) - 46.85;
	
	//GROOVE READ
	tmp = BMP085readUT(&hi2c1);								//Temperature measurement in raw value
	tmpr = BMP085calculateTemperature(tmp) * 0.1;  	//Temperature Conversion
	
	pressure = BMP085readUP(&hi2c1);					 //Pressure measurement in raw value
	press = BMP085calculatePressure(pressure); //Pressure Conversion 
	
	atm = press / 101325.0;		// conversion in atmospheric pressure
	
	// HUMIDITY
	buffer[0] = 0xF5;     // Commande pour mesure humidité ( (voir datasheet page 10)
	HAL_I2C_Master_Transmit(&hi2c1,0x40<<1,buffer,1,100);
	HAL_Delay(50);
	HAL_I2C_Master_Receive(&hi2c1,0x40<<1,buffer,2,100);
	
	buffer[1] = buffer[1] & 0xFC;
	
	raw_value = ((uint16_t) buffer[0] << 8) | (uint16_t) buffer[1];
	
	humid = (raw_value * 125.0 / 65536.0) - 6.0;
	
	
	
	//ACCELEROMETRE
		
	adresse_axe_x=0x29;
	adresse_axe_y=0x2B;
	adresse_axe_z=0x2D;		
	
	HAL_I2C_Master_Transmit(&hi2c1,0x19<<1,&adresse_axe_x,1,100);
	HAL_Delay(10);
	HAL_I2C_Master_Receive(&hi2c1,0x19<<1,&axe_x,1,100);
	HAL_Delay(10);
	
	HAL_I2C_Master_Transmit(&hi2c1,0x19<<1,&adresse_axe_y,1,100);
	HAL_Delay(10);
	HAL_I2C_Master_Receive(&hi2c1,0x19<<1,&axe_y,1,100);
	HAL_Delay(10);
	
	HAL_I2C_Master_Transmit(&hi2c1,0x19<<1,&adresse_axe_z,1,100);
	HAL_Delay(10);
	HAL_I2C_Master_Receive(&hi2c1,0x19<<1,&axe_z,1,100);
	HAL_Delay(10);
	
	axe_x_bis = complement(&axe_x)*16;
	axe_y_bis = complement(&axe_y)*16;
	axe_z_bis = complement(&axe_z)*16;
	
}

int complement(unsigned char *valeur)
{
			
		signe = *valeur;		
		signe = signe >>7;
		
		value = *valeur; 
		value = (~value) & 0x7F;

		if(signe == 1)
		{
			val = -(value+1);
		}
		else
		{
			val = (*valeur);
		}	
		
		return val;
		
}

// FONCTION POUR LE CAPTEUR GROOVE I2C BAROMETRE

int BMP085readCalibration(I2C_HandleTypeDef *I2CxHandle, uint8_t addr) 
{
	uint8_t msb, lsb;
	
	/* Reads data from registers and saves it to local variables */
	HAL_I2C_Master_Transmit(I2CxHandle, DevAdress, &addr,  1, 1000);
	HAL_I2C_Master_Receive(I2CxHandle, DevAdress, &msb, 1, 1000);
	
	/* MSB + 1 = LSB address*/
	uint8_t lsbAddress = addr + 1;
	HAL_I2C_Master_Transmit(I2CxHandle, DevAdress, &lsbAddress, 1, 1000);
	HAL_I2C_Master_Receive(I2CxHandle, DevAdress, &lsb, 1, 1000);
	
	return (int) msb << 8 | lsb;	
	
}


int BMP085calculateTemperature(long ut) 
{
	long x1, x2;
  
  x1 = ((ut - ac6)*ac5) >> 15;
  x2 = (mc << 11)/(x1 + md);
  b5 = x1 + x2;

  return ((b5 + 8) >> 4); 
}


int BMP085calculatePressure(long up) {
	long x1, x2, x3, b3, b6, p;
  unsigned long b4, b7;
  
  b6 = b5 - 4000;
	
  /* Calculate B3 */
  x1 = (b2 * (b6 * b6)>>12)>>11;
  x2 = (ac2 * b6)>>11;
  x3 = x1 + x2;
  b3 = (((((long)ac1)*4 + x3)<<OSS) + 2)>>2;
  
  /* Calculate B4 */
  x1 = (ac3 * b6)>>13;
  x2 = (b1 * ((b6 * b6)>>12))>>16;
  x3 = ((x1 + x2) + 2)>>2;
  b4 = (ac4 * (unsigned long)(x3 + 32768))>>15;
	
  /* Calculate B7 */
  b7 = ((unsigned long)(up - b3) * (50000>>OSS));
  if (b7 < 0x80000000)
    p = (b7<<1)/b4;
  else
    p = (b7/b4)<<1;
  
	/* Pressure calculation */  
  x1 = (p>>8) * (p>>8);
  x1 = (x1 * 3038)>>16;
  x2 = (-7357 * p)>>16;
  p += (x1 + x2 + 3791)>>4;
  
  return p;
}


int BMP085readUT(I2C_HandleTypeDef *I2CxHandle) 
{
	uint8_t sendArray[2];
	sendArray[0] = 0xF4;
	sendArray[1] = 0x2E;
	
	/* Writes 0x2E into 0xF4 register */
	HAL_I2C_Master_Transmit(I2CxHandle, DevAdress, sendArray, 2, 1000);
	
	/* Max conversion time for temperature is 4.5ms */
	HAL_Delay(5);
  
	ut = BMP085readCalibration(I2CxHandle, 0xF6);
	
	return ut;
}


int BMP085readUP(I2C_HandleTypeDef *I2CxHandle) 
{
	uint8_t msb, lsb, xlsb;
	long up = 0;
	uint8_t sendArray[2];
	sendArray[0] = 0xF4;
	sendArray[1] = (0x34 + (OSS << 6));

	/* Writes 0x34 + (OSS << 6) into 0xF4 register */
	HAL_I2C_Master_Transmit(I2CxHandle, DevAdress, sendArray, 2, 1000);
	
	uint8_t msbReg = 0xF6;
	uint8_t lsbReg = 0xF7;
	uint8_t xlsbReg = 0xF8;
	
	/* My OSS = 3 -> Max. Conversion time[ms] = 25.5 */
	HAL_Delay(26);
	
	/* Reads data from registers and saves it to local variables */
	HAL_I2C_Master_Transmit(I2CxHandle, DevAdress, &msbReg,  1, 1000);
	HAL_I2C_Master_Receive(I2CxHandle, DevAdress, &msb, 1, 1000);
	
	HAL_I2C_Master_Transmit(I2CxHandle, DevAdress, &lsbReg,  1, 1000);
	HAL_I2C_Master_Receive(I2CxHandle, DevAdress, &lsb, 1, 1000);
		
	HAL_I2C_Master_Transmit(I2CxHandle, DevAdress, &xlsbReg,  1, 1000);
	HAL_I2C_Master_Receive(I2CxHandle, DevAdress, &xlsb, 1, 1000);
	
	up = (((msb << 16) | (lsb << 8) | xlsb) >> (8 - OSS));
	return up;
}

