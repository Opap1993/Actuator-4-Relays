#include "Actions.h"

void ActionsClass::init(Stream *debug, EncoderClass *encoder, StorageClass *storage) {
	this->debug = debug;
	this->encoder = encoder;
	this->storage = storage;
	for(int j=1; j<=4; j++){
	pinMode(relays[j].pin, OUTPUT);
	digitalWrite(relays[j].pin, LOW);
	}
}

action_type ActionsClass::decodeActionType(byte *payload) {
	// Action mappings
	struct {
		byte byteIndex;
		byte bitMask;
		action_type action;
		const char *message;
		} actions[] = {
		{0, (1 << 7), RELAY_ON_SWITCH, "Relay On Switch"},
		{0, (1 << 6), RELAY_ON_TIME, "Relay On Time"},
		{0, (1 << 5), STOP, "STOP"},
		{1, (1 << 7), RELAY2_ON_SWITCH, "Relay 2 On Switch"},
		{1, (1 << 6), RELAY2_ON_TIME, "Relay 2 On Time"},
		{1, (1 << 5), STOP2, "STOP 2"},
		{1, (1 << 4), RELAY3_ON_SWITCH, "Relay 3 On Switch"},
		{1, (1 << 3), RELAY3_ON_TIME, "Relay 3 On Time"},
		{1, (1 << 2), STOP3, "STOP 3"},
		{1, (1 << 1), RELAY4_ON_SWITCH, "Relay 4 On Switch"},
		{1, (1 << 0), RELAY4_ON_TIME, "Relay 4 On Time"},
		{2, (1 << 7), STOP4, "STOP 4"},
		{2, (1 << 6), STOP_ALL, "STOP ALL RELAYS"},
		{0, (1 << 4), CONFIG, "Config"}
	};

	// Loop through the actions and find a match
	for (auto &a : actions) {
		if (payload[a.byteIndex] & a.bitMask) {
			debug->println(a.message);
			return a.action;
		}
	}
	
	if(payload[0] & (1 << 3)) {
		debug->println(F("Transmit "));
		transmit = true;
	}
}

void ActionsClass::reset() {
	stopAll();
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
	
	if(type == RELAY_ON_SWITCH || type == RELAY_ON_TIME || type == RELAY2_ON_SWITCH || type == RELAY2_ON_TIME || type == RELAY3_ON_SWITCH || type == RELAY3_ON_TIME || type == RELAY4_ON_SWITCH || type == RELAY4_ON_TIME)  {
		actionStart = millis();
		seconds = 0;
		minutes = 0;
		
		this->remaining = this->encoder->decodeRemaining(payload,5);
	}
	
	else if(type == CONFIG) {
		stopAll();
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
		relayOnSwitch(1);
		break;
		
		case RELAY2_ON_SWITCH:
		relayOnSwitch(2);
		break;
		
		case RELAY3_ON_SWITCH:
		relayOnSwitch(3);
		break;
		
		case RELAY4_ON_SWITCH:
		relayOnSwitch(4);
		break;
		
		case RELAY_ON_TIME:
		relayOnTime(1);
		break;
		
		case RELAY2_ON_TIME:
		relayOnTime(2);
		break;
		
		case RELAY3_ON_TIME:
		relayOnTime(3);
		break;
		
		case RELAY4_ON_TIME:
		relayOnTime(4);
		break;
		
		case STOP:
		stop(1,STOP_COMMAND);
		break;
		
		case STOP2:
		stop(2,STOP_COMMAND);
		break;
		
		case STOP3:
		stop(3,STOP_COMMAND);
		break;
		
		case STOP4:
		stop(4,STOP_COMMAND);
		break;
		
		case STOP_ALL:
		stopAll(STOP_COMMAND);
		break;
	}
	
	for (int i = 1; i < 5; i++) {
		if (relays[i].state) {
			anyRelayOn = true;
			break; 
		}
	}
	
	if (anyRelayOn && (millis() - actionStart) > malfunctionTimeout) {
		debug->println("Flow Malfunction");
		stopAll(MALFUNCTION);
		return;
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

void ActionsClass::stopAll(stop_type stop_reason) {
	this->stop_reason = stop_reason;
	transmit_stop = true;
	relayOff(1);
	relayOff(2);
	relayOff(3);
	relayOff(4);
	this->remaining = 0;
	this->type = MEASURE;
}

void ActionsClass::stop(int i, stop_type stop_reason) {
	this->stop_reason = stop_reason;
	transmit_stop = true;
	relayOff(i);
	relays[i].state = false;
	this->remaining = 0;
	this->type = MEASURE;
}


void ActionsClass::relayOn(int i){
	
	if (i < 0 || i > 4) {
		debug->println("Invalid relay index!");
		return;
	}
	digitalWrite(relays[i].pin, HIGH);
	relays[i].state = true;
	startTime = millis();
	debug->print("Relay On: ");
	stop_reason = NORMAL;

}


void ActionsClass::relayOff(int i) {
	if (i < 0 || i > 4) {
		debug->println("Invalid relay index!");
		return;
	}
	digitalWrite(relays[i].pin, LOW);
	debug->println("Relay Off: ");
	relays[i].state = false;

}


void ActionsClass::relayOnTime(int i){
	debug->print(minutes);
	debug->print(":");
	debug->println(seconds);
	
	debug->print("Remaining:");
	debug->println(remaining);
	
	if (!relays[i].state && remaining > 0) {
			relayOn(i);
			//relays[i].state = true; 
		}
		if (relays[i].state && remaining <= 0) {
			stop(i);
			//relays[i].state = false;  
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


void ActionsClass::relayOnSwitch(int i){
	if(!relays[i].state) {
		debug->println("Relay on Switch begins!");
		relayOn(i);
	}
  }
  

ActionsClass Actions;