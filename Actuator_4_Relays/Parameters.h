#ifndef PARAMETERS_H
#define PARAMETERS_H

// uno.bootloader.low_fuses=0xff
// uno.bootloader.high_fuses=0xde
// uno.bootloader.extended_fuses=0x05
// LockBit=0xFF

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#define LORA_RX	0
#define LORA_TX 1

#define LORA_SLEEP_SEC			3600

#define API_KEY	"a60bd6738560e3f2e78b1d06de021eb8"
#define APP_EUI	"70b3d57ed0023770"


#define EEPROM_ADDRESS		0X50
#define EEPROM_PAGE_SIZE	64
#define	EEPROM_WRITE_SIZE	4 // 16

#define EEPROM_LOG_POINTER_START	0
#define EEPROM_LOG_POINTER_END		9
#define EEPROM_LOG_START			200
#define EEPROM_LOG_END				1000
#define STORAGE_BUFFER				40


#define ENTRY_SIZE					4

#define DEBUG				false
#define DEBUG_EEPROM_ENCODING       0
#define EEPROM_DEBUG                0
#define STORAGE_DEBUG               1
#define STORAGE_DEBUG				1

#define CONFIRMED_UPLINK_INTERVAL		3
#define FAILED_CONFIRMATIONS_REJOIN		3

// 6 Bytes 
#define INTERVAL_ADDR	0
#define DEFAULT_INTERVAL_S 180

enum action_type {
	RELAY_ON_SWITCH,
	RELAY2_ON_SWITCH,
	RELAY3_ON_SWITCH,
	RELAY4_ON_SWITCH,
	RELAY_ON_TIME,
	RELAY2_ON_TIME,
	RELAY3_ON_TIME,
	RELAY4_ON_TIME,
	STOP,
	STOP2,
	STOP3,
	STOP4,
	STOP_ALL,
	CONFIG,
	TRANSMIT,
	MEASURE
};

enum stop_type {
	NORMAL,
	MALFUNCTION,
	CONNECTION_LOST,
	STOP_COMMAND
};

struct relayState {
	bool state;
	int pin;
};

#endif