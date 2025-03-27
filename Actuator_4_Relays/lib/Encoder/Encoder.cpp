#include "Encoder.h"

void EncoderClass::init(Stream *debug) {
	this->debug = debug;
}


void EncoderClass::setRelayStatus(bool status, bool status2) {
	relay = status;
	relay2 = status2;
}

uint8_t EncoderClass::encodeWorkingRelays(byte *payload, uint8_t start) { //This function will indicate the working relays and their status
	payload[start] = 0;
	if (relay){
		payload[start] |= 1 << 7;
	}
	if (relay2){
		payload[start] |= 1 << 6;
	}
	return 1;
}

void EncoderClass::setActionType(action_type type) {
	action = type;
}



uint8_t EncoderClass::formatPayloadPrefix(byte *payload, payload_type type) {
	payload[0] = 0;
	
	if(type == MEASUREMENT) {
		payload[0] |= 1 << 7;
		} else if (type == CONFIRMATION) {
		payload[0] |= 1 << 6;
	}
	
	if(relay || relay2) {
		payload[0] |= 1 << 5; //indicates that at least 1 relay is on
	}

	if (action == RELAY_ON_TIME) {
		payload[0] |= 1 << 4;
	}else if (action == RELAY_ON_SWITCH) {
		payload[0] |= 1 << 3;
	}
	
	if(stop_reason == MALFUNCTION){
		payload[0] |= 1 << 2;
	} else if (stop_reason == CONNECTION_LOST) {
		payload[0] |= 1 << 1;
	} else if (stop_reason == STOP_COMMAND){
		payload[0] |= 1 ;
	}
	
	return 1;
}

uint8_t EncoderClass::getPayload(byte *payload, payload_type type, int size) {
	uint8_t payload_size = 0;
	memset(payload, 0, size);
	
	payload_size += formatPayloadPrefix(payload, type);
	if(type == MEASUREMENT) {
		payload_size += encodeavailableCom(payload, payload_size);	
		
		payload_size += encodeCounter(payload, payload_size);
		
		payload_size += encodeRejoinCounter(payload, payload_size);	
		
		
	}
	else if (type == CONFIRMATION) {
		payload_size += encodeAck(payload, payload_size);
		
		payload_size += encodeCounter(payload, payload_size);
	}
	
	payload_size += encodeRemaining(payload, payload_size);
	
	payload_size += encodeWorkingRelays(payload, payload_size);
	
	// Compatibility With Old Decoder
	if(type == MEASUREMENT)	{
		
	}
	
	return payload_size;
}

void EncoderClass::setCounter(uint32_t counter) {
	this->counter = counter;
}

uint8_t EncoderClass::encodeCounter(byte *payload, uint8_t start) {
	payload[start] = (counter >> 24) & 0xFF;
	payload[start + 1] = (counter >> 16) & 0xFF;
	payload[start + 2] = (counter >> 8) & 0xFF;
	payload[start + 3] = counter & 0xFF;
	
	return 4;
}


void EncoderClass::setRejoinCounter(uint32_t RejoinCounter) {
	this->RejoinCounter = RejoinCounter;
}


uint8_t EncoderClass::encodeRejoinCounter(byte *payload, uint8_t start) {
	payload[start] = (RejoinCounter >> 24) & 0xFF;
	payload[start + 1] = (RejoinCounter >> 16) & 0xFF;
	payload[start + 2] = (RejoinCounter >> 8) & 0xFF;
	payload[start + 3] = RejoinCounter & 0xFF;
	
	return 4;
}


void EncoderClass::setStopReason(stop_type reason) {
	stop_reason = reason;
}

void EncoderClass::setRemaining(uint16_t remaining) {
	this->remaining = remaining;
}

uint8_t EncoderClass::encodeRemaining(byte *payload, uint8_t start) {
	payload[start] = (remaining >> 8) & 0xFF;
	payload[start + 1] = remaining & 0xFF;
	
	return 2;
}

void EncoderClass::setAck(uint32_t ack) {
	this->ack = ack;
}

uint16_t EncoderClass::decodeRemaining(byte *payload, uint8_t start) {
	uint16_t remaining = 0;

	remaining += (uint16_t) payload[start] << 8;
	remaining += (uint16_t) payload[start + 1];

	return remaining;
}

uint32_t EncoderClass::decodeAck(byte *payload, uint8_t start) {
	uint32_t counter = 0;

	counter += (uint32_t) payload[start] << 24;
	counter += (uint32_t) payload[start + 1] << 16;
	counter += (uint32_t) payload[start + 2] << 8;
	counter += (uint32_t) payload[start + 3];

	return counter;
}

uint32_t EncoderClass::decodeUint32(byte *payload, uint8_t start) {
	uint32_t counter = 0;

	counter += (uint32_t) payload[start] << 24;
	counter += (uint32_t) payload[start + 1] << 16;
	counter += (uint32_t) payload[start + 2] << 8;
	counter += (uint32_t) payload[start + 3];

	return counter;
}

uint8_t EncoderClass::encodeAck(byte *payload, uint8_t start) {
	payload[start] = (ack >> 24) & 0xFF;
	payload[start + 1] = (ack >> 16) & 0xFF;
	payload[start + 2] = (ack >> 8) & 0xFF;
	payload[start + 3] = ack & 0xFF;
	
	return 4;
}


void EncoderClass::setavailableCom(bool availableCom) {
	this->availableCom = availableCom;
}

uint8_t EncoderClass::encodeavailableCom(byte *payload, uint8_t start){
	payload[start] = (availableCom >> 8) & 0xFF;
	payload[start + 1] = availableCom & 0xFF;
	
	return 2;
}



EncoderClass Encoder;