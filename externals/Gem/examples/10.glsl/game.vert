// Cyrille Henry 2007

uniform float K1;
uniform float K2;
uniform float K3;

uniform vec2 offset;

void main()
{	
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = ftransform();		
}

