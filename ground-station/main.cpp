#include <SFML/Graphics.hpp>
#include <stdio.h>

#include "defs.hpp"

bool mouse_in_range(const sf::Vector2i &mousePos, const sf::Vector2f &objPos, const sf::Vector2f &objSize);

int main(){
    sf::RenderWindow window(sf::VideoMode(WinWidth, WinHeight), WinTitle);
	
    sf::RectangleShape btn;
	btn.setSize(sf::Vector2f(100, 100));
	btn.setPosition(200, 200);
    btn.setFillColor(sf::Color::Green);

    while(window.isOpen()){
        sf::Event event;
		sf::Mouse mouse;
        while (window.pollEvent(event)){
			switch(event.type){
				case sf::Event::Closed:
					window.close();
					break;
				case sf::Event::MouseButtonPressed:
					if(mouse_in_range(mouse.getPosition(window), btn.getPosition(), btn.getSize())){
					}
					break;
			}
        }
		
        window.clear(sf::Color(0x4b, 0x22, 0xbf));
        window.draw(btn);
        window.display();
    }

    return 0;
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
