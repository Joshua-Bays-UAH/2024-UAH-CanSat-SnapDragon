#include <SFML/Graphics.hpp>
#include "rs232.h"

#include <deque>
#include <thread>
#include <limits>

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

bool simMode = 0;


#define WinWidth 1300
#define WinHeight 700
#define WinTitle "Snapdragon Ground Station"
#define WinBgColor sf::Color(0x4b, 0x22, 0xbf)

//#define FontFile "fonts/static/Cinzel-Regular.ttf"
//#define FontFile "fonts/Sacramento-Regular.ttf"
//#define FontFile "fonts/Freeman/Freeman-Regular.ttf"
//#define FontFile "fonts/EncodeSansExpanded-Thin.ttf"
#define FontFile "fonts/EncodeSansExpanded-Medium.ttf"
//#define FontFile "fonts/Jersey_10/Jersey10-Regular.ttf"
//#define FontFile "fonts/PlayfairDisplay-VariableFont_wght.ttf"
//#define FontFile "fonts/VarelaRound-Regular.ttf"

#define LineWrapCount 85

#define DefaultButtonWidth 100
#define DefaultButtonHeight 60
#define DefaultButtonBgColor sf::Color(0x13, 0x5f, 0xd9)
//#define DefaultButtonTextColor sf::Color(0xff - 0x13, 0xff - 0x5f, 0xff - 0xd9)
#define DefaultButtonTextColor sf::Color(0xff, 0xff, 0xff)

#define DefaultGraphWidth 110
#define DefaultGraphHeight 110
#define DefaultGraphPoints 20

#define ValueLabelWidth 200
#define ValueLabelHeight 60

#define DefaultMargin 5
#define DefaultSpacing 10
#define CmdTermChar 0x7e

#define CmdPrefix "CMD,2079,"

#define SimpFileName "cansat_2024_simp.txt"

#define LogFileName "Flight_2079.csv"
#define LogFileHeader "TEAM_ID,MISSION_TIME,PACKET_COUNT,MODE,STATE,ALTITUDE,AIR_SPEED,HS_DEPLOYED,PC_DEPLOYED,TEMPERATURE,VOLTAGE,PRESSURE,GPS_TIME,GPS_ALTITUDE,GPS_LATITUDE,GPS_LONGITUDE,GPS_SATS,TILT_X,TILT_Y,ROT_Z,CMD_ECHO,,VELOCITY"

class Button{
	public:
		sf::RectangleShape bg;
		sf::Text text;
		
		Button(float x, float y, float w, float h, sf::Font &font, std::string str);
		void set_position(float x, float y);
		void set_size(float w, float h);
		void set_text(std::string str);
		void set_colors(sf::Color bgColor, sf::Color textColor);
		void draw(sf::RenderWindow &window);
	private:
};

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

class Label{
	public:
		sf::RectangleShape area;
		sf::Text text;
		
		Label(float x, float y, float w, float h, sf::Font &font, std::string str);
		void set_text(std::string str);
		void set_color(sf::Color color);
		void draw(sf::RenderWindow &window);
};

