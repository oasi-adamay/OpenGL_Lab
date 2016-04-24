#ifndef _GLSL_FFT_H_
#define _GLSL_FFT_H_


// glslFft shader
struct glslFftShader
{
	GLuint program;

	//attribute location
	GLuint position;

	//uniform  location
	GLuint texSrc[2];		
	GLuint texW;
	GLuint fft_dir;
	GLuint fft_N;
	GLuint fft_p;
	GLuint fft_q;

};



void glslFftInit(void);
void glslFftTerminate(void);
void glslFftUploadTexture(const Mat&src, const vector<GLuint>& texArray);
void glslFftDownloadTexture(const vector<GLuint>& texArray, Mat&dst);
void glslFft(vector<GLuint>& texArray);
void glslFft(const Mat& src, Mat& dst);


#endif