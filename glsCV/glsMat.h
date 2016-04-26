#ifndef _GLS_MAT_H_
#define _GLS_MAT_H_

class glsMat
{
private:
	glsMat(const glsMat&);              ///  Uncopyable
	glsMat& operator=(const glsMat&);   ///  Uncopyable
public:
	vector<GLuint> texArray;
	int width;
	int height;
	int blkX;
	int blkY;
	GLenum format;
	GLenum type;
	GLenum internalFormat;


	glsMat(const int _width, const int _height, const int _blkX, const int _blkY);
	~glsMat(void);

	GLuint at(const int y, const int x);
	void CopyFrom(const Mat&src);
	void CopyTo(Mat&dst);

};




#endif