class Packet{
	public:
		bool changed = 0;
		unsigned total = 0;
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
		float temperature;
		float voltage;
		float pressure;
		char gpsTime[16];
		float gpsAlt;
		float gpsLat;
		float gpsLong;
		int gpsSats;
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

void set_port(int &port);
void send_cmd(char *str, int strSize, int port);
void get_cmd_input(int port);
void sim_mode(int port);

bool mouse_in_range(const sf::Vector2i &mousePos, const sf::Vector2f &objPos, const sf::Vector2f &objSize);
bool mouse_clicked(const sf::Mouse &mouse, const Button btn, sf::Window &win);

void draw_graphs(sf::RenderWindow &window, Graph &altitudeGraph, Graph &airSpeedGraph);
void packet_handler(int port, Packet &packet);
void update_graphs(Graph &altitudeGraph, Graph &airSpeedGraph, Graph &temperatureGraph, Graph &pressureGraph, Graph &voltageGraph, Packet &packet);

int main(int argc, char* argv[]){
	srand(time(0));
	//unsigned char buf[4096];
	int port = 16; set_port(port);
	FILE* logFile = fopen(LogFileName, "a");
	fprintf(logFile, LogFileHeader);
	
	std::thread sendThread(get_cmd_input, port);
	sendThread.detach();
	
    sf::RenderWindow window(sf::VideoMode(WinWidth, WinHeight), WinTitle);
	
	sf::Font font;
	font.loadFromFile(FontFile);
	
	Button simEnableBtn(DefaultMargin, WinHeight - DefaultMargin - DefaultButtonHeight, DefaultButtonWidth, DefaultButtonHeight, font, "SIM  Enable");
	Button simActivateBtn(DefaultMargin+DefaultButtonWidth+DefaultSpacing, WinHeight - DefaultMargin - DefaultButtonHeight, DefaultButtonWidth, DefaultButtonHeight, font, "SIM  Activate");
	Button simStartBtn(DefaultMargin+2*DefaultButtonWidth+2*DefaultSpacing, WinHeight - DefaultMargin - DefaultButtonHeight, DefaultButtonWidth, DefaultButtonHeight, font, "SIM  Start");
	Button simDisableBtn(DefaultMargin+3*DefaultButtonWidth+3*DefaultSpacing, WinHeight - DefaultMargin - DefaultButtonHeight, DefaultButtonWidth, DefaultButtonHeight, font, "SIM  Disable");
	Button bcnOnBtn(DefaultMargin+4*DefaultButtonWidth+4*DefaultSpacing, WinHeight - DefaultMargin - DefaultButtonHeight, DefaultButtonWidth, DefaultButtonHeight, font, "Beacon On");
	Button bcnOffBtn(DefaultMargin+5*DefaultButtonWidth+5*DefaultSpacing, WinHeight - DefaultMargin - DefaultButtonHeight, DefaultButtonWidth, DefaultButtonHeight, font, "Beacon Off");
	Button cxOnBtn(DefaultMargin+6*DefaultButtonWidth+6*DefaultSpacing, WinHeight - DefaultMargin - DefaultButtonHeight, DefaultButtonWidth, DefaultButtonHeight, font, "Telemetry On");
	Button cxOffBtn(DefaultMargin+7*DefaultButtonWidth+7*DefaultSpacing, WinHeight - DefaultMargin - DefaultButtonHeight, DefaultButtonWidth, DefaultButtonHeight, font, "Telemetry Off");
	Button calBtn(DefaultMargin, WinHeight-2*DefaultMargin-2*DefaultButtonHeight, DefaultButtonWidth, DefaultButtonHeight, font, "CAL");
	
	std::deque<Button *> buttonDeque = {&simEnableBtn, &simActivateBtn, &simStartBtn, &simDisableBtn, &bcnOnBtn, &bcnOffBtn, &cxOnBtn, &cxOffBtn, &calBtn};
	for(Button *i : buttonDeque){ i->set_colors(DefaultButtonBgColor, DefaultButtonTextColor); }
	
	Graph altitudeGraph(DefaultGraphPoints, DefaultMargin, DefaultMargin, DefaultGraphWidth, DefaultGraphHeight, font, "Altitude");
	Graph airSpeedGraph(DefaultGraphPoints, DefaultMargin+DefaultGraphWidth+4.6*DefaultSpacing, DefaultMargin, DefaultGraphWidth, DefaultGraphHeight, font, "Air Speed");
	Graph temperatureGraph(DefaultGraphPoints, DefaultMargin, DefaultMargin+DefaultSpacing*3+DefaultGraphHeight, DefaultGraphWidth, DefaultGraphHeight, font, "Temperature");
	Graph pressureGraph(DefaultGraphPoints, DefaultMargin+DefaultGraphWidth+4.6*DefaultSpacing, DefaultMargin+DefaultSpacing*3+DefaultGraphHeight, DefaultGraphWidth, DefaultGraphHeight, font, "Pressure");
	Graph voltageGraph(DefaultGraphPoints, DefaultMargin, DefaultMargin+DefaultSpacing*6+DefaultGraphHeight*2, DefaultGraphWidth, DefaultGraphHeight, font, "Voltage");
	Graph gpsAltitudeGraph(DefaultGraphPoints, DefaultMargin+DefaultGraphWidth+4.6*DefaultSpacing, DefaultMargin+DefaultSpacing*6+DefaultGraphHeight*2, DefaultGraphWidth, DefaultGraphHeight, font, "GPS Altitude");
	Graph tiltXGraph(DefaultGraphPoints, DefaultMargin+DefaultGraphWidth*2+4.6*DefaultSpacing*2, DefaultMargin+DefaultSpacing*6+DefaultGraphHeight*2, DefaultGraphWidth, DefaultGraphHeight, font, "Tilt X");
	Graph tiltYGraph(DefaultGraphPoints, DefaultMargin+DefaultGraphWidth*3+4.6*DefaultSpacing*3, DefaultMargin+DefaultSpacing*6+DefaultGraphHeight*2, DefaultGraphWidth, DefaultGraphHeight, font, "Tilt Y");
	Graph rotZGraph(DefaultGraphPoints, DefaultMargin+DefaultGraphWidth*4+4.6*DefaultSpacing*4, DefaultMargin+DefaultSpacing*6+DefaultGraphHeight*2, DefaultGraphWidth, DefaultGraphHeight, font, "Rotation Z");
	/* GPS Sats */
	
	std::deque<Graph *> graphDeque = {&altitudeGraph, &airSpeedGraph, &temperatureGraph, &pressureGraph, &voltageGraph, &gpsAltitudeGraph, &tiltXGraph, &tiltYGraph, &rotZGraph};

	int packetLabelX = airSpeedGraph.position.x + airSpeedGraph.maxSize.x + 5.5*DefaultSpacing;
	Label packetLabel(packetLabelX, 5, WinWidth - packetLabelX - 5, 15, font, std::string(LineWrapCount + 15, '*'));
	Label packetCountLabel(WinWidth - ValueLabelWidth - DefaultMargin, WinHeight - ValueLabelHeight, ValueLabelWidth, ValueLabelHeight, font, "Packets: XXX");
	Label gpsSatsLabel(WinWidth - ValueLabelWidth - DefaultSpacing, WinHeight - ValueLabelHeight*2 - DefaultSpacing - DefaultMargin, ValueLabelWidth, ValueLabelHeight, font, "GPS Satellites: XX");
	Label StateLabel(WinWidth - ValueLabelWidth - DefaultSpacing, WinHeight - ValueLabelHeight*3 - DefaultSpacing*2 - DefaultMargin, ValueLabelWidth, ValueLabelHeight, font, "State: "+std::string(12, 'X'));
	Label modeLabel(WinWidth - ValueLabelWidth - DefaultSpacing, WinHeight - ValueLabelHeight*4 - DefaultSpacing*3 - DefaultMargin, ValueLabelWidth, ValueLabelHeight, font, "Mode: X");
	
	std::deque<Label *> labelDeque = {&packetLabel, &packetCountLabel, &gpsSatsLabel, &StateLabel, &modeLabel};
	
	Packet packet;
	std::thread	readThread(packet_handler, port, std::ref(packet));
	readThread.detach();
	
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
						std::thread(send_cmd, buff, sizeof(buff), port).detach();
					}else if(mouse_clicked(mouse, simActivateBtn, window)){
						char buff[] = "SIM,ACTIVATE";
						std::thread(send_cmd, buff, sizeof(buff), port).detach();
						simMode = 1;
					}else if(mouse_clicked(mouse, simStartBtn, window)){
						std::thread(sim_mode, port).detach();
					}else if(mouse_clicked(mouse, simDisableBtn, window)){
						char buff[] = "SIM,DISABLE";
						std::thread(send_cmd, buff, sizeof(buff), port).detach();
						simMode = 0;
					}else if(mouse_clicked(mouse, bcnOnBtn, window)){
						char buff[] = "BCN,ON";
						std::thread(send_cmd, buff, sizeof(buff), port).detach();
					}else if(mouse_clicked(mouse, bcnOffBtn, window)){
						char buff[] = "BCN,OFF";
						std::thread(send_cmd, buff, sizeof(buff), port).detach();
					}else if(mouse_clicked(mouse, cxOnBtn, window)){
						char buff[] = "CX,ON";
						std::thread(send_cmd, buff, sizeof(buff), port).detach();
					}else if(mouse_clicked(mouse, cxOffBtn, window)){
						char buff[] = "CX,OFF";
						std::thread(send_cmd, buff, sizeof(buff), port).detach();
					}else if(mouse_clicked(mouse, calBtn, window)){
						char buff[] = "CAL";
						std::thread(send_cmd, buff, sizeof(buff), port).detach();
					}
					break;
			}
        }
		
		if(packet.changed){
			packetLabel.set_text(packet.packetString);
			packetCountLabel.set_text(std::string("Packets: ")+std::to_string(packet.total));
			gpsSatsLabel.set_text(std::string("GPS Satellites: ")+std::to_string(packet.gpsSats));
			StateLabel.set_text(std::string("State: ")+packet.state);
			modeLabel.set_text(std::string("Mode: ")+packet.mode);
			update_graphs(altitudeGraph, airSpeedGraph, temperatureGraph, pressureGraph, voltageGraph, packet);
			fprintf(logFile, "%s\n", packet.packetString.c_str());
			packet.changed = 0;
		}
		
        window.clear(WinBgColor);
		for(Button *i : buttonDeque){ i->draw(window); }
		for(Graph *i : graphDeque){ i->draw(window); }
		for(Label *i : labelDeque){ i->draw(window); }
        window.display();
    }
	
	fclose(logFile);
	
    return 0;
}

