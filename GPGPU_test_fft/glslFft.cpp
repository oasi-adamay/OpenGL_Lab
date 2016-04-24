#include "stdafx.h"


#include "glslFft.h"
#include "../common/shader.hpp"
#include "Timer.hpp"

#define _USE_MATH_DEFINES
#include <math.h>


#define _USE_PBO_UP
//#define _USE_PBO_DOWN

//-----------------------------------------------------------------------------
//glslFftShader
glslFftShader::glslFftShader(void)
	:glslBaseShader()
{
	// Create and compile our GLSL program from the shaders
	program = LoadShaders("Fft_vs.glsl", "FftRadix2_fs.glsl");

	// Attribute & Uniform location
	position = glGetAttribLocation(program, "position");
	texSrc[0] = glGetUniformLocation(program, "texSrc0");
	texSrc[1] = glGetUniformLocation(program, "texSrc1");
	texW = glGetUniformLocation(program, "texW");
	i_p = glGetUniformLocation(program, "i_p");
	i_q = glGetUniformLocation(program, "i_q");
	i_N = glGetUniformLocation(program, "i_N");
	i_flag = glGetUniformLocation(program, "i_flag");
	f_xscl = glGetUniformLocation(program, "f_xscl");
	f_yscl = glGetUniformLocation(program, "f_yscl");
	f_xconj = glGetUniformLocation(program, "f_xconj");
	f_yconj = glGetUniformLocation(program, "f_yconj");
}

glslConjShader::glslConjShader(void)
	:glslBaseShader()
{
	// Create and compile our GLSL program from the shaders
	program = LoadShaders("Fft_vs.glsl", "Conj_fs.glsl");

	// Attribute & Uniform location
	position = glGetAttribLocation(program, "position");

	texSrc   = glGetUniformLocation(program, "texSrc");

}




//-----------------------------------------------------------------------------
//global 
static GLFWwindow* window = 0;
static glslFftShader* shaderFft =  0;
static glslConjShader* shaderConj = 0;

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


static Size getTextureSize(GLuint tex){
	int width;
	int height;

	//get texture size

	glBindTexture(GL_TEXTURE_RECTANGLE, tex);
	glGetTexLevelParameteriv(
		GL_TEXTURE_RECTANGLE, 0,
		GL_TEXTURE_WIDTH, &width
		);

	glGetTexLevelParameteriv(
		GL_TEXTURE_RECTANGLE, 0,
		GL_TEXTURE_HEIGHT, &height
		);

	glBindTexture(GL_TEXTURE_RECTANGLE, 0);

	return Size(width, height);
}






