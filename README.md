# OpenGL_Lab
OpenGLの実験リポジトリ

## 環境
 * Windows8
 * VC2013
  * NupenGL 0.1.0.1  (glew,glfw)
  * glm 0.9.7.1
  * OpenCV 2.1.10
  
## Project
###GPGPU_test_copy
GLSLで、Texture　から　Textureへのコピーを試してみる。

###GPGPU_test_lifegame
LifeGameをGLSLで実装してみる。  
1024x1024のCELLサイズで1000世代の遷移にかかる時間を計測  
CPU(with OpenMP)実行とGpGPU実行のパフォーマンスを比較する。  

###LifeGameEarth
GPUで、LifeGameのCELLの状態遷移を実行しつつ、地球の画像を張ったスフィアに、レンダリングしてみる。  





