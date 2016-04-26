#ifndef _GLS_FFT_H_
#define _GLS_FFT_H_

#include "glsCV.h"
#include "glsMat.h"
#include "glsShader.h"

// glsFft shader
class glsShaderFft : public glsShaderBase
{
public:
	glsShaderFft(void);

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
class  glsShaderConj : public glsShaderBase
{
public:
	glsShaderConj(void);

	//attribute location
	GLuint position;
	//uniform  location
	GLuint texSrc;
};





#define GLSL_FFT_SCALE	   (1<<1)
#define GLSL_FFT_INVERSE   (1<<2)

void glsFftInit(void);
void glsFftTerminate(void);

void glsFft(glsMat& texture, int flag = 0);
void glsFft(const Mat& src, Mat& dst, int flag = 0);



#endif