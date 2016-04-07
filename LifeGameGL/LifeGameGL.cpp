// LifeGameGL.cpp : コンソール アプリケーションのエントリ ポイントを定義します。

#include "stdafx.h"
//-----------------------------------------------------------------------------
//Lib 
#pragma comment (lib, "opengl32.lib")
#pragma comment (lib, "glew32.lib")
#pragma comment (lib, "glfw3dll.lib")
#pragma comment (lib, "freeglut.lib")


#include "../common/controls.hpp"
#include "../common/shader.hpp"


#include "Model.h"








//-----------------------------------------------------------------------------
// GLFWでエラーとなったときに呼び出される関数
void glfw_error_callback_func(int error, const char* description){
	std::cerr << "GLFW Error: " << description << std::endl;
}




//-----------------------------------------------------------------------------
// global variable

GLFWwindow* window;

//-----------------------------------------------------------------------------
// main

int _tmain(int argc, _TCHAR* argv[])
{
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
	window = glfwCreateWindow(width, height, "LifeGame GL", NULL, NULL);
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
		cout << "GL_VENDOR:"	<< glGetString(GL_VENDOR) << endl;
		cout << "GL_RENDERER:"	<< glGetString(GL_RENDERER) << endl;
		cout << "GL_VERSION:"	<< glGetString(GL_VERSION) << endl;
		cout << "GL_SHADING_LANGUAGE_VERSION:"	<< glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

	}

	GlfwControls ctrl(window);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 1.0f);
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Enable depth test & culling
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders("DefaultVertexShader.vertexshader", "DefaultFragmentShader.fragmentshader");

	// インスタンス用配列
	vector<vec3> instancePosArray;
	{
		int gx = 32;	//glid x num
		int gy = 32;	//glid y num
		for (int i = 0; i<gy; i++){
			for (int j = 0; j<gx; j++){
				float ix = j - (gx - 1) / 2.0f;
				float iy = i - (gy - 1) / 2.0f;
				float iz = 0.0;
				vec3 pos(ix, iy, iz);
				instancePosArray.push_back(pos);
			}
		}
	}

	//インスタンスVBO
	GLuint vboInstancePos;
	{
		GLuint vbo;
		GLsizei size = (GLsizei)instancePosArray.size()*sizeof(vec3);	//in byte
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, size, instancePosArray.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		vboInstancePos = vbo;
	}



	GModel* model = new GModelSphere(8, 8, 0.45f);
//	GModel* model = new GModelCube(0.5f);

#if 0
	// Projection matrix : 45ｰ Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	glm::mat4 Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	// Camera matrix
	glm::mat4 View = glm::lookAt(
		glm::vec3(4, 3, 3), // Camera is at (4,3,3), in World Space
		glm::vec3(0, 0, 0), // and looks at the origin
		glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
		);
	// Model matrix : an identity matrix (model will be at the origin)
	glm::mat4 Model = glm::mat4(1.0f);
	// Our ModelViewProjection : multiplication of our 3 matrices
	glm::mat4 MVP = Projection * View * Model; // Remember, matrix multiplication is the other way around
#endif


	vec3 lightDirection(15.0, 15.0, 15.0);		//平行光源の位置


	ctrl.setCameraPosition(vec3(0.0, 0.0, 30.0));



	do{
		ctrl.computeMatricesFromInputs();


		// Clear the screen
		glClearColor(0.0f, 0.0f, 0.4f, 1.0f);
		glClearDepth(1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);




		// Use our shader
		glUseProgram(programID);

		vec3 eyePosition = ctrl.getCameraPosition();

#if 1
		for (int i = 0; i < (int)instancePosArray.size(); i++){
			// Compute the MVP matrix from keyboard and mouse input
			glm::mat4 mtxP = ctrl.getProjectionMatrix();
			glm::mat4 mtxV = ctrl.getViewMatrix();
			glm::mat4 mtxM = glm::mat4(1.0);
			mtxM = glm::translate(mtxM, instancePosArray[i]);
			glm::mat4 mtxMVP = mtxP * mtxV * mtxM;
			glm::mat4 mtxMinv = inverse(mtxM);


			// Send our transformation to the currently bound shader, 
			glUniformMatrix4fv(glGetUniformLocation(programID, "mtxMVP"), 1, GL_FALSE, &mtxMVP[0][0]);
			glUniformMatrix4fv(glGetUniformLocation(programID, "mtxM"), 1, GL_FALSE, &mtxM[0][0]);
			glUniformMatrix4fv(glGetUniformLocation(programID, "mtxMinv"), 1, GL_FALSE, &mtxMinv[0][0]);
			glUniform3fv(glGetUniformLocation(programID, "lightDirection"), 1, &lightDirection[0]);
			glUniform3fv(glGetUniformLocation(programID, "eyePosition"), 1, &eyePosition[0]);

			model->Draw();
		}
#else
		model->DrawInstansing(vboInstancePos,(int)instancePosArray.size());
#endif

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
	glfwWindowShouldClose(window) == 0);


	delete model;

	glDeleteBuffers(1, &vboInstancePos);
	
	glDeleteProgram(programID);


	// Close OpenGL window and terminate GLFW
	glfwTerminate();


	return 0;
}

