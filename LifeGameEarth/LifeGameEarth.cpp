// LifeGameTorus.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "HookCoutCerr.hpp"

//Lib 
#pragma comment (lib, "opengl32.lib")
#pragma comment (lib, "glew32.lib")
#pragma comment (lib, "glfw3dll.lib")
//#pragma comment (lib, "freeglut.lib")


#include "../common/controls.hpp"
#include "../common/shader.hpp"


#include "Model.h"

#define USE_LIFEGAME_GPGPU		//GPUによってLifeGameの更新を行う。転送がないので高速
#define LIFE_BOUND_REPEAT




int createTexture(
	GLuint texid,						///<texture ID
	const int width,						///<texture width
	const int height,						///<texture height
	unsigned short internalFormat	///<internal fomrat
	)
{

	GLenum format;
	GLenum type;

	//https://www.khronos.org/opengles/sdk/docs/man3/html/glTexImage2D.xhtml

	switch (internalFormat){
	case(GL_R16F) : format = GL_RED; type = GL_FLOAT; break;
	case(GL_R32F) : format = GL_RED; type = GL_FLOAT; break;
	case(GL_RG16F) : format = GL_RG; type = GL_FLOAT; break;
	case(GL_RG32F) : format = GL_RG; type = GL_FLOAT; break;
	case(GL_RGB16F) : format = GL_RGB; type = GL_FLOAT; break;
	case(GL_RGB32F) : format = GL_RGB; type = GL_FLOAT; break;
	case(GL_RGBA16F) : format = GL_RGBA; type = GL_FLOAT; break;
	case(GL_RGBA32F) : format = GL_RGBA; type = GL_FLOAT; break;

	case(GL_R8UI) : format = GL_RED_INTEGER; type = GL_UNSIGNED_BYTE; break;
	case(GL_RG8UI) : format = GL_RG_INTEGER; type = GL_UNSIGNED_BYTE; break;
	case(GL_RGB8UI) : format = GL_RGB_INTEGER; type = GL_UNSIGNED_BYTE; break;
	case(GL_RGBA8UI) : format = GL_RGBA_INTEGER; type = GL_UNSIGNED_BYTE; break;

	case(GL_R8I) : format = GL_RED_INTEGER; type = GL_BYTE; break;
	case(GL_RG8I) : format = GL_RG_INTEGER; type = GL_BYTE; break;
	case(GL_RGB8I) : format = GL_RGB_INTEGER; type = GL_BYTE; break;
	case(GL_RGBA8I) : format = GL_RGBA_INTEGER; type = GL_BYTE; break;

	case(GL_R16UI) : format = GL_RED_INTEGER; type = GL_UNSIGNED_SHORT; break;
	case(GL_RG16UI) : format = GL_RG_INTEGER; type = GL_UNSIGNED_SHORT; break;
	case(GL_RGB16UI) : format = GL_RGB_INTEGER; type = GL_UNSIGNED_SHORT; break;
	case(GL_RGBA16UI) : format = GL_RGBA_INTEGER; type = GL_UNSIGNED_SHORT; break;

	case(GL_R16I) : format = GL_RED_INTEGER; type = GL_SHORT; break;
	case(GL_RG16I) : format = GL_RG_INTEGER; type = GL_SHORT; break;
	case(GL_RGB16I) : format = GL_RGB_INTEGER; type = GL_SHORT; break;
	case(GL_RGBA16I) : format = GL_RGBA_INTEGER; type = GL_SHORT; break;

	case(GL_R32UI) : format = GL_RED_INTEGER; type = GL_UNSIGNED_INT; break;
	case(GL_RG32UI) : format = GL_RG_INTEGER; type = GL_UNSIGNED_INT; break;
	case(GL_RGB32UI) : format = GL_RGB_INTEGER; type = GL_UNSIGNED_INT; break;
	case(GL_RGBA32UI) : format = GL_RGBA_INTEGER; type = GL_UNSIGNED_INT; break;

	case(GL_R32I) : format = GL_RED_INTEGER; type = GL_INT; break;
	case(GL_RG32I) : format = GL_RG_INTEGER; type = GL_INT; break;
	case(GL_RGB32I) : format = GL_RGB_INTEGER; type = GL_INT; break;
	case(GL_RGBA32I) : format = GL_RGBA_INTEGER; type = GL_INT; break;

	}

	// Bind our texture in Texture Unit 0
//	glActiveTexture(GL_TEXTURE0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texid);
	// (set texture parameters here)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

#ifdef LIFE_BOUND_REPEAT					
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
#else
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
#endif

	//create the texture
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, 0);


	glBindTexture(GL_TEXTURE_2D, 0);

	return 0;
}

