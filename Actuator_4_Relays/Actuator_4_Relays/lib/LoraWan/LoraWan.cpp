//
//
//

#include "LoraWan.h"
#include "../../Parameters.h"

void LoraWanClass::init(Stream *debug)
{
	this->debug = debug;
	memset(_buffer, 0, 256);
	Serial.begin(57600);
	setLoraModule();
}

void LoraWanClass::setLoraModule(){
	pinMode(LORA_RESET,OUTPUT);
	digitalWrite(LORA_RESET,HIGH);
	delay(200);
}

void LoraWanClass::setDataRate(uint8_t dr) {
	char cmd[64];
	memset(cmd, 0, 64);
	sprintf(cmd, "mac set dr %d\r\n", dr);
	Serial.print(cmd);
	#if _DEBUG_SERIAL_
	debug->print(cmd);
	debugPrint(DEFAULT_TIMEOUT);
	#else
	delay(DEFAULT_TIMEOUT);
	#endif
}

void LoraWanClass::setClass(char *c) {
	char cmd[64];
	memset(cmd, 0, 64);
	sprintf(cmd, "mac set class %s\r\n", c);
	Serial.print(cmd);
	#if _DEBUG_SERIAL_
	debug->print(cmd);
	debugPrint(DEFAULT_TIMEOUT);
	#else
	delay(DEFAULT_TIMEOUT);
	#endif
}
void LoraWanClass::passThrough(){
	char c;
	if (debug->available()){
		Serial.write(debug->read());
	}

	if (Serial.available()){
		debug->write(Serial.read());
	}
	
}

#if _DEBUG_SERIAL_
void LoraWanClass::debugPrint(unsigned char timeout)
{
	unsigned long timerStart, timerEnd;

	timerStart = millis();
	
	while(1)
	{
		while(Serial.available()){
			debug->write(Serial.read());
		}
		
		timerEnd = millis();
		if(timerEnd - timerStart > 1000 * timeout)break;
	}
}
#endif

// This command sets the duty cycle for the given channel
// duty : duty cycle percentage
void LoraWanClass::getEUI(){
	Serial.println("sys get hweui");
	#if _DEBUG_SERIAL_
	debugPrint(DEFAULT_TIMEOUT);
	#else
	delay(DEFAULT_TIMEOUT);
	#endif
}

void LoraWanClass::rxStop(){
	Serial.println("radio rxstop");
	#if _DEBUG_SERIAL_
	debugPrint(DEFAULT_TIMEOUT);
	#else
	delay(DEFAULT_TIMEOUT);
	#endif
}


void LoraWanClass::getDevEui(){
	Serial.println("sys get deveui");
	#if _DEBUG_SERIAL_
	debugPrint(DEFAULT_TIMEOUT);
	#else
	delay(DEFAULT_TIMEOUT);
	#endif
}


void LoraWanClass::setDutyCyle(int duty){
	int duty_cycle = (100/duty) -1;
	for (int i = 0; i <16; i++){
		Serial.print("mac set ch dcycle ");
		Serial.print(i);
		Serial.print(" ");
		Serial.println(duty_cycle);
		delay(100);
		wdt_reset();
		while (Serial.available()){
			debug->write(Serial.read());
		}
	}
}

void LoraWanClass::setDutyCycle(int channel, int duty){
	int duty_cycle = (100/duty) -1;
	Serial.print("mac set ch dycycle");
	Serial.print(channel);
	Serial.print(" ");
	Serial.println(duty_cycle);
	
	#if _DEBUG_SERIAL_
	debugPrint(DEFAULT_TIMEOUT);
	#else
	delay(DEFAULT_TIMEOUT);
	#endif
}

void LoraWanClass::setAppKey(char *AppKey){
	
	char cmd[64];
	memset(cmd, 0, 64);
	sprintf(cmd, "mac set appkey %s\r\n", AppKey);
	
	Serial.print(cmd);

	#if _DEBUG_SERIAL_
	debugPrint(DEFAULT_TIMEOUT);
	#else
	delay(DEFAULT_TIMEOUT);
	#endif
}

