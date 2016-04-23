#include "stdafx.h"


#include "glslFft.h"
#include "../common/shader.hpp"
#include "Timer.hpp"

#define _USE_MATH_DEFINES
#include <math.h>


//-----------------------------------------------------------------------------
//global 
static GLFWwindow* window = 0;
static glslFftShader shader = { 0 };

//-----------------------------------------------------------------------------
// GLFWでエラーとなったときに呼び出される関数
static void glfw_error_callback_func(int error, const char* description){
	std::cout << "GLFW Error: " << description << std::endl;
}

//---------------------------------------------------------------------------
//value is power of 2
static bool IsPow2(unsigned int x){
	return (((x)&(x - 1)) == 0);
}


//---------------------------------------------------------------------------
//
static void glslFftProcess(
	const glslFftShader& shader,	//progmra ID
	const vector<GLuint>& texSrc,	//src texture IDs
	const vector<GLuint>& texDst,	//dst texture IDs
	const GLuint texW,				//twidle texture
	const int fft_stride,			//horizontal:1 vertical:width
	const int fft_p,				//p 
	const int fft_q,				//q
	const int fft_N,				//N
	const int width,				//texture size
	const int height				//texture size
)
{
//	Timer tmr("glslFftProcess:\t");
	//program
	{
		glUseProgram(shader.program);
	}


	//uniform
	{
		glUniform1i(shader.fft_stride, fft_stride);
		glUniform1i(shader.fft_p, fft_p);
		glUniform1i(shader.fft_q, fft_q);
		glUniform1i(shader.fft_N, fft_N);
	}


	//Bind Texture
	{
		int id = 0;
		for (int i = 0; i < texSrc.size(); i++, id++){
			glActiveTexture(GL_TEXTURE0 + id);
			glBindTexture(GL_TEXTURE_RECTANGLE, texSrc[i]);
			glUniform1i(shader.texSrc[i], id);
		}

		{
			glActiveTexture(GL_TEXTURE0 + id);
			glBindTexture(GL_TEXTURE_RECTANGLE, texW);
			glUniform1i(shader.texW, id);
			id++;
		}
	}


	//dst texture
	{
		for (int i = 0; i < texDst.size(); i++){
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_RECTANGLE, texDst[i], 0);
		}
		assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

		GLenum bufs[] =
		{
			GL_COLOR_ATTACHMENT0,
			GL_COLOR_ATTACHMENT1,
		};
		glDrawBuffers(2, bufs);
	}
	

	//Viewport
	{
		glViewport(0, 0, width, height);
	}

	//Render!!
	{
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		glFlush();
	}

//	glFinish();



}




