// GPGPU_test.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"

#include "../common/shader.hpp"



//Lib 
#pragma comment (lib, "opengl32.lib")
#pragma comment (lib, "glew32.lib")
#pragma comment (lib, "glfw3dll.lib")

//-----------------------------------------------------------------------------
// GLFWでエラーとなったときに呼び出される関数
void glfw_error_callback_func(int error, const char* description){
	std::cout << "GLFW Error: " << description << std::endl;
}

//#define LIFE_BOUND_REPEAT

//常に正の数を返すmod
#define IMOD(i,j) (((i) % (j)) < 0 ? ((i) % (j)) + ((j) < 0 ? -(j) : (j)) : (i) % (j))

//-----------------------------------------------------------------------------
//https://ja.wikipedia.org/wiki/%E3%83%A9%E3%82%A4%E3%83%95%E3%82%B2%E3%83%BC%E3%83%A0
void updateCellLife(const Mat& imgSrc, Mat& imgDst){
	CV_Assert(imgSrc.type() == CV_32FC1);
	CV_Assert(imgDst.type() == CV_32FC1);
	CV_Assert(imgSrc.size() == imgDst.size());
	CV_Assert(imgSrc.isContinuous());
	CV_Assert(imgDst.isContinuous());


	int width = imgSrc.cols;
	int height = imgSrc.rows;
	const float* src = imgSrc.ptr<float>(0);
	float* dst = imgDst.ptr<float>(0);


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
					_y = IMOD(_y , height);

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
void initCellLife(Mat& imgDst){
	CV_Assert(imgDst.type() == CV_32FC1);
	CV_Assert(imgDst.isContinuous());

	int width = imgDst.cols;
	int height = imgDst.rows;
	float* dst = imgDst.ptr<float>(0);

	for (int y = 0; y < height; y++){
		for (int x = 0; x < width; x++){
			dst[y*width + x] = (rand() >(RAND_MAX / 2)) ? 1.0f : 0.0f;
		}
	}

}




int _tmain(int argc, _TCHAR* argv[])
{

	GLFWwindow* window = 0;
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

	// GL3.3 Core profile
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_VISIBLE, 0);	//オフスクリーン

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(1, 1, "GPGPU Test", NULL, NULL);
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

//	Mat imgSrc = Mat(Size(32, 24), CV_32FC1);
	Mat imgSrc = Mat(Size(8, 4), CV_32FC1);
	Mat imgDst = Mat::zeros(imgSrc.size(), imgSrc.type());
	Mat imgRef = Mat::zeros(imgSrc.size(), imgSrc.type());

	//---------------------------------
	//init Src image
	initCellLife(imgSrc);

	//---------------------------------
	//init Ref image
	updateCellLife(imgSrc,imgRef);

	//---------------------------------
	//Execute GPGPU
	{
		const int width = imgSrc.cols;
		const int height = imgSrc.rows;


		// Create and compile our GLSL program from the shaders
		GLuint programID = LoadShaders("LifeGame.vertexshader", "LifeGameUpdate.fragmentshader");

		// FBO identifier
		GLuint fbo = 0;

		//---------------------------------
		// FBO
		// create FBO (off-screen framebuffer)
		glGenFramebuffers(1, &fbo);

		// bind offscreen framebuffer (that is, skip the window-specific render target)
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		// texture

		enum E_TextureID{
			SRC,
			DST,
			SIZEOF,
		};

		unsigned int textureID[E_TextureID::SIZEOF];	//src dst
		//---------------------------------
		// CreateTexture
		{
			GLenum format = GL_RED;				//single channel
			GLenum type = GL_FLOAT;				//float
			GLenum internalFormat = GL_R32F;	//single channel float

			glGenTextures(sizeof(textureID) / sizeof(textureID[0]), textureID); // create (reference to) a new texture

			for (int i = 0; i < sizeof(textureID) / sizeof(textureID[0]); i++){
				glBindTexture(GL_TEXTURE_2D, textureID[i]);
				// (set texture parameters here)
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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
			}

		}

		//upload imgSrc to texture
		{
			GLenum format = GL_RED;				//single channel
			GLenum type = GL_FLOAT;				//float
			void* data = imgSrc.data;

			glBindTexture(GL_TEXTURE_2D, textureID[E_TextureID::SRC]);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format, type, data);
			glBindTexture(GL_TEXTURE_2D, 0);
		}


		//Execute
		{
			glUseProgram(programID);

			GLuint vao;
			GLuint vbo;

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
			glEnableVertexAttribArray(0);	//enable attribute Location
			glVertexAttribPointer(
				0,					// attribute 0. No particular reason for 0, but must match the layout in the shader.
				2,					// size	(Specifies the number of components) x,y
				GL_FLOAT,			// type
				GL_FALSE,			// normalized?
				0,					// stride (Specifies the byte offset between consecutive generic vertex attributes)
				(void*)0			// array buffer offset (Specifies a pointer to the first generic vertex attribute in the array)
				);

			//Bind Texture & Fbo
			const int textureUnit = 0;
			glActiveTexture(GL_TEXTURE0 + textureUnit);
			glBindTexture(GL_TEXTURE_2D, textureID[E_TextureID::SRC]);
			glUniform1i(glGetUniformLocation(programID, "texSrc"), textureUnit);
			glUniform2f(glGetUniformLocation(programID, "texSrcSize"), (float)width, (float)height);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID[E_TextureID::DST], 0);

			//Viewport
			glViewport(0, 0, width, height);

			//Render!!
			glDrawArrays(GL_TRIANGLE_FAN, 0, (int)(sizeof(position) / sizeof(position[0])));

			glFlush();

			// delete vao&vbo
			glBindVertexArray(0);
			glDeleteVertexArrays(1, &vao);
			glDeleteBuffers(1, &vbo);
		}

		{	//download from framebuffer

			GLenum format = GL_RED;				//single channel
			GLenum type = GL_FLOAT;				//float
			void* data = imgDst.data;
			int width = imgDst.cols;
			int height = imgDst.rows;


			//wait for Rendering
			glFinish();

			// ReadBuffer
			glReadBuffer(GL_COLOR_ATTACHMENT0);

			// ReadPixels
			glReadPixels(0, 0, width, height, format, type, data);
		}

		//clean up
		glDeleteFramebuffers(1, &fbo);
		glDeleteTextures(sizeof(textureID) / sizeof(textureID[0]), textureID);
		glDeleteProgram(programID);
	}

	//dump 
	{
		cout << "imgSrc" << endl;
		cout << imgSrc << endl;

		cout << "imgDst" << endl;
		cout << imgDst << endl;

		cout << "imgRef" << endl;
		cout << imgRef << endl;

	}

	//verify
	int errNum = 0;
	{
		//verify
		int width = imgSrc.cols;
		int height = imgSrc.rows;
		for (int y = 0; y < height; y++){
			for (int x = 0; x < width; x++){
				float ref = imgRef.at<float>(y, x);
				float dst = imgDst.at<float>(y, x);
				if (ref != dst) errNum++;
			}
		}
		cout << "ErrNum:" << errNum << endl;
	}

#if 0
	//visualize
	{
		imshow("src", imgSrc);
		imshow("dst", imgDst);
		imshow("ref", imgRef);

		waitKey();
	}
#endif

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	cout << "Hit return key" << endl;
	cin.get();


	return errNum;
}

