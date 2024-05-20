#include <SFML/Graphics.hpp>

#include <deque>
#include <thread>
#include <limits>

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define WinWidth 1900
#define WinHeight 980
#define WinTitle "Snapdragon Ground Station"
#define WinBgColor sf::Color(0x4b, 0x22, 0xbf)

//#define FontFile "fonts/static/Cinzel-Regular.ttf"
//#define FontFile "fonts/Sacramento-Regular.ttf"
#define FontFile "fonts/Freeman/Freeman-Regular.ttf"

#define DefaultButtonWidth 250
#define DefaultButtonHeight 150
#define DefaultButtonSpacing 20
#define DefaultButtonBgColor sf::Color(0x13, 0x5f, 0xd9)
//#define DefaultButtonTextColor sf::Color(0xff - 0x13, 0xff - 0x5f, 0xff - 0xd9)
#define DefaultButtonTextColor sf::Color(0xff, 0xff, 0xff)

#define CmdPrefix "CMD,2079,"

class Button{
	public:
		sf::RectangleShape bg;
		sf::Text text;
		
		Button(float x, float y, float w, float h, sf::Font &font, std::string t, int fontSize);
		void set_position(float x, float y);
		void set_size(float w, float h);
		void set_text(std::string str);
		void set_colors(sf::Color bgColor, sf::Color textColor);
		void draw(sf::RenderWindow &window);
	private:
};

Button::Button(float x, float y, float w, float h, sf::Font &font, std::string t, int fontSize){
	bg.setPosition(x, y);
	bg.setSize(sf::Vector2f(w, h));
	text.setString(t);
	text.setFont(font);
	text.setCharacterSize(500);
	text.setFillColor(sf::Color(0, 0, 0));
	text.setScale(.9 * bg.getSize().x/text.getGlobalBounds().width, .9 * bg.getSize().y/text.getGlobalBounds().height);
	text.setPosition(bg.getPosition().x + bg.getSize().x/2 - text.getGlobalBounds().width/2, bg.getPosition().y + bg.getSize().y/2 - text.getGlobalBounds().height/2);
	text.move(text.getPosition().x - text.getGlobalBounds().left, text.getPosition().y - text.getGlobalBounds().top);
}

void Button::set_text(std::string str){
}

void Button::set_colors(sf::Color bgColor, sf::Color textColor){
	bg.setFillColor(bgColor);
	text.setFillColor(textColor);
}

void Button::draw(sf::RenderWindow &window){
	window.draw(bg);
	window.draw(text);
}

class Graph{
	public:
		sf::VertexArray points;//(sf::LineStrip, 2);
		int maxPoints;
		sf::Vector2f position;
		sf::Vector2f maxSize;
		std::deque<float> values = {0, 0, 0};
		//float min = std::numeric_limits<float>::max();
		//float max = std::numeric_limits<float>::min();
		float min = 0;
		float max = 0;
		std::string title;
		sf::Text label;
		sf::Text maxLabel;
		sf::Text minLabel;
		
		Graph(int m, int x, int y, int w, int h, sf::Font &font, std::string str);
		void add_point(float y);
		void draw(sf::RenderWindow &window);
		void generate_points();
	private:
};

