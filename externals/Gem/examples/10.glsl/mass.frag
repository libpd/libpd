// Cyrille Henry 2008
#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect texture_mass_old,texture_link;
varying vec2 coord;
uniform float init;

void main (void)
{
	vec4 color = vec4(0.,0.,1.,1.);
	if ( (coord.x < 92.) && (coord.y <  64.) )
	{
		vec4 pos = texture2DRect(texture_mass_old, coord)-vec4(0.5);
		vec4 force = texture2DRect(texture_link, coord)-vec4(0.5);

		force = min(max(force,vec4(-0.5)),vec4(0.5));
			// on vire les +inf et -inf qui peuvent poser pb en cas d'instabilités
		pos += force; 
			//increment de la position
	
		float reset = step(coord.x,1.);
//		reset *= step(mod(coord.y,10.),4.);
			// les point en x<1 sont tjrs reseté : ils sont dc imobiles

		color = mix(pos,vec4(coord.x/1000.,(coord.y-32.)/1000.,0.,0.),reset+(1.-reset)*init);
			// couleur de sortie mixé avec couleur d'initialisation

		color += vec4(0.5);
		color.a = 1.;
			// preparation de la sortie (ajout de l'offset + virer l'alpha)
	}

	gl_FragColor = color;
}
