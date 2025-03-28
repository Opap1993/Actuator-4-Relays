#ifndef _Actions_h
#define _Actions_h

#include "../Encoder/Encoder.h"
#include "../Storage/Storage.h"

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif
#include "../LoRaProcess/LoRaProcess.h"

class ActionsClass {
	public:
	void init(Stream *debug, EncoderClass *encoder, StorageClass *storage);
	action_type decodeActionType(byte *payload);
	void reset();
	void setAction(byte *payload);
	void restart();
	bool shouldTransmit();
	void applyConfigurations();
	void configure(byte *payload, uint8_t size);
	void run();
	bool transmitStopReason();
	uint32_t count(uint32_t counter);
	
	void stop(int i, stop_type reason = NORMAL);
	void stopAll(stop_type reason = NORMAL);
	
	void relayOn(int i);
	
	void relayOff(int i);
	
	void relayOnTime(int i);
	
	void relayOnSwitch(int i);

	
	uint32_t ack = 0;
	uint16_t remaining = 0;
	uint8_t seconds = 0;
	uint16_t minutes = 0;
	
	unsigned long lastCountTime = 0;
	unsigned long actionStart = 0;
	unsigned long malfunctionTimeout = 1800000;
	unsigned long startTime = 0;

	bool config = false;
	bool anyRelayOn = false;
	
	action_type type = MEASURE;
	stop_type stop_reason;
	
	relayState relays[5] = {
		{0,0}, //offset
		{false, 4},  // Relay 1 (OFF, Pin 4)
		{false, 5},  // Relay 2 (OFF, Pin 5)
		{false, 7},  // Relay 3 (OFF, Pin 7)
		{false, 9}   // Relay 4 (OFF, Pin 9)
	};
	
	bool t0 = true;
	unsigned long startPulse = 0;
	unsigned long endPulse = 0;
	
	private:
	bool transmit_stop = false;
	Stream *debug;
	EncoderClass *encoder;
	StorageClass *storage;
	
	boolean config_counter = false;
	boolean config_interval = false;
	uint32_t counter = 0;
	uint32_t interval = 0;
	bool transmit = false;
};

extern ActionsClass Actions;

#endif