Graph::Graph(int m, int x, int y, int w, int h, sf::Font &font, std::string str){
	//points.resize(maxPoints);
	maxPoints = m;
	points.resize(2);
	points.setPrimitiveType(sf::LineStrip);
	points[0].color = sf::Color(0, 0, 0);
	points[1].color = sf::Color(0, 0, 0);
	points[0].position.x = x;
	points[1].position.x = x;
	points[0].position.y = y;
	points[1].position.y = y;
	
	position.x = x;
	position.y = y;
	maxSize.x = w;
	maxSize.y = h;
	
	label.setString(str);
	label.setFont(font);
	label.setCharacterSize(500);
	label.setFillColor(sf::Color(0, 0, 0));
	label.setScale(.9 * w/label.getGlobalBounds().width, .9 * w/label.getGlobalBounds().width);
	label.setPosition(x + w/2 - label.getGlobalBounds().width/2, y+h);
	label.move(label.getPosition().x - label.getGlobalBounds().left, label.getPosition().y - label.getGlobalBounds().top);
	
	minLabel.setString("123.456");
	minLabel.setFont(font);
	minLabel.setCharacterSize(500);
	minLabel.setFillColor(sf::Color(0x88, 0x88, 0x88));
	minLabel.setScale(.1 * maxSize.y/minLabel.getGlobalBounds().height, .1 * maxSize.y/minLabel.getGlobalBounds().height);
	minLabel.setPosition(position.x + maxSize.x, position.y + maxSize.y - minLabel.getGlobalBounds().height/2);
	minLabel.move(minLabel.getPosition().x - minLabel.getGlobalBounds().left, minLabel.getPosition().y - minLabel.getGlobalBounds().top);
	
	maxLabel.setString("123.456");
	maxLabel.setFont(font);
	maxLabel.setCharacterSize(500);
	maxLabel.setFillColor(sf::Color(0x88, 0x88, 0x88));
	maxLabel.setScale(.1 * maxSize.y/maxLabel.getGlobalBounds().height, .1 * maxSize.y/maxLabel.getGlobalBounds().height);
	maxLabel.setPosition(position.x + maxSize.x, position.y + maxLabel.getGlobalBounds().height/2);
	maxLabel.move(maxLabel.getPosition().x - maxLabel.getGlobalBounds().left, maxLabel.getPosition().y - maxLabel.getGlobalBounds().top);
}

void Graph::add_point(float y){
	values.push_back(y);
	if(values.size() > maxPoints){
		values.pop_front();
	}
}

void Graph::draw(sf::RenderWindow &window){
	char buff[10];
	sprintf(buff, "%.3f\n", min);
	minLabel.setString(std::string(buff));
	sprintf(buff, "%.3f\n", max);
	maxLabel.setString(std::string(buff));
	
	
	generate_points();
	window.draw(points);
	window.draw(label);
	window.draw(minLabel);
	window.draw(maxLabel);
}

void Graph::generate_points(){
	int size = values.size();
	min = std::numeric_limits<float>::max();
	max = std::numeric_limits<float>::min();
	float p;
	for(int i = 0; i < size; i++){
		p = values[i];
		if(p < min){ min = p; }
		if(p > max){ max = p; }
	}
	int pointWidth = (int)(maxSize.x/size);
	float scale = maxSize.y/(max - min);
	//printf("min: %f max: %f scale: %f\n", min, max, scale);
	points.clear();
	for(int i = 0; i < size; i++){
		points.append(sf::Vertex(sf::Vector2f(position.x+i*pointWidth, position.y + (max-values[i])*scale), sf::Color(0, 0, 0)));
		//printf("%f %f\n", points[i].position.x, points[i].position.y);
	}
}

class Packet{
	public:
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
		void parse_packet(char *str, unsigned strLen);
		void print();
	private:
};


Packet::Packet(){}

Packet::Packet(char *str, unsigned strLen){
	parse_packet(str, strLen);
}

