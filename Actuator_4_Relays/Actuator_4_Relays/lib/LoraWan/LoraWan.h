// LoraWan.h

#ifndef _LORAWAN_h
#define _LORAWAN_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include <avr/wdt.h>

#define _DEBUG_SERIAL_      1
#define DEFAULT_TIMEOUT     1 // second
#define DEFAULT_TIMEWAIT    100 // millisecond
#define DEFAULT_DEBUGTIME   1 // second

#define LORA_RESET 6
#define LORA_RX 0
#define LORA_TX 1
#define DEBUG_SERIAL	3

#define	BEFFER_LENGTH_MAX		256
class LoraWanClass
{
	protected:
	Stream *debug;

	public:
	void init(Stream *debug);
	void setLoraModule();
	void setDataRate(uint8_t dr);
	void setClass(char *c);
	void passThrough();
	void setDutyCycle(int channel, int duty);
	void setAppKey(char *AppKey);
	void setAppEUI(char *AppEUI);
	void setDevEUI(char *DevEUI);
	bool joinOtaa(unsigned char timeout);
	bool transferPacket(int channel, unsigned char *buffer, unsigned char length, unsigned char timeout, bool *availableCom);
	void setReceiceWindowSecond(uint8_t dr, float f);
	void setRetransmissions(uint8_t n);
	bool transferConfirmedPacket(int channel, unsigned char *buffer,unsigned char length, unsigned char timeout);
	void sleep(int timeout);
	void wake();
	short receivePacket(char *buffer, short length, int *port, int delay = 1);
	#if _DEBUG_SERIAL_
	void debugPrint(unsigned char timeout);
	#endif
	void getEUI();
	void rxStop();
	void getDevEui();
	void setDutyCyle(int duty);
	short readBuffer(char *buffer, short length, unsigned char timeout);
	private:
	char _buffer[BEFFER_LENGTH_MAX];
	
	private:
	uint16_t cntnr = 0;
};

extern LoraWanClass LoraWan;

#endif
