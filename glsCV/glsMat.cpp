#include "stdafx.h"


#include "glsMat.h"
#include "Timer.h"



#define _USE_PBO_UP
#define _USE_PBO_DOWN



glsMat::glsMat(
	const int _width,				// orignal image width
	const int _height,				// orignal image height
	const int _blkX,				// block num X
	const int _blkY					// block num Y
){
	assert((_width%_blkX) == 0);
	assert((_height%_blkY) == 0);

	format = GL_RG;
	type = GL_FLOAT;
	internalFormat = GL_RG32F;

	width = _width;
	height = _height;
	blkX = _blkX;
	blkY = _blkY;
	int texWidth  = width / blkX;
	int texHeight = height / blkY;

	texArray.resize(blkY*blkX);

	glGenTextures((GLsizei)texArray.size(), &texArray[0]); // create (reference to) a new texture

	for (int i = 0; i < (int)texArray.size(); i++){
		glBindTexture(GL_TEXTURE_RECTANGLE, texArray[i]);
		// (set texture parameters here)
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		//create the texture
		glTexImage2D(GL_TEXTURE_RECTANGLE, 0, internalFormat, texWidth, texHeight, 0, format, type, 0);

		glBindTexture(GL_TEXTURE_RECTANGLE, 0);
	}
}

glsMat::~glsMat(void){
	glDeleteTextures((GLsizei)texArray.size(), &texArray[0]);
}

GLuint glsMat::at(const int y, const int x){
	assert(y*blkX + x < texArray.size());
	return texArray[y*blkX + x];
}

//-----------------------------------------------------------------------------
//Upload texture from cv::Mat to GL texture
void glsMat::CopyFrom(const Mat&src){
	Timer tmr("-upload :\t");
	CV_Assert(src.type() == CV_32FC2);
	CV_Assert(src.cols == src.rows);
	CV_Assert(src.cols == width);
	CV_Assert(src.rows == height);

	// texture
	const int texWidth = src.cols / blkX;
	const int texHeight = src.rows / blkY;

	{
#ifdef _USE_PBO_UP
		int size = texWidth*texHeight * (int)src.elemSize();
		vector<GLuint> pbo(texArray.size());
		glGenBuffers((GLsizei)pbo.size(), &pbo[0]);
		for (int i = 0; i < pbo.size(); i++){
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo[i]);
			glBufferData(GL_PIXEL_UNPACK_BUFFER, size, 0, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
		}


		for (int by = 0; by < blkY; by++){
			for (int bx = 0; bx < blkX; bx++){
				int i = by*blkX + bx;
				int x = (bx)* texWidth;
				int y = (by)* texHeight;
				Rect rect(x, y, texWidth, texHeight);
				Mat roi = Mat(src, rect);	// 1/2  1/2 rect

				//bind current pbo for app->pbo transfer
				glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo[i]); //bind pbo
				GLubyte* ptr = (GLubyte*)glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, size,
					GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
				assert(ptr != 0);

				int lSize = roi.cols * (int)roi.elemSize();	// line size in byte
#ifdef _OPENMP
#pragma omp parallel for
#endif
				for (int y = 0; y < roi.rows; y++){
					uchar* pSrc = roi.ptr<uchar>(y);
					uchar* pDst = ptr + lSize * y;
					memcpy(pDst, pSrc, lSize);
				}
				glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

				//Copy pixels from pbo to texture object
				glBindTexture(GL_TEXTURE_RECTANGLE, texArray[i]);
				glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo[i]); //bind pbo
				glTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, 0, 0, texWidth, texHeight, format, type, 0);
				glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
				glBindTexture(GL_TEXTURE_RECTANGLE, 0);
			}
		}

		glDeleteBuffers((GLsizei)pbo.size(), &pbo[0]);
#else
		for (int by = 0; by < blkY; by++){
			for (int bx = 0; bx < blkX; bx++){
				int i = by*blkX + bx;
				int x = (bx)* texWidth;
				int y = (by)* texHeight;
				Rect rect(x, y, texWidth, texHeight);
				Mat roi = Mat(src, rect).clone();	// 1/2  1/2 rect
				CV_Assert(roi.isContinuous());
				void* data = roi.data;

				glBindTexture(GL_TEXTURE_RECTANGLE, texArray[i]);
				glTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, 0, 0, texWidth, texHeight, format, type, data);
				glBindTexture(GL_TEXTURE_RECTANGLE, 0);
			}
		}
#endif
	}

}

void glsMat::CopyTo(Mat&dst){
	Timer tmr("-download:\t");

	dst = Mat(Size(width,height), CV_32FC2);

	// texture
	const int texWidth = width / blkX;
	const int texHeight = height / blkY;

	{	//download from texture
#ifdef _USE_PBO_DOWN
		int size = texWidth*texHeight * (int)dst.elemSize();
		vector<GLuint> pbo(texArray.size());
		glGenBuffers((GLsizei)pbo.size(), &pbo[0]);
		for (int i = 0; i < pbo.size(); i++){
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo[i]);
			glBufferData(GL_PIXEL_UNPACK_BUFFER, size, 0, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
		}
		for (int by = 0; by < blkY; by++){
			for (int bx = 0; bx < blkX; bx++){
				int i = by*blkX + bx;
				int x = (bx)* texWidth;
				int y = (by)* texHeight;

				Rect rect(x, y, texWidth, texHeight);
				Mat roi = Mat(dst, rect);	// 1/2  1/2 rect

				//Copy pixels from texture object to pbo_bank
				glBindTexture(GL_TEXTURE_RECTANGLE, texArray[i]);
				glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo[i]); //bind pbo
				glGetTexImage(GL_TEXTURE_RECTANGLE, 0, format, type, 0);
				glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
				glBindTexture(GL_TEXTURE_RECTANGLE, 0);

				//bind current pbo for app->pbo transfer
				glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo[i]); //bind pbo
				GLubyte* ptr = (GLubyte*)glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, size,
					GL_MAP_READ_BIT);
				assert(ptr != 0);

				int lSize = roi.cols * (int)roi.elemSize();	// line size in byte
#ifdef _OPENMP
#pragma omp parallel for
#endif
				for (int y = 0; y < roi.rows; y++){
					uchar* pSrc = ptr + lSize * y;
					uchar* pDst = roi.ptr<uchar>(y);
					memcpy(pDst, pSrc, lSize);
				}
				glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
			}
		}
		glDeleteBuffers((GLsizei)pbo.size(), &pbo[0]);
#else
		Mat tmp = Mat(Size(texWidth, texHeight), dst.type());
		for (int by = 0; by < blkY; by++){
			for (int bx = 0; bx < blkX; bx++){
				int i = by*blkX + bx;
				int x = (bx)* texWidth;
				int y = (by)* texHeight;
				void* data = tmp.data;

				glBindTexture(GL_TEXTURE_RECTANGLE, texArray[i]);
				glGetTexImage(GL_TEXTURE_RECTANGLE, 0, format, type, data);
				glBindTexture(GL_TEXTURE_RECTANGLE, 0);

				Rect rect(x, y, texWidth, texHeight);
				Mat roi = Mat(dst, rect);	// 1/2  1/2 rect
				tmp.copyTo(roi);
			}
		}
#endif
	}


}