void Packet::parse_packet(char *str, unsigned strLen){
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

void draw_graphs(sf::RenderWindow &window, Graph &altGraph, Graph &g2);
void update_graphs(Graph &altGraph, Graph &g2, sf::RenderWindow &window);
bool connect_device(FILE* &deviceFile);
void send_cmd(FILE* &deviceFile, char *str, int strSize);
void get_cmd(FILE* &deviceFile);
bool mouse_in_range(const sf::Vector2i &mousePos, const sf::Vector2f &objPos, const sf::Vector2f &objSize);
bool mouse_clicked(const sf::Mouse &mouse, const Button btn, sf::Window &win);



int main(){
	srand(time(0));
	FILE* usbDevice;
	if(connect_device(usbDevice)){
		printf("Could not find USB device, exiting\n");
		return 1;
	}
	std::thread sendThread(get_cmd, std::ref(usbDevice));
	sendThread.detach();
	
    sf::RenderWindow window(sf::VideoMode(WinWidth, WinHeight), WinTitle);
	
	sf::Font font;
	font.loadFromFile(FontFile);
	
	Button simEnableBtn(DefaultButtonSpacing, WinHeight - DefaultButtonSpacing - DefaultButtonHeight, DefaultButtonWidth, DefaultButtonHeight, font, "SIM  Enable", 30);
	Button simActivateBtn(DefaultButtonSpacing*2+DefaultButtonWidth, WinHeight - DefaultButtonSpacing - DefaultButtonHeight, DefaultButtonWidth, DefaultButtonHeight, font, "SIM  Activate", 30);
	simEnableBtn.set_colors(DefaultButtonBgColor, DefaultButtonTextColor);
	simActivateBtn.set_colors(DefaultButtonBgColor, DefaultButtonTextColor);
	
	Graph altGraph(15, 5, 5, 200, 200, font, "Altitude");
	Graph g2(15, 450, 5, 200, 200, font, "Speed");
	
	std::thread updateThread(update_graphs, std::ref(altGraph), std::ref(g2), std::ref(window));
	//std::thread drawGraphThread(draw_graphs, std::ref(window), std::ref(altGraph), std::ref(g2));
	
    while(window.isOpen()){
        sf::Event event;
		sf::Mouse mouse;
        while (window.pollEvent(event)){
			switch(event.type){
				case sf::Event::Closed:
					window.close();
					break;
				case sf::Event::MouseButtonPressed:
					if(mouse_clicked(mouse, simEnableBtn, window)){
						char buff[] = "SIM,ENABLE";
						std::thread t(send_cmd, std::ref(usbDevice), buff, sizeof(buff));
						t.detach();
					}else if(mouse_clicked(mouse, simActivateBtn, window)){
						char buff[] = "SIM,ACTIVATE";
						std::thread t(send_cmd, std::ref(usbDevice), buff, sizeof(buff));
						t.detach();
					}
					break;
			}
        }
		
        window.clear(WinBgColor);
		simEnableBtn.draw(window);
		simActivateBtn.draw(window);
		altGraph.draw(window);
		g2.draw(window);
        window.display();
    }
	updateThread.detach();
	
    return 0;
}

void draw_graphs(sf::RenderWindow &window, Graph &altGraph, Graph &g2){
	altGraph.draw(window);
	g2.draw(window);
}

void update_graphs(Graph &altGraph, Graph &g2, sf::RenderWindow &window){
	float f1, f2;
	char buff[16];
	while(1){
		printf("Enter points:\n");
		//fgets(buff, sizeof(buff), stdin);
		scanf("%f", &f1);
		scanf("%f", &f2);
		altGraph.add_point(f1);
		g2.add_point(f2);
		//altGraph.add_point(rand() % 200);
		//g2.add_point(-400 + rand() % 800);
		//altGraph.generate_points();
		//g2.generate_points();
		//altGraph.draw(window);
		sf::sleep(sf::milliseconds(200));
	}
}

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
	printf("Enter all commands to be sent to the XBee here\n");
	printf("Do not include the command prefix (%s)\n\n", CmdPrefix);
	for(;;){}
	while(1){
		fgets(buff, sizeof(buff), stdin);
		buff[strnlen(buff, sizeof(buff)) - 1] = '\0';
		send_cmd(deviceFile, buff, sizeof(buff));
	}
}

bool mouse_in_range(const sf::Vector2i &mousePos, const sf::Vector2f &objPos, const sf::Vector2f &objSize){
	if(mousePos.x < objPos.x || mousePos.y < objPos.y){
		return 0;
	}
	if(mousePos.x > objPos.x + objSize.x || mousePos.y > objPos.y + objSize.y){
		return 0;
	}
	return 1;
}

bool mouse_clicked(const sf::Mouse &mouse, const Button btn, sf::Window &win){
	sf::Vector2i mousePos = mouse.getPosition(win);
	if(mousePos.x < btn.bg.getPosition().x || mousePos.y < btn.bg.getPosition().y){
		return 0;
	}
	if(mousePos.x > btn.bg.getPosition().x + btn.bg.getSize().x || mousePos.y > btn.bg.getPosition().y + btn.bg.getSize().y){
		return 0;
	}
	return 1;
}