//---------------------------------------------------------------------------
//
static void glslFftProcess(
	const glslFftShader* shader,	//progmra ID
	const vector<GLuint>& texSrc,	//src texture IDs
	const vector<GLuint>& texDst,	//dst texture IDs
	const GLuint texW,				//twidle texture
	const int flag,					//bit0 : horizontal:0 vertical:1
	const int p,					//p 
	const int q,					//q
	const int N,					//N
	const float xscl,				//xscl
	const float yscl,				//yscl
	const float xconj,				//xconj
	const float yconj				//yconj
)
{
	Size size = getTextureSize(texSrc[0]);
	int width = size.width;
	int height = size.height;

//	Timer tmr("glslFftProcess:\t");
	//program
	{
		glUseProgram(shader->program);
	}


	//uniform
	{
		glUniform1i(shader->i_flag, flag);
		glUniform1i(shader->i_p, p);
		glUniform1i(shader->i_q, q);
		glUniform1i(shader->i_N, N);
		glUniform1f(shader->f_xscl, xscl);
		glUniform1f(shader->f_yscl, yscl);
		glUniform1f(shader->f_xconj, xconj);
		glUniform1f(shader->f_yconj, yconj);
	}


	//Bind Texture
	{
		int id = 0;
		for (int i = 0; i < texSrc.size(); i++, id++){
			glActiveTexture(GL_TEXTURE0 + id);
			glBindTexture(GL_TEXTURE_RECTANGLE, texSrc[i]);
			glUniform1i(shader->texSrc[i], id);
		}

		{
			glActiveTexture(GL_TEXTURE0 + id);
			glBindTexture(GL_TEXTURE_RECTANGLE, texW);
			glUniform1i(shader->texW, id);
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


static void glslConjProcess(
	const glslConjShader* shader,	//progmra ID
	const vector<GLuint>& texSrc,	//src texture IDs
	const vector<GLuint>& texDst	//dst texture IDs
	)
{
	Size size = getTextureSize(texSrc[0]);
	int width = size.width;
	int height = size.height;

	//program
	{
		glUseProgram(shader->program);
	}

	//uniform
	{
	}

	for (int i = 0; i < texSrc.size(); i++){
		//Bind Texture
		{
			int id = 0;
			glActiveTexture(GL_TEXTURE0 + id);
			glBindTexture(GL_TEXTURE_RECTANGLE, texSrc[i]);
			glUniform1i(shader->texSrc, id);
		}
		//dst texture
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, texDst[i], 0);
			assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
		}

		GLenum bufs[] =
		{
			GL_COLOR_ATTACHMENT0,
		};
		glDrawBuffers(1, bufs);

		//Viewport
		{
			glViewport(0, 0, width, height);
		}

		//Render!!
		{
			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
			glFlush();
		}
	}
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

	shaderFft = new glslFftShader();
	shaderConj = new glslConjShader();

}

//-----------------------------------------------------------------------------
//Terminate glslFft
void glslFftTerminate(void){

	delete shaderFft;
	delete shaderConj;

	// Close OpenGL window and terminate GLFW
	glfwTerminate();
}


//-----------------------------------------------------------------------------
//Upload texture from cv::Mat to GL texture
void glslFftUploadTexture(const Mat&src, const vector<GLuint>& texArray){
	CV_Assert(src.type() == CV_32FC2);
	CV_Assert(src.cols == src.rows);
	assert(texArray.size() == 4);

	// texture
	const int width = src.cols/2;
	const int height = src.rows/ 2;
	GLenum format = GL_RG;
	GLenum type = GL_FLOAT;

	{
		Timer tmr("-upload :\t");
#ifdef _USE_PBO_UP
		int size = width*height * (int)src.elemSize();
		GLuint pbo[4];
		glGenBuffers(4, pbo);
		for (int i = 0; i < 4; i++){
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo[i]);
			glBufferData(GL_PIXEL_UNPACK_BUFFER, size, 0, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
		}

		for (int i = 0; i < 4; i++){
			int x = (i % 2) * width;
			int y = (i / 2) * height;
			Rect rect(x, y, width, height);
			Mat roi = Mat(src, rect);	// 1/2  1/2 rect

			//bind current pbo for app->pbo transfer
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo[i]); //bind pbo
			GLubyte* ptr = (GLubyte*)glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, size,
				GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
			assert(ptr != 0);

			int lSize = roi.cols * (int)roi.elemSize();	// line size in byte
#ifdef _OPENMP
#pragma omp parallel for
#endif
			for (int y = 0; y < roi.rows; y++){
				uchar* pSrc = roi.ptr<uchar>(y);
				uchar* pDst = ptr + lSize * y;
				memcpy(pDst, pSrc, lSize);
			}
			glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);			//Copy pixels from pbo to texture object
			glBindTexture(GL_TEXTURE_RECTANGLE, texArray[i]);
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo[i]); //bind pbo
			glTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, 0, 0, width, height, format, type, 0);
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
			glBindTexture(GL_TEXTURE_RECTANGLE, 0);

		}

		glDeleteBuffers(4, pbo);
#else
		for (int i = 0; i < 4; i++){
			int x = (i % 2) * width;
			int y = (i / 2) * height;
			Rect rect(x, y, width, height);
			Mat roi = Mat(src, rect).clone();	// 1/2  1/2 rect
			CV_Assert(roi.isContinuous());
			void* data = roi.data;

			glBindTexture(GL_TEXTURE_RECTANGLE, texArray[i]);
			glTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, 0, 0, width, height, format, type, data);
			glBindTexture(GL_TEXTURE_RECTANGLE, 0);
		}



