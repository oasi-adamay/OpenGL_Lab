// GPGPU_test.cpp : �R���\�[�� �A�v���P�[�V�����̃G���g�� �|�C���g���`���܂��B
//

#include "stdafx.h"

#include "../common/shader.hpp"


//Lib 
#pragma comment (lib, "opengl32.lib")
#pragma comment (lib, "glew32.lib")
#pragma comment (lib, "glfw3dll.lib")

//-----------------------------------------------------------------------------
//#define LIFE_BOUND_REPEAT



class Timer{
private:
	cv::TickMeter meter;
	string msg;
public:
	Timer(string _msg){ msg = _msg;  meter.start(); }
	~Timer(void){ meter.stop(); std::cout <<msg << meter.getTimeMilli() << "ms" << std::endl; }

};


//-----------------------------------------------------------------------------
// GLFW�ŃG���[�ƂȂ����Ƃ��ɌĂяo�����֐�
void glfw_error_callback_func(int error, const char* description){
	std::cout << "GLFW Error: " << description << std::endl;
}


//��ɐ��̐���Ԃ�mod
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


#ifdef _OPENMP
#pragma omp parallel for
#endif
	for (int y = 0; y < height; y++){
		const float* src = imgSrc.ptr<float>(y);
		float* dst = imgDst.ptr<float>(y);

		for (int x = 0; x < width; x++){
			int liveCell = 0;
			for (int dy = -1; dy <= 1; dy++){
				for (int dx = -1; dx <= 1; dx++){
					if (dx == 0 && dy == 0) continue;//�����͐����Ȃ�
					int _x = x+dx;
					int _y = y+dy;


#ifdef LIFE_BOUND_REPEAT					
					_x = IMOD(_x , width );
					_y = IMOD(_y , height);

#else
					//���E
					if (_x < 0)continue;
					if (_x >= width)continue;
					if (_y < 0)continue;
					if (_y >= height)continue;
#endif

//					int idx = _y*width + _x;
					int idx = (_y-y)*width + _x;
					if (src[idx] != 0.0) liveCell++;

				}
			}

//			int idx = (y*width + x);
			int idx = (x);

			//�a��:����ł���Z���ɗאڂ��鐶�����Z�������傤��3����΁A���̐��オ�a������B
			if (src[idx] == 0.0 && liveCell == 3) dst[idx] = 1.0;
			//����:�����Ă���Z���ɗאڂ��鐶�����Z����2��3�Ȃ�΁A���̐���ł���������B
			else if (src[idx] != 0.0 && (liveCell == 2 || liveCell == 3)) dst[idx] = 1.0;
			//�ߑa:	�����Ă���Z���ɗאڂ��鐶�����Z����1�ȉ��Ȃ�΁A�ߑa�ɂ�莀�ł���B
			else if (src[idx] != 0.0 && (liveCell <= 1)) dst[idx] = 0.0;
			//�ߖ�:	�����Ă���Z���ɗאڂ��鐶�����Z����4�ȏ�Ȃ�΁A�ߖ��ɂ�莀�ł���B
			else if (src[idx] != 0.0 && (liveCell >= 4)) dst[idx] = 0.0;
			//����ێ�
			else dst[idx] = src[idx];
		}
	}
}

//Cell������(�����_��)
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


void executeGpGpuProcess(
	const GLuint pid,				//progmra ID
	GLuint _fbo,					//FBO 0�̏ꍇ�A�����Ő����A�j�� ���readPixel�œǂ݂����Ȃ�
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
	if (_fbo==0)	glGenFramebuffers(1, &fbo);

	// bind offscreen framebuffer (that is, skip the window-specific render target)
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	GLuint vao = 0;
	GLuint vbo = 0;

	// [-1, 1] �̐����`
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

	glfwWindowHint(GLFW_VISIBLE, 0);	//�I�t�X�N���[��

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
	glewExperimental = GL_TRUE;			///!!!! important for core profile // �R�A�v���t�@�C���ŕK�v�ƂȂ�܂�
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

#ifdef _DEBUG
	Mat imgSrc = Mat(Size(8, 4), CV_32FC1);
#else
	Mat imgSrc = Mat(Size(1024, 1024), CV_32FC1);
#endif
	Mat imgDst = Mat::zeros(imgSrc.size(), imgSrc.type());
	Mat imgRef = Mat::zeros(imgSrc.size(), imgSrc.type());

	//����
#ifdef _DEBUG
	const int generations = 1;
//	const int generations = 3;
#else
	const int generations = 1000;
#endif
	{
		cout << "Cell Size:" << imgSrc.size() << endl;
		cout << "generations:" << generations << endl;
	}

	//---------------------------------
	//init Src image
	initCellLife(imgSrc);



	//---------------------------------
	//Execute GPGPU
	{
		cout << "Execute GPGPU" << endl;

		const int width = imgSrc.cols;
		const int height = imgSrc.rows;


		// Create and compile our GLSL program from the shaders
		GLuint programID = LoadShaders("LifeGame.vertexshader", "LifeGameUpdate.fragmentshader");

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
			//Timer tmr("upload:");

			GLenum format = GL_RED;				//single channel
			GLenum type = GL_FLOAT;				//float
			void* data = imgSrc.data;

			glBindTexture(GL_TEXTURE_2D, textureID[E_TextureID::SRC]);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format, type, data);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		// FBO identifier
		GLuint fbo = 0;

		//---------------------------------
		// FBO
		// create FBO (off-screen framebuffer)
		glGenFramebuffers(1, &fbo);

		// bind offscreen framebuffer (that is, skip the window-specific render target)
		//		glBindFramebuffer(GL_FRAMEBUFFER, fbo);


		//Execute
		{
			Timer tmr("LifeGame@gpu:");
			for (int i = 0; i < generations; i++){
				GLuint texSrc = textureID[(i % 2)];
				GLuint texDst = textureID[(i % 2) ^ 1];
				executeGpGpuProcess(programID, fbo, texSrc, texDst);
			}
		}

		{	//download from framebuffer
			//Timer tmr("download:");


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
		glDeleteTextures(sizeof(textureID) / sizeof(textureID[0]), textureID);
		glDeleteFramebuffers(1, &fbo);

		glDeleteProgram(programID);
	}

	//---------------------------------
	//Execute CPU
	{
		cout << "Execute CPU" << endl;

		Mat imgBank[2] = { Mat::zeros(imgSrc.size(), imgSrc.type()), Mat::zeros(imgSrc.size(), imgSrc.type()) };
		int bank = 0;
		imgBank[bank] = imgSrc.clone();
		{
			Timer tmr("LifeGame@cpu:");
			for (int i = 0; i < generations; i++){
				updateCellLife(imgBank[bank], imgBank[bank ^ 1]);
				bank = bank ^ 1;
			}
		}
		imgRef = imgBank[bank].clone();

	}


#ifdef _DEBUG
	//dump 
	{
		cout << "imgSrc" << endl;
		cout << imgSrc << endl;

		cout << "imgDst" << endl;
		cout << imgDst << endl;

		cout << "imgRef" << endl;
		cout << imgRef << endl;
	}
#endif

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

