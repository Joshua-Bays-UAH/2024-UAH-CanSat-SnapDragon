class Graph{
	public:
		sf::VertexArray points;//(sf::LineStrip, 2);
		int maxPoints;
		sf::Vector2f position;
		sf::Vector2f maxSize;
		std::deque<float> values = {0, 0, 0};
		//float min = std::numeric_limits<float>::max();
		//float max = std::numeric_limits<float>::min();
		float min = 0;
		float max = 0;
		std::string title;
		sf::Text label;
		sf::Text maxLabel;
		sf::Text minLabel;
		
		Graph(int m, int x, int y, int w, int h, sf::Font &font, std::string str);
		void add_point(float y);
		void draw(sf::RenderWindow &window);
		void generate_points();
	private:
};

Graph::Graph(int m, int x, int y, int w, int h, sf::Font &font, std::string str){
	//points.resize(maxPoints);
	maxPoints = m;
	points.resize(2);
	points.setPrimitiveType(sf::LineStrip);
	points[0].color = sf::Color(0, 0, 0);
	points[1].color = sf::Color(0, 0, 0);
	points[0].position.x = x;
	points[1].position.x = x;
	points[0].position.y = y;
	points[1].position.y = y;
	
	position.x = x;
	position.y = y;
	maxSize.x = w;
	maxSize.y = h;
	
	label.setString(str);
	label.setFont(font);
	label.setCharacterSize(500);
	label.setFillColor(sf::Color(0, 0, 0));
	label.setScale(.9 * w/label.getGlobalBounds().width, .9 * w/label.getGlobalBounds().width);
	label.setPosition(x + w/2 - label.getGlobalBounds().width/2, y+h);
	label.move(label.getPosition().x - label.getGlobalBounds().left, label.getPosition().y - label.getGlobalBounds().top);
	
	minLabel.setString("123.456");
	minLabel.setFont(font);
	minLabel.setCharacterSize(500);
	minLabel.setFillColor(sf::Color(0x88, 0x88, 0x88));
	minLabel.setScale(.1 * maxSize.y/minLabel.getGlobalBounds().height, .1 * maxSize.y/minLabel.getGlobalBounds().height);
	minLabel.setPosition(position.x + maxSize.x, position.y + maxSize.y - minLabel.getGlobalBounds().height/2);
	minLabel.move(minLabel.getPosition().x - minLabel.getGlobalBounds().left, minLabel.getPosition().y - minLabel.getGlobalBounds().top);
	
	maxLabel.setString("123.456");
	maxLabel.setFont(font);
	maxLabel.setCharacterSize(500);
	maxLabel.setFillColor(sf::Color(0x88, 0x88, 0x88));
	maxLabel.setScale(.1 * maxSize.y/maxLabel.getGlobalBounds().height, .1 * maxSize.y/maxLabel.getGlobalBounds().height);
	maxLabel.setPosition(position.x + maxSize.x, position.y + maxLabel.getGlobalBounds().height/2);
	maxLabel.move(maxLabel.getPosition().x - maxLabel.getGlobalBounds().left, maxLabel.getPosition().y - maxLabel.getGlobalBounds().top);
}

void Graph::add_point(float y){
	values.push_back(y);
	if(values.size() > maxPoints){
		values.pop_front();
	}
}

void Graph::draw(sf::RenderWindow &window){
	char buff[10];
	sprintf(buff, "%.3f\n", min);
	minLabel.setString(std::string(buff));
	sprintf(buff, "%.3f\n", max);
	maxLabel.setString(std::string(buff));
	
	
	generate_points();
	window.draw(points);
	window.draw(label);
	window.draw(minLabel);
	window.draw(maxLabel);
}

void Graph::generate_points(){
	int size = values.size();
	min = std::numeric_limits<float>::max();
	max = std::numeric_limits<float>::min();
	float p;
	for(int i = 0; i < size; i++){
		p = values[i];
		if(p < min){ min = p; }
		if(p > max){ max = p; }
	}
	int pointWidth = (int)(maxSize.x/size);
	float scale = maxSize.y/(max - min);
	//printf("min: %f max: %f scale: %f\n", min, max, scale);
	points.clear();
	for(int i = 0; i < size; i++){
		points.append(sf::Vertex(sf::Vector2f(position.x+i*pointWidth, position.y + (max-values[i])*scale), sf::Color(0, 0, 0)));
		//printf("%f %f\n", points[i].position.x, points[i].position.y);
	}
}
