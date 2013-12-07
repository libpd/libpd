//jack/RYBN 2010
varying vec2 texcoord0;

void main()
{
	texcoord0 = (gl_TextureMatrix[0]*gl_MultiTexCoord0).st;
	gl_Position = ftransform();

}
