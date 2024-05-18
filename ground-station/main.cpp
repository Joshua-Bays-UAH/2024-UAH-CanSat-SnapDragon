#include <SFML/Graphics.hpp>
#include <thread>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "button.hpp"
#include "graph.hpp"
#include "defs.hpp"

bool connect_device(FILE* &deviceFile);
bool mouse_in_range(const sf::Vector2i &mousePos, const sf::Vector2f &objPos, const sf::Vector2f &objSize);
void update_graphs(Graph &altGraph, Graph &g2);

int main(){
	srand(time(0));
	FILE* usbDevice;
	if(connect_device(usbDevice)){
		printf("Could not find USB device, exiting\n");
		//return 1;
	}
	
    sf::RenderWindow window(sf::VideoMode(WinWidth, WinHeight), WinTitle);
	
	sf::Font font;
	font.loadFromFile(FontFile);
	
	Button b(DefaultButtonSpacing, WinHeight - DefaultButtonSpacing - DefaultButtonHeight, DefaultButtonWidth, DefaultButtonHeight, font, "SIM Enable", 30);
	
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
					if(mouse_in_range(mouse.getPosition(window), b.bg.getPosition(), b.bg.getSize())){
						printf("Simulation mode enabled\n");
					}
					break;
			}
        }
		
        window.clear(sf::Color(0x4b, 0x22, 0xbf));
        window.draw(altGraph.points);
        window.draw(g2.points);
		b.draw(window);
        window.display();
    }
	updateThread.detach();
	
    return 0;
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
	return deviceFile == NULL;
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

void update_graphs(Graph &altGraph, Graph &g2){
	while(1){
		altGraph.add_point(-1);
		g2.add_point(-1);
		sf::sleep(sf::milliseconds(100));
	}
}