void LoraWanClass::setAppEUI(char *AppEUI){
	
	char cmd[64];
	memset(cmd, 0, 64);
	sprintf(cmd, "mac set appeui %s\r\n", AppEUI);
	
	Serial.print(cmd);

	#if _DEBUG_SERIAL_
	debug->print(cmd);
	debugPrint(DEFAULT_TIMEOUT);
	#else
	delay(DEFAULT_TIMEOUT);
	#endif
	
}

void LoraWanClass::setDevEUI(char *DevEUI){
	
	char cmd[64];
	memset(cmd, 0, 64);
	sprintf(cmd, "mac set deveui %s\r\n", DevEUI);
	
	Serial.print(cmd);

	#if _DEBUG_SERIAL_
	debug->print(cmd);
	debugPrint(DEFAULT_TIMEOUT);
	#else
	delay(DEFAULT_TIMEOUT);
	#endif
	
}

bool LoraWanClass::joinOtaa(unsigned char timeout){
	wdt_reset();
	char *response;
	
	Serial.println("mac join otaa");
	
	memset(_buffer, 0, BEFFER_LENGTH_MAX);
	readBuffer(_buffer, BEFFER_LENGTH_MAX, timeout);
	
	#if _DEBUG_SERIAL_
	debug->println(_buffer);
	debug->println("mac join otaa");
	debugPrint(DEFAULT_TIMEOUT);
	#else
	delay(DEFAULT_TIMEOUT);
	#endif
	
	response = strstr(_buffer, "no_free_ch");
	if(response)return false;
	response = strstr(_buffer, "silent");
	if(response)return false;
	response = strstr(_buffer, "busy");
	if(response)return false;
	response = strstr(_buffer, "mac_paused");
	if(response)return false;
	response = strstr(_buffer, "denied");
	if(response)return false;

	response = strstr(_buffer, "accepted");
	if(response)return true;
	
	return false;
}

bool LoraWanClass::transferPacket(int channel, unsigned char *buffer,unsigned char length, unsigned char timeout, bool *availableCom){
	
	char temp[2] = {0};
	Serial.print("mac tx uncnf ");
	Serial.print(channel);
	Serial.print(" ");
	for(unsigned char i = 0; i < length; i ++){
		sprintf(temp,"%02x", buffer[i]);
		Serial.print(temp);
		debug->write(temp);
	}
	debug->println();
	Serial.println();
	
	memset(_buffer, 0, BEFFER_LENGTH_MAX);
	readBuffer(_buffer, BEFFER_LENGTH_MAX, timeout);
	
	#if _DEBUG_SERIAL_
	debug->print(_buffer);
	debugPrint(DEFAULT_TIMEOUT);
	#else
	delay(DEFAULT_TIMEOUT);
	#endif
	
	if(strstr(_buffer, "mac_tx_ok") || strstr(_buffer, "mac_ok")){
		*availableCom = true;
		debug->println("ok message!");
		return true;
	}
	
	debug->println("not ok message!");
	//if (cntnr >= 5){
	//	asm volatile (" jmp 0");
	//}	
	wake();
	//cntnr++;
	

	return false;
	
}

void LoraWanClass::setReceiceWindowSecond(uint8_t dr, float f){
	char cmd[64];
	memset(cmd, 0, 64);
	sprintf(cmd, "mac set rx2 %d %d00000\r\n", (short) dr , (int) (f * 10));
	
	Serial.print(cmd);

	#if _DEBUG_SERIAL_
	debug->print(cmd);
	debugPrint(DEFAULT_TIMEOUT);
	#else
	delay(DEFAULT_TIMEOUT);
	#endif
}

void LoraWanClass::setRetransmissions(uint8_t n) {
	char cmd[64];
	memset(cmd, 0, 64);
	sprintf(cmd, "mac set retx %d\r\n", n);
	
	Serial.print(cmd);

	#if _DEBUG_SERIAL_
	debug->print(cmd);
	debugPrint(DEFAULT_TIMEOUT);
	#else
	delay(DEFAULT_TIMEOUT);
	#endif
}

