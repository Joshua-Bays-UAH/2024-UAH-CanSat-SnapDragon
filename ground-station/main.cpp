#include <SFML/Graphics.hpp>
#include <thread>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "defs.hpp" /* Global parameters */
#include "button.hpp" /* Button class */
#include "graph.hpp" /* Graph class */
#include "cmd-funcs.cpp" /* Functions for sending/receiving commands */
#include "win-funcs.cpp" /* Functions for window interaction */

void update_graphs(Graph &altGraph, Graph &g2);

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
	simEnableBtn.set_colors(sf::Color(0x13, 0x5f, 0xd9), sf::Color(0xff - 0x13, 0xff - 0x5f, 0xff - 0xd9));
	simActivateBtn.set_colors(sf::Color(0x13, 0x5f, 0xd9), sf::Color(0xff - 0x13, 0xff - 0x5f, 0xff - 0xd9));
	
	Graph altGraph(20);
	Graph g2(20);
	g2.points[0].position.x = 400;
	g2.points[1].position.x = 400;
	
	std::thread updateThread(update_graphs, std::ref(altGraph), std::ref(g2));
	
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
		
        window.clear(sf::Color(0x4b, 0x22, 0xbf));
        window.draw(altGraph.points);
        window.draw(g2.points);
		simEnableBtn.draw(window);
		simActivateBtn.draw(window);
        window.display();
    }
	updateThread.detach();
	
    return 0;
}

void update_graphs(Graph &altGraph, Graph &g2){
	while(1){
		altGraph.add_point(-1);
		g2.add_point(-1);
		sf::sleep(sf::milliseconds(100));
	}
}

