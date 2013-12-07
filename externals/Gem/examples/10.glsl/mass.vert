varying vec2 coord;


void main()
{
	
    coord = (gl_TextureMatrix[0] * gl_MultiTexCoord0).st;
    gl_Position = ftransform();

}
