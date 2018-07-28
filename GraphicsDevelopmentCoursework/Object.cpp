#include "Object.h"

using namespace std;

// constructors
	Object::Object(const vector<float>& newVertices, const vector<float>& newColors) : vertices(newVertices), colors(newColors) {}
	Object::Object(const Object& other) : vertices(other.vertices), colors(other.colors) {}
	Object::Object(Object&& other) noexcept : vertices(other.vertices), colors(other.colors) {}
// destructor
	Object::~Object() {}
// operators
	Object& Object::operator=(const Object& other) { return (*this = move(Object(other))); }
	Object& Object::operator=(Object&& other) noexcept {
		if(this == &other)
			return *this;

		vertices = other.vertices;
		colors = other.colors;

		return *this;
	}
// getters
	const vector<float>& Object::getVertices() const { return vertices; }
	const vector<float>& Object::getColors() const { return colors; }
// other
	bool Object::draw(const shared_ptr<Viewport> viewport, const Shader& shader) const {
		/*
		if(viewport != last_used_viewport.lock()) {
			last_used_viewport = weak_ptr<Viewport>(viewport);
			viewport->call();
		}
		*/
		cout << "I am being called !" << endl;
		return false;
	}