#ifndef __Model_H__
#define __Model_H__


#include <glm/glm.hpp>
using namespace glm;


struct GVertexAttribute{
	f32vec3 position;		///<vertex local position
	f32vec3 normal;			///<vertex normal vector
	f32vec4 color;			///<vertex color (rgba)
};



class GModel
{
private:


public:
	GLuint vbo;
	GLuint ibo;
	GLuint vao;

	vector<GVertexAttribute> VertexAttribute;
	vector<u16vec3>	Index;

	GModel(void);
	~GModel(void);

	void CreateVBO(void);
	void CreateIBO(void);

	void BindVBO(void);
	void BindIBO(void);
	void Draw(void);


};

class GModelSphere : public GModel
{
public:
	GModelSphere(const int row,const int col, const float rad);
};

class GModelCube : public GModel
{
public:
	GModelCube(const float side);
};





#endif