bool LoraWanClass::transferConfirmedPacket(int channel, unsigned char *buffer,unsigned char length, unsigned char timeout){
	
	char temp[2] = {0};
	Serial.print("mac tx cnf ");
	Serial.print(channel);
	Serial.print(" ");
	for(unsigned char i = 0; i < length; i ++){
		sprintf(temp,"%02x", buffer[i]);
		Serial.print(temp);
		debug->write(temp);
	}
	debug->println();
	Serial.println();
	
	memset(_buffer, 0, BEFFER_LENGTH_MAX);
	readBuffer(_buffer, BEFFER_LENGTH_MAX, timeout);
	
	#if _DEBUG_SERIAL_
	debug->print(_buffer);
	debugPrint(DEFAULT_TIMEOUT);
	#else
	delay(DEFAULT_TIMEOUT);
	#endif
	
	if(strstr(_buffer, "mac_tx_ok") || strstr(_buffer, "mac_ok")){
		//*availableCom = true;
		debug->println("ok message!");
		return true;
	}
	
	return false;
}

void LoraWanClass::sleep(int timeout){
	Serial.println("sys sleep 5000");
	
	#if _DEBUG_SERIAL_
	debugPrint(DEFAULT_TIMEOUT);
	#else
	delay(DEFAULT_TIMEOUT);
	#endif
}

void LoraWanClass::wake(){
	debug->println("LoRa Wake");
	Serial.end();
	pinMode(LORA_TX,OUTPUT);
	digitalWrite(LORA_TX,LOW);
	delay(10);
	digitalWrite(LORA_TX,HIGH);
	delay(10);
	Serial.begin(57600);
	Serial.write(0x55);
	
	#if _DEBUG_SERIAL_
	debugPrint(DEFAULT_TIMEOUT);
	#else
	delay(DEFAULT_TIMEOUT);
	#endif
}

short LoraWanClass::receivePacket(char *buffer, short length, int *port, int delay) {
	char *ptr;
	short number = 0;
	int counter = 0;

	memset(buffer, 0, length);
	readBuffer(_buffer, BEFFER_LENGTH_MAX, delay);
	ptr = strstr(_buffer, "mac_rx ");

	
	if(ptr) {
		ptr += 7;
		port = atoi(ptr);
		
		int bitStep = 7;
		
		if(port >= 100) {
			bitStep = 4;
			} else if (port >= 10) {
			bitStep = 3;
			} else {
			bitStep = 2;
		}
		
		for(short i = 0; i < BEFFER_LENGTH_MAX; i ++) {
			char temp[2] = {0};
			unsigned char tmp, result = 0;
			
			if(i % 2 == 0) {
				
				temp[0] = *(ptr + i + bitStep);
				temp[1] = *(ptr + i + bitStep + 1);
				
				for(unsigned char j = 0; j < 2; j ++)
				{
					if((temp[j] >= '0') && (temp[j] <= '9'))
					tmp = temp[j] - '0';
					else if((temp[j] >= 'A') && (temp[j] <= 'F'))
					tmp = temp[j] - 'A' + 10;
					else if((temp[j] >= 'a') && (temp[j] <= 'f'))
					tmp = temp[j] - 'a' + 10;

					result = result * 16 + tmp;
				}
				
				if(i < length)
				{
					buffer[counter] = result - 0;
				}
				counter++;
			}
			
			if(_buffer[i+7+bitStep] == '\r' && _buffer[i+7+bitStep+1] == '\n')
			{
				break;
			}
		}
	}
	memset(_buffer, 0, BEFFER_LENGTH_MAX);
	if (counter) number = counter - 1;
	return number;
}

short LoraWanClass::readBuffer(char *buffer, short length, unsigned char timeout)
{
	short i = 0;
	unsigned long timerStart, timerEnd;

	timerStart = millis();

	while(1)
	{
		wdt_reset();
		if(i < length)
		{
			while(Serial.available())
			{
				char c = Serial.read();
				buffer[i ++] = c;
				wdt_reset();
			}
		}
		
		timerEnd = millis();
		if(timerEnd - timerStart > 1000 * timeout) break;
	}
	
	return i;
}

LoraWanClass LoraWan;
