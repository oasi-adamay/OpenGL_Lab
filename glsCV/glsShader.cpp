#include "stdafx.h"


#include "glsShader.h"
#include "Timer.h"




void glsShaderBase::LoadShadersCode(const std::string& VertexShaderCode, const std::string& FragmentShaderCode){
	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);


	GLint Result = GL_FALSE;
	int InfoLogLength;


	// Compile Vertex Shader
	//	printf("Compiling shader : %s\n", vertex_file_path);
//	cout << "Compiling shader : " << vertex_file_path << endl;
	cout << "Compiling vertex shader" << endl;
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0){
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		//		printf("%s\n", &VertexShaderErrorMessage[0]);
		cout << &VertexShaderErrorMessage[0] << endl;
	}
	assert(Result == GL_TRUE);



	// Compile Fragment Shader
//	cout << "Compiling shader : " << fragment_file_path << endl;
	cout << "Compiling fragment shader" << endl;
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0){
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		//		printf("%s\n", &FragmentShaderErrorMessage[0]);
		cout << &FragmentShaderErrorMessage[0] << endl;
	}
	assert(Result == GL_TRUE);


	// Link the program
	//	printf("Linking program\n");
	cout << "Linking program" << endl;
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0){
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		//		printf("%s\n", &ProgramErrorMessage[0]);
		cout << &ProgramErrorMessage[0] << endl;
	}
	assert(Result == GL_TRUE);


	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	program = ProgramID;

}

void glsShaderBase::LoadShadersFile(const char * vertex_file_path, const char * fragment_file_path){


	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if (VertexShaderStream.is_open()){
		std::string Line = "";
		while (getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}
	else{
		cout << "Impossible to open:" << vertex_file_path << endl;
		assert(0);
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if (FragmentShaderStream.is_open()){
		std::string Line = "";
		while (getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}
	else{
		cout << "Impossible to open:" << fragment_file_path << endl;
		assert(0);
	}

	LoadShadersCode(VertexShaderCode, FragmentShaderCode);

}

