bool connect_device(FILE* &deviceFile);
void send_cmd(FILE* &deviceFile, char *str, int strSize);
void get_cmd(FILE* &deviceFile);


bool connect_device(FILE* &deviceFile){
	int x = 0; int max = 32;
	char fileName[6];
	strncpy(fileName, "COM", sizeof(fileName));
	do{
		sprintf(fileName+3, "%i", x);
		deviceFile = fopen(fileName, "r+");
		x++;
	}while(deviceFile == NULL && x < max);
	if(deviceFile != NULL){
		deviceFile = fopen(fileName, "w");
		fprintf(deviceFile, "");
		deviceFile = fopen(fileName, "r+");
	}
	return deviceFile == NULL;
}

void send_cmd(FILE* &deviceFile, char *str, int strSize){
	fprintf(deviceFile, "CMD,2079,%s\n", str);
}


void get_cmd(FILE* &deviceFile){
	char buff[256];
	while(1){
		fgets(buff, sizeof(buff), stdin);
		buff[strnlen(buff, sizeof(buff)) - 1] = '\0';
		send_cmd(deviceFile, buff, sizeof(buff));
	}
}
