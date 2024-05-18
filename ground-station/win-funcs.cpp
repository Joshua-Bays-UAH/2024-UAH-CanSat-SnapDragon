bool mouse_in_range(const sf::Vector2i &mousePos, const sf::Vector2f &objPos, const sf::Vector2f &objSize);
bool mouse_clicked(const sf::Mouse &mouse, const Button btn, sf::Window &win);


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

