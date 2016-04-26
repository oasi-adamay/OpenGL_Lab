#version 330 core
precision highp float;
uniform sampler2DRect	texSrc0;
uniform sampler2DRect	texSrc1;
uniform sampler2DRect	texW;
uniform  int i_flag;	//bit0:(0:holizontal 1:vertical)
uniform  int i_N;
uniform  int i_p;
uniform  int i_q;
uniform  float f_xscl;
uniform  float f_yscl;
uniform  float f_xconj;
uniform  float f_yconj;

layout (location = 0) out vec2 dst0;
layout (location = 1) out vec2 dst1;

#define FLAG_DIR	 (1<<0)


int insertZeroBits(
	const int src,
	const int idx,
	const int num
	)
{
	int ret = src << num;
	ret &= ~((1 << (idx + num)) - 1);
	ret |= src & ((1 << idx) - 1);
	return ret;
}

void main(void)
{
	int p = i_p;
	int q = i_q;
	int N = i_N;
	int dir = ((i_flag & FLAG_DIR)==0) ?0:1;
	float xscl = f_xscl;
	float yscl = f_yscl;
	float xconj = f_xconj;
	float yconj = f_yconj;

	int n;
	vec2 x0;
	vec2 x1;
	vec2 w;

	n= int(gl_FragCoord[dir]);
	int iw = (n >> q) << q;
	int ix0 = insertZeroBits(n, q, 1);
	int ix1 = ix0 + (1 << q);
	w = texture(texW,vec2(iw,0)).rg;


	if(dir ==0){
		if(ix0 < N/2) x0 = texture(texSrc0,vec2(ix0,gl_FragCoord.y)).rg;
		else x0 = texture(texSrc1,vec2(ix0-N/2,gl_FragCoord.y)).rg;

		if(ix1 < N/2) x1 = texture(texSrc0,vec2(ix1,gl_FragCoord.y)).rg;
		else x1 = texture(texSrc1,vec2(ix1-N/2,gl_FragCoord.y)).rg;
	}
	else{
		if(ix0 < N/2) x0 = texture(texSrc0,vec2(gl_FragCoord.x,ix0)).rg;
		else x0 = texture(texSrc1,vec2(gl_FragCoord.x,ix0-N/2)).rg;

		if(ix1 < N/2) x1 = texture(texSrc0,vec2(gl_FragCoord.x,ix1)).rg;
		else x1 = texture(texSrc1,vec2(gl_FragCoord.x,ix1-N/2)).rg;
	}

	x0 = x0*xscl;
	x1 = x1*xscl;
	x0.g = x0.g*xconj;
	x1.g = x1.g*xconj;

	vec2 tmp;
	tmp.r = x1.r * w.r - x1.g * w.g;
	tmp.g = x1.r * w.g + x1.g * w.r;

	vec2 y0;
	vec2 y1;

	y0 = x0 + tmp;
	y1 = x0 - tmp;

	y0 = y0*yscl;
	y1 = y1*yscl;
	y0.g = y0.g*yconj;
	y1.g = y1.g*yconj;

	dst0 = y0;
	dst1 = y1;

}
