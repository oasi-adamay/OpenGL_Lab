// GPGPU_test_copy.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
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

	//---------------------------------
	//init Src image
	{
		const int width = imgSrc.cols;
		const int height = imgSrc.rows;

		for (int y = 0; y < height; y++){
			for (int x = 0; x < width; x++){
				imgSrc.at<float>(y,x) = y*100.0f + x;
			}
		}
	}


	//---------------------------------
	//Execute GPGPU
	{
		const int width = imgSrc.cols;
		const int height = imgSrc.rows;


		// Create and compile our GLSL program from the shaders
		GLuint programID = LoadShaders("GpGpuVertexShader.vertexshader", "GpGpuFragmentShader.fragmentshader");

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
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

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
			GLint attrLoc = glGetAttribLocation(programID, "position");
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
			const int textureUnit = 0;
			glActiveTexture(GL_TEXTURE0 + textureUnit);
			glBindTexture(GL_TEXTURE_2D, textureID[E_TextureID::SRC]);
			glUniform1i(glGetUniformLocation(programID, "texSrc"), textureUnit);
			glUniform2f(glGetUniformLocation(programID, "texSrcSize"),width,height);
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
	}

	//verify
	int errNum = 0;
	{
		//verify
		int width = imgSrc.cols;
		int height = imgSrc.rows;
		for (int y = 0; y < height; y++){
			for (int x = 0; x < width; x++){
				float src = imgSrc.at<float>(y, x);
				float dst = imgDst.at<float>(y, x);
				if (src != dst) errNum++;
			}
		}
		cout << "ErrNum:" << errNum << endl;
	}

#if 0
	//visualize
	{
		imshow("src", imgSrc);
		imshow("dst", imgDst);
		waitKey();
	}
#endif

	// Close OpenGL window and terminate GLFW
	glfwTerminate();
	
	cout << "Hit return key" << endl;
	cin.get();

	return errNum;
}

