#include "stdafx.h"

#include "Model.h"

#define _USE_MATH_DEFINES
#include <math.h>


static
vec3 hsv2rgb(vec3 hsv)
{
	float      hh, p, q, t, ff;
	long        i;
	vec3     out;

	struct hsv_t{
		float h;
		float s;
		float v;
	};

	hsv_t in;
	in.h = hsv.x;
	in.s = hsv.y;
	in.v = hsv.z;

	if (in.s <= 0.0) {       // < is bogus, just shuts up warnings
		out.r = in.v;
		out.g = in.v;
		out.b = in.v;
		return out;
	}
	hh = in.h;
	if (hh >= 360.0) hh = 0.0;
	hh /= 60.0;
	i = (long)hh;
	ff = hh - i;
	p = in.v * (1.0f - in.s);
	q = in.v * (1.0f - (in.s * ff));
	t = in.v * (1.0f - (in.s * (1.0f - ff)));

	switch (i) {
	case 0:
		out.r = in.v;
		out.g = t;
		out.b = p;
		break;
	case 1:
		out.r = q;
		out.g = in.v;
		out.b = p;
		break;
	case 2:
		out.r = p;
		out.g = in.v;
		out.b = t;
		break;

	case 3:
		out.r = p;
		out.g = q;
		out.b = in.v;
		break;
	case 4:
		out.r = t;
		out.g = p;
		out.b = in.v;
		break;
	case 5:
	default:
		out.r = in.v;
		out.g = p;
		out.b = q;
		break;
	}
	return out;
}



GlModel::GlModel(void){
	vao = 0;
	vbo = 0;
	ibo = 0;
	model = 0;
}

GlModel::GlModel(PureModel* _model){
	model = _model;
	glGenVertexArrays(1, &vao);
	CreateVBO();
	CreateIBO();
	BindVBO();
	BindIBO();

}


GlModel::~GlModel(void){
	if (vbo) glDeleteBuffers(1, &vbo);
	if (ibo) glDeleteBuffers(1, &ibo);

	glDeleteVertexArrays(1, &vao);

}


