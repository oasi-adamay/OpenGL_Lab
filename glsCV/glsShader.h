#ifndef _GLS_SHADER_H_
#define _GLS_SHADER_H_

#include "glsMat.h"


// glslBase shader
class glsShaderBase
{
private:
	glsShaderBase(const glsShaderBase&);              ///  Uncopyable
	glsShaderBase& operator=(const glsShaderBase&);   ///  Uncopyable

public:
	glsShaderBase(void){ program = 0;}
	~glsShaderBase(void){ glDeleteProgram(program); }
	void LoadShadersFile(const char * vertex_file_path, const char * fragment_file_path);
	void LoadShadersCode(const std::string& VertexShaderCode, const std::string& FragmentShaderCode);

	//program
	GLuint program;

};


#endif