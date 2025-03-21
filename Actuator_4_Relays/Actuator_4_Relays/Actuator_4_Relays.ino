#include <SoftwareSerial.h>
#include "lib/LoraWan/LoraWan.h"
#include "lib/LoRaProcess/LoRaProcess.h"
#include "lib/EEPROM/EEPROM.h"
#include "lib/Actions/Actions.h"
#include "lib/Encoder/Encoder.h"
#include "lib/Storage/Storage.h"
#include "lib/TimerOne/TimerOne.h"
#include "avr/wdt.h"
#include "avr/interrupt.h"

SoftwareSerial debug(6, 8);

//_________Global Vars__________

bool paused = false;

unsigned long  lastUplink = 0;
uint32_t interval;
volatile uint32_t counter;
uint32_t RejoinCounter = 1;
uint32_t confirmed_interval;
uint32_t failed_before_rejon;


void setup()
{
	wdt_enable(WDTO_8S);
	pinMode(10,OUTPUT);
	digitalWrite(10, HIGH);
	debug.begin(9600);
	debug.println("Setup");
	LoraWan.init(&debug);
	//LoRaPrs.joinLora();
	EEPROM.init(&debug);
	Storage.init(&debug,&EEPROM);
	Actions.init(&debug, &Encoder, &Storage);
	wdt_reset();
	
	counter = Storage.readCounter();
	interval = Storage.readInterval();
	
	confirmed_interval = round(43200 / interval);
	failed_before_rejon = round(21600 / interval);
	
	debug.print("Unconfirmed Packets : ");
	debug.println(confirmed_interval);
	
	debug.print("Failed Confirmations before rejoin : ");
	debug.println(failed_before_rejon);
	
	// Timers
	Timer1.initialize(1000000 * 1); // 1 second
	Timer1.attachInterrupt(everySecond); // blinkLED to run every n seconds
	
	// Interrupt
	pinMode(3,INPUT_PULLUP);
	delay(1000);
	attachInterrupt(digitalPinToInterrupt(3), sensorPulse, CHANGE);
	delay(1000);
	LoRaPrs.sendData(false, counter, RejoinCounter, &debug, confirmed_interval);
	
}

void loop()
{
	//wdt_reset();
	if (counter > 20000000){
		counter = 0;
		Storage.formatLog();
		} else {
		Storage.saveCounter(counter);
	}

	if(millis() - lastUplink > (interval * 1000) || Actions.shouldTransmit() ) {
		debug.print(F("Interval :"));
		debug.println(interval);
		RejoinCounter += 1;
		LoRaPrs.sendData(false, counter, RejoinCounter, &debug, confirmed_interval);
		lastUplink = millis();
	}
	run();
}

void run() {
	unsigned char buffer[20];
	short size = 0;
	int port = 0;
	
	size = LoraWan.receivePacket(buffer,20, port,1);
	if(size) {
		paused = true;
		debug.print(F("Downlink Size:"));
		debug.println(size);
		Actions.setAction(buffer);
		paused = false;
		wait(3);
		LoRaPrs.sendData(true, counter, RejoinCounter, &debug, confirmed_interval);
	}
	
	if (Actions.config) {
		detachInterrupt(digitalPinToInterrupt(3));
		Timer1.detachInterrupt();
		Actions.applyConfigurations();
	}
}


void everySecond() {
	if(!paused) {
		Actions.run();
	}
}

void wait(int seconds){
	unsigned long start = millis();
	
	while((millis() - start) <= (seconds * 1000));
}

void sensorPulse() {
	debug.println(F("Pulse"));
	counter = Actions.count(counter);
}