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
bool AlmostEqualUlpsAbsEps(float A, float B, int maxUlps , float maxDiff = 1e-3)
{
	// Check if the numbers are really close -- needed
	// when comparing numbers near zero.
	float absDiff = fabs(A - B);
	if (absDiff <= maxDiff)
		return true;

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

//	const int N = 4;

//	const int N = 256;
//	const int N = 512;
	const int N = 1024;


	Mat imgSrc = Mat(Size(N, N), CV_32FC2);
	Mat imgFft = Mat::zeros(imgSrc.size(), imgSrc.type());
	Mat imgFftRef = Mat::zeros(imgSrc.size(), imgSrc.type());

	Mat imgIfft = Mat::zeros(imgSrc.size(), imgSrc.type());
	Mat imgIfftRef = Mat::zeros(imgSrc.size(), imgSrc.type());



	cout << "Size:" << imgSrc.size() << endl;

	//---------------------------------
	//init Src image
	{
		RNG rng(0xFFFFFFFF);

		const int width = imgSrc.cols;
		const int height = imgSrc.rows;
		for (int y = 0; y < height; y++){
			for (int x = 0; x < width; x++){
//				imgSrc.at<Vec2f>(y,x) = Vec2f((float)(x+y),0.0);
				imgSrc.at<Vec2f>(y, x) = Vec2f((float)rng.gaussian(1.0), (float)rng.gaussian(1.0));

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
		flags |= DFT_SCALE;
		cv::dft(imgSrc, imgFftRef, flags);

		//imgFftRef = imgSrc.clone();
	}
	{
		Timer tmr("cv:dft:   \t");
		int flags = 0;
		flags |= DFT_INVERSE;
		cv::dft(imgFftRef, imgIfftRef, flags);
	}



#else
	{
		Timer tmr("fft_dit_Stockham_radix2_type1:   \t");
		fft_dit_Stockham_radix2_type1(imgSrc, imgFftRef);
	}
#endif




#if 1
	{
		//1回目は遅い　なぜ？
		Timer tmr("glslFft:\t");
		Mat tmp;
		int flags = 0;
		flags |= GLSL_FFT_SCALE;
		glslFft(imgSrc, tmp, flags);
	}
	{
		Timer tmr("glslFft:\t");
		int flags = 0;
		flags |= GLSL_FFT_SCALE;
		glslFft(imgSrc, imgFft, flags);
	}

#elif 1
	{
		Timer tmr("fft_dit_Stockham_radix2:\t");
		//fft_dit_Stockham_radix2_type0(imgSrc, imgFft);
		fft_dit_Stockham_radix2_type1(imgSrc, imgFft);

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

#if 1
	{
		Timer tmr("glslFft(I):\t");
		Mat tmp;
		int flags = 0;
		flags |= GLSL_FFT_INVERSE;
		glslFft(imgFft, tmp, flags);
	}
	{
		Timer tmr("glslFft(I):\t");
		int flags = 0;
		flags |= GLSL_FFT_INVERSE;
		glslFft(imgFft, imgIfft, flags);
	}
#endif

//	int ULPS = MAX(imgSrc.cols / 4, 16);
	int ULPS = imgSrc.cols * 8;
//	int ULPS = 4096;


//	int ULPS = 16;

#if 0
	//dump 
	if (imgSrc.cols<=8){
		cout << "imgSrc" << endl;
		cout << imgSrc << endl;

		cout << "imgFftRef" << endl;
		cout << imgFftRef << endl;

		cout << "imgFft" << endl;
		cout << imgFft << endl;

		cout << "imgIfftRef" << endl;
		cout << imgIfftRef << endl;

		cout << "imgIfft" << endl;
		cout << imgIfft << endl;


	}
#elif 1
	{

		Mat imgErr = imgFft - imgFftRef;
		for (int y = 0; y < imgSrc.rows; y++){
			for (int x = 0; x < imgSrc.cols; x++){
				Vec2f src = imgSrc.at<Vec2f>(y, x);
				Vec2f dst = imgFft.at<Vec2f>(y, x);
				Vec2f ref = imgFftRef.at<Vec2f>(y, x);
				Vec2f err = imgErr.at<Vec2f>(y, x);

				if (!AlmostEqualUlpsAbsEps(dst[0], ref[0], ULPS)){
					cout << cv::format("r(%4d,%4d)\t", x, y);
					cout << cv::format("%8g\t", dst[0]);
					cout << cv::format("%8g\t", ref[0]);
					cout << cv::format("%8g\t", err[0]);
					cout << endl;
				}

				if (!AlmostEqualUlpsAbsEps(dst[1], ref[1], ULPS)){
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
				Vec2f ref = imgFftRef.at<Vec2f>(y, x);
				Vec2f dst = imgFft.at<Vec2f>(y, x);
				if (!AlmostEqualUlpsAbsEps(ref[0], dst[0], ULPS)){
					errNum++;
				}
				if (!AlmostEqualUlpsAbsEps(ref[1], dst[1], ULPS)){
					errNum++;
				}
			}
		}

#if 1
		for (int y = 0; y < height; y++){
			for (int x = 0; x < width; x++){
				Vec2f ref = imgIfftRef.at<Vec2f>(y, x);
				Vec2f dst = imgIfft.at<Vec2f>(y, x);
				if (!AlmostEqualUlpsAbsEps(ref[0], dst[0], ULPS)){
					errNum++;
				}
				if (!AlmostEqualUlpsAbsEps(ref[1], dst[1], ULPS)){
					errNum++;
				}
			}
		}
#endif



		cout << "ErrNum:" << errNum << endl;
	}

#if 0
	//visualize
	{
		imshow("src", imgSrc);
		imshow("dst", imgFft);
		waitKey();
	}
#endif

	
	cout << "Hit return key" << endl;
	cin.get();


	glslFftTerminate();
	return errNum;
}











