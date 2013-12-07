// Cyrille Henry 2008
#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect texture;
uniform float K2, K;

vec2 pos_(vec2 V, float x, float y)
{
	V += vec2(x,y);
	return(max(min(V,vec2(91.,63.)),vec2(0.)));
}


void main (void)
{
	vec2 pos = gl_TexCoord[0].st * vec2(90.,62.)/256. ; 

	vec4 color1 = texture2DRect(texture,pos_(pos,-1.,-1.));
	vec4 color2 = texture2DRect(texture,pos_(pos, 0.,-1.));
	vec4 color3 = texture2DRect(texture,pos_(pos, 1.,-1.));
	vec4 color4 = texture2DRect(texture,pos_(pos,-1., 0.));
	vec4 color5 = texture2DRect(texture,pos_(pos, 0., 0.));
	vec4 color6 = texture2DRect(texture,pos_(pos, 1., 0.));
	vec4 color7 = texture2DRect(texture,pos_(pos,-1., 1.));
	vec4 color8 = texture2DRect(texture,pos_(pos, 0., 1.));
	vec4 color9 = texture2DRect(texture,pos_(pos, 1., 1.));

	vec4 colorBG = color1 + color2 + color4 + color5;
	vec4 colorBD = color3 + color2 + color6 + color5; 
	vec4 colorHG = color4 + color5 + color7 + color8; 
	vec4 colorHD = color5 + color6 + color8 + color9;

	vec2 fract_pos = (fract(pos)); 

	vec4 XB = mix(colorBG,colorBD,fract_pos.x);
	vec4 XH = mix(colorHG,colorHD,fract_pos.x);
 	vec4 X = mix(XB,XH,fract_pos.y) / 4.;

	X -= vec4(0.5);
	X.xyz = normalize(X.xyz);

	if(gl_FrontFacing)
	{X *= -1.;}

	float tmp =  dot(X.xyz,normalize(vec3(0.,1.,1.)));
	tmp = -tmp;
	tmp = max(0.,tmp) + 0.3 * min(0.,tmp);

	tmp = 0.1 + 0.9 * tmp;
	vec4 color = tmp * vec4(1,0.95,0.9,1);
	color.a = smoothstep(0.75,1.,pos.x);
    gl_FragColor = color;

}