int uploadTexture(
	const GLuint texId,		///<texture id
	const void* data,		///<src data
	const GLenum format = GL_RED,	///<format
	const GLenum type = GL_FLOAT		///<type
	)
{

	GLint width  = 0;
	GLint height = 0;

	// Bind our texture in Texture Unit 0
//	glActiveTexture(GL_TEXTURE0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texId);

	glGetTexLevelParameteriv(
		GL_TEXTURE_2D, 0,
		GL_TEXTURE_WIDTH, &width
		);

	glGetTexLevelParameteriv(
		GL_TEXTURE_2D, 0,
		GL_TEXTURE_WIDTH, &height
		);


	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format, type, data);

	return 0;
}

//download from framebuffer
int downloadFrameBuffer(
	GLint width,
	GLint height,
	void* data,		///<src data
	const GLenum format = GL_RED,	///<format
	const GLenum type = GL_FLOAT		///<type
	)
{
	//download from framebuffer

	// ReadBuffer
	glReadBuffer(GL_COLOR_ATTACHMENT0);

	// ReadPixels
	glReadPixels(0, 0, width, height, format, type, data);

	return 0;
}

void executeGpGpuProcess(
	const GLuint pid,				//progmra ID
	GLuint _fbo,					//FBO 0の場合、内部で生成、破棄 後でreadPixelで読みだせない
	GLuint texSrc,					//src texture ID
	GLuint texDst					//dst texture ID
	)
{
	int width;
	int height;

	//get texture size
	{
		glBindTexture(GL_TEXTURE_2D, texDst);

		glGetTexLevelParameteriv(
			GL_TEXTURE_2D, 0,
			GL_TEXTURE_WIDTH, &width
			);

		glGetTexLevelParameteriv(
			GL_TEXTURE_2D, 0,
			GL_TEXTURE_HEIGHT, &height
			);

		glBindTexture(GL_TEXTURE_2D, 0);
	}



	glUseProgram(pid);

	// FBO identifier
	GLuint fbo = _fbo;

	//---------------------------------
	// FBO
	// create FBO (off-screen framebuffer)
	if (_fbo == 0)	glGenFramebuffers(1, &fbo);

	// bind offscreen framebuffer (that is, skip the window-specific render target)
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	GLuint vao = 0;
	GLuint vbo = 0;

	// [-1, 1] の正方形
	static GLfloat position[][2] = {
		{ -1.0f, -1.0f },
		{ 1.0f, -1.0f },
		{ 1.0f, 1.0f },
		{ -1.0f, 1.0f }
	};

	// create vao&vbo
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	// bind vao & vbo
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	// upload vbo data
	glBufferData(GL_ARRAY_BUFFER, (int)sizeof(position), position, GL_STATIC_DRAW);

	// Set VertexAttribute

	GLint attrLoc = glGetAttribLocation(pid, "position");
	glEnableVertexAttribArray(attrLoc);	//enable attribute Location
	glVertexAttribPointer(
		attrLoc,			// attribute 0. No particular reason for 0, but must match the layout in the shader.
		2,					// size	(Specifies the number of components) x,y
		GL_FLOAT,			// type
		GL_FALSE,			// normalized?
		0,					// stride (Specifies the byte offset between consecutive generic vertex attributes)
		(void*)0			// array buffer offset (Specifies a pointer to the first generic vertex attribute in the array)
		);

	//Bind Texture & Fbo
	if (texSrc){
		const int textureUnit = 0;			///@@@@@
		glActiveTexture(GL_TEXTURE0 + textureUnit);
		glBindTexture(GL_TEXTURE_2D, texSrc);
		glUniform1i(glGetUniformLocation(pid, "texSrc"), textureUnit);
		glUniform2f(glGetUniformLocation(pid, "texSrcSize"), (float)width, (float)height);
	}

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texDst, 0);

	//Viewport
	glViewport(0, 0, width, height);

	//Render!!
	glDrawArrays(GL_TRIANGLE_FAN, 0, (int)(sizeof(position) / sizeof(position[0])));

	glFlush();

	// delete vao&vbo
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);

	//clean up
	if (_fbo == 0)	glDeleteFramebuffers(1, &fbo);

}


