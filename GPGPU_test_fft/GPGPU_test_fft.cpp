// GPGPU_test_copy.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "../common/shader.hpp"
#include "glslFft.h"
#include "HookCoutCerr.hpp"
#include "Timer.hpp"




//Lib 
#pragma comment (lib, "opengl32.lib")
#pragma comment (lib, "glew32.lib")
#pragma comment (lib, "glfw3dll.lib")

// Usable AlmostEqual function
bool AlmostEqual2sComplement(float A, float B, int maxUlps)
{
	// Make sure maxUlps is non-negative and small enough that the
	// default NAN won't compare as equal to anything.
	assert(maxUlps > 0 && maxUlps < 4 * 1024 * 1024);
	int aInt = *(int*)&A;
	// Make aInt lexicographically ordered as a twos-complement int
	if (aInt < 0)
		aInt = 0x80000000 - aInt;
	// Make bInt lexicographically ordered as a twos-complement int
	int bInt = *(int*)&B;
	if (bInt < 0)
		bInt = 0x80000000 - bInt;
	int intDiff = abs(aInt - bInt);
	if (intDiff <= maxUlps)
		return true;
	return false;
}






void fft_dit_Stockham_radix2_type0(const Mat& src, Mat &dst);
void fft_dit_Stockham_radix2_type1(const Mat& src, Mat &dst);

typedef std::complex<double> complex_t;
void fft_type0_dif(int n, complex_t* x);
void fft_type0_dit(int n, complex_t* x);
void fft_type1_dif(int n, complex_t* x);
void fft_type1_dit(int n, complex_t* x);


