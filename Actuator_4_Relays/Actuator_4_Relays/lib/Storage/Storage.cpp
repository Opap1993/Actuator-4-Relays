#include "Storage.h"

void StorageClass::init(Stream *debug, EEPROMClass *storage) {
	this->debug = debug;
	this->storage = storage;
	this->getLastAddress();
}

uint16_t StorageClass::getLastAddress() {
	uint16_t start = EEPROM_LOG_START;
	uint16_t end = EEPROM_LOG_END;
	
	uint16_t address = start;
	uint16_t next_address = start;
	uint32_t counter = 0;
	uint32_t max = 0;
	
	byte buffer[4];
	debug->println(F("Address --"));
	
	while (address <= end) {
		memset(buffer,0,4);
		storage->readBuffer(address, buffer, 4);
		counter = toCounter(buffer);
		
		debug->print(address);
		debug->print(" ");
		debug->println(counter);

		if (counter >= max & (counter > 0)) {
			max = counter;
			next_address = address;
		}
		
		address+= ENTRY_SIZE;
	}
	
	#if STORAGE_DEBUG
	debug->print(F("Last Address : "));
	debug->println(next_address);
	#endif

	this->address = next_address;
	this->counter = counter;
	return next_address;
}

void StorageClass::formatLog() {
	uint16_t start = EEPROM_LOG_START;
	uint16_t end = EEPROM_LOG_END;
	uint16_t total = end - start;
	int buffer_size = 250;
	byte page[250];
	uint16_t address = start;
	
	memset(page,0, 250);
	
	do {
		if (total < 250) {
			buffer_size = total;
		}
		
		#if STORAGE_DEBUG
		debug->print(F("Format from addr: "));
		debug->print(address);
		debug->print(F(" - "));
		debug->println(address + buffer_size);
		#endif
		
		EEPROM.WritePage(address, page, buffer_size);
		address += buffer_size;
		debug->println(address);
	} while  (address <= end);
	this->getLastAddress();
}

void StorageClass::printLogs() {
	uint16_t start = EEPROM_LOG_START;
	uint16_t end = EEPROM_LOG_END;
	uint16_t address = start;
	byte buffer[4];
	uint32_t counter = 0;

	while(address < end) {
		memset(buffer, 0, 4);
		storage->readBuffer(address, buffer, 4);
		counter = toCounter(buffer);
		debug->print(address);
		debug->print(" ");
		debug->print(counter);
		if(this->address == address) {
			debug->print(" -");
		}
		debug->println();
		address += ENTRY_SIZE;
	}
}

void StorageClass::printEEPROM() {
	uint16_t start = EEPROM_LOG_START;
	uint16_t end = EEPROM_LOG_END;
	uint16_t total = end - start;
	int buffer_size = 20;
	byte page[250];
	uint16_t address = start;
	
	do {
		if (total < 20) {
			buffer_size = total;
		}
		
		EEPROM.readBuffer(address, page, buffer_size);
		
		#if STORAGE_DEBUG
		debug->print(address + buffer_size);
		debug->println(F("#"));
		
		
		int c = 0;
		while(c < buffer_size) {
			debug->print(page[c]);
			debug->print(F(" "));
			c++;
		}
		debug->println();
		#endif
		
		address += buffer_size;
		total -= buffer_size;
	} while  (total > 0);
}

void StorageClass::incrementCounter(){
	debug->print(F("Increment Address :"));
	debug->print(this->address);

	if(this->address + ENTRY_SIZE < EEPROM_LOG_END){
		this->address += ENTRY_SIZE;
		} else {
		this->address = EEPROM_LOG_START;
	}

	debug->print(F(" Address :"));
	debug->println(this->address);
}

bool StorageClass::saveUint32(uint32_t number, uint16_t address) {
	byte data[6];
	
	data[0] = (number >> 24) & 0xFF;
	data[1] = (number >> 16) & 0xFF;
	data[2] = (number >> 8) & 0xFF;
	data[3] = number & 0xFF;

	uint16_t crc = calcCRC(data, 4);

	data[4] = (crc >> 8) & 0xFF;
	data[5] = crc & 0xFF;
	
	storage->WritePage(address, data, 6);
	
	#if STORAGE_DEBUG
	debug->print(F("Save Uint32 : "));
	debug->print(number, DEC);
	debug->print(F(" Address :"));
	debug->print(address, DEC);
	debug->print(F(" CRC :"));
	debug->println(crc, DEC);
	#endif
}

byte StorageClass::calcCRC(const byte *data, byte len) {
	byte crc = 0x00;
	while (len--) {
		byte extract = *data++;
		for (byte tempI = 8; tempI; tempI--) {
			byte sum = (crc ^ extract) & 0x01;
			crc >>= 1;
			if (sum) {
				crc ^= 0x8C;
			}
			extract >>= 1;
		}
	}
	return crc;
}

bool StorageClass::readUint32T(uint32_t *number, uint16_t address) {
	byte data[6] = {0};
	uint16_t crc = 0;
	bool crc_ok = false;
	
	storage->readBuffer(address,data, 6);
	*number = toCounter(data);
	
	crc += (uint16_t) data[4] << 8;
	crc += (uint16_t) data[5];

	uint16_t calc_crc = calcCRC(data, 4);
	
	
	#if STORAGE_DEBUG
	if(crc == calc_crc) {
		crc_ok = true;
		debug->print(F("Valid CRC:"));
		debug->print(calc_crc);
		debug->print("-");
		debug->println(crc);
		} else {
		crc_ok = false;
		debug->print(F("CRC Error"));
		debug->print(calc_crc);
		debug->print("-");
		debug->println(crc);
	}
	
	#endif
	
	return crc_ok;
}

bool StorageClass::saveCounter(uint32_t counter) {
	byte data[4];
	
	if (counter == this->counter) {
		return false;
	}
	
	data[0] = (counter >> 24) & 0xFF;
	data[1] = (counter >> 16) & 0xFF;
	data[2] = (counter >> 8) & 0xFF;
	data[3] = counter & 0xFF;

	incrementCounter();

	debug->print(F("Save Address: "));
	debug->println(this->address);
	this->counter = counter;
	storage->WritePage(this->address, data, 4);
	
	#if STORAGE_DEBUG
	debug->print(F("Save Counter :"));
	debug->println(counter, DEC);
	#endif
}

uint32_t StorageClass::toCounter(byte *buffer) {
	uint32_t counter = 0;
	counter += (uint32_t) buffer[0] << 24;
	counter += (uint32_t) buffer[1] << 16;
	counter += (uint32_t) buffer[2] << 8;
	counter += (uint32_t) buffer[3];
	
	return counter;
}

uint32_t StorageClass::readInterval() {
	bool crc_ok = false;
	uint32_t interval;
	
	crc_ok = readUint32T(&interval, INTERVAL_ADDR);
	debug->print("Read Interval :");
	debug->print(interval);
	debug->print(" crc :");
	debug->println(crc_ok);
	
	if (!crc_ok || interval == 0 || interval <= 300) {
		interval = DEFAULT_INTERVAL_S;
		saveUint32(interval, INTERVAL_ADDR);
	} 
	
	return interval;
}

uint32_t StorageClass::readCounter() {
	
	byte data[4] = {0};
	uint16_t address = this->address;

	debug->print(F("Read Address:"));
	debug->println(address);
	storage->readBuffer(address,data, 4);
	uint32_t counter = toCounter(data);
	
	this->counter = counter;
	
	#if STORAGE_DEBUG
	debug->print(F("Read Counter :"));
	debug->println(counter);
	#endif
	
	return counter;
}


StorageClass Storage;