//=============================================================================
//-----------------------------------------------------------------------------
//Initialtize glslFft
void glslFftInit(void){
	glfwSetErrorCallback(glfw_error_callback_func);


	// Initialise GLFW
	if (!glfwInit())
	{
		cerr<< "Failed to initialize GLFW" << endl;
		assert(0);
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
	window = glfwCreateWindow(1, 1, "GLSL FFT", NULL, NULL);
	if (window == NULL){
		cerr << "Failed to open GLFW window." << endl;
		glfwTerminate();
		assert(0);
	}
	glfwMakeContextCurrent(window);

#if defined _WIN32
	// Initialize GLEW
	glewExperimental = GL_TRUE;			///!!!! important for core profile // コアプロファイルで必要となります
	if (glewInit() != GLEW_OK) {
		cerr << "Failed to initialize GLEW." << endl;
		glfwTerminate();
		assert(0);
	}
#endif


	{
		cout << "GL_VENDOR:" << glGetString(GL_VENDOR) << endl;
		cout << "GL_RENDERER:" << glGetString(GL_RENDERER) << endl;
		cout << "GL_VERSION:" << glGetString(GL_VERSION) << endl;
		cout << "GL_SHADING_LANGUAGE_VERSION:" << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

	}

	// Create and compile our GLSL program from the shaders
	shader.program = LoadShaders("Fft.vertexshader", "Fft.fragmentshader");
	shader.position = glGetAttribLocation(shader.program, "position");
	shader.texSrc[0] = glGetUniformLocation(shader.program, "texSrc0");
	shader.texSrc[1] = glGetUniformLocation(shader.program, "texSrc1");
	shader.texW = glGetUniformLocation(shader.program, "texW");
	shader.fft_p = glGetUniformLocation(shader.program, "fft_p");
	shader.fft_q = glGetUniformLocation(shader.program, "fft_q");
	shader.fft_stride = glGetUniformLocation(shader.program, "fft_stride");
	shader.fft_N = glGetUniformLocation(shader.program, "fft_N");

}

//-----------------------------------------------------------------------------
//Terminate glslFft
void glslFftTerminate(void){
	glDeleteProgram(shader.program);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();
}

//-----------------------------------------------------------------------------
//
void glslFft(const Mat& src, Mat& dst){
	CV_Assert(src.type() == CV_32FC2);
	CV_Assert(src.cols == src.rows);

	int N = src.cols;
	CV_Assert(IsPow2(N));


	//FBO 
	GLuint fbo = 0;

	//---------------------------------
	// FBO
	// create FBO (off-screen framebuffer)
	glGenFramebuffers(1, &fbo);

	// bind offscreen framebuffer (that is, skip the window-specific render target)
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	//---------------------------------
	// vbo
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
	glEnableVertexAttribArray(shader.position);	//enable attribute Location
	glVertexAttribPointer(
		shader.position,			// attribute 0. No particular reason for 0, but must match the layout in the shader.
		2,					// size	(Specifies the number of components) x,y
		GL_FLOAT,			// type
		GL_FALSE,			// normalized?
		0,					// stride (Specifies the byte offset between consecutive generic vertex attributes)
		(void*)0			// array buffer offset (Specifies a pointer to the first generic vertex attribute in the array)
		);



	// texture
	// 1/2,1/2 size
	const int width = src.cols / 2;
	const int height = src.rows / 2;

	GLenum format = GL_RG;
	GLenum type = GL_FLOAT;
	GLenum internalFormat = GL_RG32F;
	unsigned int texid[2 * 2 * 2] = { 0 };	//src dst 2bank * (2*2)
	unsigned int texW =  0 ;	//twidle

	//---------------------------------
	// CreateTexture
	{
		//Timer tmr("CreateTexture:\t");
		glGenTextures(sizeof(texid)/sizeof(texid[0]), texid); // create (reference to) a new texture

		for (int i = 0; i < sizeof(texid) / sizeof(texid[0]); i++){
			glBindTexture(GL_TEXTURE_RECTANGLE, texid[i]);
			// (set texture parameters here)
			glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

			//create the texture
			glTexImage2D(GL_TEXTURE_RECTANGLE, 0, internalFormat, width, height, 0, format, type, 0);

			glBindTexture(GL_TEXTURE_RECTANGLE, 0);
		}

		//twidle texture
		{
			glGenTextures(1, &texW); // create (reference to) a new texture
			glBindTexture(GL_TEXTURE_RECTANGLE, texW);
			// (set texture parameters here)
			glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

			//create the texture
			glTexImage2D(GL_TEXTURE_RECTANGLE, 0, internalFormat, width, 1, 0, format, type, 0);

			glBindTexture(GL_TEXTURE_RECTANGLE, 0);
		}

	}

	//upload src to texture
	{
		Timer tmr("-upload :\t");
#if 0
		for (int i = 0; i < 4; i++){
			int x = (i % 2) * width;
			int y = (i / 2) * height;
			Rect rect(x, y, width, height);
			Mat roi = Mat(src, rect).clone();	// 1/2  1/2 rect
			CV_Assert(roi.isContinuous());
			void* data = roi.data;

			glBindTexture(GL_TEXTURE_RECTANGLE, texid[i]);
			glTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, 0, 0, width, height, format, type, data);
			glBindTexture(GL_TEXTURE_RECTANGLE, 0);
		}

		{
			vector<vec2> w(N / 2);
			// --- twidle ----
			for (int n = 0; n < N / 2; n++){
				float jw = (float)(-2 * M_PI * n / N);
				w[n][0] = cos(jw);
				w[n][1] = sin(jw);
			}
			void* data = &w[0];

			glBindTexture(GL_TEXTURE_RECTANGLE, texW);
			glTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, 0, 0, width, 1, format, type, data);
			glBindTexture(GL_TEXTURE_RECTANGLE, 0);
		}
#else
		int size = width*height * 2 * 4;
		GLuint pbo[2];
		glGenBuffers(2, pbo);
		for (int i = 0; i < 2; i++){
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo[i]);
			glBufferData(GL_PIXEL_UNPACK_BUFFER, size , 0, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
		}

		for (int i = 0 ,bank = 0; i < 4; i++ , bank = bank^1){
			int x = (i % 2) * width;
			int y = (i / 2) * height;
			Rect rect(x, y, width, height);
			Mat roi = Mat(src, rect);	// 1/2  1/2 rect

			//bind current pbo for app->pbo transfer
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo[bank]); //bind pbo
			GLubyte* ptr = (GLubyte*)glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, size,
				GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
			assert(ptr != 0);

			int lSize = roi.cols * roi.elemSize();	// line size in byte
#ifdef _OPENMP
#pragma omp parallel for
#endif
			for (int y = 0; y < roi.rows; y++){
				uchar* pSrc = roi.ptr<uchar>(y);
				uchar* pDst = ptr + lSize * y;
				memcpy(pDst, pSrc, lSize);  
			}
			glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);			//Copy pixels from pbo to texture object
			glBindTexture(GL_TEXTURE_RECTANGLE, texid[i]);
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo[bank]); //bind pbo
			glTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, 0, 0, width, height, format, type, 0);
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0); 
			glBindTexture(GL_TEXTURE_RECTANGLE, 0);

		}

		{
			vector<vec2> w(N / 2);
			// --- twidle ----
			for (int n = 0; n < N / 2; n++){
				float jw = (float)(-2 * M_PI * n / N);
				w[n][0] = cos(jw);
				w[n][1] = sin(jw);
			}
			void* data = &w[0];

			glBindTexture(GL_TEXTURE_RECTANGLE, texW);
			glTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, 0, 0, width, 1, format, type, data);
			glBindTexture(GL_TEXTURE_RECTANGLE, 0);
		}

		glDeleteBuffers(2, pbo);

