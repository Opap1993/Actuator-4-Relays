#include "Actions.h"

void ActionsClass::init(Stream *debug, EncoderClass *encoder, StorageClass *storage) {
	this->debug = debug;
	this->encoder = encoder;
	this->storage = storage;
}

action_type ActionsClass::decodeActionType(byte *payload) {
	bool relay_on_switch = payload[0]  & (1 << 7);
	bool relay_on_time = payload[0]  & (1 << 6);
	bool stop = payload[0]  & (1 << 5);
	bool config = payload[0]  & (1 << 4);
	bool transmit = payload[0] & (1 << 3);
	
	bool relay2_on_switch = payload[1]  & (1 << 7);
	bool relay2_on_time = payload[1]  & (1 << 6);
	bool stop2 = payload[1]  & (1 << 5);

	
	if(stop) {
		debug->println("STOP");
		return STOP;
	}
	
	if(relay_on_switch) {
		debug->println("Relay On Switch");
		return RELAY_ON_SWITCH;
	}
	
	if(relay_on_time) {
		debug->println("Relay On Time");
		return RELAY_ON_TIME;
	}
	
	if(stop2) {
		debug->println("STOP 2");
		return STOP2;
	}
	
	if(relay2_on_switch) {
		debug->println("Relay 2 On Switch");
		return RELAY2_ON_SWITCH;
	}
	
	if(relay2_on_time) {
		debug->println("Relay 2 On Time");
		return RELAY2_ON_TIME;
	}
	
	if(config) {
		debug->println("Config");
		return CONFIG;
	}
	
	if(transmit) {
		debug->println(F("Transmit "));
		transmit = true;
	}
}

void ActionsClass::reset() {
	stop();
	type = MEASURE;
	ack = 0;
	remaining = 0;
	config_counter = 0;
	config_interval = 0;
	counter = 0;
	interval = 0;
	config = false;
}

void ActionsClass::setAction(byte *payload){
	
	this->type = decodeActionType(payload);
	this->ack = this->encoder->decodeAck(payload,1);
	
	if(type == RELAY_ON_SWITCH || type == RELAY_ON_TIME)  {
		actionStart = millis();
		seconds = 0;
		minutes = 0;
		
		this->remaining = this->encoder->decodeRemaining(payload,5);
	}
	else if (type == RELAY2_ON_SWITCH || type == RELAY2_ON_TIME) {
		actionStart = millis();
		seconds2 = 0;
		minutes2 = 0;
		
		this->remaining2 = this->encoder->decodeRemaining(payload,5);
		
	}
	
	else if(type == CONFIG) {
		stop();
		this->configure(payload,5);
	}
	
}


void ActionsClass::restart() {
	while(true){
		debug->println(".");
		delay(1000);
	}
}

bool ActionsClass::shouldTransmit(){
	if(transmit) {
		transmit = false;
		return true;
	}
	
	if(transmit_stop) {
		transmit_stop = false;
		return true;
	}
	
	return false;
}

void ActionsClass::applyConfigurations() {
	if(config_counter) {
		debug->print(F("Set Counter: "));
		debug->println(counter);
		this->storage->formatLog();
		this->storage->saveCounter(counter);
		
		config_counter = false;
		counter = 0;
	}
	
	if(config_interval) {
		debug->print(F("Set Interval: "));
		debug->println(this->interval);
		storage->saveUint32(this->interval, INTERVAL_ADDR);
		this->interval = 0;
		config_interval = 0;
	}
	
	config = false;
	type = MEASURE;
	restart();
}

void ActionsClass::configure(byte *payload, uint8_t size) {
	this->config_counter = payload[0]  & (1 << 3);
	this->config_interval = payload[0] & (1 << 2);
	
	this->config = true;
	
	if (this->config_counter) {
		this->counter = this->encoder->decodeUint32(payload, size);
		size += 4;
	}
	
	if(this->config_interval) {
		this->interval = this->encoder->decodeUint32(payload, size);
		size += 4;
	}
}

