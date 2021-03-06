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
//https://ja.wikipedia.org/wiki/%E3%83%A9%E3%82%A4%E3%83%95%E3%82%B2%E3%83%BC%E3%83%A0
void updateCellLife(const float src[], float dst[], int gy, int gx){
	for (int y = 0; y < gy; y++){
		for (int x = 0; x < gx; x++){
			int liveCell = 0;
			for (int dy = -1; dy <= 1; dy++){
				for (int dx = -1; dx <= 1; dx++){
					if (dx == 0 && dy == 0) continue;//自分は数えない
					//境界
					if (x + dx < 0 )continue;
					if (x + dx >= gx)continue;
					if (y + dy < 0 )continue;
					if (y + dy >= gy)continue;

					if (src[(y + dy)*gx+(x + dx)] != 0.0) liveCell++;
				}
			}

			int idx = y*gx + x;
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
void initCellLife(float dst[], int gy, int gx){
	for (int y = 0; y < gy; y++){
		for (int x = 0; x < gx; x++){
			dst[y*gx + x] = (rand() >(RAND_MAX / 2)) ? 1.0f : 0.0f;
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
#if 0
	const int gx = 32;	//glid x num
	const int gy = 32;	//glid y num
#elif 0
	const int gx = 64;	//glid x num
	const int gy = 48;	//glid y num
#elif 0
	const int gx = 128;	//glid x num
	const int gy = 96;	//glid y num
#elif 1
	const int gx = 256;	//glid x num
	const int gy = 192;	//glid y num
#endif
	vector<vec3> instancePosArray;
	{
		for (int i = 0; i<gy; i++){
			for (int j = 0; j<gx; j++){
				float ix = (j - (gx - 1) / 2.0f) / (float)gx;
				float iy = (i - (gy - 1) / 2.0f) / (float)gx;
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

	//モデル生成
//	GModel* model = new GModelSphere(8, 8, 0.45f);
	GModel* model = new GModelSphere(8, 8, 1/(float)gx/2.0f);

//	GModel* model = new GModelCube(0.8f);

	//Cell生成 (2bank)
	float cell[2][gx*gy];
	int cellBank = 0;

	//Cell初期化(ランダム)
	initCellLife(&cell[cellBank][0], gy, gx);



	vec3 lightDirection(15.0, 15.0, 15.0);		//平行光源の位置


	ctrl.setCameraPosition(vec3(0.0, 0.0, 2.0));	//カメラ初期位置


	long framecount = 0;
	do{
		ctrl.computeMatricesFromInputs();	//コントロールのupdate


		// Clear the screen
		glClearColor(0.0f, 0.0f, 0.4f, 1.0f);
		glClearDepth(1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		// Use our shader
		glUseProgram(programID);

		vec3 eyePosition = ctrl.getCameraPosition();

#if 1
		for (int i = 0; i < (int)instancePosArray.size(); i++){
			if (cell[cellBank][i] == 0.0) continue;

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
		if(framecount%1000){
			//update cell state
			updateCellLife(&cell[cellBank][0], &cell[cellBank ^ 1][0], gy, gx);
			cellBank = cellBank ^ 1;
		}
		else{
			initCellLife(&cell[cellBank][0], gy, gx);
		}




		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
		framecount++;
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