//-----------------------------------------------------------------------------
// GLFWでエラーとなったときに呼び出される関数
void glfw_error_callback_func(int error, const char* description){
	std::cerr << "GLFW Error: " << description << std::endl;
}

//常に正の数を返すmod
#define IMOD(i,j) (((i) % (j)) < 0 ? ((i) % (j)) + ((j) < 0 ? -(j) : (j)) : (i) % (j))



//-----------------------------------------------------------------------------
//https://ja.wikipedia.org/wiki/%E3%83%A9%E3%82%A4%E3%83%95%E3%82%B2%E3%83%BC%E3%83%A0
void updateCellLife(const float src[], float dst[], int height, int width){
	for (int y = 0; y < height; y++){
		for (int x = 0; x < width; x++){
			int liveCell = 0;
			for (int dy = -1; dy <= 1; dy++){
				for (int dx = -1; dx <= 1; dx++){
					if (dx == 0 && dy == 0) continue;//自分は数えない
					int _x = x+dx;
					int _y = y+dy;


#ifdef LIFE_BOUND_REPEAT					
					_x = IMOD(_x , width );
					_y = IMOD(_y, height);

#else
					//境界
					if (_x < 0)continue;
					if (_x >= width)continue;
					if (_y < 0)continue;
					if (_y >= height)continue;
#endif

					int idx = _y*width + _x;
					if (src[idx] != 0.0) liveCell++;
				}
			}

			int idx = (y*width + x);
			//誕生:死んでいるセルに隣接する生きたセルがちょうど3つあれば、次の世代が誕生する。
			if (src[idx] == 0.0 && liveCell == 3) dst[idx] = 1.0;
			//生存:生きているセルに隣接する生きたセルが2つか3つならば、次の世代でも生存する。
			else if (src[idx] != 0.0 && (liveCell == 2 || liveCell == 3)) dst[idx] = 1.0;
			//過疎:	生きているセルに隣接する生きたセルが1つ以下ならば、過疎により死滅する。
			else if (src[idx] != 0.0 && (liveCell <= 1)) dst[idx] = 0.0;
			//過密:	生きているセルに隣接する生きたセルが4つ以上ならば、過密により死滅する。
			else if (src[idx] != 0.0 && (liveCell >= 4)) dst[idx] = 0.0;
			//現状維持（ここには来ない？）
			else dst[idx] = src[idx];
		}
	}
}

//Cell初期化(ランダム)
void initCellLife(float dst[], int height, int width){
	for (int y = 0; y < height; y++){
		for (int x = 0; x < width; x++){
			dst[y*width + x] = (rand() >(RAND_MAX / 2)) ? 1.0f : 0.0f;
		}
	}
}



//-----------------------------------------------------------------------------
// global variable

GLFWwindow* window;

//-----------------------------------------------------------------------------
// main

