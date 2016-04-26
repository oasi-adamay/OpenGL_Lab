#version 330 core
precision highp float;
uniform sampler2DRect	texSrc;
layout (location = 0) out vec2 dst;

void main(void)
{
	vec2 src = texture(texSrc,gl_FragCoord.xy).rg;
	dst = vec2(src.r,-1.0*src.g);
}
