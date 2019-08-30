/*****************************************************************************/
/*                                                                           */
/*  Giga-Concept                            File "appli.c"                   */
/*                                                                           */
/*  Main program of the application                                          */
/*                                                                           */
/*  Auteur : Giga-concept                 																	 */
/*                                                                           */
/*  (c) Giga-concept                                                         */
/*****************************************************************************/

/*---------------------------------------------------------------------------*/
/* Version |    Date   |                     Description                     */
/*---------------------------------------------------------------------------*/
/*   1.0   | 26-04-18 | Creation.  			   KALIMBA BODO Soter    						 */
/*   1.2   | 18-09-18 | Update  			   	Deveui/appkey/appeui via UART			 */
/*---------------------------------------------------------------------------*/

/*

SOURCE FILE IN WHICH THE LORAWAN APP IS LOCATED

*/

#include "appli.h"
#include "stdbool.h"
#include "math.h"
#include "hw_rtc.h"
#include "main.h"
#include "math.h"

/* VARIABLES */  
uint8_t 	UART_Receive[FIFO_MAX];				//Global variables allowing the processing of messages received on the UART
uint8_t 	UART_Received_Char_Nb = 0;		//Counter of the number of characters received on the UART
uint8_t MessageSize;

extern ADC_HandleTypeDef hadc;
extern UART_HandleTypeDef huart2;

extern float temper;
extern float humid;
extern long press;

extern int axe_x_bis;
extern int axe_y_bis;
extern int axe_z_bis;

int acc_result = 0;
int ancien_result = 0;

bool MessageUart = false;

int cpt_lora =0;
uint8_t buff[4];

double batterie;
int batt;



extern uint8_t AppKey[];
extern uint8_t AppEui[]; 
extern uint8_t DevEui[];

long appkey_uart;
long appeui_uart;

uint8_t buff_appkey[3] = {0x46,0x0D, 0x0B};
uint8_t buff_appeui[3] = {0x35,0x0D, 0x0F};


/* MAIN FUNCTION THAT RUNS THE LORA STATE MACHINE */ 

