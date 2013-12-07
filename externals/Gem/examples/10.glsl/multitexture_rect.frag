#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect MyTex;
uniform sampler2DRect MyTex1;

varying vec2 texcoord1;
varying vec2 texcoord2;

void main (void)
{
 vec4 color = texture2DRect(MyTex,  texcoord1);
 vec4 color2 = texture2DRect(MyTex1, texcoord1); // texcoord2 does not work.
 gl_FragColor = (color + color2) / 2.;
}