int _tmain(int argc, _TCHAR* argv[])
{
	HookCoutCerr hoge;


	const int width = 1024;
	const int height = 768;


	glfwSetErrorCallback(glfw_error_callback_func);


	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	//-----------------------------------------------------------------------------
	glfwWindowHint(GLFW_SAMPLES, 4);

#if 1
	// GL3.3 Core profile
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#elif 0
	// GL3.3 profile
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#elif 0
	// GLES 3.0profile
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	//	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#else
	// GL4.0 Core profile
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	//	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#endif

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(width, height, "LifeGameEarth", NULL, NULL);
	if (window == NULL){
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

#if defined _WIN32
	// Initialize GLEW
	glewExperimental = GL_TRUE;			///!!!! important for core profile // コアプロファイルで必要となります
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}
#endif


	{
		cout << "GL_VENDOR:" << glGetString(GL_VENDOR) << endl;
		cout << "GL_RENDERER:" << glGetString(GL_RENDERER) << endl;
		cout << "GL_VERSION:" << glGetString(GL_VERSION) << endl;
		cout << "GL_SHADING_LANGUAGE_VERSION:" << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

	}

	GlfwControls ctrl(window);

	cv::Mat img = cv::imread("earth512x512.jpg");
//	cv::imshow("Earth",img);



	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 1.0f);
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Enable depth test & culling
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);

	//Program生成
	enum E_PID{
		PID_Draw,
#ifdef USE_LIFEGAME_GPGPU
		PID_LifeGameInit,
		PID_LifeGameUpdate,
#endif
		PID_SIZEOF,
	};

	GLuint programID[E_PID::PID_SIZEOF];

	// Create and compile our GLSL program from the shaders
	programID[E_PID::PID_Draw] = LoadShaders("DefaultVertexShader.vertexshader", "TextureMapFragmentShader.fragmentshader");

#ifdef USE_LIFEGAME_GPGPU
	// Create and compile our GLSL program from the shaders
	programID[E_PID::PID_LifeGameInit] = LoadShaders("LifeGame.vertexshader", "LifeGameInit.fragmentshader");
	programID[E_PID::PID_LifeGameUpdate] = LoadShaders("LifeGame.vertexshader", "LifeGameUpdate.fragmentshader");
#endif




	//モデル生成
	PureModel* pureModel = new PureModelSphere(32, 32, 0.5f);
	GlModel* model = new GlModel(pureModel, img);

	//Life Gameのセル数
#if 0
	const int gx = 8;	//glid x num
	const int gy = 8;	//glid y num
#elif 0
	const int gx = 32;	//glid x num
	const int gy = 32;	//glid y num
#elif 0
	const int gx = 64;	//glid x num
	const int gy = 64;	//glid y num
#elif 0
	const int gx = 128;	//glid x num
	const int gy = 128;	//glid y num
#elif 0
	const int gx = 256;	//glid x num
	const int gy = 256;	//glid y num
#elif 1
	const int gx = 512;	//glid x num
	const int gy = 512;	//glid y num
#elif 1
	const int gx = 1024;	//glid x num
	const int gy = 1024;	//glid y num

#endif

	int cellBank = 0;

#ifndef USE_LIFEGAME_GPGPU
	//Cell生成 (2bank)
//	float cell[2][gx*gy];
	float* cell0 = new float[gx*gy];
	float* cell1 = new float[gx*gy];
	float* cell[2] = { cell0, cell1 };


	//Cell初期化(ランダム)
	initCellLife(&cell[cellBank][0], gy, gx);
#endif

	//texture生成
	enum E_TID{
		TID_CELL_STATE0,
		TID_CELL_STATE1,
		TID_SIZEOF,
	};

	GLuint textureID[E_TID::TID_SIZEOF];

	// ---- ---- ---- ---- ---- ---- ---- ----
	// CreateTexture
	glGenTextures(sizeof(textureID) / sizeof(textureID[0]), textureID); // create (reference to) a new texture
	createTexture(textureID[0], gx, gy, GL_R32F);
	createTexture(textureID[1], gx, gy, GL_R32F);

#ifdef USE_LIFEGAME_GPGPU
	executeGpGpuProcess(programID[E_PID::PID_LifeGameInit], 0,0, textureID[cellBank]);
#else
	uploadTexture(textureID[cellBank], &cell[cellBank][0]);
#endif