void appli ()
{
	/* 1st BATTERY LEVEL MEASUREMENT */ 
	
	batterie = HW_GetBatteryLevel( );
	batt = 1800 + (7.08 * HW_GetBatteryLevel());

	
	while(1)
	{
		/* STARTING AND INITIALIZING ADC */
//		HAL_ADC_Init(&hadc);
//		HAL_ADC_Start(&hadc);
		
		/* UART RECEPTION */ 
		
		HAL_UART_Receive_IT(&huart2, buff, 4);
		
		// RECEIVING THE APP_KEY VIA UART (frame ended by  0x46 0x0D 0x0B)
		if(((UART_Receive[UART_Received_Char_Nb-3] == 0x46 && UART_Receive[UART_Received_Char_Nb-2] == 0x0D) && (UART_Receive[UART_Received_Char_Nb-1] == 0x0B)) &&(UART_Received_Char_Nb == 19))
		{
			HAL_UART_Transmit(&huart2, buff_appkey, 3, HAL_MAX_DELAY); // Replying by the end frame
			
			MessageSize = UART_Received_Char_Nb;			
			UART_Received_Char_Nb = 0;
			
			//WRITE IN FLASH MEMORY
				
				unsigned long Address;
				uint32_t PageError;
				
				appkey_uart = UART_Receive[3]<<24 | UART_Receive[2]<<16 | UART_Receive[1]<<8 | UART_Receive[0];  // Format the 4 first bytes 
				 
				FLASH_EraseInitTypeDef EraseInit;						
				
				Address = EEPROM_SYSTEM_ADDRESS_APPKEY;					// Adress of the appkey in the memory
			
				EraseInit.TypeErase   = FLASH_TYPEERASE_PAGES;   // Erase one page (128 bytes)
				EraseInit.PageAddress = Address;					
				EraseInit.NbPages     = 1;

				
				HAL_FLASH_Unlock();															// Unlocking the memory to do operations
				
				__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP| FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR );
				
				HAL_FLASHEx_Erase(&EraseInit, &PageError);			//	Erase the page
				HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, Address, appkey_uart); // Write 4 1st bytes of the appkey
				
				appkey_uart = UART_Receive[7]<<24 | UART_Receive[6]<<16 | UART_Receive[5]<<8 | UART_Receive[4];
				HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, Address+4, appkey_uart); //Write 4 2nd bytes of the appkey
				
				appkey_uart = UART_Receive[11]<<24 | UART_Receive[10]<<16 | UART_Receive[9]<<8 | UART_Receive[8];
				HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, Address+8, appkey_uart); // Write 4 3rd bytes of the appkey
				
				appkey_uart = UART_Receive[15]<<24 | UART_Receive[14]<<16 | UART_Receive[13]<<8 | UART_Receive[12];
				HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, Address+12, appkey_uart); // Write 4 last bytes of the appkey
												
				HAL_FLASH_Lock ();									// Lock the memory
					
			
		}
		
		
		
		// RECEIVING THE APP_EUI VIA UART (frame ended by  0x35 0x0D 0x0F)
		if(((UART_Receive[UART_Received_Char_Nb-3] == 0x35 && UART_Receive[UART_Received_Char_Nb-2] == 0x0D) && (UART_Receive[UART_Received_Char_Nb-1] == 0x0F)) &&(UART_Received_Char_Nb == 11))
		{
			HAL_UART_Transmit(&huart2, buff_appeui, 3, HAL_MAX_DELAY);
			
			MessageSize = UART_Received_Char_Nb;
			UART_Received_Char_Nb = 0;
			
			//WRITE IN FLASH MEMORY
				
				unsigned long Address_eui;
				uint32_t PageError;
				
				appeui_uart = UART_Receive[3]<<24 | UART_Receive[2]<<16 | UART_Receive[1]<<8 | UART_Receive[0];
				 
				FLASH_EraseInitTypeDef EraseInit;
				
				Address_eui = EEPROM_SYSTEM_ADDRESS_APPEUI;
			
				EraseInit.TypeErase   = FLASH_TYPEERASE_PAGES;
				EraseInit.PageAddress = Address_eui;
				EraseInit.NbPages     = 1;

				
				HAL_FLASH_Unlock();
				__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP| FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR );
				
				HAL_FLASHEx_Erase(&EraseInit, &PageError);				
				HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, Address_eui, appeui_uart);
				
				appeui_uart = UART_Receive[7]<<24 | UART_Receive[6]<<16 | UART_Receive[5]<<8 | UART_Receive[4];
				HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, Address_eui+4, appeui_uart);
																
				HAL_FLASH_Lock ();					
			
		}
		
		
		
		//Reading current appeui (frame ended by  0x53 0xD0 0xF0)
		if((UART_Receive[UART_Received_Char_Nb-3] == 0x53 && UART_Receive[UART_Received_Char_Nb-2] == 0xD0) && (UART_Receive[UART_Received_Char_Nb-1] == 0xF0))
		{
			HAL_UART_Transmit(&huart2, AppEui, 8, HAL_MAX_DELAY);
			UART_Received_Char_Nb = 0;
		}
		
		//Reading current appkey (frame ended by  0x64 0xD0 0xB0)
		if((UART_Receive[UART_Received_Char_Nb-3] == 0x64 && UART_Receive[UART_Received_Char_Nb-2] == 0xD0) && (UART_Receive[UART_Received_Char_Nb-1] == 0xB0))
		{
			HAL_UART_Transmit(&huart2, AppKey, 16, HAL_MAX_DELAY);
			UART_Received_Char_Nb = 0;
		}
		
		//Reading current DevEui (frame ended by  0xF4 0xD0 0xD3)
		if((UART_Receive[UART_Received_Char_Nb-3] == 0xF4 && UART_Receive[UART_Received_Char_Nb-2] == 0xD0) && (UART_Receive[UART_Received_Char_Nb-1] == 0xD3))
		{
			HAL_UART_Transmit(&huart2, DevEui, 8, HAL_MAX_DELAY);
			UART_Received_Char_Nb = 0;
		}
		
		
		/*************************************************************/
		
		
		/* MANAGEMENT OF THE EVENT "PRESS" AND CALL OF "ON EVENT FUNCTION" 
				
		if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_14) == GPIO_PIN_RESET)
		{
			OnSendEvent();
			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_6);			
			HAL_Delay(300);
		};*/
		
		
		
		
		/* LEDS MANAGEMENT */
		
		switch( DeviceState )
		{	  
			case DEVICE_STATE_INIT:
			{
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET); //SWITCH ON Led
				HAL_Delay(100);
								
				break;
			}
			case DEVICE_STATE_JOIN:
			{
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
				HAL_Delay(100);
				
				break;
			}
			case DEVICE_STATE_JOINED:
			{
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6|GPIO_PIN_7, GPIO_PIN_RESET);
				HAL_Delay(100);
				
				break;
			}
			case DEVICE_STATE_SEND:
			{							
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
				HAL_Delay(100);
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
								
				break;
			}
			case DEVICE_STATE_SLEEP:
			{
				HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7, GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5|GPIO_PIN_6, GPIO_PIN_RESET);
				
				break;
			}
		}
				
		/* LORA STATE MACHINE */
		
		lora_fsm( );
		
		/* USER CODE */
					
		
		/* LOW POWER */ 
		
		#if defined ( LOW_POWER_ACTIVATED )		
		
			//low_power();
		
		#endif
					
	}
	
}


