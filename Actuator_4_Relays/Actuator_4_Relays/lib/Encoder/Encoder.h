// Storage.h

#ifndef _Encoder_h
#define _Encoder_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#endif
#include "../../Parameters.h"

enum payload_type {
	MEASUREMENT,
	CONFIRMATION
};

class EncoderClass
{
	public:
	void init(Stream *debug);
	void setRelayStatus(bool status);
	void setActionType(action_type type);
	uint8_t formatPayloadPrefix(byte *payload, payload_type type);
	uint8_t getPayload(byte *payload, payload_type type, int size);
	
	uint8_t encodeRemaining(byte *payload, uint8_t start);
	void setAck(uint32_t ack);
	uint16_t decodeRemaining(byte *payload, uint8_t start);
	uint32_t decodeAck(byte *payload, uint8_t start);
	uint32_t decodeUint32(byte *payload, uint8_t start);
	uint8_t encodeAck(byte *payload, uint8_t start);
	
	void setCounter(uint32_t counter);
	uint8_t encodeCounter(byte *payload, uint8_t start);
	void setRejoinCounter(uint32_t RejoinCounter);
	uint8_t encodeRejoinCounter(byte *payload, uint8_t start);
	void setavailableCom(bool availableCom);
	uint8_t encodeavailableCom(byte *payload, uint8_t start);
	void setStopReason(stop_type reason);
	void setRemaining(uint16_t remaining);
	uint8_t encodeRemainingPulses(byte *payload, uint8_t start);
	stop_type stop_reason;
	
	private:
	Stream *debug;
	bool relay = false;
	uint32_t counter = 0;
	uint32_t RejoinCounter = 0;
	bool availableCom = false;
	uint32_t ack = 0;
	uint16_t remaining = 0;
	payload_type type;
	action_type action;

	
};

extern EncoderClass Encoder;

#endif
