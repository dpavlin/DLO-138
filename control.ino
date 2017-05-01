
enum { TRIGGER_AUTO, TRIGGER_NORM, TRIGGER_SINGLE };
uint8_t triggerType;


// ------------------------
void setTriggerType(uint8_t tType)	{
// ------------------------
	triggerType = tType;
	// break any running capture loop
	keepSampling = false;
}




// ------------------------
void controlLoop()	{
// ------------------------
	// start by reading the state of analog system
	readInpSwitches();
	if(triggerType == TRIGGER_AUTO)	{
		captureDisplayCycle(true);
	}
	else if(triggerType == TRIGGER_NORM)	{
		captureDisplayCycle(false);
	}
	else	{
		// single trigger
		clearWaves();
		indicateCapturing();
		// blocking call - until trigger
		sampleWaves(false);
		indicateCapturingDone();
		hold = true;
		// request repainting of screen labels in next draw cycle
		repaintLabels();
		// draw the waveform
		drawWaves();
		blinkLED();
		// dump captured data on serial port
		dumpSamples();

		// freeze display
		while(hold);
		
		// update display indicating hold released
		drawLabels();
	}

	processSerial();
}

void processSerial() {
	// process any long pending operations which cannot be serviced in ISR
	if ( Serial.available() ) {
		int serial = Serial.read();
		switch(serial) {
			case 10:
				break; // ignore enter
			case '?':
				Serial.println("# commandss: [d]umpSamples, print[s]tats, [hjkl] - encoder emulation");
				break;

			case 's':
				changeStats();
				break;
			case 'd':
				dumpSamples();
				break;

			// keyboard emulation of encoder for testing
			case 'h':
				hold = ! hold;
				repaintLabels();
				DBG_PRINT("hold ");
				DBG_PRINTLN(hold);
				break;
			case 'j':
				encoderChanged(1);
				break;
			case 'k':
				encoderChanged(-1);
				break;
			case 'l':
				readESwitchISR();
				break;


			default:
				Serial.print(serial, HEX);
		}
	}
}




// ------------------------
void captureDisplayCycle(boolean wTimeOut)	{
// ------------------------
	indicateCapturing();
	// blocking call - until timeout or trigger
	sampleWaves(wTimeOut);
	// draw the waveform
	indicateCapturingDone();
	drawWaves();
	// inter wait before next sampling
	if(triggered)
		blinkLED();
	
	if(hold)	{
		// update UI labels
		drawLabels();
		// dump captured data on serial port
		dumpSamples();
	}
	
	// freeze display if requested
	while(hold) processSerial(); // buttons are in ISR, serial is not
}