void GlModel::CreateVBO(void){
	GLsizei size = (GLsizei)model->vecVertexAttribute.size()*sizeof(VertexAttribute);	//in byte
	if (size){
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, size, model->vecVertexAttribute.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}

void GlModel::CreateIBO(void){
	GLsizei size = (GLsizei)model->vecIndex.size()*sizeof(u16vec3);
	if (size){
		glGenBuffers(1, &ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, model->vecIndex.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
}

void GlModel::BindVBO(void){

	if (vbo) {
		glBindVertexArray(vao);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);


		int offset = 0;
		int stride = sizeof(VertexAttribute);
		VertexAttribute* pVtxAttr = NULL;		//sizeofのためのポインタ

		//position
		int size = sizeof(pVtxAttr->position) / sizeof(float);
		glEnableVertexAttribArray(0);	//enable attribute Location
		glVertexAttribPointer(
			0,					// attribute 0. No particular reason for 0, but must match the layout in the shader.
			size,				// size	(Specifies the number of components)
			GL_FLOAT,			// type
			GL_FALSE,			// normalized?
			stride,				// stride (Specifies the byte offset between consecutive generic vertex attributes)
			(void* )offset		// array buffer offset (Specifies a pointer to the first generic vertex attribute in the array)
			);
		offset += sizeof(pVtxAttr->position);

		//normal
		size = sizeof(pVtxAttr->normal) / sizeof(float);
		glEnableVertexAttribArray(1);	//enable attribute Location
		glVertexAttribPointer(
			1,					// attribute 0. No particular reason for 0, but must match the layout in the shader.
			size,				// size
			GL_FLOAT,			// type
			GL_FALSE,			// normalized?
			stride,				// stride
			(void*)offset		// array buffer offset
			);
		offset += sizeof(pVtxAttr->normal);


		//color
		size = sizeof(pVtxAttr->color) / sizeof(float);
		glEnableVertexAttribArray(2);	//enable attribute Location
		glVertexAttribPointer(
			2,					// attribute 0. No particular reason for 0, but must match the layout in the shader.
			size,				// size
			GL_FLOAT,			// type
			GL_FALSE,			// normalized?
			stride,				// stride
			(void*)offset		// array buffer offset
			);
		offset += sizeof(pVtxAttr->color);

		//texCoord
		size = sizeof(pVtxAttr->texCoord) / sizeof(float);
		glEnableVertexAttribArray(3);	//enable attribute Location
		glVertexAttribPointer(
			3,					// attribute 0. No particular reason for 0, but must match the layout in the shader.
			size,				// size
			GL_FLOAT,			// type
			GL_FALSE,			// normalized?
			stride,				// stride
			(void*)offset		// array buffer offset
			);
		offset += sizeof(pVtxAttr->texCoord);

		glBindVertexArray(0);
	}

}

void GlModel::BindIBO(void){

	if (ibo) {
		glBindVertexArray(vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glBindVertexArray(0);

	}


}


void GlModel::Draw(void){
	if (vao == 0) return;

	glBindVertexArray(vao);

	glEnableVertexAttribArray(0);
	GLsizei elements = (GLsizei)model->vecIndex.size()*sizeof(u16vec3);
	glDrawElements(GL_TRIANGLES, elements, GL_UNSIGNED_SHORT, 0);
	glDisableVertexAttribArray(0);

	glBindVertexArray(0);
}

void GlModel::DrawPoints(void){
	if (vao == 0) return;

	glBindVertexArray(vao);
	GLsizei points = (GLsizei)model->vecVertexAttribute.size();
	glEnableVertexAttribArray(0);
	glDrawArrays(GL_POINTS, 0, points);
	glDisableVertexAttribArray(0);
	glBindVertexArray(0);
}




void GlModel::DrawInstansing(const GLuint vboInstPos, int instCount){
	if (vao == 0) return;

	glBindVertexArray(vao);		//Bind Model VAO

	int attrLoc = 3;		////@@Todo glGetAttribLocation  

	glEnableVertexAttribArray(attrLoc);	//enable attribute Location  
	glVertexAttribPointer(
		attrLoc,					// attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,				// size	(Specifies the number of components)
		GL_FLOAT,			// type
		GL_FALSE,			// normalized?
		0,				// stride (Specifies the byte offset between consecutive generic vertex attributes)
		(void*)0		// array buffer offset (Specifies a pointer to the first generic vertex attribute in the array)
		);


	// これらの関数はglDrawArrays *Instanced* 特有です。
	// 最初のパラメータは注目してる属性バッファです。
	// 二つ目のパラメータは、複数のインスタンスを描画するときに一般的な頂点属性が進む割合を意味します。
	// http://www.opengl.org/sdk/docs/man/xhtml/glVertexAttribDivisor.xml

	// インスタンスを有効化し除数を指定する
	glVertexAttribDivisor(attrLoc, 1); // 位置：modelごとに一つ（中心）->1

	GLsizei elements = (GLsizei)model->vecIndex.size()*sizeof(u16vec3);
	glDrawElementsInstanced(GL_TRIANGLES, elements, GL_UNSIGNED_SHORT, 0, instCount);

	glBindVertexArray(0);

}




PureModelSphere::PureModelSphere(const int rows, const int cols, const float rad)
{
	vecVertexAttribute.clear();
	vecIndex.clear();

	for (int i = 0; i < rows + 1; i++){
		float r = (float)M_PI / (float)rows * i;
		float ry = cos(r);
		float rr = sin(r);

		for (int j = 0; j < cols + 1; j++){
			float tr = (float)M_PI * 2 / (float)cols * j;
			float tx = rr * rad * cos(tr);
			float ty = ry * rad;
			float tz = rr * rad * sin(tr);
			float rx = rr * cos(tr);
			float rz = rr * sin(tr);

			VertexAttribute vertexAttr;
			vertexAttr.position = vec3(tx, ty, tz);
			vertexAttr.normal = vec3(rx, ry, rz);
//			vertexAttr.color = vec4(1.0, 0.0, 1.0, 1.0);
			vec3 hsv(360.0f / (float)rows * i, 1.0, 1.0);
			vertexAttr.color = vec4(hsv2rgb(hsv), 1.0);
			vertexAttr.texCoord = vec2(1.0 - j / (float)cols, i / (float)rows);

			vecVertexAttribute.push_back(vertexAttr);
		}

		for (int i = 0; i < rows; i++){
			for (int j = 0; j < cols; j++){
				int r = (cols + 1) * i + j;
				vecIndex.push_back(u16vec3(r, r + 1, r + cols + 2));
				vecIndex.push_back(u16vec3(r, r + cols + 2, r + cols + 1));
			}
		}

	}

}

/*
for(var i = 0; i <= row; i++){
var r = Math.PI / row * i;
var ry = Math.cos(r);
var rr = Math.sin(r);
for(var ii = 0; ii <= column; ii++){
var tr = Math.PI * 2 / column * ii;
var tx = rr * rad * Math.cos(tr);
var ty = ry * rad;
var tz = rr * rad * Math.sin(tr);
var rx = rr * Math.cos(tr);
var rz = rr * Math.sin(tr);
if(color){
var tc = color;
}else{
tc = hsva2rgba(360 / row * i, 1, 1, 1);
}
pos.push(tx, ty, tz);
nor.push(rx, ry, rz);
col.push(tc[0], tc[1], tc[2], tc[3]);
st.push(1 - 1 / column * ii, 1 / row * i);
}
}
r = 0;
for(i = 0; i < row; i++){
for(ii = 0; ii < column; ii++){
r = (column + 1) * i + ii;
idx.push(r, r + 1, r + column + 2);
idx.push(r, r + column + 2, r + column + 1);
}
}
return {p : pos, n : nor, c : col, t : st, i : idx};

*/

PureModelCube::PureModelCube(const float side){
	const float hs = side*0.5f;
	vecVertexAttribute.resize(4 * 6);
	int i = 0;
	vecVertexAttribute[i++].position = vec3(-hs, -hs, hs);
	vecVertexAttribute[i++].position = vec3(hs, -hs, hs);
	vecVertexAttribute[i++].position = vec3(hs, hs, hs);
	vecVertexAttribute[i++].position = vec3(-hs, hs, hs);

	vecVertexAttribute[i++].position = vec3(-hs, -hs, -hs);
	vecVertexAttribute[i++].position = vec3(-hs, hs, -hs);
	vecVertexAttribute[i++].position = vec3(hs, hs, -hs);
	vecVertexAttribute[i++].position = vec3(hs, -hs, -hs);

	vecVertexAttribute[i++].position = vec3(-hs, hs, -hs);
	vecVertexAttribute[i++].position = vec3(-hs, hs, hs);
	vecVertexAttribute[i++].position = vec3(hs, hs, hs);
	vecVertexAttribute[i++].position = vec3(hs, hs, -hs);

	vecVertexAttribute[i++].position = vec3(-hs, -hs, -hs);
	vecVertexAttribute[i++].position = vec3(hs, -hs, -hs);
	vecVertexAttribute[i++].position = vec3(hs, -hs, hs);
	vecVertexAttribute[i++].position = vec3(-hs, -hs, hs);

	vecVertexAttribute[i++].position = vec3(hs, -hs, -hs);
	vecVertexAttribute[i++].position = vec3(hs, hs, -hs);
	vecVertexAttribute[i++].position = vec3(hs, hs, hs);
	vecVertexAttribute[i++].position = vec3(hs, -hs, hs);

	vecVertexAttribute[i++].position = vec3(-hs, -hs, -hs);
	vecVertexAttribute[i++].position = vec3(-hs, -hs, hs);
	vecVertexAttribute[i++].position = vec3(-hs, hs, hs);
	vecVertexAttribute[i++].position = vec3(-hs, hs, -hs);

	i = 0;
	vecVertexAttribute[i++].normal = vec3(-1.0, -1.0, 1.0);
	vecVertexAttribute[i++].normal = vec3(1.0, -1.0, 1.0);
	vecVertexAttribute[i++].normal = vec3(1.0, 1.0, 1.0);
	vecVertexAttribute[i++].normal = vec3(-1.0, 1.0, 1.0);

	vecVertexAttribute[i++].normal = vec3(-1.0, -1.0, -1.0);
	vecVertexAttribute[i++].normal = vec3(-1.0, 1.0, -1.0);
	vecVertexAttribute[i++].normal = vec3(1.0, 1.0, -1.0);
	vecVertexAttribute[i++].normal = vec3(1.0, -1.0, -1.0);

	vecVertexAttribute[i++].normal = vec3(-1.0, 1.0, -1.0);
	vecVertexAttribute[i++].normal = vec3(-1.0, 1.0, 1.0);
	vecVertexAttribute[i++].normal = vec3(1.0, 1.0, 1.0);
	vecVertexAttribute[i++].normal = vec3(1.0, 1.0, -1.0);

	vecVertexAttribute[i++].normal = vec3(-1.0, -1.0, -1.0);
	vecVertexAttribute[i++].normal = vec3(1.0, -1.0, -1.0);
	vecVertexAttribute[i++].normal = vec3(1.0, -1.0, 1.0);
	vecVertexAttribute[i++].normal = vec3(-1.0, -1.0, 1.0);

	vecVertexAttribute[i++].normal = vec3(1.0, -1.0, -1.0);
	vecVertexAttribute[i++].normal = vec3(1.0, 1.0, -1.0);
	vecVertexAttribute[i++].normal = vec3(1.0, 1.0, 1.0);
	vecVertexAttribute[i++].normal = vec3(1.0, -1.0, 1.0);

	vecVertexAttribute[i++].normal = vec3(-1.0, -1.0, -1.0);
	vecVertexAttribute[i++].normal = vec3(-1.0, -1.0, 1.0);
	vecVertexAttribute[i++].normal = vec3(-1.0, 1.0, 1.0);
	vecVertexAttribute[i++].normal = vec3(-1.0, 1.0, -1.0);


	for (int i = 0; i < vecVertexAttribute.size(); i++){
		vec3 hsv(360.0f / (float)i, 1.0, 1.0);
		vecVertexAttribute[i].color = vec4(hsv2rgb(hsv), 1.0);
	}


	vecIndex.resize(2 * 6);
	i = 0;
	vecIndex[i++] = u16vec3(0, 1, 2);
	vecIndex[i++] = u16vec3(0, 2, 3);

	vecIndex[i++] = u16vec3(4, 5, 6);
	vecIndex[i++] = u16vec3(4, 6, 7);

	vecIndex[i++] = u16vec3(8, 9, 10);
	vecIndex[i++] = u16vec3(8, 10, 11);

	vecIndex[i++] = u16vec3(12, 13, 14);
	vecIndex[i++] = u16vec3(12, 14, 15);

	vecIndex[i++] = u16vec3(16, 17, 18);
	vecIndex[i++] = u16vec3(16, 18, 19);

	vecIndex[i++] = u16vec3(20, 21, 22);
	vecIndex[i++] = u16vec3(20, 22, 23);



}

/*
for(var i = 0; i < pos.length / 3; i++){
if(color){
var tc = color;
}else{
tc = hsva2rgba(360 / pos.length / 3 * i, 1, 1, 1);
}
col.push(tc[0], tc[1], tc[2], tc[3]);
}

*/
