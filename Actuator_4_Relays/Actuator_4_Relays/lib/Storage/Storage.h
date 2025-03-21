// Storage.h

#ifndef _Storage_h
#define _Storage_h
#include "util/crc16.h"

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#endif

#include "../../Parameters.h"
#include "../EEPROM/EEPROM.h"
#include "avr/wdt.h"

class StorageClass
{
	public:
	void init(Stream *debug, EEPROMClass *storage);
	void formatLog();
	void printEEPROM();
	void printLogs();
	uint16_t getLastAddress();
	void incrementCounter();
	byte calcCRC(const byte *data, byte len);
	bool saveUint32(uint32_t number, uint16_t address);
	bool readUint32T(uint32_t *number, uint16_t address);
	uint16_t findMaxCounter(byte * buffer, int n , uint32_t *max);
	bool saveCounter(uint32_t counter);
	uint32_t readCounter();
	uint32_t toCounter(byte *buffer);
	uint32_t readInterval();
	private:
	Stream *debug;
	EEPROMClass *storage;
	uint16_t address;
	uint32_t counter;
};

uint16_t getLastAddress();
extern StorageClass Storage;

#endif
