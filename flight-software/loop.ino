void loop(){
	// Update how long the program has been running
	programTime = millis();

	if(mode == 'F'){
		if(bcn){BcnCmd:
			digitalWrite(BuzzerPin, bcnOn ? HIGH : LOW);
			digitalWrite(LEDPin, bcnOn ? HIGH : LOW);
			bcnOn = !bcnOn;
		}
	}else if(mode == 'S'){ReadPacketCmd:
		// Read packet
		packet = "";
		while(XBee.available()){ packet += (char)XBee.read(); }
		// Parse packet
	}
	// Make decisions
	if(altitude >= AscentHeight && state == 'W'){AscentCmd: // Check to move into ascent
		state = 'A';
	}
	if(altitude >= ReleaseHeight && state == 'A'){ReleaseCmd: // Check to move into release (and release)
		//spin_servo(servo1);
		state = 'R';
	}
	if(state == 'R'){DescentCmd: // 
		
	}
	if(state == 'D'){HSCmd:
		
	}
	if(state == 'H'){LandCmd:
		
	}

	if(XBee.available){
		XBee.readBytesUntil('\0', packetBuff, sizeof(packetBuff)); if(strncmp(packetBuff, "CMD,2079,", 9) == 0){
			if(strncmp(packetBuff+9, "CX,", 3) == 0){
				if(strncmp(packetBuff+12, "ON", 2) == 0){
					cx = 1;
				}else if(strncmp(packetBuff+12, "OFF", 3) == 0){
					cx = 0;
				}
			}
		}
	}

	if(cx){SendPacketCmd:
		
	}
}

