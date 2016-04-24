#ifndef _GLSL_FFT_H_
#define _GLSL_FFT_H_

// glslBase shader
class glslBaseShader
{
public:
	glslBaseShader(void){ program = 0;}
	~glslBaseShader(void){ glDeleteProgram(program); }

	//program
	GLuint program;

};


// glslFft shader
class glslFftShader : public glslBaseShader
{
public:
	glslFftShader(void);

	//attribute location
	GLuint position;

	//uniform  location

	GLuint texSrc[2];		
	GLuint texW;
	GLuint i_flag;
	GLuint i_N;
	GLuint i_p;
	GLuint i_q;
	GLuint f_xscl;
	GLuint f_yscl;
	GLuint f_xconj;
	GLuint f_yconj;

};

// glslConj shader
class  glslConjShader : public glslBaseShader
{
public:
	glslConjShader(void);

	//attribute location
	GLuint position;
	//uniform  location
	GLuint texSrc;
};


#define GLSL_FFT_SCALE	   (1<<1)
#define GLSL_FFT_INVERSE   (1<<2)

void glslFftInit(void);
void glslFftTerminate(void);
void glslFftUploadTexture(const Mat&src, const vector<GLuint>& texArray);
void glslFftDownloadTexture(const vector<GLuint>& texArray, Mat&dst);
void glslFft(vector<GLuint>& texArray, int flag = 0);
void glslFft(const Mat& src, Mat& dst, int flag = 0);


#endif