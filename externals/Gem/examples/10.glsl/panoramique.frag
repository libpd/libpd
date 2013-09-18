#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect MyTex;
uniform float dZ, ShadeL, ShadeR, sizeX, sizeY;

void main (void)
{

 vec2 pos_out;
 pos_out.x=-1.+(2.*gl_TexCoord[0].s)/sizeX;
 pos_out.y=-1.+(2.*gl_TexCoord[0].t)/sizeY;
 pos_out.y *= sqrt(pos_out.x*pos_out.x+dZ*dZ)/dZ;

 pos_out.x = sizeX*(pos_out.x+1.)/2.;
 pos_out.y = sizeY*(pos_out.y+1.)/2.;

 vec4 color = texture2DRect(MyTex, pos_out);

 color.a = 1.;
 float tmp = mix(0.,1.,gl_TexCoord[0].s/ShadeL);
 tmp = min(tmp,1.);
 color.a *= tmp;
 tmp = mix(0.,1.,(sizeX-gl_TexCoord[0].s)/ShadeR);
 tmp = min(tmp,1.);
 color.a *= tmp;

 gl_FragColor = color;
}