//	vec3 lightDirection(15.0, 15.0, 15.0);		//平行光源の位置
	vec3 lightDirection(3.0, 3.0, 15.0);		//平行光源の位置
	ctrl.setCameraPosition(vec3(0.0, 0.0, 1.5));	//カメラ初期位置


	long framecount = 0;
	glm::mat4 mtxM = glm::mat4(1.0);

	do{
		ctrl.computeMatricesFromInputs();	//コントロールのupdate


		// Clear the screen
		glClearColor(0.0f, 0.0f, 0.4f, 1.0f);
		glClearDepth(1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		vec3 eyePosition = ctrl.getCameraPosition();

		{
			const GLuint pid = programID[E_PID::PID_Draw];
			
			// Use our shader
			glUseProgram(pid);


			// Compute the MVP matrix from keyboard and mouse input
			glm::mat4 mtxP = ctrl.getProjectionMatrix();
			glm::mat4 mtxV = ctrl.getViewMatrix();
//			glm::mat4 mtxM = glm::mat4(1.0);
			mtxM = glm::rotate(mtxM, float(0.05/180.0f*3.14), vec3(0.0, 1.0, 0.0));	//rotate Y axis
			glm::mat4 mtxMVP = mtxP * mtxV * mtxM;
			glm::mat4 mtxMinv = inverse(mtxM);


			// Send our transformation to the currently bound shader, 
			glUniformMatrix4fv(glGetUniformLocation(pid, "mtxMVP"), 1, GL_FALSE, &mtxMVP[0][0]);
			glUniformMatrix4fv(glGetUniformLocation(pid, "mtxM"), 1, GL_FALSE, &mtxM[0][0]);
			glUniformMatrix4fv(glGetUniformLocation(pid, "mtxMinv"), 1, GL_FALSE, &mtxMinv[0][0]);
			glUniform3fv(glGetUniformLocation(pid, "lightDirection"), 1, &lightDirection[0]);
			glUniform3fv(glGetUniformLocation(pid, "eyePosition"), 1, &eyePosition[0]);
			glUniform1i(glGetUniformLocation(pid, "texImg"), 0);  // = use GL_TEXTURE0
			glUniform1i(glGetUniformLocation(pid, "texCell"), 1);  // = use GL_TEXTURE1

			//set viewport (client windows size)
			glViewport(0, 0, width, height);

			glActiveTexture(GL_TEXTURE1);	// = use GL_TEXTURE1
			glBindTexture(GL_TEXTURE_2D, textureID[cellBank]);
			model->Draw();

			glBindTexture(GL_TEXTURE_2D, 0);	//un-bind


//			model->DrawPoints();



		}

#ifdef USE_LIFEGAME_GPGPU
		//---------------------------------
		//Execute GPGPU
		if (framecount % 1000){
			const GLuint pid = programID[E_PID::PID_LifeGameUpdate];
			GLuint texSrc = textureID[cellBank];
			GLuint texDst = textureID[cellBank ^ 1];
			executeGpGpuProcess(pid, 0,texSrc, texDst);
			//togle cell bank
			cellBank = cellBank ^ 1;
		}
		else{
			executeGpGpuProcess(programID[E_PID::PID_LifeGameInit], 0,0, textureID[cellBank]);
		}
#else
		if (framecount % 1000){
			//update cell state
			updateCellLife(&cell[cellBank][0], &cell[cellBank ^ 1][0], gy, gx);
			cellBank = cellBank ^ 1;
			uploadTexture(textureID[cellBank], &cell[cellBank][0]);

		}
		else{
			initCellLife(&cell[cellBank][0], gy, gx);
		}
#endif


		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
		framecount++;
	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
	glfwWindowShouldClose(window) == 0);

	delete pureModel;
	delete model;

	glDeleteTextures(sizeof(textureID) / sizeof(textureID[0]), textureID);

#ifndef USE_LIFEGAME_GPGPU
	delete cell0;
	delete cell1;
#endif


	// Delete program
	for (int i = 0; i < E_PID::PID_SIZEOF; i++)	{
		glDeleteProgram(programID[i]);
	}

	// Close OpenGL window and terminate GLFW
	glfwTerminate();


	return 0;
}



