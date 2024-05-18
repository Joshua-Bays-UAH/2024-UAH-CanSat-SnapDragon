class Button{
	public:
		sf::RectangleShape bg;
		sf::Text text;
		
		Button(float x, float y, float w, float h, sf::Font &font, std::string t, int fontSize);
		void set_position(float x, float y);
		void set_size(float w, float h);
		void set_text(std::string str);
		void draw(sf::RenderWindow &window);
	private:
};

Button::Button(float x, float y, float w, float h, sf::Font &font, std::string t, int fontSize){
	bg.setPosition(x, y);
	bg.setSize(sf::Vector2f(w, h));
	text.setString(t);
	text.setFont(font);
	text.setCharacterSize(500);
	text.setFillColor(sf::Color(255, 255, 0));
	//text.setScale(w/(text.findCharacterPos(text.getString().getSize() - 1).x - text.findCharacterPos(0).x), 1);
	//text.setScale(w/(text.findCharacterPos(1).x - text.findCharacterPos(0).x), 1);
	text.setScale(.9 * bg.getSize().x/text.getGlobalBounds().width, .9 * bg.getSize().y/text.getGlobalBounds().height);
	text.setPosition(bg.getPosition().x + bg.getSize().x/2 - text.getGlobalBounds().width/2, bg.getPosition().y + bg.getSize().y/2 - text.getGlobalBounds().height/2);
	text.move(text.getPosition().x - text.getGlobalBounds().left, text.getPosition().y - text.getGlobalBounds().top);
}

void Button::set_text(std::string str){
}

void Button::draw(sf::RenderWindow &window){
	window.draw(bg);
	window.draw(text);
}
