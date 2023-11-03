void setup(){
	baseTime = get_baseTime();
	
	// Communication
	Serial.begin(9600); // Remove once finalized
	XBee.begin(9600);
	XBee.println("XBee connected");

	// SD card
	SD.begin(9600);
	
	while(!sox.begin_I2C()){ delay(10); XBee.println("LSM!"); }
	XBee.println("LSM6DSOX connected");
	
	XBee.print("Accelerometer range set to: ");
	switch(sox.getAccelRange()){
		case LSM6DS_ACCEL_RANGE_2_G: XBee.println("+-2G"); break;
		case LSM6DS_ACCEL_RANGE_4_G: XBee.println("+-4G"); break;
		case LSM6DS_ACCEL_RANGE_8_G: XBee.println("+-8G"); break;
		case LSM6DS_ACCEL_RANGE_16_G: XBee.println("+-16G"); break;
	}

	XBee.print("Gyro range set to: ");
	switch(sox.getGyroRange()){
		case LSM6DS_GYRO_RANGE_125_DPS: XBee.println("125 degrees/s"); break;
		case LSM6DS_GYRO_RANGE_250_DPS: XBee.println("250 degrees/s"); break;
		case LSM6DS_GYRO_RANGE_500_DPS: XBee.println("500 degrees/s"); break;
		case LSM6DS_GYRO_RANGE_1000_DPS: XBee.println("1000 degrees/s"); break;
		case LSM6DS_GYRO_RANGE_2000_DPS: XBee.println("2000 degrees/s"); break;
		case ISM330DHCX_GYRO_RANGE_4000_DPS: break; //unsupported range for the DSOX
	}

	XBee.print("Accelerometer data rate set to: ");
	switch(sox.getAccelDataRate()){
		case LSM6DS_RATE_SHUTDOWN: XBee.println("0 Hz"); break;
		case LSM6DS_RATE_12_5_HZ: XBee.println("12.5 Hz"); break;
		case LSM6DS_RATE_26_HZ: XBee.println("26 Hz"); break;
		case LSM6DS_RATE_52_HZ: XBee.println("52 Hz"); break;
		case LSM6DS_RATE_104_HZ: XBee.println("104 Hz"); break;
		case LSM6DS_RATE_208_HZ: XBee.println("208 Hz"); break;
		case LSM6DS_RATE_416_HZ: XBee.println("416 Hz"); break;
		case LSM6DS_RATE_833_HZ: XBee.println("833 Hz"); break;
		case LSM6DS_RATE_1_66K_HZ: XBee.println("1.66 KHz"); break;
		case LSM6DS_RATE_3_33K_HZ: XBee.println("3.33 KHz"); break;
		case LSM6DS_RATE_6_66K_HZ: XBee.println("6.66 KHz"); break;
	}

	XBee.print("Gyro data rate set to: ");
	switch(sox.getGyroDataRate()){
		case LSM6DS_RATE_SHUTDOWN: XBee.println("0 Hz"); break;
		case LSM6DS_RATE_12_5_HZ: XBee.println("12.5 Hz"); break;
		case LSM6DS_RATE_26_HZ: XBee.println("26 Hz"); break;
		case LSM6DS_RATE_52_HZ: XBee.println("52 Hz"); break;
		case LSM6DS_RATE_104_HZ: XBee.println("104 Hz"); break;
		case LSM6DS_RATE_208_HZ: XBee.println("208 Hz"); break;
		case LSM6DS_RATE_416_HZ: XBee.println("416 Hz"); break;
		case LSM6DS_RATE_833_HZ: XBee.println("833 Hz"); break;
		case LSM6DS_RATE_1_66K_HZ: XBee.println("1.66 KHz"); break;
		case LSM6DS_RATE_3_33K_HZ: XBee.println("3.33 KHz"); break;
		case LSM6DS_RATE_6_66K_HZ: XBee.println("6.66 KHz"); break;
	}

	XBee.println("Adafruit BMP388 / BMP390 test");
	if(!bmp.begin_I2C()){ XBee.println("Could not find a valid BMP3 sensor, check wiring!"); }

	// Set up oversampling, filter initialization, and ground elevation
	bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
	bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);
	bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
	bmp.setOutputDataRate(BMP3_ODR_50_HZ);
	while(!bmp.performReading()){ XBee.println("Failed to perform reading"); }
	for(unsigned i = 0; i < 100; i++){ GroundElevation = bmp.readAltitude(SEALEVELPRESSURE_HPA); } XBee.println("Ground Elevation Set");

	// Print out packet format
	packet = "TEAM_ID,MISSION_TIME,PACKET_COUNT,SW_STATE, PL_STATE,ALTITUDE, TEMP, VOLTAGE, GPS_LATITUDE, GPS_LONGITUDE, GYRO_R,GYRO_P,GYRO_Y";
	for(unsigned i = 0; i < packet.size(); i++){ XBee.print(packet[i]);} XBee.println();

	pinMode(BuzzerPin, OUTPUT); pinMode(LEDPin, OUTPUT);
	packetTimer = time(0);

	oldAlt = 0; velocity = 0;
}

