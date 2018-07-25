#include "Scene.hpp"

using namespace std;
using namespace glm;

// public
	Scene::Scene(string name, unsigned int height, unsigned int width)
		: window(nullptr), openGLContext(nullptr), glew(0), mainCondition(false), height(height), width(width), events(SDL_Event())/*,
		leftViewport(SDL_Rect()), rightViewport(SDL_Rect())*/ {

		if(createViewports() && SDLInitialization() && windowCreation(name) && contextCreation() && glewInitialization()) {
			mainCondition = true;
			myPainter = unique_ptr<Painter>(new Painter());
		}
	}
	Scene::Scene(const Scene& other)
		: window(other.window), openGLContext(other.openGLContext), events(other.events), glew(other.glew),
		mainCondition(other.mainCondition), height(other.height), width(other.width), availableObjects(other.availableObjects) {}
	Scene::Scene(Scene&& other) noexcept
		: window(other.window), openGLContext(other.openGLContext), events(other.events), glew(other.glew),
		mainCondition(other.mainCondition), height(other.height), width(other.width), availableObjects(other.availableObjects) {

		SDL_GL_DeleteContext(other.openGLContext);
		SDL_DestroyWindow(other.window);
	}
	Scene::~Scene() {
		SDL_GL_DeleteContext(openGLContext);
		SDL_DestroyWindow(window);
	}
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
		height = other.height;
		width = other.width;
		availableObjects = other.availableObjects;
		return *this;
	}
	bool Scene::mainLoop() {
		// matrix
			mat4 projection = perspective(70.0, (double)height / width, 1.0, 100.0), modelview = mat4(1.0);
		// colors
			createPalette();
		// objects
			createObjects();
			auto triangle = myPainter->addVertices(vector<float>({ 0.5, 0.5, 0.0, -0.5, -0.5, 0.5 })),
				cube = myPainter->addVertices(availableObjects.at("cube").getVertices());
		// shader
			auto shader = myPainter->addShader("color3D.vert", "color3D.frag");

		if(shader) {
			while(mainCondition) {
				// reduce processor consumption
				SDL_Delay(20);
				// handle events
				eventsHandler();
				// clear screen
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				// set camera position
				modelview = lookAt(vec3(3, 3, 3), vec3(0, 0, 0), vec3(0, 1, 0));
				// send shader to graphic card
				glUseProgram(myPainter->getShader(*shader)->getProgramID());
				{
					// create vertexAttribArrays for color & vertices
					myPainter->useColor("cubeColor3D");
					myPainter->useVertices(cube);
					// sending the matrices
					glUniformMatrix4fv(
						glGetUniformLocation(myPainter->getShader(*shader)->getProgramID(), "projection"), 1, GL_FALSE, value_ptr(projection)
					);
					glUniformMatrix4fv(
						glGetUniformLocation(myPainter->getShader(*shader)->getProgramID(), "modelview"), 1, GL_FALSE, value_ptr(modelview)
					);
					// send vertices and colors to the shader
					myPainter->drawVertices(cube);
					// a bit of cleaning
					myPainter->disableVertexAttribArrays();
				}
				// disable shader
				glUseProgram(0);
				SDL_GL_SwapWindow(window);
			}
		}
		return false;
	}
// protected
	bool Scene::createViewports() {
		/*
		leftViewport.x = 0;
		leftViewport.y = 0;
		leftViewport.w = SCREEN_WIDTH / 2;
		leftViewport.h = SCREEN_HEIGHT / 2;
		SDL_RenderSetViewport(gRenderer, &leftViewport);
		*/
		return true;
	}
	bool Scene::SDLInitialization() {
		if(SDL_Init(SDL_INIT_VIDEO) < 0) {
			cout << "Erreur lors de l'initialisation de la SDL : " << SDL_GetError() << endl;
			SDL_Quit();
			return false;
		}
		return true;
	}
	bool Scene::windowCreation(string name) {
		setOpenGLAttributes();
		window = SDL_CreateWindow(
			name.c_str(),
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			height,
			width,
			SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
		);
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
	bool Scene::import3DSMaxFile(std::string filename, vector<float>& output) {
		stringstream fileStream = stringstream(extractFileContent("C:/temp/" + filename));
		string buffer;
		unsigned int count = 10, verticesCount = 0, facesCount = 0;
		float x, y, z;

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
					exit(1);
				}
			}
		}
		return verticesCount == output.size() / 3;
	}
	void Scene::setOpenGLAttributes() {
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
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
						resize();
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
	void Scene::resize() {
		int width = SDL_GetWindowSurface(window)->w, height = SDL_GetWindowSurface(window)->h;

		glViewport(0, 0, (GLsizei)width, (GLsizei)height);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		// in replacement of gluPerspective()
		{
			const GLdouble pi = M_PI;
			GLdouble fW, fH;

			//fH = tan( (60.0f / 2) / 180 * pi ) * 1.0f;
			fH = tan(60.0f / 360 * pi) * 1.0f;
			fW = fH * float(width);

			glFrustum(-fW, fW, -fH, fH, 1.0f, 100.0f);
		}

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}