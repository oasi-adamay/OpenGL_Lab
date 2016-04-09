#ifndef __Model_H__
#define __Model_H__


#include <glm/glm.hpp>
using namespace glm;


struct VertexAttribute{
	vec3 position;		///<vertex local position
	vec3 normal;		///<vertex normal vector
	vec4 color;			///<vertex color (rgba)
	vec2 texCoord;		///<vertex texture coodinate
};



class PureModel
{
private:


public:

	vector<VertexAttribute> vecVertexAttribute;
	vector<u16vec3>	vecIndex;

	PureModel(void){ ; }
	~PureModel(void){ ; }

};


class PureModelSphere : public PureModel
{
public:
	PureModelSphere(const int row, const int col, const float rad);
};

class PureModelCube : public PureModel
{
public:
	PureModelCube(const float side);
};



class GlModel
{
private:


public:
	GLuint vbo;
	GLuint ibo;
	GLuint vao;
	PureModel* model;

	GlModel(void);
	GlModel(PureModel* _model);
	~GlModel(void);

	void CreateVBO(void);
	void CreateIBO(void);

	void BindVBO(void);
	void BindIBO(void);
	void Draw(void);
	void DrawPoints(void);

	void DrawInstansing(const GLuint vboInstPos,int instCount);


};






#endif