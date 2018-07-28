#pragma once

#include <vector>

class Object {
	/* ATTRIBUTES */
		private:
			std::vector<float> vertices;
			std::vector<float> colors;
	/* MEMBERS */
		public:
			// constructors
				Object(const std::vector<float>& newVertices, const std::vector<float>& newColors);
				Object(const Object& other);
				Object(Object&& other) noexcept;
			// destructor
				~Object() noexcept;
			// operators
				Object& operator=(const Object& other);
				Object& operator=(Object&& other) noexcept;
			// getters
				// thou shalt not modify these objects
				const std::vector<float>& getVertices() const;
				const std::vector<float>& getColors() const;
};