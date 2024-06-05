#include <SFML/Graphics.hpp>
#include "RS-232/rs232.h"

#include <deque>
#include <thread>
#include <limits>

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "defs.hpp" /* Global parameters */
#include "button.hpp" /* Button class */
#include "graph.hpp" /* Graph class */
#include "label.hpp" /* Graph class */
#include "packet.hpp" /* Packet class */
#include "cmd-funcs.cpp" /* Functions for sending/receiving commands */
#include "win-funcs.cpp" /* Functions for window interaction */

void draw_graphs(sf::RenderWindow &window, Graph &altitudeGraph, Graph &airSpeedGraph);
void packet_handler(int port, Packet &packet);
void update_graphs(Graph &altitudeGraph, Graph &airSpeedGraph, Graph &temperatureGraph, Graph &pressureGraph, Graph &voltageGraph, Graph &gpsAltitudeGraph, Graph &tiltXGraph, Graph &tiltYGraph, Graph &rotZGraph, Packet &packet);

int main(int argc, char* argv[]){
	srand(time(0));
	int port = 16;
	if(argc > 1){ port = std::stoi(argv[1]); }
	set_port(port);
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
	
	Button calBtn(DefaultMargin, WinHeight-2*DefaultMargin-2*DefaultButtonHeight, DefaultButtonWidth, DefaultButtonHeight, font, "CAL");
	Button cxOnBtn(DefaultMargin+DefaultButtonWidth+DefaultSpacing, WinHeight-2*DefaultMargin-2*DefaultButtonHeight, DefaultButtonWidth, DefaultButtonHeight, font, "Telemetry On");
	Button cxOffBtn(DefaultMargin+2*DefaultButtonWidth+2*DefaultSpacing, WinHeight-2*DefaultMargin-2*DefaultButtonHeight, DefaultButtonWidth, DefaultButtonHeight, font, "Telemetry Off");
	Button stUTCBtn(DefaultMargin+3*DefaultButtonWidth+3*DefaultSpacing, WinHeight-2*DefaultMargin-2*DefaultButtonHeight, DefaultButtonWidth, DefaultButtonHeight, font, "ST UTC");
	Button stGPSBtn(DefaultMargin+4*DefaultButtonWidth+4*DefaultSpacing, WinHeight-2*DefaultMargin-2*DefaultButtonHeight, DefaultButtonWidth, DefaultButtonHeight, font, "ST GPS");
	
	std::deque<Button *> buttonDeque = {&simEnableBtn, &simActivateBtn, &simStartBtn, &simDisableBtn, &bcnOnBtn, &bcnOffBtn, &cxOnBtn, &cxOffBtn, &calBtn, &stUTCBtn, &stGPSBtn};
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
	
	std::deque<Graph *> graphDeque = {&altitudeGraph, &airSpeedGraph, &temperatureGraph, &pressureGraph, &voltageGraph, &gpsAltitudeGraph, &tiltXGraph, &tiltYGraph, &rotZGraph};

	int packetLabelX = airSpeedGraph.position.x + airSpeedGraph.maxSize.x + 5.5*DefaultSpacing;
	Label packetLabel(packetLabelX, 5, WinWidth - packetLabelX - 5, 15, font, std::string(LineWrapCount + 15, '*'));
	Label packetCountLabel(WinWidth - ValueLabelWidth - DefaultMargin, WinHeight - ValueLabelHeight, ValueLabelWidth, ValueLabelHeight, font, "Packets: XXX");
	Label StateLabel(WinWidth - ValueLabelWidth - DefaultSpacing, WinHeight - ValueLabelHeight*2 - DefaultSpacing - DefaultMargin, ValueLabelWidth, ValueLabelHeight, font, "State: "+std::string(12, 'X'));
	Label modeLabel(WinWidth - ValueLabelWidth - DefaultSpacing, WinHeight - ValueLabelHeight*3 - DefaultSpacing*2 - DefaultMargin, ValueLabelWidth, ValueLabelHeight, font, "Mode: X");
	
	Label gpsSatsLabel(WinWidth - ValueLabelWidth*2 - DefaultSpacing*3, WinHeight - ValueLabelHeight, ValueLabelWidth, ValueLabelHeight, font, "GPS Satellites: XX");
	Label gpsLattLabel(WinWidth - ValueLabelWidth*2 - DefaultSpacing*3, WinHeight - ValueLabelHeight*2 - DefaultSpacing - DefaultMargin, ValueLabelWidth, ValueLabelHeight, font, "GPS Latt: XXX.XXXX");
	Label gpsLongLabel(WinWidth - ValueLabelWidth*2 - DefaultSpacing*3, WinHeight - ValueLabelHeight*3 - DefaultSpacing*2 - DefaultMargin, ValueLabelWidth, ValueLabelHeight, font, "GPS Long: XXX.XXXX");
	
	std::deque<Label *> labelDeque = {&packetLabel, &packetCountLabel, &gpsSatsLabel, &StateLabel, &modeLabel, &gpsLattLabel, &gpsLongLabel};
	
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
					}else if(mouse_clicked(mouse, stUTCBtn, window)){
						char buff[12] = "ST,";
						time_t n = time(0);
						struct tm *tim = localtime(&n);
						unsigned hms[3];
						hms[0] = tim->tm_hour;
						hms[1] = tim->tm_min;
						hms[2] = tim->tm_sec;
						if(hms[0] < 10){ sprintf(buff+strlen(buff), "0%u", hms[0]); }
						else{ sprintf(buff+strlen(buff), "%u:", hms[0]); }
						if(hms[1] < 10){ sprintf(buff + strlen(buff), "0%u", hms[1]); }
						else{ sprintf(buff+strlen(buff), "%u", hms[1]); }
						if(hms[2] < 10){ sprintf(buff + strlen(buff), ":0%u", hms[2]); }
						else{ sprintf(buff+strlen(buff), ":%u", hms[2]); }
						std::thread(send_cmd, buff, sizeof(buff), port).detach();
					}else if(mouse_clicked(mouse, stGPSBtn, window)){
						char buff[] = "ST,GPS";
						std::thread(send_cmd, buff, sizeof(buff), port).detach();
					}
					break;
			}
        }
		
		if(packet.changed){
			packetLabel.set_text(packet.packetString);
			packetCountLabel.set_text(std::string("Packets: ")+std::to_string(packet.total));
			gpsSatsLabel.set_text(std::string("GPS Satellites: ")+std::to_string(packet.gpsSats));
			gpsLattLabel.set_text(std::string("GPS Long: ")+std::to_string(packet.gpsLat));
			gpsLongLabel.set_text(std::string("GPS Latt: ")+std::to_string(packet.gpsLong));
			StateLabel.set_text(std::string("State: ")+packet.state);
			modeLabel.set_text(std::string("Mode: ")+packet.mode);
			update_graphs(altitudeGraph, airSpeedGraph, temperatureGraph, pressureGraph, voltageGraph, gpsAltitudeGraph, tiltXGraph, tiltYGraph, rotZGraph, packet);
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

void update_graphs(Graph &altitudeGraph, Graph &airSpeedGraph, Graph &temperatureGraph, Graph &pressureGraph, Graph &voltageGraph, Graph &gpsAltitudeGraph, Graph &tiltXGraph, Graph &tiltYGraph, Graph &rotZGraph, Packet &packet){
	altitudeGraph.add_point(packet.altitude);
	airSpeedGraph.add_point(packet.airSpeed);
	temperatureGraph.add_point(packet.temperature);
	pressureGraph.add_point(packet.pressure);
	voltageGraph.add_point(packet.voltage);
	gpsAltitudeGraph.add_point(packet.gpsAlt);
	tiltXGraph.add_point(packet.tiltX);
	tiltYGraph.add_point(packet.tiltY);
	rotZGraph.add_point(packet.rotZ);


	
}