void draw_graphs(sf::RenderWindow &window, Graph &altitudeGraph, Graph &airSpeedGraph){
	altitudeGraph.draw(window);
	airSpeedGraph.draw(window);
}

void packet_handler(int port, Packet &packet){
	unsigned char buff[256];
	int bytesRead = 0;
	while(1){
		bytesRead = RS232_PollComport(port, buff, sizeof(buff));
		if(bytesRead > 0){
			buff[bytesRead] = 0;
			//printf("B: %i\n", bytesRead);
			//printf("P: %s_\n", buff);
			for(int i = 0; i < bytesRead; i++){
				packet.packetString += buff[i];
			}
			if(buff[bytesRead - 1] == '\n'){
				packet.packetString = packet.packetString.substr(0, packet.packetString.size() - 2);
				//printf("S: %li | %s\n", packet.packetString.size(), packet.packetString.c_str());
				packet.parse_packet(packet.packetString.c_str(), packet.packetString.size());
				sf::sleep(sf::milliseconds(502));
				packet.packetString = "";
			}
		}
		for(int i = sizeof(buff); i--;){ buff[i] = 0; }
	}
}

void update_graphs(Graph &altitudeGraph, Graph &airSpeedGraph, Graph &temperatureGraph, Graph &pressureGraph, Graph &voltageGraph, Packet &packet){
	altitudeGraph.add_point(packet.altitude);
	airSpeedGraph.add_point(packet.airSpeed);
	temperatureGraph.add_point(packet.temperature);
	pressureGraph.add_point(packet.pressure);
	voltageGraph.add_point(packet.pressure);
}

