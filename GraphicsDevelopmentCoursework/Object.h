#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#pragma warning(push, 0) // disable warnings
#include <glm/gtc/type_ptr.hpp>
#pragma warning(pop)

#include "Shader.h"
#include "Viewport.h"

class Object {
	/* ATTRIBUTES */
		private:
			std::vector<float> vertices;
			std::vector<float> colors;
			static std::weak_ptr<Viewport> last_used_viewport;
			static unsigned int vertexAttribArrays;
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
			// other
				bool draw(const std::shared_ptr<Viewport> viewport, const Shader& shader);
};