int _tmain(int argc, _TCHAR* argv[])
{
//	HookCoutCerr hoge;

	glslFftInit();

//	Mat imgSrc = Mat(Size(4, 1), CV_32FC2);
//	Mat imgSrc = Mat(Size(16, 1), CV_32FC2);
//	Mat imgSrc = Mat(Size(8, 1), CV_32FC2);

//	Mat imgSrc = Mat(Size(4, 4), CV_32FC2);
//	Mat imgSrc = Mat(Size(8, 8), CV_32FC2);
	Mat imgSrc = Mat(Size(16, 16), CV_32FC2);

//	Mat imgSrc = Mat(Size(256, 256), CV_32FC2);
//	Mat imgSrc = Mat(Size(1024, 1024), CV_32FC2);
//	Mat imgSrc = Mat(Size(4096, 4096), CV_32FC2);

	Mat imgDst = Mat::zeros(imgSrc.size(), imgSrc.type());
	Mat imgRef = Mat::zeros(imgSrc.size(), imgSrc.type());

	cout << "Size:" << imgSrc.size() << endl;

	//---------------------------------
	//init Src image
	{
		//imgSrc.at<Vec2f>(0, 0) = Vec2f(1.5f,-1.0f);
		//imgSrc.at<Vec2f>(0, 1) = Vec2f(-2.3f, 2.6f);
		//imgSrc.at<Vec2f>(0, 2) = Vec2f(4.65f, 3.75f);
		//imgSrc.at<Vec2f>(0, 3) = Vec2f(-3.51f, -2.32f);

		//	double r[4] = { 1.5, -2.3, 4.65, -3.51 }, i[4] = { -1.0, 2.6, 3.75, -2.32 };

		const int width = imgSrc.cols;
		const int height = imgSrc.rows;
		for (int y = 0; y < height; y++){
			for (int x = 0; x < width; x++){
				imgSrc.at<Vec2f>(y,x) = Vec2f((float)(x+y),0.0);
			}
		}
	}
#if 1
	//---------------------------------
	//CPU FFT(cv::dft)
	{
		Timer tmr("cv:dft:   \t");
		const int width = imgSrc.cols;
		const int height = imgSrc.rows;

		int flags = 0;
//		flags |= DFT_SCALE;
		cv::dft(imgSrc, imgRef, flags);

		//imgRef = imgSrc.clone();
	}
#else
	{
		fft_dit_Stockham_radix2_type1(imgSrc, imgRef);
	}
#endif




#if 1
	{
		//1回目は遅い　なぜ？
		Timer tmr("glslFft:\t");
		glslFft(imgSrc, imgDst);
	}
	{
		Timer tmr("glslFft:\t");
		glslFft(imgSrc, imgDst);
	}
#elif 1
	{
		Timer tmr("fft_dit_Stockham_radix2:\t");
		//fft_dit_Stockham_radix2_type0(imgSrc, imgDst);
		fft_dit_Stockham_radix2_type1(imgSrc, imgDst);

	}
#else 
	{
		const int n = 8;
		complex_t* y = new complex_t[n];
//		cout << "==fft_type0_dif==" << endl;
//		fft_type0_dif(n, y);
//		cout << "==fft_type1_dif==" << endl;
//		fft_type1_dif(n, y);
		cout << "==fft_type0_dit==" << endl;
		fft_type0_dit(n, y);
//		cout << "==fft_type1_dit==" << endl;
//		fft_type1_dit(n, y);

		delete y;
	}
#endif

//	int ULPS = MAX(imgSrc.cols / 4, 65536);
	int ULPS = 16;

#if 0
	//dump 
	if (imgSrc.cols<=8){
		cout << "imgSrc" << endl;
		cout << imgSrc << endl;

		cout << "imgRef" << endl;
		cout << imgRef << endl;

		cout << "imgDst" << endl;
		cout << imgDst << endl;

		cout << "errt" << endl;
		cout << imgDst - imgRef << endl;

	}
#elif 1
	{

		Mat imgErr = imgDst - imgRef;
		for (int y = 0; y < imgSrc.rows; y++){
			for (int x = 0; x < imgSrc.cols; x++){
				Vec2f src = imgSrc.at<Vec2f>(y, x);
				Vec2f dst = imgDst.at<Vec2f>(y, x);
				Vec2f ref = imgRef.at<Vec2f>(y, x);
				Vec2f err = imgErr.at<Vec2f>(y, x);

				if (!AlmostEqual2sComplement(dst[0], ref[0], ULPS)){
					cout << cv::format("r(%4d,%4d)\t", x, y);
					cout << cv::format("%8g\t", dst[0]);
					cout << cv::format("%8g\t", ref[0]);
					cout << cv::format("%8g\t", err[0]);
					cout << endl;
				}

				if (!AlmostEqual2sComplement(dst[1], ref[1], ULPS)){
					cout << cv::format("i(%4d,%4d)\t", x, y);
					//				cout << cv::format("%8.2e\t", src[1]);
					cout << cv::format("%8g\t", dst[1]);
					cout << cv::format("%8g\t", ref[1]);
					cout << cv::format("%8g\t", err[1]);
					cout << endl;
				}

			}
		}

	}
#endif

	//verify
	int errNum = 0;
	{
		//verify
		int width = imgSrc.cols;
		int height = imgSrc.rows;
		for (int y = 0; y < height; y++){
			for (int x = 0; x < width; x++){
				Vec2f ref = imgRef.at<Vec2f>(y, x);
				Vec2f dst = imgDst.at<Vec2f>(y, x);
				if (!AlmostEqual2sComplement(ref[0], dst[0], ULPS)){
					errNum++;
				}
				if (!AlmostEqual2sComplement(ref[1], dst[1], ULPS)){
					errNum++;
				}


//				if (fabs(ref[0] - dst[0]) > EPS) errNum++;
//				if (fabs(ref[1] - dst[1]) > EPS) errNum++; 
			}
		}
		cout << "ErrNum:" << errNum << endl;
	}

#if 0
	//visualize
	{
		imshow("src", imgSrc);
		imshow("dst", imgDst);
		waitKey();
	}
#endif

	
	cout << "Hit return key" << endl;
	cin.get();


	glslFftTerminate();
	return errNum;
}











