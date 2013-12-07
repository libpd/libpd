// Cyrille Henry 2008
#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect MyTex;
uniform float K;

void main()
{
	vec4 v = vec4(gl_Vertex);
	vec4 color = texture2DRect(MyTex, gl_MultiTexCoord0.st * vec2(91./128.,63./128.));

	gl_TexCoord[0] = gl_MultiTexCoord0;

	v.x = color.r -0.5;	
	v.y = color.g -0.5;	
	v.z = color.b -0.5;	

	gl_Position = gl_ModelViewProjectionMatrix * v;
	
}

