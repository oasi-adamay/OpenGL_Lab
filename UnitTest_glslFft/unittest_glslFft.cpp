#include "stdafx.h"
#include "CppUnitTest.h"
#include "HookCoutCerr.h"
#include "Timer.h"

#include "../GPGPU_test_fft/glslFft.h"

//Lib 
#pragma comment (lib, "opengl32.lib")
#pragma comment (lib, "glew32.lib")
#pragma comment (lib, "glfw3dll.lib")


using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest_glslFft
{	
	HookCoutCerr hook;
#if 0
	TEST_MODULE_INITIALIZE(test_module_initialize) {
		cout << __FUNCTION__ << endl;
		glslFftInit();

	}
	TEST_MODULE_CLEANUP(test_module_cleanup)  {
		cout << __FUNCTION__ << endl;
		glslFftTerminate();
	}
#endif

	static
	bool AlmostEqualUlpsAbsEps(float A, float B, int maxUlps, float maxDiff = 1e-3)
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

	int test_glslFft(const int N, const int flags){
		int ULPS = 4096;
		float EPS = 1e-3;
		Mat imgSrc = Mat(Size(N, N), CV_32FC2);
		Mat imgFft = Mat::zeros(imgSrc.size(), imgSrc.type());
		Mat imgFftRef = Mat::zeros(imgSrc.size(), imgSrc.type());

		cout << "Size:" << imgSrc.size() << endl;

		//---------------------------------
		//init Src image
		{
			RNG rng(0xFFFFFFFF);

			const int width = imgSrc.cols;
			const int height = imgSrc.rows;
			for (int y = 0; y < height; y++){
				for (int x = 0; x < width; x++){
					imgSrc.at<Vec2f>(y, x) = Vec2f((float)rng.gaussian(1.0), (float)rng.gaussian(1.0));
				}
			}
		}
		//---------------------------------
		//CPU FFT(cv::dft)
		{
			Timer tmr("cv:dft:   \t");
			cv::dft(imgSrc, imgFftRef, flags);
		}

		//---------------------------------
		{
			Timer tmr("glslFft:  \t");
			int _flags = 0;
			if (flags & DFT_SCALE)	_flags |= GLSL_FFT_SCALE;
			if (flags & DFT_INVERSE)_flags |= GLSL_FFT_INVERSE;

			glslFft(imgSrc, imgFft, _flags);
		}

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
					Vec2f err = dst - ref;
					if (!AlmostEqualUlpsAbsEps(ref[0], dst[0], ULPS, EPS)){
						cout << cv::format("r(%4d,%4d)\t", x, y);
						cout << cv::format("%8g\t", dst[0]);
						cout << cv::format("%8g\t", ref[0]);
						cout << cv::format("%8g\t", err[0]);
						cout << endl;
						errNum++;
					}
					if (!AlmostEqualUlpsAbsEps(ref[1], dst[1], ULPS, EPS)){
						cout << cv::format("i(%4d,%4d)\t", x, y);
						cout << cv::format("%8g\t", dst[1]);
						cout << cv::format("%8g\t", ref[1]);
						cout << cv::format("%8g\t", err[1]);
						cout << endl;
						errNum++;
					}
				}
			}
		}
		cout << "errNum:" << errNum << endl;

		return errNum;
	}


	TEST_CLASS(UnitTest_glslFft)
	{
	public:

		TEST_CLASS_INITIALIZE(test_class_initialize)
		{
			cout << __FUNCTION__ << endl;
			glslFftInit();
		}

		TEST_CLASS_CLEANUP(test_class_cleanup)
		{
			cout << __FUNCTION__ << endl;
			glslFftTerminate();
		}


		TEST_METHOD(TestMethod_FFT)
		{
			cout << __FUNCTION__ << endl;
			const int N = 256;
			const int flags = 0;
			int errNum = test_glslFft(N, flags);
			Assert::AreEqual(0, errNum);
		}

		TEST_METHOD(TestMethod_FFT_SCALE)
		{
			cout << __FUNCTION__ << endl;
			const int N = 128;
			const int flags = DFT_SCALE;
			int errNum = test_glslFft(N, flags);
			Assert::AreEqual(0, errNum);
		}

		TEST_METHOD(TestMethod_IFFT)
		{
			cout << __FUNCTION__ << endl;
			const int N = 64;
			const int flags = DFT_INVERSE;
			int errNum = test_glslFft(N, flags);
			Assert::AreEqual(0, errNum);
		}

		TEST_METHOD(TestMethod_IFFT_SCALE)
		{
			cout << __FUNCTION__ << endl;
			const int N = 32;
			const int flags = DFT_INVERSE + DFT_SCALE;
			int errNum = test_glslFft(N, flags);
			Assert::AreEqual(0, errNum);
		}



	};
}