#include "LoRaProcess.h"

bool LoRaProcess::joinLora(Stream *debug) {
	bool joined = false;
	
	LoraWan.wake();
	LoraWan.init(debug);
	LoraWan.setAppEUI(AppEUI);
	LoraWan.setAppKey(Appkey);
	LoraWan.setClass("c");
	LoraWan.setDataRate(0);
	LoraWan.setDutyCyle(100);
	joined = LoraWan.joinOtaa(15);

	if(joined) {
		debug->println(F("=============== Joined"));
		} else {
		debug->println(F("=============== Join Failed"));
	}
	return joined;
	
}

void LoRaProcess::sendData(bool confirmation, uint32_t counter, uint32_t rejoinCounter, Stream *debug, uint32_t confirmed_interval) {
	unsigned char payload[30];
	confirmed = false;
	uint32_t ack = Actions.ack;
	uint16_t remaining = Actions.remaining;
	uint8_t size = 0;
	
	if(!joined) {
		joined = joinLora(debug);
	}
	
	if(joined) {
		if(!confirmation) {
			debug->print(F("Counter :"));
			debug->println(counter);
			debug->print(F("RejoinCounter :"));
			debug->println(rejoinCounter);
			
			Encoder.setStopReason(Actions.stop_reason);
			Encoder.setActionType(Actions.type);
			Encoder.setRelayStatus(Actions.relays[1].state, Actions.relays[2].state, Actions.relays[3].state, Actions.relays[4].state);
			Encoder.setCounter(counter);
			Encoder.setRejoinCounter(rejoinCounter);
			Encoder.setRemaining(remaining);
			Encoder.setavailableCom(availableCom);
			size = Encoder.getPayload(payload, MEASUREMENT, 20);
		}
		else {
			debug->print(F("Ack :"));
			debug->println(ack);
			debug->print(F("Remaining :"));
			debug->println(remaining);
			debug->print(F("Counter :"));
			debug->println(counter);
			
			Encoder.setStopReason(Actions.stop_reason);
			Encoder.setActionType(Actions.type);
			Encoder.setRelayStatus(Actions.relays[1].state, Actions.relays[2].state, Actions.relays[3].state, Actions.relays[4].state);
			Encoder.setRejoinCounter(rejoinCounter);
			Encoder.setRemaining(remaining);
			Encoder.setCounter(counter);
			Encoder.setAck(ack);
			Encoder.setavailableCom(availableCom);
			size = Encoder.getPayload(payload, CONFIRMATION, 20);
		}
		
		if (Actions.anyRelayOn || confirmation || (unconfirmed_sent >= confirmed_interval) ) {
			debug->println(F("Transmit Confirmed Packet"));
			confirmed = LoraWan.transferConfirmedPacket(2, payload, size, 10); //to be checked
			countUnconfirmedPackets(confirmed, debug);	
		}
		else {
			debug->println("Unconfirmed packet");
			unconfirmed_sent = unconfirmed_sent + 15;
			debug->print(F("Transmit Packet "));
			debug->println(unconfirmed_sent);
			LoraWan.transferPacket(10, payload, size, 5, &availableCom);
			debug->print("availableCom: ");
			debug->println(availableCom);
		}
	}
}


void LoRaProcess::countUnconfirmedPackets(bool confirmed, Stream *debug) {
	if (confirmed) {
		ack_failed = 0;
		unconfirmed_sent = 0;
		
		debug->println(F("Confirmed"));
		return;
	}
	
	ack_failed++;
	debug->print(F("Confirmation Failed :"));
	debug->println(ack_failed);
	
	if (ack_failed >= 2) {
		LoraWan.wake();
	}
	
	if(ack_failed >= 3) {  //failed_before_rejon
		debug->println(F("Connection Lost"));
		ack_failed = 0;
		unconfirmed_sent = 0;
		joined = false;
		Actions.stop(CONNECTION_LOST);
		debug->println(F("Connection Lost!"));
		asm volatile (" jmp 0");
	}
	
}

LoRaProcess LoRaPrs;