Button::Button(float x, float y, float w, float h, sf::Font &font, std::string str){
	bg.setPosition(x, y);
	bg.setSize(sf::Vector2f(w, h));
	text.setString(str);
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

void Label::set_text(std::string str){
	int f = 1;
	//text.setCharacterSize(5);
	if(str.size() > LineWrapCount*2){
		f = 2;
		std::string s = str.substr(0, LineWrapCount)+"\n"+str.substr(LineWrapCount,LineWrapCount)+"\n"+str.substr(LineWrapCount*2);
		//std::string s = str;
		text.setString(s);
	}if(str.size() > LineWrapCount){
		f = 2;
		std::string s = str.substr(0, LineWrapCount)+"\n"+str.substr(LineWrapCount);
		//std::string s = str;
		text.setString(s);
	}else{
		text.setString(str);
	}
	//text.setCharacterSize(500);
	//text.setScale(area.getSize().x/text.getLocalBounds().width, area.getSize().y*f/text.getLocalBounds().height);
	//text.setPosition(area.getPosition().x, area.getPosition().y);
	//text.move(text.getPosition().x - text.getGlobalBounds().left, text.getPosition().y - text.getGlobalBounds().top);
}

void Label::draw(sf::RenderWindow &window){
	window.draw(text);
}

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
	sscanf(buff, "%f", &temperature);
	
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
	sscanf(buff, "%i", &gpsSats);
	
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
	total++;
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
	printf("Temperature: %f\n", temperature);
	printf("Voltage: %f\n", voltage);
	printf("Pressure: %f\n", pressure);
	printf("GPS Time: %s\n", gpsTime);
	printf("GPS Altitude: %f\n", gpsAlt);
	printf("GPS Latitude: %f\n", gpsLat);
	printf("GPS longitude: %f\n", gpsLong);
	printf("GPS Satellites %i\n", gpsSats);
	printf("Tilt X: %f\n", tiltX);
	printf("Tilt Y: %f\n", tiltY);
	printf("Rotation Z: %f\n", rotZ);
	printf("CMD_ECHO: %s\n", cmdEcho);
}

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
	
	//sprintf(buff, "SIM,ENABLE");
	send_cmd(buff, sizeof(buff), port);
	sf::sleep(sf::milliseconds(1500));
	
	//sprintf(buff, "SIM,ACTIVATE");
	send_cmd(buff, sizeof(buff), port);
	sf::sleep(sf::milliseconds(1500));
	simMode = 1;
	
	sprintf(buff, "PCKT,0");
	send_cmd(buff, sizeof(buff), port);
	sf::sleep(sf::milliseconds(2000));
	
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

