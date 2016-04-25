// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、または
// 参照回数が多く、かつあまり変更されない、プロジェクト固有のインクルード ファイル
// を記述します。
//

#pragma once

#include "targetver.h"

// CppUnitTest のヘッダー
#include "CppUnitTest.h"


#include <stdio.h>
#include <tchar.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
using namespace std;


// TODO: プログラムに必要な追加ヘッダーをここで参照してください。
//---------------------
// OpenCV
#include <opencv2/opencv.hpp>
using namespace cv;


//---------------------
// GL
// Include GLEW
#include <GL/glew.h>

// Include GLFW
#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;
