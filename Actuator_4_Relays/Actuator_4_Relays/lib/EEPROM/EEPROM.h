// EEPROM.h

#ifndef _EEPROM_h
#define _EEPROM_h

#include "stdlib.h"
#include "../../Parameters.h"
#include <Wire.h>

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#endif

class EEPROMClass
{
	protected:
	Stream *debug;

	public:
	void init(Stream *debug);
	void writeByte(uint16_t address, byte data = 0x00);
	byte readByte(uint16_t address);
	void writeAddress(uint16_t address);
	void readBuffer(uint16_t address, byte * buffer, int length);
	void WritePage(uint16_t address, byte *data, unsigned int data_len);
	uint16_t readCurrentAddress();
};

extern EEPROMClass EEPROM;

#endif