/* FUNCTION THAT SHAPES THE FRAME TO SEND */ 

void LoraTxData( lora_AppData_t *AppData, FunctionalState* IsTxConfirmed)
{
	uint32_t i = 0;
	int16_t temperature = 0;
  uint16_t humidity = 0;
	
	cpt_lora = cpt_lora + 1;
	
	if(!MessageUart)
	{
		calcul();
		
		//batterie = HW_GetBatteryLevel( )*100;
		batt = 1800 + (7 * HW_GetBatteryLevel());
		
		temperature = ( int16_t )( round(temper*10) );     /* in ?C * 10 */
		humidity    = ( uint16_t )( humid );      
			
		AppData->Port = LPP_APP_PORT;
		*IsTxConfirmed =  LORAWAN_CONFIRMED_MSG;
		
		/* LORA MESSAGE COUNTER */
		AppData->Buff[i++] = cpt_lora-1;				
		
		/* TEMPERATURE */
		AppData->Buff[i++] = ( temperature >> 8 ) & 0xFF;
		AppData->Buff[i++] = temperature & 0xFF;
		
		/* HUMIDITY */
		AppData->Buff[i++] = humidity & 0xFF;
		
		/* BATTERY */				
		AppData->Buff[i++] = ( batt>> 8 ) & 0xFF;
		AppData->Buff[i++] = batt & 0xFF;
		
		/* BAROMETER */
		#if defined ( GROOVE_CONNECTED )		
		
			AppData->Buff[i++] = ( press >> 16 ) & 0xFFF;
			AppData->Buff[i++] = ( press >> 8 ) & 0xFFF;
			AppData->Buff[i++] = press & 0xFFF;
		
		#endif
	
		/* ACCELEROMETER*/	
		AppData->Buff[i++] = axe_x_bis >> 8; 
		AppData->Buff[i++] = axe_x_bis; 
		AppData->Buff[i++] = axe_y_bis >> 8; 
		AppData->Buff[i++] = axe_y_bis; 
		AppData->Buff[i++] = axe_z_bis >> 8; 
		AppData->Buff[i++] = axe_z_bis;
		
		/* BUFFER SIZE */
		AppData->BuffSize = i;	
		
		
		/*SEND THE MESSAGE ALSO ON THE UART */
		//HAL_UART_Transmit(&huart2, AppData->Buff, i, HAL_MAX_DELAY);
	}
	
	
	/* WHEN A MESSAGE IS SENT FROM THE UART AND IS RETRANSMITTED IN LORA
	if(MessageUart)
	{
		AppData->Buff = UART_Receive;
		AppData->BuffSize = MessageSize;
		
		MessageUart = false;
	}*/
	
		
}	


/* FUNCTION THAT PROCESSES THE RECEIVED FRAMEWORK */ 

void LoraRxData( lora_AppData_t *AppData )
{
	/* Blue led flashes */
	HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_6);
				HAL_Delay(200);
	HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_6); 
				HAL_Delay(200);
	HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_6); 
				HAL_Delay(200);
	HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_6);
				HAL_Delay(200);
	HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_6); 
				HAL_Delay(200);
	HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_6); 
	
	
	/*lora message received from the cloudgate is retransmitted on the UART*/
	//HAL_UART_Transmit(&huart2, AppData->Buff, AppData->BuffSize, HAL_MAX_DELAY);
	
}

void low_power()
{
	  DISABLE_IRQ( );
    // if an interrupt has occurred after DISABLE_IRQ, it is kept pending and cortex will not enter low power anyway  
      
    if ( lora_getDeviceState( ) == DEVICE_STATE_SLEEP )
    {
			#ifndef LOW_POWER_DISABLE
						LowPower_Handler( );
			#endif
    }
    ENABLE_IRQ();
}
