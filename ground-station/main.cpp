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

void draw_graphs(sf::RenderWindow &window, Graph &altGraph, Graph &g2);
void packet_handler(int port, Packet &packet);
void update_graphs(Graph &altGraph, Graph &g2, Packet &packet);

int main(int argc, char* argv[]){
	srand(time(0));
	//unsigned char buf[4096];
	int port = 16; set_port(port);
	FILE* logFile = fopen(LogFileName, "a");
	
	std::thread sendThread(get_cmd, port);
	sendThread.detach();
	
    sf::RenderWindow window(sf::VideoMode(WinWidth, WinHeight), WinTitle);
	
	sf::Font font;
	font.loadFromFile(FontFile);
	
	Button simEnableBtn(DefaultButtonSpacing, WinHeight - DefaultButtonSpacing - DefaultButtonHeight, DefaultButtonWidth, DefaultButtonHeight, font, "SIM  Enable");
	Button simActivateBtn(DefaultButtonSpacing*2+DefaultButtonWidth, WinHeight - DefaultButtonSpacing - DefaultButtonHeight, DefaultButtonWidth, DefaultButtonHeight, font, "SIM  Activate");
	simEnableBtn.set_colors(DefaultButtonBgColor, DefaultButtonTextColor);
	simActivateBtn.set_colors(DefaultButtonBgColor, DefaultButtonTextColor);
	
	Graph altGraph(15, 5, 5, 175, 175, font, "Altitude");
	Graph g2(15, 305, 5, 175, 175, font, "  Speed ");
	
	Packet packet;
	
	int x = 775;
	Label packetLabel(x , 5, WinWidth - x - 5, 15, font, std::string(95, '*'));

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
						std::thread t(send_cmd, buff, sizeof(buff), port);
						t.detach();
					}else if(mouse_clicked(mouse, simActivateBtn, window)){
						char buff[] = "SIM,ACTIVATE";
						std::thread t(send_cmd, buff, sizeof(buff), port);
						t.detach();
					}
					break;
			}
        }
		
		if(packet.changed){
			update_graphs(altGraph, g2, packet);
			packetLabel.set_text(packet.packetString);
			packet.changed = 0;
		}
		
        window.clear(WinBgColor);
		simEnableBtn.draw(window);
		simActivateBtn.draw(window);
		altGraph.draw(window);
		g2.draw(window);
		packetLabel.draw(window);
        window.display();
    }
	
    return 0;
}

void draw_graphs(sf::RenderWindow &window, Graph &altGraph, Graph &g2){
	altGraph.draw(window);
	g2.draw(window);
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
				printf("S: %li | %s\n", packet.packetString.size(), packet.packetString.c_str());
				packet.parse_packet(packet.packetString.c_str(), packet.packetString.size());
				sf::sleep(sf::milliseconds(502));
				packet.packetString = "";
			}
		}
		for(int i = sizeof(buff); i--;){ buff[i] = 0; }
	}
}

void update_graphs(Graph &altGraph, Graph &g2, Packet &packet){
	float f1, f2;
	char buff[16];
	//while(1){
		altGraph.add_point(packet.altitude);
		//altGraph.add_point(rand() % 200);
		g2.add_point(-400 + rand() % 800);
		//altGraph.generate_points();
		//g2.generate_points();
		//altGraph.draw(window);
		//sf::sleep(sf::milliseconds(200));
	//}
}

