// GPGPU_test_copy.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "../common/shader.hpp"



//Lib 
#pragma comment (lib, "opengl32.lib")
#pragma comment (lib, "glew32.lib")
#pragma comment (lib, "glfw3dll.lib")

// Usable AlmostEqual function
bool AlmostEqual2sComplement(float A, float B, int maxUlps)
{
	// Make sure maxUlps is non-negative and small enough that the
	// default NAN won't compare as equal to anything.
	assert(maxUlps > 0 && maxUlps < 4 * 1024 * 1024);
	int aInt = *(int*)&A;
	// Make aInt lexicographically ordered as a twos-complement int
	if (aInt < 0)
		aInt = 0x80000000 - aInt;
	// Make bInt lexicographically ordered as a twos-complement int
	int bInt = *(int*)&B;
	if (bInt < 0)
		bInt = 0x80000000 - bInt;
	int intDiff = abs(aInt - bInt);
	if (intDiff <= maxUlps)
		return true;
	return false;
}

//-----------------------------------------------------------------------------
class Timer{
private:
	cv::TickMeter meter;
	string msg;
public:
	Timer(string _msg){	msg = _msg;  meter.start(); }
	~Timer(void){meter.stop(); std::cout << msg << meter.getTimeMilli() << "[ms]" << std::endl; }

};



//-----------------------------------------------------------------------------
// GLFWでエラーとなったときに呼び出される関数
void glfw_error_callback_func(int error, const char* description){
	std::cout << "GLFW Error: " << description << std::endl;
}
//-----------------------------------------------------------------------------
//
void convCvMatType2GlFormat(
	const Mat& cvmat,
	GLenum& format,
	GLenum& type
)
{
	bool isFloat = false;

	switch (cvmat.depth()){
	case(CV_8U) : type = GL_UNSIGNED_BYTE; break;
	case(CV_8S) : type = GL_BYTE; break;
	case(CV_16U) : type = GL_UNSIGNED_SHORT; break;
	case(CV_16S) : type = GL_SHORT; break;
	case(CV_32S) : type = GL_INT; break;
	case(CV_32F) : type = GL_FLOAT; break;
	default: assert(0);
	}

	if (cvmat.depth() == CV_32F || cvmat.depth() == CV_64F){
		switch (cvmat.channels()){
		case(1) : format = GL_RED; break;
		case(2) : format = GL_RG; break;
		case(3) : format = GL_RGB; break;
		case(4) : format = GL_RGBA; break;
		default: assert(0);
		}
	}
	else{
		switch (cvmat.channels()){
		case(1) : format = GL_RED_INTEGER; break;
		case(2) : format = GL_RG_INTEGER; break;
		case(3) : format = GL_RGB_INTEGER; break;
		case(4) : format = GL_RGBA_INTEGER; break;
		default: assert(0);
		}
	}
}



