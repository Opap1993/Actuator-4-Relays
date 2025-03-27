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
	void stop(stop_type reason = NORMAL);
	void stop2(stop_type reason = NORMAL);
	void relayOn();
	void relay2On();
	void relayOff();
	void relay2Off();
	void relayOnTime();
	void relay2OnTime();
	void relayOnSwitch();
	void relay2OnSwitch();
	uint32_t ack = 0;
	uint16_t remaining = 0;
	uint16_t remaining2 = 0;
	uint8_t seconds = 0;
	uint16_t minutes = 0;
	uint8_t seconds2 = 0;
	uint16_t minutes2 = 0;
	unsigned long lastCountTime = 0;
	unsigned long actionStart = 0;
	unsigned long malfunctionTimeout = 1800000;
	unsigned long startTime = 0;
	bool relay = false;
	bool relay2 = false;
	bool config = false;
	
	action_type type = MEASURE;
	stop_type stop_reason;
	
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