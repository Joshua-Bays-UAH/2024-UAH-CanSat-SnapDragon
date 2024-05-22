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
void read_packet(int port, std::string &packet);
void update_graphs(Graph &altGraph, Graph &g2, sf::RenderWindow &window);

int main(){
	srand(time(0));
	unsigned char buf[4096];
	int port = 16; set_port(port);

	std::string packet;
	std::thread	readThread(read_packet, port, std::ref(packet));
	readThread.detach();
	
	std::thread sendThread(get_cmd, port);
	sendThread.detach();
	
    sf::RenderWindow window(sf::VideoMode(WinWidth, WinHeight), WinTitle);
	
	sf::Font font;
	font.loadFromFile(FontFile);
	
	Button simEnableBtn(DefaultButtonSpacing, WinHeight - DefaultButtonSpacing - DefaultButtonHeight, DefaultButtonWidth, DefaultButtonHeight, font, "SIM  Enable");
	Button simActivateBtn(DefaultButtonSpacing*2+DefaultButtonWidth, WinHeight - DefaultButtonSpacing - DefaultButtonHeight, DefaultButtonWidth, DefaultButtonHeight, font, "SIM  Activate");
	simEnableBtn.set_colors(DefaultButtonBgColor, DefaultButtonTextColor);
	simActivateBtn.set_colors(DefaultButtonBgColor, DefaultButtonTextColor);
	
	Graph altGraph(15, 5, 5, 200, 200, font, "Altitude");
	Graph g2(15, 450, 5, 200, 200, font, "Speed");
	
	std::thread updateThread(update_graphs, std::ref(altGraph), std::ref(g2), std::ref(window));
	//std::thread drawGraphThread(draw_graphs, std::ref(window), std::ref(altGraph), std::ref(g2));
	
	int x = 5;
	Label l(x , 500, WinWidth - 10, 40, font, "2079,01:16:03,5,SIMULATION,LAUNCH_WAIT,-0.6,0.000000,N,N,25.0,99.6,0.0,01:16:03,0.000000,0.000000,0.000000,0,0.000000,0.000000,0.000000,SIM,ACTIVATE,,-0.069962");
	l.set_text("I like waffles I like waffles I like waffles I like waffles I like waffles I like waffles I like waffles I like waffles I like waffles I like waffles!");
	l.set_text("I like waffles");
	
    while(window.isOpen()){
		l.set_text(packet);
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
		
        window.clear(WinBgColor);
		simEnableBtn.draw(window);
		simActivateBtn.draw(window);
		altGraph.draw(window);
		g2.draw(window);
		l.draw(window);
        window.display();
    }
	updateThread.detach();
	
    return 0;
}

void draw_graphs(sf::RenderWindow &window, Graph &altGraph, Graph &g2){
	altGraph.draw(window);
	g2.draw(window);
}

void read_packet(int port, std::string &packet){
	unsigned char buff[256];
	int bytesRead = 0;
	while(1){
		bytesRead = RS232_PollComport(port, buff, sizeof(buff));
		if(bytesRead > 0){
			printf("C: %s\n", buff);
			std::string str (reinterpret_cast<const char *> (buff), sizeof (buff) / sizeof (buff[0]));
			packet = str;
		}
	}
}

void update_graphs(Graph &altGraph, Graph &g2, sf::RenderWindow &window){
	float f1, f2;
	char buff[16];
	while(1){
		//altGraph.add_point(rand() % 200);
		//g2.add_point(-400 + rand() % 800);
		//altGraph.generate_points();
		//g2.generate_points();
		//altGraph.draw(window);
		sf::sleep(sf::milliseconds(200));
	}
}