//-----------------------------------------------------------------------------
//Texture生成
GLuint createTexture(
	const int width,						///<texture width
	const int height,						///<texture height
	unsigned short internalFormat	///<internal fomrat
	)
{

	GLuint texid;						///<texture ID

	glGenTextures(1,&texid); // create (reference to) a new texture

	glBindTexture(GL_TEXTURE_2D, texid);
	// (set texture parameters here)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

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

	//create the texture
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	return texid;
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

//http://dsp.stackexchange.com/questions/3481/radix-4-fft-implementation

#define _USE_MATH_DEFINES
#include <math.h>

void fftr4(
	const Complexf x[4],		//src 
	Complexf y[4]		//dst
)
{
	float ang, C, S;

	// 4 point DFT
	for (int k = 0; k < 4; k++) {
		ang = (float)(2.0*M_PI*k / 4.0);
		for (int j = 0; j < 4; j++) {
			C = cos(j*ang);       S = sin(j*ang);
			y[k].re = (y[k].re + x[j].re * C + x[j].im * S);   // ( C - jS )*( r + ji )
			y[k].im = (y[k].im + x[j].im * C - x[j].re * S);   // = ( rC + iS ) + j( iC - rS )
		}
	}
}


void fft_dit_Stockham_radix2_type0(const Mat& src, Mat &dst);
void fft_dit_Stockham_radix2_type1(const Mat& src, Mat &dst);

typedef std::complex<double> complex_t;
void fft_type0_dif(int n, complex_t* x);
void fft_type0_dit(int n, complex_t* x);
void fft_type1_dif(int n, complex_t* x);
void fft_type1_dit(int n, complex_t* x);


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
	window = glfwCreateWindow(1, 1, "GPGPU FFT", NULL, NULL);
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

//	Mat imgSrc = Mat(Size(4, 1), CV_32FC2);
//	Mat imgSrc = Mat(Size(16, 1), CV_32FC2);
//	Mat imgSrc = Mat(Size(8, 1), CV_32FC2);

//	Mat imgSrc = Mat(Size(8, 8), CV_32FC2);
//	Mat imgSrc = Mat(Size(4, 4), CV_32FC2);
//	Mat imgSrc = Mat(Size(16, 16), CV_32FC2);

//	Mat imgSrc = Mat(Size(256, 256), CV_32FC2);
	Mat imgSrc = Mat(Size(1024, 1024), CV_32FC2);

	Mat imgDst = Mat::zeros(imgSrc.size(), imgSrc.type());
	Mat imgRef = Mat::zeros(imgSrc.size(), imgSrc.type());

	//---------------------------------
	//init Src image
	{
		//imgSrc.at<Vec2f>(0, 0) = Vec2f(1.5f,-1.0f);
		//imgSrc.at<Vec2f>(0, 1) = Vec2f(-2.3f, 2.6f);
		//imgSrc.at<Vec2f>(0, 2) = Vec2f(4.65f, 3.75f);
		//imgSrc.at<Vec2f>(0, 3) = Vec2f(-3.51f, -2.32f);

		//	double r[4] = { 1.5, -2.3, 4.65, -3.51 }, i[4] = { -1.0, 2.6, 3.75, -2.32 };

		const int width = imgSrc.cols;
		const int height = imgSrc.rows;
		for (int y = 0; y < height; y++){
			for (int x = 0; x < width; x++){
				imgSrc.at<Vec2f>(y,x) = Vec2f((float)(x+y),0.0);
			}
		}
	}

	//---------------------------------
	//CPU FFT(cv::dft)
	{
		Timer tmr("cv:dft:\t");
		const int width = imgSrc.cols;
		const int height = imgSrc.rows;

		int flags = 0;
//		flags |= DFT_SCALE;
		cv::dft(imgSrc, imgRef, flags);

		//imgRef = imgSrc.clone();
	}




#if 1
	{
		Timer tmr("fft_dit_Stockham_radix2:\t");
		//fft_dit_Stockham_radix2_type0(imgSrc, imgDst);
		fft_dit_Stockham_radix2_type1(imgSrc, imgDst);

	}
#elif 1
	{
		const int n = 8;
		complex_t* y = new complex_t[n];
//		cout << "==fft_type0_dif==" << endl;
//		fft_type0_dif(n, y);
//		cout << "==fft_type1_dif==" << endl;
//		fft_type1_dif(n, y);
		cout << "==fft_type0_dit==" << endl;
		fft_type0_dit(n, y);
//		cout << "==fft_type1_dit==" << endl;
//		fft_type1_dit(n, y);

		delete y;
	}
#else
	//---------------------------------
	//Execute GPGPU
	{
		const int width = imgSrc.cols;
		const int height = imgSrc.rows;


		// Create and compile our GLSL program from the shaders
		GLuint programID = LoadShaders("Fft.vertexshader", "Fft.fragmentshader");

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

		unsigned int textureID[E_TextureID::SIZEOF] = {0};	//src dst

		//---------------------------------
		// CreateTexture
		{
			const int width = imgSrc.cols;
			const int height = imgSrc.rows;

			textureID[E_TextureID::SRC] = createTexture(width,height,GL_RG32F);		//complex
			textureID[E_TextureID::DST] = createTexture(width,height,GL_RG32F);		//complex

		}

		//upload imgSrc to texture
		{
			GLenum format;				//dual  channel
			GLenum type;				//float
			void* data = imgSrc.data;
			convCvMatType2GlFormat(imgSrc, format, type);

			glBindTexture(GL_TEXTURE_2D, textureID[E_TextureID::SRC]);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format, type, data);
			glBindTexture(GL_TEXTURE_2D, 0);
		}


		//Execute
		{
			Timer tmr("gpu:");
			GLuint texSrc = textureID[E_TextureID::SRC];
			GLuint texDst = textureID[E_TextureID::DST];
			executeGpGpuProcess(programID, fbo, texSrc, texDst);

		}

		{	//download from framebuffer
			GLenum format;				//dual  channel
			GLenum type;				//float
			void* data = imgDst.data;
			convCvMatType2GlFormat(imgDst, format, type);

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
#endif

#if 0
	//dump 
	{
		cout << "imgSrc" << endl;
		cout << imgSrc << endl;

		cout << "imgRef" << endl;
		cout << imgRef << endl;

		cout << "imgDst" << endl;
		cout << imgDst << endl;
	}
#endif

	//verify
	int errNum = 0;
	{
		//float EPS = FLT_MIN * 2;
//		float EPS = 0.00001f;
		int ULPS = imgSrc.cols/4;
		//verify
		int width = imgSrc.cols;
		int height = imgSrc.rows;
		for (int y = 0; y < height; y++){
			for (int x = 0; x < width; x++){
				Vec2f ref = imgRef.at<Vec2f>(y, x);
				Vec2f dst = imgDst.at<Vec2f>(y, x);
				if (!AlmostEqual2sComplement(ref[0], dst[0], ULPS)){
					errNum++;
				}
				if (!AlmostEqual2sComplement(ref[0], dst[0], ULPS)){
					errNum++;
				}


//				if (fabs(ref[0] - dst[0]) > EPS) errNum++;
//				if (fabs(ref[1] - dst[1]) > EPS) errNum++; 
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


#define _USE_GLM

//---------------------------------------------------------------------------
//value is power of 2
bool IsPow2(unsigned int x){
	return (((x)&(x - 1)) == 0);
}



unsigned int insertZeroBits(
	const unsigned int src,		//src
	const int idx,				//挿入する位置
	const int num				//挿入する数
	)
{
	unsigned int ret = src << num;
	ret &= ~((1 << (idx + num)) - 1);
	ret |= src & ((1 << (idx)) - 1);
	return ret;
}

//http://graphics.stanford.edu/~seander/bithacks.html#SwappingBitsXOR
unsigned int bitSwap(
	const unsigned int b,		 // bits to swap reside in b
	const int i,	// positions of bit sequences to swap
	const int j,	//
	const int n		// number of consecutive bits in each sequence
	)
{
	unsigned int x = ((b >> i) ^ (b >> j)) & ((1U << n) - 1); // XOR temporary
	return  b ^ ((x << i) | (x << j));
}

unsigned int modq(unsigned int x, unsigned int q) { return  x& ((1 << q) - 1); }
unsigned int divq(unsigned int x, unsigned int q) { return  x >> q; }





/* FFTの疑似コード - Stockham,DIT */
void fft_dit_Stockham_radix2_type0(const Mat& src, Mat &dst){
	CV_Assert(src.type() == CV_32FC2);
	CV_Assert(src.rows == 1);		//Only raw vector

	enum { _re_ = 0, _im_ = 1 };

#ifndef  _USE_GLM
#define T Vec2f 
#else
#define T vec2 
#endif


	int N = src.cols;
	CV_Assert(IsPow2(N));

	vector<T> w(N / 2);
	vector<T> x(N);
	vector<T> y(N);

	/* コピー */
	for (int n = 0; n < N; n++){
#ifndef  _USE_GLM
		x[n] = src.at<T>(0, n);
#else
		Vec2f val = src.at<Vec2f>(0, n);
		x[n] = T(val[0], val[1]);
#endif
	}


	/* 係数の事前計算 */
	for (int n = 0; n < N / 2; n++){
		//オイラーの公式より
		float jw = (float)(-2 * M_PI * n / N);
		w[n][_re_] = cos(jw);
		w[n][_im_] = sin(jw);
	}

	/* FFTの計算 */
	int q = 0;
	int p = 0;
	while ((2 << q) < N){ q++; }

	for (p = 0; q >= 0; p++, q--) {

		int a = 1 << p;
		int b = 1 << q;

		cout << "------------------------" << endl;
		cout << "p:" << p << "\t";
		cout << "q:" << q << "\t";
		cout << "a:" << a << "\t";
		cout << "b:" << b << "\t";
		cout << endl;

		for (int n = 0; n<N/2; n = n++) {
			int iw  = modq(n, p)*b;
			int ix0 = n;
			int ix1 = ix0 + N / 2;
			int iy0 = insertZeroBits(n,p,1);
			int iy1 = iy0 + (1<<p);

			cout << "w(" << iw << ")\t";
			cout << "src(" << ix0 << "," << ix1 << ")\t";
			cout << "dst(" << iy0 << "," << iy1 << ")\t";
			cout << endl;

			T tmp;
			tmp[_re_] = x[ix1][_re_] * w[iw][_re_] - x[ix1][_im_] * w[iw][_im_];
			tmp[_im_] = x[ix1][_re_] * w[iw][_im_] + x[ix1][_im_] * w[iw][_re_];


			y[iy0] = x[ix0] + tmp;
			y[iy1] = x[ix0] - tmp;
		}
		for (int n = 0; n<N; n = n + 1) x[n] = y[n];

	}

	/* コピー */
	for (int n = 0; n < N; n++){
#ifndef  _USE_GLM
		dst.at<Vec2f>(0, n) = x[n];
#else
		T val = x[n];
		dst.at<Vec2f>(0, n) = Vec2f(val.r, val.g);
#endif

	}

}

/* FFT Stockham,DIT */
void fft_dit_Stockham_radix2_type1(const Mat& src, Mat &dst){
	CV_Assert(src.type() == CV_32FC2);
	CV_Assert(src.cols == src.rows);
	int N = src.cols;
	CV_Assert(IsPow2(N));

	enum { _re_ = 0, _im_ = 1 };

	Mat buf[2];
	buf[0] = src.clone();
	buf[1] = Mat(src.size(), src.type());

	vector<vec2> w(N / 2);

	// --- twidle ----
	for (int n = 0; n < N / 2; n++){
		float jw = (float)(-2 * M_PI * n / N);
		w[n][_re_] = cos(jw);
		w[n][_im_] = sin(jw);
	}

	int Q = 0;
	while ((1 << Q) < N){ Q++; }

	// --- FFT rows ----
#ifdef _OPENMP
#pragma omp parallel for
#endif
	for (int row = 0; row < src.rows; row++){
		int bank = 0;

		for (int p = 0, q = Q - 1; q >= 0; p++, q--, bank = bank ^ 1) {
			vec2* x = (vec2*)buf[bank].ptr<Vec2f>(row);
			vec2* y = (vec2*)buf[bank^1].ptr<Vec2f>(row);

			//cout << "------------------------" << endl;
			//cout << "p:" << p << "\t";
			//cout << "q:" << q << "\t";
			//cout << endl;

			for (int n = 0; n<N / 2; n = n++) {
				int iw = (n >> q) << q;
				int ix0 = insertZeroBits(n, q, 1);
				int ix1 = ix0 + (1 << q);
				int iy0 = n;
				int iy1 = iy0 + N / 2;

				//cout << "w(" << iw << ")\t";
				//cout << "src(" << ix0 << "," << ix1 << ")\t";
				//cout << "dst(" << iy0 << "," << iy1 << ")\t";
				//cout << endl;

				T tmp;
				tmp[_re_] = x[ix1][_re_] * w[iw][_re_] - x[ix1][_im_] * w[iw][_im_];
				tmp[_im_] = x[ix1][_re_] * w[iw][_im_] + x[ix1][_im_] * w[iw][_re_];

				y[iy0] = x[ix0] + tmp;
				y[iy1] = x[ix0] - tmp;

			}
		}
	}

	// --- FFT cols ----
#ifdef _OPENMP
#pragma omp parallel for
#endif
	for (int col = 0; col < src.cols; col++){

		int bank = (Q&1);	//FFT rows の回数が奇数ならば奇数bankから

		for (int p = 0, q = Q - 1; q >= 0; p++, q--, bank = bank ^ 1) {
			vec2* x = (vec2*)buf[bank].ptr<Vec2f>(0,col);
			vec2* y = (vec2*)buf[bank ^ 1].ptr<Vec2f>(0, col);

			for (int n = 0; n<N / 2; n = n++) {
				int iw = (n >> q) << q;
				int ix0 = insertZeroBits(n, q, 1);
				int ix1 = ix0 + (1 << q);
				int iy0 = n;
				int iy1 = iy0 + N / 2;

				ix0 *= src.cols;
				ix1 *= src.cols;
				iy0 *= src.cols;
				iy1 *= src.cols;

				T tmp;
				tmp[_re_] = x[ix1][_re_] * w[iw][_re_] - x[ix1][_im_] * w[iw][_im_];
				tmp[_im_] = x[ix1][_re_] * w[iw][_im_] + x[ix1][_im_] * w[iw][_re_];

				y[iy0] = x[ix0] + tmp;
				y[iy1] = x[ix0] - tmp;

			}
		}
	}


	dst = buf[0];

}




