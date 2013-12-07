uniform sampler2D MyTex;
varying vec4 C;
void main()
{
	vec4 v = vec4(gl_Vertex);
	vec4 color = texture2D(MyTex, (gl_TextureMatrix[0] * gl_MultiTexCoord0).st);
	v.z = color.r;	
//	v.x += (color.b-0.5)/2.;
//	v.y += (color.g-0.5)/2.;

	C=color;
	gl_Position = gl_ModelViewProjectionMatrix * v;
	
}

