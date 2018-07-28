#include "Scene.hpp"

using namespace std;
using namespace glm;

// public
	// constructors
		Scene::Scene(string name, unsigned int height, unsigned int width)
			: window(nullptr), openGLContext(nullptr), glew(0), mainCondition(false), events(SDL_Event()),
			availableObjects(map<string, Object>()), viewports(vector<Viewport>()) {

			if(SDLInitialization() && windowCreation(name, width, height) && contextCreation() && glewInitialization()) {
				mainCondition = true;
				myPainter = unique_ptr<Painter>(new Painter());
			}
		}
		Scene::Scene(const Scene& other)
			: window(other.window), openGLContext(other.openGLContext), events(other.events), glew(other.glew),
			mainCondition(other.mainCondition), availableObjects(other.availableObjects), viewports(other.viewports) {}
		Scene::Scene(Scene&& other) noexcept
			: window(other.window), openGLContext(other.openGLContext), events(other.events), glew(other.glew),
			mainCondition(other.mainCondition), availableObjects(other.availableObjects), viewports(other.viewports) {

			SDL_GL_DeleteContext(other.openGLContext);
			SDL_DestroyWindow(other.window);
		}
	// destructor
		Scene::~Scene() {
			SDL_GL_DeleteContext(openGLContext);
			SDL_DestroyWindow(window);
		}
	// operators
		Scene& Scene::operator=(const Scene& other) { return (*this = move(Scene(other))); }
		Scene& Scene::operator=(Scene&& other) noexcept {
			if(this == &other)
				return *this;

			SDL_GL_DeleteContext(openGLContext);
			SDL_DestroyWindow(window);

			openGLContext = other.openGLContext;
			window = other.window;
			events = other.events;
			glew = other.glew;
			mainCondition = other.mainCondition;
			availableObjects = other.availableObjects;
			viewports = other.viewports;

			return *this;
		}
	// program loop
		bool Scene::mainLoop() {
			// colors
				createPalette();
			// objects
				createObjects();
				auto triangle = myPainter->addVertices(vector<float>({ 0.5, 0.5, 0.0, -0.5, -0.5, 0.5 })),
					cube = myPainter->addVertices(availableObjects.at("cube").getVertices());
			// shader
				auto shader = myPainter->addShader("color3D.vert", "color3D.frag");
			// viewports
				auto width = SDL_GetWindowSurface(window)->w, height = SDL_GetWindowSurface(window)->h;
				// emplacing viewports in the container
				viewports.emplace_back(
					vec2(0, 0), vec2(width / 2, height),
					perspective(70.0, (double)(width / 2) / height, 1.0, 100.0), lookAt(vec3(1, 5, 1), vec3(0, 0, 0), vec3(0, 1, 0))
				);
				viewports.emplace_back(
					vec2(width / 2, 0), vec2(width / 2, height),
					perspective(70.0, (double)(width / 2) / height, 1.0, 100.0), lookAt(vec3(3, 3, 3), vec3(0, 0, 0), vec3(0, 1, 0))
				);

			while(mainCondition) {
				auto start = SDL_GetTicks();

				eventsHandler();
				// clear screen
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				// viewport
				viewports.at(0).call();
				// send shader to graphic card
				glUseProgram(myPainter->getShader(*shader)->getProgramID());
				{
					// create vertexAttribArrays for color & vertices
					myPainter->useColor("cubeColor3D");
					myPainter->useVertices(cube);
					// sending the matrices
					glUniformMatrix4fv(
						glGetUniformLocation(myPainter->getShader(*shader)->getProgramID(), "projection"),
						1, GL_FALSE, value_ptr(viewports.at(0).getProjection())
					);
					glUniformMatrix4fv(
						glGetUniformLocation(myPainter->getShader(*shader)->getProgramID(), "modelview"),
						1, GL_FALSE, value_ptr(viewports.at(0).getModelview())
					);
					// send vertices and colors to the shader
					myPainter->drawVertices(cube);
					// a bit of cleaning
					myPainter->disableVertexAttribArrays();
				}

				// viewport
				viewports.at(1).call();
				// send shader to graphic card
				glUseProgram(myPainter->getShader(*shader)->getProgramID());
				{
					// create vertexAttribArrays for color & vertices
					myPainter->useColor("cubeColor3D");
					myPainter->useVertices(cube);
					// sending the matrices
					glUniformMatrix4fv(
						glGetUniformLocation(myPainter->getShader(*shader)->getProgramID(), "projection"),
						1, GL_FALSE, value_ptr(viewports.at(1).getProjection())
					);
					glUniformMatrix4fv(
						glGetUniformLocation(myPainter->getShader(*shader)->getProgramID(), "modelview"),
						1, GL_FALSE, value_ptr(viewports.at(1).getModelview())
					);
					// send vertices and colors to the shader
					myPainter->drawVertices(cube);
					// a bit of cleaning
					myPainter->disableVertexAttribArrays();
				}

				// disable shader
				glUseProgram(0);
				SDL_GL_SwapWindow(window);

				// reduce processor charge
				auto end = SDL_GetTicks();
				auto elapsedTime = end - start;
				if(elapsedTime < framerate)
					SDL_Delay(framerate - elapsedTime);

			}
			return false;
		}
