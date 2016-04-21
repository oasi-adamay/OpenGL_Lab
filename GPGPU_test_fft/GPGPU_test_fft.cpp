// GPGPU_test_copy.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "../common/shader.hpp"



//Lib 
#pragma comment (lib, "opengl32.lib")
#pragma comment (lib, "glew32.lib")
#pragma comment (lib, "glfw3dll.lib")

//-----------------------------------------------------------------------------
class Timer{
private:
	cv::TickMeter meter;
	string msg;
public:
	Timer(string _msg){ msg = _msg;  meter.start(); }
	~Timer(void){ meter.stop(); std::cout << msg << meter.getTimeMilli() << "ms" << std::endl; }

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
	Mat imgSrc = Mat(Size(8, 1), CV_32FC2);
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
				imgSrc.at<Vec2f>(y,x) = Vec2f((float)x,0.0);
			}
		}
	}

	//---------------------------------
	//CPU FFT(cv::dft)
	{
		const int width = imgSrc.cols;
		const int height = imgSrc.rows;

		int flags = 0;
//		flags |= DFT_SCALE;
		cv::dft(imgSrc, imgRef, flags);
		//imgRef = imgSrc.clone();

	}




#if 1
	{
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

#if 1
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
		float EPS = 0.00001f;
		//verify
		int width = imgSrc.cols;
		int height = imgSrc.rows;
		for (int y = 0; y < height; y++){
			for (int x = 0; x < width; x++){
				Vec2f ref = imgRef.at<Vec2f>(y, x);
				Vec2f dst = imgDst.at<Vec2f>(y, x);
				if (fabs(ref[0] - dst[0]) > EPS) errNum++;
				if (fabs(ref[1] - dst[1]) > EPS) errNum++;
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


/* FFTの疑似コード - Cooley-Tukey,DIT */
void fft_dit_Cooley_Tukey_radix2(const Mat& src, Mat &dst){
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
	vector<int> bitrev(N);

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
	/* ビットリバース順の事前計算 */
	bitrev[0] = 0;
	for (int a = 1, b = N / 2; a<N; a = a * 2, b = b / 2) {
		for (int k = 0; k<a; k++) {
			bitrev[k + a] = bitrev[k] + b;
		}
	}

	/* 入力の並び替え */
	for (int n = 0; n<N; n = n++) {
		if (n < bitrev[n]) {
			T tmp = x[n];
			x[n] = x[bitrev[n]];
			x[bitrev[n]] = tmp;
		}
	}
	/* FFTの計算 */
	for (int a = 1, b = N / 2, p = 0; a<N; a *= 2, b /= 2, p++) {

		cout << "------------------------" << endl;
		cout << "p:" << p << endl;

		for (int k = 0; k<a; k++) {
			for (int n = k; n<N; n = n + 2 * a) {
				T tmp;
				tmp[_re_] = x[n + a][_re_] * w[b*k][_re_] - x[n + a][_im_] * w[b*k][_im_];
				tmp[_im_] = x[n + a][_re_] * w[b*k][_im_] + x[n + a][_im_] * w[b*k][_re_];

				x[n + a] = x[n] - tmp;
				x[n] = x[n] + tmp;

				cout << "(" << n << "," << n + a << "," << b*k << ")";
				cout << endl;


			}
		}
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



/* FFTの疑似コード - Cooley-Tukey,DIT */
void fft_dit_Cooley_Tukey_radix2_swap(const Mat& src, Mat &dst){
	cout << "====================================" << endl;
	cout << "fft_dit_Cooley_Tukey_radix2_swap" << endl;

	CV_Assert(src.type() == CV_32FC2);
	CV_Assert(src.rows == 1);		//Only raw vector

	enum { _re_ = 0, _im_ = 1 };

#ifndef  _USE_GLM
#define T Vec2f 
#else
#define T vec2 
#endif

	const int radix_exp = 1;   //radix 2 = 2^1


	int N = src.cols;
	CV_Assert(IsPow2(N));

	vector<T> w(N >> radix_exp);
	vector<T> x(N);
	vector<T> y(N);
	vector<int> bitrev(N);

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
	for (int n = 0; n < (N >> radix_exp); n++){
		//オイラーの公式より
		float jw = (float)(-2 * M_PI * n / N);
		w[n][_re_] = cos(jw);
		w[n][_im_] = sin(jw);
	}
	/* ビットリバース順の事前計算 */
	bitrev[0] = 0;
	for (int a = 1, b = (N >> radix_exp); a<N; a <<= radix_exp, b >>= radix_exp) {
		for (int k = 0; k<a; k++) {
			bitrev[k + a] = bitrev[k] + b;
		}
	}

	/* 入力の並び替え */
	for (int n = 0; n<N; n = n++) {
		if (n < bitrev[n]) {
			T tmp = x[n];
			x[n] = x[bitrev[n]];
			x[bitrev[n]] = tmp;
		}
	}
	/* FFTの計算 */
#if 0
	for (int a = 1, b = (N >> radix_exp), p = 0; a<N; a <<= radix_exp, b >>= radix_exp, p++) {

		cout << "------------------------" << endl;
		cout << "p:" << p << endl;

		for (int k = 0; k<a; k++) {
			for (int n = k; n<N; n = n + (a << radix_exp)) {
				T tmp;
				tmp[_re_] = x[n + a][_re_] * w[b*k][_re_] - x[n + a][_im_] * w[b*k][_im_];
				tmp[_im_] = x[n + a][_re_] * w[b*k][_im_] + x[n + a][_im_] * w[b*k][_re_];

				x[n + a] = x[n] - tmp;
				x[n] = x[n] + tmp;

				cout << "(" << n << "," << n + a << "," << b*k << ")" << endl;

			}
		}

		cout << endl;
	}


#else

	int q = 0;
	int p = 0;
	while ((2 << q) < N){ q++; }
	int msb = q;

	/* FFTの計算 */
	for (p = 0; q >=0 ; p++,q--) {

		int a = 1 << p;
		int b = 1 << q;

		cout << "------------------------" << endl;
		cout << "p:" << p << "\t";
		cout << "q:" << q << "\t";
		cout << "a:" << a << "\t";
		cout << "b:" << b << "\t";
		cout << endl;


		for (int n = 0; n < (N >> radix_exp); n++) {
			unsigned int m = insertZeroBits(n, p, radix_exp);

			int k = modq(n, p);				// n/b
			int iw = k*b;		//(n  & ((1 << p) - 1))*b;
			int ix0 = m;
			int ix1 = m + a;

			int iy0 = m;
			int iy1 = m + a;

#if 0
			if (p -1 >=0){
				//swap bit 
				ix0 = bitSwap(ix0, p - 1, p + q, 1);
				ix1 = bitSwap(ix1, p - 1, p + q, 1);
			}
			{
				//swap bit 
				iy0 = bitSwap(iy0, p, p + q, 1);
				iy1 = bitSwap(iy1, p, p + q, 1);
			}
#endif
			cout << "n(" << n << ")\t";
			cout << "m(" << m << ")\t";
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

#endif

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
	vector<int> bitrev(N/2);

	/* コピー */
	for (int n = 0; n < N; n++){
#ifndef  _USE_GLM
		x[n] = src.at<T>(0, n);
#else
		Vec2f val = src.at<Vec2f>(0, n);
		x[n] = T(val[0], val[1]);
#endif
	}

	/* ビットリバース順の事前計算 */
	bitrev[0] = 0;
	for (int a = 1, b = (N /4); a<N/2; a <<= 1, b >>= 1) {
		for (int k = 0; k<a; k++) {
			bitrev[k + a] = bitrev[k] + b;
		}
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
		cout << "------------------------" << endl;
		cout << "p:" << p << "\t";
		cout << "q:" << q << "\t";
		cout << endl;

		for (int n = 0; n<N / 2; n = n++) {
			int iw = (n >> q)<<q;
			int ix0 = insertZeroBits(n, q, 1);
			int ix1 = ix0 + (1 << q);
			int iy0 = n;
			int iy1 = iy0 + N / 2;

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







/* FFTの疑似コード - Stockham,DIT */
void fft_dit_Stockham_radix2_swap(const Mat& src, Mat &dst){
	cout << "====================================" << endl;
	cout << "fft_dit_Stockham_radix2_swap" << endl;


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
	vector<int> bitrev(N);

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

	int q = 0;
	int p = 0;
	while ((2 << q) < N){ q++; }
	int msb = q;

	/* FFTの計算 */
	for (p = 0; q >=0 ; p++,q--) {

		int a = 1 << p;
		int b = 1 << q;

		cout << "------------------------" << endl;
		cout << "p:" << p << "\t";
		cout << "q:" << q << "\t";
		cout << "a:" << a << "\t";
		cout << "b:" << b << "\t";
		cout << endl;
#if 0
		for (int n = 0; n<b; n++) {
			for (int k = 0; k<a; k++) {
				int j = n;
				cout <<"j:" << j <<"\t";
				cout <<"k:" << k <<"\t";
#else


//		for (int n = 0; n<(N / 2); n++) {
		for (int m = 0; m<(N / 2); m++) {

			int n = m;
#if 1
			if(p==0){
				//  210 => 021  Rotate RSFT
				n = bitSwap(n, 0 , 1, 1);  //
				n = bitSwap(n, 1 , 2, 1);
			}
			if (p == 1){
				//  210 => 012
				n = bitSwap(n, 0, 2, 1);  //
			}
			if (p == 2){
				//  210 => 102 Rotate LSFT
				n = bitSwap(n, 0, 2, 1);  //
				n = bitSwap(n, 2, 1, 1);  //
			}
#endif

			int j = modq(n,q);     // n%b
			int k = divq(n,q);				// n/b
			cout << "m:" << m << "\t";
			cout << "n:" << n << "\t";
			cout << "j:" << j << "\t";
			cout << "k:" << k << "\t";

			{
#endif
				T tmp;

				//int iw = (n >> q) << q;					//b*k;
				//int ix0 = (j << p) + k;					//a*j + k;  //
				//int ix1 = ix0 + N / 2;					//a*j + k + N /2
				//int iy0 = (j << (p+1)) + k;				//2 * a*j + k;
				//int iy1 = iy0 + (1<<p);					//2 * a*j + k + a;
				int iw = b*k;
				int ix0 = a*j + k;
				int ix1 = a*j + k + N /2;
				int iy0 = 2 * a*j + k;
				int iy1 = 2 * a*j + k + a;

#if 1
				if (p - 1 >= 0){
					//					cout << "in swap" << endl;
					//swap bit 
					ix0 = bitSwap(ix0, p - 1, msb, 1);
					ix1 = bitSwap(ix1, p - 1, msb, 1);
				}
				{
					//swap bit 
					iy0 = bitSwap(iy0, p, msb, 1);
					iy1 = bitSwap(iy1, p, msb, 1);
				}
#endif

				tmp[_re_] = x[ix1][_re_] * w[iw][_re_] - x[ix1][_im_] * w[iw][_im_];
				tmp[_im_] = x[ix1][_re_] * w[iw][_im_] + x[ix1][_im_] * w[iw][_re_];

				y[iy0] = x[ix0] + tmp;
				y[iy1] = x[ix0] - tmp;

				cout << "w(" << iw << ")\t";
				cout << "src(" << ix0 << "," << ix1 << ")\t";
				cout << "dst(" << iy0 << "," << iy1 << ")\t";
				cout << endl;


			}
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

