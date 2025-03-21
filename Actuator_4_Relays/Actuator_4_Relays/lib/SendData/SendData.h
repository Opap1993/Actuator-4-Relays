
#ifndef SENDDATA_H_
#define SENDDATA_H_

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "../../Parameters.h"
#include "../lib/LoraWan/LoraWan.h"
#include "../lib/LoRaProcess/LoRaProcess.h"
#include "../lib/Actions/Actions.h"
#include "../lib/Encoder/Encoder.h"

class SendData {
	public:
	boolean joined = false;
	bool sendData(bool confirmation, uint32_t counter, uint32_t rejoinCounter);
	void EncodeData(uint32_t counter, uint32_t rejoinCounter);
	
	private:
	
	protected:
	Stream *debug;
	
	};



#endif /* SENDDATA_H_ */