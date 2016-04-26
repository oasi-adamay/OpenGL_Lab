#include "stdafx.h"


#include "glsCV.h"


//-----------------------------------------------------------------------------
//global 
static GLFWwindow* window = 0;

//-----------------------------------------------------------------------------
// GLFWでエラーとなったときに呼び出される関数
static void glfw_error_callback_func(int error, const char* description){
	std::cout << "GLFW Error: " << description << std::endl;
}


//=============================================================================
//-----------------------------------------------------------------------------
//Initialtize glsCV lib
void glsCvInit(void){
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


}

//-----------------------------------------------------------------------------
//Terminate glsFft
void glsCvTerminate(void){


	// Close OpenGL window and terminate GLFW
	glfwTerminate();
}


