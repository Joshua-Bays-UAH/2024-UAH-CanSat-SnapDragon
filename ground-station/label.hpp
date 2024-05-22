class Label{
	public:
		sf::RectangleShape area;
		sf::Text text;
		
		Label(float x, float y, float w, float h, sf::Font &font, std::string str);
		void set_text(std::string str);
		void set_color(sf::Color color);
		void draw(sf::RenderWindow &window);
};

Label::Label(float x, float y, float w, float h, sf::Font &font, std::string str){
	area.setPosition(x, y);
	area.setSize(sf::Vector2f(w, h));
	text.setString(str);
	text.setFont(font);
	text.setCharacterSize(500);
	text.setFillColor(sf::Color(0 , 0, 0));
	text.setScale(area.getSize().x/text.getGlobalBounds().width, area.getSize().y/text.getGlobalBounds().height);
	text.setPosition(area.getPosition().x, area.getPosition().y);
	text.move(text.getPosition().x - text.getGlobalBounds().left, text.getPosition().y - text.getGlobalBounds().top);
}

void Label::set_text(std::string str){
	//printf("change to %s\n", str.c_str());
	text.setString(str);
	text.setCharacterSize(500);
	text.setScale(area.getSize().x/text.getLocalBounds().width, area.getSize().y/text.getLocalBounds().height);
	text.setPosition(area.getPosition().x, area.getPosition().y);
	text.move(text.getPosition().x - text.getGlobalBounds().left, text.getPosition().y - text.getGlobalBounds().top);
}

void Label::draw(sf::RenderWindow &window){
	window.draw(text);
}

