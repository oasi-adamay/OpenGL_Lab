#include "stdafx.h"


#include "glsFft.h"
#include "Timer.h"

#define _USE_MATH_DEFINES
#include <math.h>

//-----------------------------------------------------------------------------
//global 
glsShaderFft* shaderFft = 0;
glsShaderConj* shaderConj = 0;

void glsFftInit(void){
	shaderFft = new glsShaderFft();
	shaderConj = new glsShaderConj();
}

void glsFftTerminate(void){
	delete shaderFft;
	delete shaderConj;
}




//-----------------------------------------------------------------------------
//glsShaderFft
glsShaderFft::glsShaderFft(void)
	:glsShaderBase()
{

	const char vertexShaderCode[] = 
"#version 330 core\n"
"layout (location = 0)in  vec2 position;\n"
"void main(void)\n"
"{\n"
"   gl_Position  = vec4(position,0.0,1.0);\n"
"}\n"
;


	const char fragmentShaderCode[] = 
"#version 330 core\n"
"precision highp float;\n"
"uniform sampler2DRect	texSrc0;\n"
"uniform sampler2DRect	texSrc1;\n"
"uniform sampler2DRect	texW;\n"
"uniform  int i_flag;	//bit0:(0:holizontal 1:vertical)\n"
"uniform  int i_N;\n"
"uniform  int i_p;\n"
"uniform  int i_q;\n"
"uniform  float f_xscl;\n"
"uniform  float f_yscl;\n"
"uniform  float f_xconj;\n"
"uniform  float f_yconj;\n"
"\n"
"layout (location = 0) out vec2 dst0;\n"
"layout (location = 1) out vec2 dst1;\n"
"\n"
"#define FLAG_DIR	 (1<<0)\n"
"\n"
"\n"
"int insertZeroBits(\n"
"	const int src,\n"
"	const int idx,\n"
"	const int num\n"
"	)\n"
"{\n"
"	int ret = src << num;\n"
"	ret &= ~((1 << (idx + num)) - 1);\n"
"	ret |= src & ((1 << idx) - 1);\n"
"	return ret;\n"
"}\n"
"\n"
"void main(void)\n"
"{\n"
"	int p = i_p;\n"
"	int q = i_q;\n"
"	int N = i_N;\n"
"	int dir = ((i_flag & FLAG_DIR)==0) ?0:1;\n"
"	float xscl = f_xscl;\n"
"	float yscl = f_yscl;\n"
"	float xconj = f_xconj;\n"
"	float yconj = f_yconj;\n"
"\n"
"	int n;\n"
"	vec2 x0;\n"
"	vec2 x1;\n"
"	vec2 w;\n"
"\n"
"	n= int(gl_FragCoord[dir]);\n"
"	int iw = (n >> q) << q;\n"
"	int ix0 = insertZeroBits(n, q, 1);\n"
"	int ix1 = ix0 + (1 << q);\n"
"	w = texture(texW,vec2(iw,0)).rg;\n"
"\n"
"\n"
"	if(dir ==0){\n"
"		if(ix0 < N/2) x0 = texture(texSrc0,vec2(ix0,gl_FragCoord.y)).rg;\n"
"		else x0 = texture(texSrc1,vec2(ix0-N/2,gl_FragCoord.y)).rg;\n"
"\n"
"		if(ix1 < N/2) x1 = texture(texSrc0,vec2(ix1,gl_FragCoord.y)).rg;\n"
"		else x1 = texture(texSrc1,vec2(ix1-N/2,gl_FragCoord.y)).rg;\n"
"	}\n"
"	else{\n"
"		if(ix0 < N/2) x0 = texture(texSrc0,vec2(gl_FragCoord.x,ix0)).rg;\n"
"		else x0 = texture(texSrc1,vec2(gl_FragCoord.x,ix0-N/2)).rg;\n"
"\n"
"		if(ix1 < N/2) x1 = texture(texSrc0,vec2(gl_FragCoord.x,ix1)).rg;\n"
"		else x1 = texture(texSrc1,vec2(gl_FragCoord.x,ix1-N/2)).rg;\n"
"	}\n"
"\n"
"	x0 = x0*xscl;\n"
"	x1 = x1*xscl;\n"
"	x0.g = x0.g*xconj;\n"
"	x1.g = x1.g*xconj;\n"
"\n"
"	vec2 tmp;\n"
"	tmp.r = x1.r * w.r - x1.g * w.g;\n"
"	tmp.g = x1.r * w.g + x1.g * w.r;\n"
"\n"
"	vec2 y0;\n"
"	vec2 y1;\n"
"\n"
"	y0 = x0 + tmp;\n"
"	y1 = x0 - tmp;\n"
"\n"
"	y0 = y0*yscl;\n"
"	y1 = y1*yscl;\n"
"	y0.g = y0.g*yconj;\n"
"	y1.g = y1.g*yconj;\n"
"\n"
"	dst0 = y0;\n"
"	dst1 = y1;\n"
"\n"
"}\n"
;
	
	

	// Create and compile our GLSL program from the shaders
//	LoadShadersFile("Fft_vs.glsl", "FftRadix2_fs.glsl");
	LoadShadersCode(vertexShaderCode, fragmentShaderCode);

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
static void glsFftProcess(
	const glsShaderFft* shader,	//progmra ID
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

//	Timer tmr("glsFftProcess:\t");
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







//-----------------------------------------------------------------------------
// execute FFT 
void glsFft(glsMat& texture, int flag){

	int N = texture.width;
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

	// [-1, 1] ‚Ì³•ûŒ`
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

	glsMat texTmp(texture.width, texture.height, texture.blkX, texture.blkY);
	GLuint texW = 0;	//twidle

	//---------------------------------
	// CreateTexture
	{

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
			glTexImage2D(GL_TEXTURE_RECTANGLE, 0, texture.internalFormat, N / 2, 1, 0, texture.format, texture.type, 0);

			glBindTexture(GL_TEXTURE_RECTANGLE, 0);
		}

	}

	//---------------------------------
	// upload twidle texture
	{
		Timer tmr("-twidle:  \t");

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
		glTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, 0, 0, (GLsizei)w.size(), 1, texture.format, texture.type, data);
		glBindTexture(GL_TEXTURE_RECTANGLE, 0);
	}


	glsMat* texid[2] = { &texture, &texTmp };


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
					texSrc[j] = texid[bank    ]->at(i, j);
					texDst[j] = texid[bank ^ 1]->at(i, j);
				}
				float xscl = ((flag & GLSL_FFT_SCALE) && (flag & GLSL_FFT_INVERSE) && (p == 0)) ? 1.0f / (float)N : 1.0f;
				float yscl = ((flag & GLSL_FFT_SCALE) && !(flag & GLSL_FFT_INVERSE) && (q == 0)) ? 1.0f / (float)N : 1.0f;
				float xconj = ((flag & GLSL_FFT_INVERSE) && (p == 0)) ? -1.0f : 1.0f;
				float yconj = 1.0f;
				glsFftProcess(shaderFft, texSrc, texDst, texW, 0, p, q, N, xscl, yscl, xconj,yconj);
			}
		}
		// --- FFT cols ----
		for (int p = 0, q = Q - 1; q >= 0; p++, q--, bank = bank ^ 1) {
			for (int j = 0; j < 2; j++){
				for (int i = 0; i < 2; i++){
					texSrc[i] = texid[bank    ]->at(i, j);
					texDst[i] = texid[bank ^ 1]->at(i, j);
				}
				float yscl = ((flag & GLSL_FFT_SCALE) && !(flag & GLSL_FFT_INVERSE) && (q == 0)) ? 1.0f / (float)N : 1.0f;
				float xscl = ((flag & GLSL_FFT_SCALE) && (flag & GLSL_FFT_INVERSE) && (p == 0)) ? 1.0f / (float)N : 1.0f;
				float xconj = 1.0f;
				float yconj = ((flag & GLSL_FFT_INVERSE) && (q == 0)) ? -1.0f : 1.0f;
				glsFftProcess(shaderFft, texSrc, texDst, texW, 1, p, q, N, xscl, yscl, xconj, yconj);
			}
		}
	}

	//clean up
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);

	glDeleteFramebuffers(1, &fbo);
	glDeleteTextures(1, &texW);

}



