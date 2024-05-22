void set_port(int &port);
void send_cmd(char *str, int strSize, int port);
void get_cmd(int port);

void set_port(int &port){
	char mode[]={'8','N','1',0};
	while(RS232_OpenComport(port, 9600, mode, 0)){
		printf("Can not open comport\n");
		port++;
	}
}

void send_cmd(char *str, int strSize, int port){
	char buff[strSize + 10];
	sprintf(buff, "CMD,2079,%s\n", str);
	RS232_cputs(port, buff);
}


void get_cmd(int port){
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
