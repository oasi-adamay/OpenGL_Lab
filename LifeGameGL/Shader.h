#ifndef __Shader_H__
#define __Shader_H__

//include
#ifdef _MSC_VER
// Include GLEW
#include <GL/glew.h>

// Include GLFW
#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>
#endif



//-----------------------------------------------------------------------------
/*!
@brief
シェーダーの生成
@return
Program Handle
*/
GLuint createShaders(const char * vertex_file_path, const char * fragment_file_path);







#endif __Shader_H__