void glsFft(const Mat& src, Mat& dst, int flag){
	CV_Assert(src.type() == CV_32FC2);
	CV_Assert(src.cols == src.rows);

	int N = src.cols;
	CV_Assert(IsPow2(N));

	glsMat texture(src.cols, src.rows, 2, 2);

	//---------------------------------
	//upload
	texture.CopyFrom(src);

	//---------------------------------
	//fft
	glsFft(texture, flag);

	//---------------------------------
	//download
	texture.CopyTo(dst);


}




//=============================================================================
glsShaderConj::glsShaderConj(void)
	:glsShaderBase()
{
#if 1
	const char vertexShaderCode[] = 
		"#version 330 core\n"
		"layout (location = 0)in  vec2 position;\n"
		"void main(void)\n"
		"{\n"
		"   gl_Position  = vec4(position,0.0,1.0);\n"
		"}\n"
		;

	const char fragmentShaderCode[] =
		"#version 330 core\n"
		"precision highp float;\n"
		"uniform sampler2DRect	texSrc;\n"
		"layout (location = 0) out vec2 dst;\n"
		"\n"
		"void main(void)\n"
		"{\n"
		"	vec2 src = texture(texSrc, gl_FragCoord.xy).rg;\n"
		"	dst = vec2(src.r, -1.0*src.g);\n"
		"}\n"
		;

	LoadShadersCode(vertexShaderCode, fragmentShaderCode);


#else
	// Create and compile our GLSL program from the shaders
	LoadShadersFile("Fft_vs.glsl", "Conj_fs.glsl");
#endif

	// Attribute & Uniform location
	position = glGetAttribLocation(program, "position");

	texSrc = glGetUniformLocation(program, "texSrc");

}

static void glslConjProcess(
	const glsShaderConj* shader,	//progmra ID
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



