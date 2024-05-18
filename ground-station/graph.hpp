class Graph{
	public:
		sf::VertexArray points;//(sf::LineStrip, 2);
		int maxPoints;
		
		Graph(int maxPoints);
		void add_point(int y);
	private:
};

Graph::Graph(int m){
	//points.resize(maxPoints);
	maxPoints = m;
	points.resize(2);
	points.setPrimitiveType(sf::LineStrip);
	points[0].color = sf::Color(0, 0, 0);
	points[1].color = sf::Color(0, 0, 0);
}

void Graph::add_point(int y){
		if(y == -1){ y = rand() % 500; }
		if(points.getVertexCount() > maxPoints){
			for(int i = 0; i < maxPoints; i++){
				points[i].position.y = points[i + 1].position.y;
			}
			points[maxPoints].position.y = y;
		}else{
			points.append(sf::Vertex(sf::Vector2f(points[points.getVertexCount() - 1].position.x + 10, y), sf::Color(0, 0, 0)));
		}
}
