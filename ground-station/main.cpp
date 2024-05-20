#include <SFML/Graphics.hpp>

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
#include "packet.hpp" /* Packet class */
#include "cmd-funcs.cpp" /* Functions for sending/receiving commands */
#include "win-funcs.cpp" /* Functions for window interaction */

void draw_graphs(sf::RenderWindow &window, Graph &altGraph, Graph &g2);
void update_graphs(Graph &altGraph, Graph &g2, sf::RenderWindow &window);

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

