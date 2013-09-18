// Cyrille Henry 2007

#extension GL_ARB_texture_rectangle : enable

uniform float K; 
uniform sampler2DRect texture, texture1, texture2;

const float dx = 1.; 
const float dy = 1.; 
const float dp = 1.; 

void main (void)
{
	float light;
	vec2 position = gl_TexCoord[0].st;
	vec4 C, C1;
	C  = texture2DRect(texture2, position)  ;

	vec4 color2 = texture2DRect(texture1, (position+K*C.rg));

	gl_FragColor = color2;

}
