/*

INCLUDE FILE OF THE LORAWAN APPLICATION

*/

//INCLUDES  *********************************************************************

#include "hw.h"
#include "low_power.h"
#include "lora.h"
#include "bsp.h"
#include "timeServer.h"
#include "version.h"
#include "capteuri2c.h"
#include "stm32l0xx_hal.h"
#include "stm32l0xx_hal_flash.h"


// DEFINES  **********************************************************************

#define APP_TX_DUTYCYCLE                         1000 //2sec //300000  // 5 mins
/*!
 * LoRaWAN Adaptive Data Rate
 * @note Please note that when ADR is enabled the end-device should be static
 */
#define LORAWAN_ADR_ON                              1
/*!
 * LoRaWAN confirmed messages
 */
#define LORAWAN_CONFIRMED_MSG                    DISABLE //ENABLE
/*!
 * LoRaWAN application port
 * @note do not use 224. It is reserved for certification
 */
#define LORAWAN_APP_PORT                            2
/*!
 * Number of trials for the join request.
 */
#define JOINREQ_NBTRIALS                            3

#define LPP_APP_PORT 																4

#define	FIFO_MAX				96		// Taille FIFO UART(Rx et Tx)

// To comment if the groove i2c barometer is not connected
//#define GROOVE_CONNECTED                 1

// Activate or not the low power mode
//#define LOW_POWER_ACTIVATED                 1

#define EEPROM_SYSTEM_ADDRESS_APPKEY  	((uint32_t)0x08015000)														
#define EEPROM_SYSTEM_ADDRESS_APPEUI 		EEPROM_SYSTEM_ADDRESS_APPKEY+FLASH_PAGE_SIZE

// FONCTIONS  **********************************************************************

void LoraTxData( lora_AppData_t *AppData, FunctionalState* IsTxConfirmed);

/* call back when LoRa has received a frame*/
void LoraRxData( lora_AppData_t *AppData);

void appli(void);

void low_power(void);




// VARIABLES  **********************************************************************

extern DeviceState_t DeviceState;

static LoRaMainCallback_t LoRaMainCallbacks ={ HW_GetBatteryLevel,
                                               HW_GetUniqueId,
                                               HW_GetRandomSeed,
                                               LoraTxData,
                                               LoraRxData};

/** 
		TO COMMENT OR UNCOMMENT ACCORDING TO THE TYPE OF "TX" WANTED 
			- IF YOU WANT TO SEND LORA MESSAGE WITHOUT TIMER, CHOOSE "TX_ON_EVENT" 
			- IF YOU WANT TO SEND LORA MESSAGE ON TIMER CHOOSE "TX_ON_TIMER" 
**/ 
																							 
static  LoRaParam_t LoRaParamInit= {TX_ON_TIMER,
                                    APP_TX_DUTYCYCLE,
                                    CLASS_A,
                                    LORAWAN_ADR_ON,
                                    DR_0,
                                    LORAWAN_PUBLIC_NETWORK,
                                    JOINREQ_NBTRIALS};

																		

																		
/*static  LoRaParam_t LoRaParamInit= {TX_ON_EVENT,
                                    APP_TX_DUTYCYCLE,
                                    CLASS_A,
                                    LORAWAN_ADR_ON,
                                    DR_0,
                                    LORAWAN_PUBLIC_NETWORK,
                                    JOINREQ_NBTRIALS};*/
