#ifndef __Model_H__
#define __Model_H__


#include <glm/glm.hpp>
using namespace glm;


struct GVertexAttribute{
	vec3 position;		///<vertex local position
	vec3 normal;		///<vertex normal vector
	vec4 color;			///<vertex color (rgba)
	vec2 texCoord;		///<vertex texture coodinate
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
	void DrawPoints(void);

	void DrawInstansing(const GLuint vboInstPos,int instCount);


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