#endif
	}

}

//-----------------------------------------------------------------------------
//Download texture from GL textures to cv::Mat
void glslFftDownloadTexture(const vector<GLuint>& texArray, Mat&dst)
{
	assert(texArray.size() == 4);
	Size size = getTextureSize(texArray[0]);
	dst = Mat(size*2, CV_32FC2);

	// texture
	const int width = size.width;
	const int height = size.height;
	GLenum format = GL_RG;
	GLenum type = GL_FLOAT;

	{	//download from texture
		Timer tmr("-download:\t");
#ifdef _USE_PBO_DOWN
		int size = width*height * dst.elemSize();
		GLuint pbo[4];
		glGenBuffers(4, pbo);
		for (int i = 0; i < 4; i++){
			glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo[i]);
			glBufferData(GL_PIXEL_PACK_BUFFER, size, 0, GL_DYNAMIC_READ);
			glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
		}

		for (int i = 0; i < 4; i++){
			int x = (i % 2) * width;
			int y = (i / 2) * height;
			Rect rect(x, y, width, height);
			Mat roi = Mat(dst, rect);	// 1/2  1/2 rect

			//Copy pixels from texture object to pbo_bank
			glBindTexture(GL_TEXTURE_RECTANGLE, texArray[i]);
			glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo[i]); //bind pbo
			glGetTexImage(GL_TEXTURE_RECTANGLE, 0, format, type, 0);
			glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
			glBindTexture(GL_TEXTURE_RECTANGLE, 0);

			//bind current pbo for app->pbo transfer
			glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo[i]); //bind pbo
			GLubyte* ptr = (GLubyte*)glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, size,
				GL_MAP_READ_BIT);
			assert(ptr != 0);

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
#else
		Mat tmp = Mat(Size(width, height), dst.type());
		for (int i = 0; i < 4; i++){
			void* data = tmp.data;

			glBindTexture(GL_TEXTURE_RECTANGLE, texArray[i]);
			glGetTexImage(GL_TEXTURE_RECTANGLE, 0, format, type, data);
			glBindTexture(GL_TEXTURE_RECTANGLE, 0);

			int x = (i % 2) * width;
			int y = (i / 2) * height;
			Rect rect(x, y, width, height);
			Mat roi = Mat(dst, rect);	// 1/2  1/2 rect
			tmp.copyTo(roi);
		}


#endif
	}


}



