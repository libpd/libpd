// Cyrille Henry 2007

#extension GL_ARB_texture_rectangle : enable

uniform float K1,K3,D1,K;
uniform sampler2DRect text;

const float dx = 1.; 
const float dy = 1.;

void main (void)
{
	float x,y;
	vec4 C;
	vec2 texture1 = gl_TexCoord[0].st ;

	x=texture1.x;
	y=texture1.y;

	C = texture2DRect(text, vec2(x, y)); 

	vec4 color_cote  = texture2DRect(text, vec2(x-dx, y-dy));
	color_cote += texture2DRect(text, vec2(x+dx, y-dy));
	color_cote += texture2DRect(text, vec2(x-dx, y+dy));
	color_cote += texture2DRect(text, vec2(x+dx, y+dy));
	color_cote += texture2DRect(text, vec2(x, y-dy));
	color_cote += texture2DRect(text, vec2(x-dx, y));
	color_cote += texture2DRect(text, vec2(x+dx, y));
	color_cote += texture2DRect(text, vec2(x, y+dy));


	float K = K1 * color_cote.r + K3 * 0.5 - (8. * K1 + K3) * C.r;
	float D = D1 * (C.r - C.g);

	C.b = C.g;
	C.g = C.r;
	C.r = (2.* C.g) - C.b + K - D ;

	gl_FragColor = vec4(C.rgb,1.);
}
