
#ifndef _LORAPROCESS_H
#define _LORAPROCESS_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "../../Parameters.h"
#include "../LoraWan/LoraWan.h"
#include "../Actions/Actions.h"
#include "../Encoder/Encoder.h"
#include "../EEPROM/EEPROM.h"
#include "../Storage/Storage.h"
#include "../TimerOne/TimerOne.h"
#include "avr/interrupt.h"



class LoRaProcess
{
	protected:
	Stream *debug;
	
	public:
	bool joined = false;
	bool confirmed = false;
	bool availableCom = false;
	uint16_t unconfirmed_sent = 0;
	uint16_t relay_ack_failed = 0;
	uint16_t ack_failed = 0;
	
	
	bool joinLora(Stream *debug);
	
	void sendData(bool confirmation, uint32_t counter, uint32_t rejoinCounter, Stream *debug, uint32_t confirmed_interval);
	void countUnconfirmedPackets(bool confirmed, Stream *debug);
	
	
	private:
	char Appkey[33] = API_KEY;
	char AppEUI[17] = APP_EUI;
	};
	
extern LoRaProcess LoRaPrs;





#endif /* _LORAPROCESS_H*/