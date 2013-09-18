// Cyrille Henry 2010

//#extension GL_ARB_texture_rectangle : enable
//uniform sampler2DRect MyTex;
uniform sampler2D MyTex;
uniform vec2 overlap, geometry_screen, geometry_computer;

void main (void)
{
// FSAA

// change coord from computer matrix to screen matrice
	vec2 coord = (gl_TextureMatrix[0] * gl_TexCoord[0]).st;
   	vec2 pos_new = coord;
    pos_new *= geometry_computer;
    float screen_num = floor(pos_new.x)+floor(pos_new.y)*geometry_computer.x; // number of the screen
    pos_new = fract(pos_new); // coord in 1 screen (from 0 to 1)
    pos_new.x += fract(screen_num/geometry_screen.x)*geometry_screen.x;
    pos_new.y += floor(screen_num/geometry_screen.x);
    pos_new /= geometry_screen;

// compute position regarding to the overlap
    vec2 pos = pos_new;
    pos *= geometry_screen;

    vec2 pos_over = fract(pos);
    pos_over *= overlap;
    pos_over -= overlap/2.;
    pos += pos_over;
    pos += overlap/2.;
    pos /= geometry_screen + overlap;
    vec4 color = texture2D(MyTex, pos);

// compute fade on Top and Right
    vec2 black = pos_new;
    black *= geometry_screen;
    black = fract(black);
    black *= vec2(1.)+overlap;
    black -= 1.;
    black = max(black,0.)/max(overlap,1.);
    if ( floor(pos_new.x*geometry_screen.x) < geometry_screen.x-1. )
        color *= (1.-black.x);
    if ( floor(pos_new.y*geometry_screen.y) < geometry_screen.y-1. )
        color *= (1.-black.y);

// compute fade on Left and bottom
    black = pos_new;
    black *= geometry_screen;
    black = fract(black);
    black *= vec2(1.)+overlap;
    black -= overlap;
    black = max(-black,0.)/max(overlap,1.);
    if ( floor(pos_new.x*geometry_screen.x) > 0. )
        color *= (1.-black.x);
    if ( floor(pos_new.y*geometry_screen.y) > 0. )
        color *= (1.-black.y);

    gl_FragColor = color;
}
