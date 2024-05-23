void set_port(int &port);
void send_cmd(char *str, int strSize, int port);
void get_cmd_input(int port);
void sim_mode(int port);

bool simMode = 0;

void set_port(int &port){
	char mode[]={'8','N','1',0};
	while(RS232_OpenComport(port, 9600, mode, 0)){
		printf("Can not open comport\n");
		port++;
	}
}

void send_cmd(char *str, int strSize, int port){
	char buff[strSize + 11];
	sprintf(buff, "CMD,2079,%s%c", str, CmdTermChar);
	printf("S: %s\n", buff);
	RS232_cputs(port, buff);
}

void get_cmd_input(int port){
	char buff[256];
	char b[256];
	printf("Enter all commands to be sent to the XBee here\n");
	printf("Do not include the command prefix (%s)\n\n", CmdPrefix);
	while(1){
		fgets(buff, sizeof(buff), stdin);
		buff[strnlen(buff, sizeof(buff)) - 1] = '\0';
		send_cmd(buff, sizeof(buff), port);
	}
}

void sim_mode(int port){
	FILE *readFile = fopen(SimpFileName, "r");
	char buff[128];
	
	sprintf(buff, "SIM,ENABLE");
	send_cmd(buff, sizeof(buff), port);
	sf::sleep(sf::milliseconds(1000));
	
	sprintf(buff, "SIM,ACTIVATE");
	send_cmd(buff, sizeof(buff), port);
	sf::sleep(sf::milliseconds(1000));
	simMode = 1;
	
	sprintf(buff, "PCKT,0");
	send_cmd(buff, sizeof(buff), port);
	sf::sleep(sf::milliseconds(1000));
	
	while(fgets(buff, sizeof(buff), readFile) != NULL){
		if(strncmp(buff, "CMD,$,SIMP,", 11) == 0 && simMode){
			buff[strlen(buff) - 1] = '\0';
			sprintf(buff, "%s", buff+6);
			send_cmd(buff, sizeof(buff), port);
			//fprintf(usbFile, "%s", buff);
			sleep(1);
		}else{
			printf("%s!\n", buff);
		}
	}
}
