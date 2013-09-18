varying vec2 texcoord1;
varying vec2 texcoord2;

void main()
{
	
    texcoord1 = (gl_TextureMatrix[0] * gl_MultiTexCoord0).st;
    texcoord2 = (gl_TextureMatrix[1] * gl_MultiTexCoord1).st;
    gl_Position = ftransform();

}
