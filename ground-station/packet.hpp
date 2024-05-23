class Packet{
	public:
		bool changed = 0;
		std::string packetString;
		
		int teamId;
		char timeVals[16];
		int packetCount;
		char mode;
		char state[16];
		float altitude;
		float airSpeed;
		char hsDeployed;
		char pcDeployed;
		float temp;
		float voltage;
		float pressure;
		char gpsTime[16];
		float gpsAlt;
		float gpsLat;
		float gpsLong;
		int gpsStats;
		float tiltX;
		float tiltY;
		float rotZ;
		char cmdEcho[32];
		
		Packet();
		Packet(char *str, unsigned strLen);
		void parse_packet(const char *str, unsigned strLen);
		void print();
	private:
};


Packet::Packet(){}

Packet::Packet(char *str, unsigned strLen){
	parse_packet(str, strLen);
}

void Packet::parse_packet(const char *str, unsigned strLen){
	int startIndex = 0;
	int endIndex = 0;
	char buff[64];
	
	for(int i = startIndex; i < strnlen(str, strLen) && str[i] != ','; i++){
		endIndex = i;
	}
	snprintf(buff, endIndex-startIndex+2, "%s", str+startIndex);
	startIndex = endIndex + 2;
	sscanf(buff, "%i", &teamId);
	
	for(int i = startIndex; i < strnlen(str, strLen) && str[i] != ','; i++){
		endIndex = i;
	}
	snprintf(buff, endIndex-startIndex+2, "%s", str+startIndex);
	startIndex = endIndex + 2;
	sscanf(buff, "%s", timeVals);
	//printf("%s!\n", buff); return;
	
	for(int i = startIndex; i < strnlen(str, strLen) && str[i] != ','; i++){
		endIndex = i;
	}
	snprintf(buff, endIndex-startIndex+2, "%s", str+startIndex);
	startIndex = endIndex + 2;
	sscanf(buff, "%i", &packetCount);
	
	for(int i = startIndex; i < strnlen(str, strLen) && str[i] != ','; i++){
		endIndex = i;
	}
	snprintf(buff, endIndex-startIndex+2, "%s", str+startIndex);
	startIndex = endIndex + 2;
	sscanf(buff, "%c", &mode);
	
	for(int i = startIndex; i < strnlen(str, strLen) && str[i] != ','; i++){
		endIndex = i;
	}
	snprintf(buff, endIndex-startIndex+2, "%s", str+startIndex);
	startIndex = endIndex + 2;
	sscanf(buff, "%s", state);
	
	for(int i = startIndex; i < strnlen(str, strLen) && str[i] != ','; i++){
		endIndex = i;
	}
	snprintf(buff, endIndex-startIndex+2, "%s", str+startIndex);
	startIndex = endIndex + 2;
	sscanf(buff, "%f", &altitude);
	
	for(int i = startIndex; i < strnlen(str, strLen) && str[i] != ','; i++){
		endIndex = i;
	}
	snprintf(buff, endIndex-startIndex+2, "%s", str+startIndex);
	startIndex = endIndex + 2;
	sscanf(buff, "%f", &airSpeed);
	
	for(int i = startIndex; i < strnlen(str, strLen) && str[i] != ','; i++){
		endIndex = i;
	}
	snprintf(buff, endIndex-startIndex+2, "%s", str+startIndex);
	startIndex = endIndex + 2;
	sscanf(buff, "%c", &hsDeployed);
	
	for(int i = startIndex; i < strnlen(str, strLen) && str[i] != ','; i++){
		endIndex = i;
	}
	snprintf(buff, endIndex-startIndex+2, "%s", str+startIndex);
	startIndex = endIndex + 2;
	sscanf(buff, "%c", &pcDeployed);
	
	for(int i = startIndex; i < strnlen(str, strLen) && str[i] != ','; i++){
		endIndex = i;
	}
	snprintf(buff, endIndex-startIndex+2, "%s", str+startIndex);
	startIndex = endIndex + 2;
	sscanf(buff, "%f", &temp);
	
	for(int i = startIndex; i < strnlen(str, strLen) && str[i] != ','; i++){
		endIndex = i;
	}
	snprintf(buff, endIndex-startIndex+2, "%s", str+startIndex);
	startIndex = endIndex + 2;
	sscanf(buff, "%f", &voltage);
	
	for(int i = startIndex; i < strnlen(str, strLen) && str[i] != ','; i++){
		endIndex = i;
	}
	snprintf(buff, endIndex-startIndex+2, "%s", str+startIndex);
	startIndex = endIndex + 2;
	sscanf(buff, "%f", &pressure);
	
	for(int i = startIndex; i < strnlen(str, strLen) && str[i] != ','; i++){
		endIndex = i;
	}
	snprintf(buff, endIndex-startIndex+2, "%s", str+startIndex);
	startIndex = endIndex + 2;
	sscanf(buff, "%s", gpsTime);
	
	for(int i = startIndex; i < strnlen(str, strLen) && str[i] != ','; i++){
		endIndex = i;
	}
	snprintf(buff, endIndex-startIndex+2, "%s", str+startIndex);
	startIndex = endIndex + 2;
	sscanf(buff, "%f", &gpsAlt);
	
	for(int i = startIndex; i < strnlen(str, strLen) && str[i] != ','; i++){
		endIndex = i;
	}
	snprintf(buff, endIndex-startIndex+2, "%s", str+startIndex);
	startIndex = endIndex + 2;
	sscanf(buff, "%f", &gpsLat);
	
	for(int i = startIndex; i < strnlen(str, strLen) && str[i] != ','; i++){
		endIndex = i;
	}
	snprintf(buff, endIndex-startIndex+2, "%s", str+startIndex);
	startIndex = endIndex + 2;
	sscanf(buff, "%f", &gpsLong);
	
	for(int i = startIndex; i < strnlen(str, strLen) && str[i] != ','; i++){
		endIndex = i;
	}
	snprintf(buff, endIndex-startIndex+2, "%s", str+startIndex);
	startIndex = endIndex + 2;
	sscanf(buff, "%i", &gpsStats);
	
	for(int i = startIndex; i < strnlen(str, strLen) && str[i] != ','; i++){
		endIndex = i;
	}
	snprintf(buff, endIndex-startIndex+2, "%s", str+startIndex);
	startIndex = endIndex + 2;
	sscanf(buff, "%f", &tiltX);
	
	for(int i = startIndex; i < strnlen(str, strLen) && str[i] != ','; i++){
		endIndex = i;
	}
	snprintf(buff, endIndex-startIndex+2, "%s", str+startIndex);
	startIndex = endIndex + 2;
	sscanf(buff, "%f", &tiltY);
	
	for(int i = startIndex; i < strnlen(str, strLen) && str[i] != ','; i++){
		endIndex = i;
	}
	snprintf(buff, endIndex-startIndex+2, "%s", str+startIndex);
	startIndex = endIndex + 2;
	sscanf(buff, "%f", &rotZ);
	
	for(int i = startIndex; i < strnlen(str, strLen) && str[i] != ','; i++){
		endIndex = i;
	}
	snprintf(buff, endIndex-startIndex+2, "%s", str+startIndex);
	startIndex = endIndex + 2;
	sscanf(buff, "%s", cmdEcho);
	
	changed = 1;
}

void Packet::print(){
	printf("Team ID: %i\n", teamId);
	printf("Time: %s\n", timeVals);
	printf("Packet Count: %i\n", packetCount);
	printf("Mode: %c\n", mode);
	printf("State: %s\n", state);
	printf("Altitude: %f\n", altitude);
	printf("Airspeed: %f\n", airSpeed);
	printf("HS: %c\n", hsDeployed);
	printf("PC: %c\n", pcDeployed);
	printf("Temperature: %f\n", temp);
	printf("Voltage: %f\n", voltage);
	printf("Pressure: %f\n", pressure);
	printf("GPS Time: %s\n", gpsTime);
	printf("GPS Altitude: %f\n", gpsAlt);
	printf("GPS Latitude: %f\n", gpsLat);
	printf("GPS longitude: %f\n", gpsLong);
	printf("GPS Satellites %i\n", gpsStats);
	printf("Tilt X: %f\n", tiltX);
	printf("Tilt Y: %f\n", tiltY);
	printf("Rotation Z: %f\n", rotZ);
	printf("CMD_ECHO: %s\n", cmdEcho);
}