void ActionsClass::run() {
	switch(type) {
		case RELAY_ON_SWITCH:
		relayOnSwitch();
		break;
		case RELAY_ON_TIME:
		relayOnTime();
		break;
		case STOP:
		stop(STOP_COMMAND);
		break;
		case RELAY2_ON_SWITCH:
		relay2OnSwitch();
		break;
		case RELAY2_ON_TIME:
		relay2OnTime();
		break;
		case STOP2:
		stop2(STOP_COMMAND);
		break;
	}
	
	if(relay || relay2) { 
		if((millis() - actionStart) > malfunctionTimeout ){
			debug->println("Flow Malfunction");
			stop(MALFUNCTION);
			stop2(MALFUNCTION);
			return;
		}
	}
}

uint32_t ActionsClass::count(uint32_t counter) {
	// time > 600 ms 
	
	if((millis() - lastCountTime) > 600) {
		actionStart = millis();
		lastCountTime = millis();
		
		if(t0) {
			startPulse = millis();
		} else {
			endPulse = millis();
		}
		
		if ( (startPulse < endPulse ) && ( (endPulse - startPulse) > 500 ) ) {
			lastCountTime = millis();
			counter++;

		}
		
		if (!t0) {
			startPulse = endPulse;
		} else {
			t0 = false;
		}
	}

	return counter;
}

void ActionsClass::stop(stop_type stop_reason) {
	this->stop_reason = stop_reason;
	transmit_stop = true;
	relayOff();
	this->remaining = 0;
	this->type = MEASURE;
}

void ActionsClass::stop2(stop_type stop_reason) {
	this->stop_reason = stop_reason;
	transmit_stop = true;
	relay2Off();
	this->remaining = 0;
	this->type = MEASURE;
}

void ActionsClass::relayOn(){
	pinMode(7, OUTPUT);
	digitalWrite(7, HIGH);
	relay = true;
	startTime = millis();
	debug->print("Relay On ");
	stop_reason = NORMAL;
}

void ActionsClass::relay2On(){
	pinMode(9, OUTPUT);
	digitalWrite(9, HIGH);
	relay2 = true;
	startTime = millis();
	debug->print("Relay 2 On ");
	stop_reason = NORMAL;
}

void ActionsClass::relayOff() {
	digitalWrite(7, LOW);
	debug->println("Relay Off");
	relay = false;

}

void ActionsClass::relay2Off() {
	digitalWrite(9, LOW);
	debug->println("Relay 2 Off");
	relay2 = false;

}


void ActionsClass::relayOnTime(){
	debug->print(minutes);
	debug->print(":");
	debug->println(seconds);
	
	debug->print("Remaining:");
	debug->println(remaining);
	
	if(!relay && remaining > 0) {
		relayOn();
	}
	
	if (relay &&  remaining <= 0){
		stop();
		this->type = MEASURE;
	}
	
	if(seconds < 59){
		seconds += 1;
	}
	else {
		seconds = 0;
		minutes++;
		if(remaining > 0) {
			remaining--;
		}
	}
}


void ActionsClass::relay2OnTime(){
	debug->print(minutes2);
	debug->print(":");
	debug->println(seconds2);
	
	debug->print("Remaining:");
	debug->println(remaining2);
	
	if(!relay2 && remaining2 > 0) {
		relay2On();
	}
	
	if (relay2 &&  remaining2 <= 0){
		stop();
		this->type = MEASURE;
	}
	
	if(seconds2 < 59){
		seconds2 += 1;
	}
	else {
		seconds2 = 0;
		minutes2++;
		if(remaining2 > 0) {
			remaining2--;
		}
	}
}

void ActionsClass::relayOnSwitch(){
	if(!relay) {
		debug->println("Relay on Switch begins!");
		relayOn();
	}
}

void ActionsClass::relay2OnSwitch(){
	if(!relay2) {
		debug->println("Relay 2 on Switch begins!");
		relay2On();
	}
}

ActionsClass Actions;