#endif
	}


	//Execute
	int bank = 0;

	{
		Timer tmr("-execute:\t");

		int Q = 0;
		while ((1 << Q) < N){ Q++; }

		vector<GLuint> texSrc(2);
		vector<GLuint> texDst(2);

		// --- FFT rows ----
		for (int i = 0; i < 2; i++){
			int bank = 0;
			for (int p = 0, q = Q - 1; q >= 0; p++, q--, bank = bank ^ 1) {
				for (int j = 0; j < 2; j++){
					texSrc[j] = texid[bank * 4 + i * 2 + j];
					texDst[j] = texid[(bank ^ 1) * 4 + i * 2 + j];
				}
				glslFftProcess(shader, texSrc, texDst, texW,1, p, q, N ,width, height);
			}
		}
		// --- FFT cols ----
		for (int j = 0; j < 2; j++){
			int bank = (Q & 1);	//FFT rows の回数が奇数ならば奇数bankから
			for (int p = 0, q = Q - 1; q >= 0; p++, q--, bank = bank ^ 1) {
				for (int i = 0; i < 2; i++){
					texSrc[i] = texid[bank * 4 + i * 2 + j];
					texDst[i] = texid[(bank ^ 1) * 4 + i * 2 + j];
				}
				glslFftProcess(shader, texSrc, texDst, texW, width, p, q, N, width, height);
			}
		}
	}

	dst = Mat(src.size(), src.type());
	{	//download from texture
		Timer tmr("-download:\t");
#if 0
		Mat tmp = Mat(Size(width, height), src.type());
		for (int i = 0; i < 4; i++){
			void* data = tmp.data;

			glBindTexture(GL_TEXTURE_RECTANGLE, texid[bank*4+i]);
			glGetTexImage(GL_TEXTURE_RECTANGLE, 0, format, type, data);
			glBindTexture(GL_TEXTURE_RECTANGLE, 0);

			int x = (i % 2) * width;
			int y = (i / 2) * height;
			Rect rect(x, y, width, height);
			Mat roi = Mat(dst, rect);	// 1/2  1/2 rect
			tmp.copyTo(roi);
		}
#else
		int size = width*height * 2 * 4;
		GLuint pbo[2];
		glGenBuffers(2, pbo);
		for (int i = 0; i < 2; i++){
			glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo[i]);
			glBufferData(GL_PIXEL_PACK_BUFFER, size, 0, GL_DYNAMIC_READ);
			glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
		}

		for (int i = 0, pbo_bank = 0; i < 4; i++, pbo_bank = bank ^ 1){
			int x = (i % 2) * width;
			int y = (i / 2) * height;
			Rect rect(x, y, width, height);
			Mat roi = Mat(dst, rect);	// 1/2  1/2 rect

			//Copy pixels from texture object to pbo_bank
			glBindTexture(GL_TEXTURE_RECTANGLE, texid[bank*4 + i]);
			glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo[pbo_bank]); //bind pbo
			glGetTexImage(GL_TEXTURE_RECTANGLE, 0, format, type, 0);
			glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
			glBindTexture(GL_TEXTURE_RECTANGLE, 0);

			//bind current pbo for app->pbo transfer
			glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo[pbo_bank]); //bind pbo
			GLubyte* ptr = (GLubyte*)glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, size,
				GL_MAP_READ_BIT);
			assert(ptr!=0);

			int lSize = roi.cols * roi.elemSize();	// line size in byte
#ifdef _OPENMP
#pragma omp parallel for
#endif
			for (int y = 0; y < roi.rows; y++){
				uchar* pSrc = ptr + lSize * y;
				uchar* pDst = roi.ptr<uchar>(y);
				memcpy(pDst, pSrc, lSize);
			}
			glUnmapBuffer(GL_PIXEL_PACK_BUFFER);		}
		glDeleteBuffers(2, pbo);


#endif
	}

	//clean up
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);

	glDeleteFramebuffers(1, &fbo);
	glDeleteTextures(sizeof(texid) / sizeof(texid[0]), texid);
	glDeleteTextures(1 , &texW);


}





