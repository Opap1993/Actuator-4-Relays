#include "EEPROM.h"

void EEPROMClass::init(Stream *debug)
{
	this->debug = debug;
	Wire.begin();
}

void EEPROMClass::writeByte(uint16_t address, byte data)
{

	Wire.beginTransmission(EEPROM_ADDRESS);
	writeAddress(address);
	Wire.write((int)data);
	Wire.endTransmission();
	delay(6);
}

byte EEPROMClass::readByte(uint16_t address)
{
	byte data = 0;

	Wire.beginTransmission(EEPROM_ADDRESS);
	writeAddress(address);
	Wire.endTransmission();
	Wire.requestFrom(EEPROM_ADDRESS, 1);
	if (Wire.available())
	{
		data = (byte)Wire.read();
	}

	return data;
}

uint16_t EEPROMClass::readCurrentAddress()
{
	uint16_t address = 0;
	int offset = 8;
	uint8_t c = 0;

	Wire.beginTransmission(EEPROM_ADDRESS);
	Wire.endTransmission();
	Wire.requestFrom(EEPROM_ADDRESS, 2);

	while (Wire.available())
	{
		c = Wire.read();
		debug->println(c);
		address |= (c << offset) & 0xFF;
		offset -= 8;
	}

	return address;
}

void EEPROMClass::writeAddress(uint16_t address)
{
	Wire.write((int)(address >> 8));
	Wire.write((int)(address & 0xFF));
}

void EEPROMClass::readBuffer(uint16_t address, byte *buffer, int length)
{
	Wire.beginTransmission(EEPROM_ADDRESS);
	writeAddress(address);

	Wire.endTransmission();
	Wire.requestFrom(EEPROM_ADDRESS, length);
	#if EEPROM_DEBUG
	debug->print("Address :");
	debug->println(address);
	debug->print("Read Buffer ");
	#endif
	for (int c = 0; c < length; c++)
	{
		if (Wire.available())
		{
			buffer[c] = (byte)Wire.read();
			#if EEPROM_DEBUG
			debug->print(buffer[c], HEX);
			debug->print(" ");
			#endif
		}
	}

	#if EEPROM_DEBUG
	debug->println();
	#endif
}
//  0                               64                               128
//  |------------page----------------|--------------page--------------|-----
// 								|------------------data-------------------|

void EEPROMClass::WritePage(uint16_t address, byte *data, unsigned int data_len)
{

	// Calculate space available in first page
	int page_space = int(((address / EEPROM_PAGE_SIZE) + 1) * EEPROM_PAGE_SIZE) - address;

	// Calculate first write size
	unsigned char first_write_size;
	if (page_space > EEPROM_WRITE_SIZE)
	{
		first_write_size = page_space - ((page_space / EEPROM_WRITE_SIZE) * EEPROM_WRITE_SIZE);

		if (first_write_size == 0)
		{
			first_write_size = EEPROM_WRITE_SIZE;
		}
	}
	else
	{
		first_write_size = page_space;
	}

	// Calculate size of last write
	int last_write_size = 0;
	if (data_len > first_write_size)
	{
		last_write_size = (data_len - first_write_size) % EEPROM_WRITE_SIZE;
	}

	// Calculate how many writes we need
	int num_writes = 0;
	if (data_len > first_write_size)
	{
		num_writes = ((data_len - first_write_size) / EEPROM_WRITE_SIZE) + 2;
	}
	else
	{
		num_writes = 1;
	}

	int page = 0;
	int write_size = 0;
	int i = 0;

	#if EEPROM_DEBUG
	debug->print("Write Buffer ");
	debug->print(address);
	debug->print(" : ");
	#endif

	for (page = 0; page < num_writes; page++)
	{

		if (page == 0)
		{
			write_size = first_write_size;
		}
		else if (page == (num_writes - 1))
		{
			write_size = last_write_size;
		}
		else
		{
			write_size = EEPROM_WRITE_SIZE;
		}

		Wire.beginTransmission(EEPROM_ADDRESS);
		writeAddress(address);
		int counter = 0;
		do
		{
			Wire.write(data[i]);
			#if EEPROM_DEBUG
			debug->print(i);
			debug->print("-");
			debug->print(data[i], HEX);
			debug->print(" ");
			#endif
			counter++;
			i++;
		} while ((counter < write_size));
		Wire.endTransmission();
		address += write_size;
		delay(20); // page write needs 5ms
	}

	#if EEPROM_DEBUG
	debug->println(" ");
	#endif

	#if DEBUG
	debug->print("Page Space");
	debug->println(page_space);

	debug->print("First write size:");
	debug->println(first_write_size);

	debug->print("Last write size:");
	debug->println(first_write_size);

	debug->print("Number of writes:");
	debug->println(num_writes);
	debug->println("-----------");
	#endif
}

EEPROMClass EEPROM;