//-----------------------------------------------------------------------------
// execute FFT 
void glslFft(vector<GLuint>& texArray,int flag){
	assert(texArray.size() == 4);

	Size texSize = 	getTextureSize(texArray[0]);
	int N = texSize.width * 2;
	assert(IsPow2(N));


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
	glEnableVertexAttribArray(shaderFft->position);	//enable attribute Location
	glVertexAttribPointer(
		shaderFft->position,	// attribute location.
		2,					// size	(Specifies the number of components) x,y
		GL_FLOAT,			// type
		GL_FALSE,			// normalized?
		0,					// stride (Specifies the byte offset between consecutive generic vertex attributes)
		(void*)0			// array buffer offset (Specifies a pointer to the first generic vertex attribute in the array)
		);



	// texture
	const int width = texSize.width;
	const int height = texSize.height;

	GLenum format = GL_RG;
	GLenum type = GL_FLOAT;
	GLenum internalFormat = GL_RG32F;
	vector<GLuint> texTmp(texArray.size());
	GLuint texW = 0;	//twidle

	//---------------------------------
	// CreateTexture
	{
		//Timer tmr("CreateTexture:\t");
		glGenTextures((GLsizei)texTmp.size(), &texTmp[0]); // create (reference to) a new texture

		for (int i = 0; i < texTmp.size(); i++){
			glBindTexture(GL_TEXTURE_RECTANGLE, texTmp[i]);
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

	//---------------------------------
	// upload teidle texture
	{
		vector<vec2> w(N / 2);
		// --- twidle ----
		//#ifdef _OPENMP
		//#pragma omp parallel for
		//#endif
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


	//texidにセット
	vector<vector<GLuint>> texid(2);
	texid[0].resize(texArray.size());
	texid[1].resize(texTmp.size());
	for (int i = 0; i < (int)texArray.size(); i++){
		texid[0][i] = texArray[i];
	}
	for (int i = 0; i < (int)texTmp.size(); i++){
		texid[1][i] = texTmp[i];
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
		for (int p = 0, q = Q - 1; q >= 0; p++, q--, bank = bank ^ 1) {
			for (int i = 0; i < 2; i++){
				for (int j = 0; j < 2; j++){
					texSrc[j] = texid[bank    ][i * 2 + j];
					texDst[j] = texid[bank ^ 1][i * 2 + j];
				}
				float xscl = ((flag & GLSL_FFT_SCALE) && (flag & GLSL_FFT_INVERSE) && (p == 0)) ? 1.0f / (float)N : 1.0f;
				float yscl = ((flag & GLSL_FFT_SCALE) && !(flag & GLSL_FFT_INVERSE) && (q == 0)) ? 1.0f / (float)N : 1.0f;
				float xconj = ((flag & GLSL_FFT_INVERSE) && (p == 0)) ? -1.0f : 1.0f;
				float yconj = 1.0f;
				glslFftProcess(shaderFft, texSrc, texDst, texW, 0, p, q, N, xscl, yscl, xconj,yconj);
			}
		}
		// --- FFT cols ----
		for (int p = 0, q = Q - 1; q >= 0; p++, q--, bank = bank ^ 1) {
			for (int j = 0; j < 2; j++){
				for (int i = 0; i < 2; i++){
					texSrc[i] = texid[bank    ][i * 2 + j];
					texDst[i] = texid[bank ^ 1][i * 2 + j];
				}
				float yscl = ((flag & GLSL_FFT_SCALE) && !(flag & GLSL_FFT_INVERSE) && (q == 0)) ? 1.0f / (float)N : 1.0f;
				float xscl = ((flag & GLSL_FFT_SCALE) && (flag & GLSL_FFT_INVERSE) && (p == 0)) ? 1.0f / (float)N : 1.0f;
				float xconj = 1.0f;
				float yconj = ((flag & GLSL_FFT_INVERSE) && (q == 0)) ? -1.0f : 1.0f;
				glslFftProcess(shaderFft, texSrc, texDst, texW, 1, p, q, N, xscl, yscl, xconj, yconj);
			}
		}
	}

	//clean up
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);

	glDeleteFramebuffers(1, &fbo);
	glDeleteTextures((GLsizei)texTmp.size(), &texTmp[0]);
	glDeleteTextures(1, &texW);

}


void glslFft(const Mat& src, Mat& dst, int flag){
	CV_Assert(src.type() == CV_32FC2);
	CV_Assert(src.cols == src.rows);

	int N = src.cols;
	CV_Assert(IsPow2(N));

	const int width = src.cols / 2;
	const int height = src.rows / 2;

	GLenum format = GL_RG;
	GLenum type = GL_FLOAT;
	GLenum internalFormat = GL_RG32F;

	vector<GLuint> texArray(4);
	//---------------------------------
	// CreateTexture
	{
		//Timer tmr("CreateTexture:\t");
		glGenTextures((GLsizei)texArray.size(), &texArray[0]); // create (reference to) a new texture

		for (int i = 0; i < (int)texArray.size(); i++){
			glBindTexture(GL_TEXTURE_RECTANGLE, texArray[i]);
			// (set texture parameters here)
			glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

			//create the texture
			glTexImage2D(GL_TEXTURE_RECTANGLE, 0, internalFormat, width, height, 0, format, type, 0);

			glBindTexture(GL_TEXTURE_RECTANGLE, 0);
		}
	}

	//---------------------------------
	//upload
	glslFftUploadTexture(src, texArray);

	//---------------------------------
	//fft
	glslFft(texArray, flag);

	//---------------------------------
	//download
	glslFftDownloadTexture(texArray,dst);

	//---------------------------------
	//clean up
	glDeleteTextures((GLsizei)texArray.size(), &texArray[0]);

}




