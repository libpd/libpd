// Cyrille Henry 2007

uniform sampler2D tex0;

void main()
{
    vec2 C =  (gl_TextureMatrix[0] * gl_TexCoord[0]).st;
    gl_FragColor = texture2D(tex0, C ) ;
}