// protected
	// initializations
		bool Scene::SDLInitialization() {
			if(SDL_Init(SDL_INIT_VIDEO) < 0) {
				cout << "Erreur lors de l'initialisation de la SDL : " << SDL_GetError() << endl;
				SDL_Quit();
				return false;
			}
			return true;
		}
		void Scene::setOpenGLAttributes() {
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
			SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
			SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		}
		bool Scene::windowCreation(string name, unsigned int width, unsigned int height) {
			setOpenGLAttributes();
			// create the window
			window = SDL_CreateWindow(
				name.c_str(),
				SDL_WINDOWPOS_CENTERED,
				SDL_WINDOWPOS_CENTERED,
				width,
				height,
				SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
			);
			// check window
			if(window == nullptr) {
				cerr << "Error during the window creation : " << SDL_GetError() << endl;
				SDL_Quit();
				return false;
			}
			return true;
		}
		bool Scene::contextCreation() {
			openGLContext = SDL_GL_CreateContext(window);
			if(openGLContext == nullptr) {
				cout << "Error during the context creation : " << SDL_GetError() << endl;
				SDL_DestroyWindow(window);
				return false;
			}
			return true;
		}
		bool Scene::glewInitialization() {
			glew = glewInit();
			if(glew != GLEW_OK) {
				cout << "Error during the initialization of GLEW : " << glewGetErrorString(glew) << endl;
				SDL_GL_DeleteContext(openGLContext);
				SDL_DestroyWindow(window);
				return false;
			}
			glEnable(GL_DEPTH_TEST); // depth buffer
			return true;
		}
	// set up colors and objects
		void Scene::createPalette() {
			myPainter->addColor("blue1", vector<float>({
				0.0, (float)(204.0 / 255.0), 1.0,
				0.0, (float)(204.0 / 255.0), 1.0,
				0.0, (float)(204.0 / 255.0), 1.0
			}));
			myPainter->addColor("multicolor1", vector<float>({
				1.0, 0.0, 0.0,
				0.0, 1.0, 0.0,
				0.0, 0.0, 1.0
			}));
			myPainter->addColor("cubeColor3D", vector<float>({
				1.0, 0.0, 0.0,   1.0, 0.0, 0.0,   1.0, 0.0, 0.0,
				1.0, 0.0, 0.0,   1.0, 0.0, 0.0,   1.0, 0.0, 0.0,
				0.0, 1.0, 0.0,   0.0, 1.0, 0.0,   0.0, 1.0, 0.0,
				0.0, 1.0, 0.0,   0.0, 1.0, 0.0,   0.0, 1.0, 0.0,
				0.0, 0.0, 1.0,   0.0, 0.0, 1.0,   0.0, 0.0, 1.0,
				0.0, 0.0, 1.0,   0.0, 0.0, 1.0,   0.0, 0.0, 1.0,
				1.0, 0.0, 0.0,   1.0, 0.0, 0.0,   1.0, 0.0, 0.0,
				1.0, 0.0, 0.0,   1.0, 0.0, 0.0,   1.0, 0.0, 0.0,
				0.0, 1.0, 0.0,   0.0, 1.0, 0.0,   0.0, 1.0, 0.0,
				0.0, 1.0, 0.0,   0.0, 1.0, 0.0,   0.0, 1.0, 0.0,
				0.0, 0.0, 1.0,   0.0, 0.0, 1.0,   0.0, 0.0, 1.0,
				0.0, 0.0, 1.0,   0.0, 0.0, 1.0,   0.0, 0.0, 1.0
			}));
		}
		void Scene::createObjects() {
			list<string> filesToImport = { "chair.txt", "fan.txt", "floor.txt", "table.txt", "chair.txt", "wallfb.txt", "wallrl.txt" };
			vector<float> buffer;
			for(const auto& element : filesToImport) {
				buffer.clear();
				/*
				if(import3DSMaxFile(element, buffer))
					availableObjects.emplace(element.substr(0, element.size() - 4), Object(buffer, vector<float>()));
				else
					cerr << "Error importing file " + element << endl;
				*/
			}
			availableObjects.emplace("cube", Object(
				vector<float>({
					-1.0, -1.0, -1.0,	 1.0, -1.0, -1.0,	 1.0,  1.0, -1.0,
					-1.0, -1.0, -1.0,	-1.0,  1.0, -1.0,	 1.0,  1.0, -1.0,
					 1.0, -1.0,  1.0,	 1.0, -1.0, -1.0,	 1.0,  1.0, -1.0,
					 1.0, -1.0,  1.0,	 1.0,  1.0,  1.0,	 1.0,  1.0, -1.0,
					-1.0, -1.0,  1.0,	 1.0, -1.0,  1.0,	 1.0, -1.0, -1.0,
					-1.0, -1.0,  1.0,	-1.0, -1.0, -1.0,	 1.0, -1.0, -1.0,
					-1.0, -1.0,  1.0,	 1.0, -1.0,  1.0,	 1.0,  1.0,  1.0,
					-1.0, -1.0,  1.0,	-1.0,  1.0,  1.0,	 1.0,  1.0,  1.0,
					-1.0, -1.0, -1.0,	-1.0, -1.0,  1.0,	-1.0,  1.0,  1.0,
					-1.0, -1.0, -1.0,	-1.0,  1.0, -1.0,	-1.0,  1.0,  1.0,
					-1.0,  1.0,  1.0,	 1.0,  1.0,  1.0,	 1.0,  1.0, -1.0,
					-1.0,  1.0,  1.0,	-1.0,  1.0, -1.0,	 1.0,  1.0, -1.0
				}),
				myPainter->getColor("cubeColor3D")
			));
		}
	// other
		bool Scene::import3DSMaxFile(std::string filename, vector<float>& output) {
			auto file_content = extractFileContent("C:/temp/" + filename);
			if(!file_content)
				return false;
			stringstream fileStream = stringstream(*file_content);
			string buffer;
			unsigned int verticesCount = 0, facesCount = 0;
			float x = 0.0, y = 0.0, z = 0.0;

			while(!fileStream.eof()) {
				fileStream >> buffer;
				if(buffer == "*MESH_NUMVERTEX")
					fileStream >> verticesCount;
				else if(buffer == "*MESH_NUMFACES")
					fileStream >> facesCount;
				else if(buffer == "*MESH_VERTEX") {
					fileStream >> buffer >> x >> y >> z;
					cout << x << '\t' << y << '\t' << z << endl;
					try {
						output.push_back(x);
						output.push_back(y);
						output.push_back(z);
					}
					catch(bad_alloc &memoryAllocationException) {
						cerr << "Exception occurred: " << memoryAllocationException.what() << endl;
					}
				}
			}

			return verticesCount == output.size() / 3;
		}
		void Scene::eventsHandler() {
			// async events catcher
			SDL_PollEvent(&events);
			// find key pressed
			switch(events.type) {
				case SDL_WINDOWEVENT:
					switch(events.window.event) {
						case SDL_WINDOWEVENT_CLOSE:
							mainCondition = false;
							break;
						case SDL_WINDOWEVENT_RESIZED:
							// automatic, for I work from the window dimensions given by SDL ( SDL_GetWindowSurface(window) )
							break;
					}
					break;
				case SDL_KEYDOWN:
					switch(events.key.keysym.sym) {
						case SDLK_KP_1:
							cout << "1" << endl;
							break;
						case SDLK_KP_2:
							cout << "2" << endl;
							break;
						case SDLK_KP_3:
							cout << "3" << endl;
							break;
						case SDLK_KP_4:
							cout << "4" << endl;
							break;
						case SDLK_s:
							cout << "s" << endl;
							break;
						case SDLK_d:
							cout << "d" << endl;
							break;
						case SDLK_z:
							cout << "z" << endl;
							break;
							/*
						case SDLK_z:
							cout << "Z" << endl;
							break;
							*/
						case SDLK_t:
							cout << "t" << endl;
							break;
						case SDLK_r:
							cout << "r" << endl;
							break;
						case SDLK_b:
							cout << "b" << endl;
							break;
						case SDLK_f:
							cout << "f" << endl;
							break;
					}
					break;
			}
		}