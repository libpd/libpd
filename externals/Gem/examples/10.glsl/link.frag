// Cyrille Henry 2008
#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect texture_mass, texture_mass_old, texture_normal;
uniform float init, gravite;
uniform vec2 wind;
uniform float D; 
uniform float K1; // rigiditée liaison
uniform float W,f,N; // amplitude du Wind et frequence; Noise
uniform float t; //temps

varying vec2 coord;

vec2 test1(vec2 V)
{
//	return(max(min(V,vec2(64.)),vec2(0.)));
	return(V);
}

void main (void)
{
	vec4 force = vec4(0,1.,0.,1.);

	if ( (coord.x < 92.) && (coord.y <  64.) )
	{
		vec3 dist;
		float taille;

		vec4 pos     = texture2DRect(texture_mass,    coord             );// -vec4(0.5);
		vec4 pos_old = texture2DRect(texture_mass_old,coord             );// -vec4(0.5);
		vec4 posG    = texture2DRect(texture_mass, (coord+vec2(-1., 0.)));// -vec4(0.5);
		vec4 posD    = texture2DRect(texture_mass, (coord+vec2( 1., 0.)));// -vec4(0.5); 
		vec4 posH    = texture2DRect(texture_mass, (coord+vec2( 0., 1.)));// -vec4(0.5);
		vec4 posB    = texture2DRect(texture_mass, (coord+vec2( 0.,-1.)));// -vec4(0.5);
		vec4 posHD   = texture2DRect(texture_mass, (coord+vec2( 1.,-1.)));// -vec4(0.5);
		vec4 posBG   = texture2DRect(texture_mass, (coord+vec2(-1., 1.)));// -vec4(0.5);
		vec4 posHG   = texture2DRect(texture_mass, (coord+vec2( 1., 1.)));// -vec4(0.5);
		vec4 posBD   = texture2DRect(texture_mass, (coord+vec2(-1.,-1.)));// -vec4(0.5);
		vec4 pos2G   = texture2DRect(texture_mass, (coord+vec2(-2., 0.)));// -vec4(0.5);
		vec4 pos2D   = texture2DRect(texture_mass, (coord+vec2( 2., 0.)));// -vec4(0.5);
		vec4 pos2H   = texture2DRect(texture_mass, (coord+vec2( 0., 2.)));// -vec4(0.5);
		vec4 pos2B   = texture2DRect(texture_mass, (coord+vec2( 0.,-2.)));// -vec4(0.5);
			// lecture des position des masses voisinnes

		force = pos-pos_old; 
			// ajout de la force d'inertie (conservation de la vitesse)

		force *= 1.-D; 
			// damping relatif a un point fix
			// ATTENTION, c'est le seul damping du system!


	// 4 liens direct (gauche / droite et haut / bas)
		// on ajoute une force ssi la taille est > 0
		// on limite aussi ds l'espace pour ne prendre en compte que le lien valide (effet de bord)

		dist = pos.xyz - posG.xyz ; 
		taille = length(dist) ;
		if ( (taille > 0.) && (coord.x >  1.) )
			{ force.xyz += -K1 * (taille - 1./1000.)* normalize(dist); }

		dist = pos.xyz - posD.xyz ; 
		taille = length(dist) ;
		if ( (taille > 0.) && (coord.x < 91.) )
			{ force.xyz += -K1 * (taille - 1./1000.)* normalize(dist); }

		dist = pos.xyz - posH.xyz ; 
		taille = length(dist) ;
		if ( (taille > 0.) && (coord.y < 63.) )
			{ force.xyz += -K1 * (taille - 1./1000.)* normalize(dist); }

		dist = pos.xyz - posB.xyz ; 
		taille = length(dist) ;
		if ( (taille > 0.) && (coord.y >  1.) )
			{ force.xyz += -K1 * (taille - 1./1000.)* normalize(dist); }

	// 4 liens diagonaux (haut gauche, bas droite, etc)
		dist = pos.xyz - posHD.xyz ; 
		taille = length(dist) ;
		if ( (taille > 0.) && (coord.x < 91.) && (coord.y >  1.) )
			{ force.xyz += -K1 * (taille - 1.4142/1000.)* normalize(dist); }
	 
		dist = pos.xyz - posBG.xyz ; 
		taille = length(dist) ;
		if ( (taille > 0.) && (coord.x >  1.) && (coord.y < 63.) )
			{ force.xyz += -K1 * (taille - 1.4142/1000.)* normalize(dist); }

		dist = pos.xyz - posHG.xyz ; 
		taille = length(dist) ;
		if ( (taille > 0.) && (coord.x < 91.) && (coord.y < 63.) )
			{ force.xyz += -K1 * (taille - 1.4142/1000.)* normalize(dist); }

		dist = pos.xyz - posBD.xyz ; 
		taille = length(dist) ;
		if ( (taille > 0.) && (coord.x >  1.) && (coord.y >  1.) )
			{ force.xyz += -K1 * (taille - 1.4142/1000.)* normalize(dist); }

	// 4 liens double longeur (rigidité de flexion)
		dist = pos.xyz - pos2G.xyz ; 
		taille = length(dist) ;
		if ( (taille > 0.) && (coord.x >  2.) )
			{ force.xyz += -K1 * (taille - 2./1000.)* normalize(dist); }

		dist = pos.xyz - pos2D.xyz ; 
		taille = length(dist) ;
		if ( (taille > 0.) && (coord.x < 90.) )
			{ force.xyz += -K1 * (taille - 2./1000.)* normalize(dist); }

		dist = pos.xyz - pos2H.xyz ; 
		taille = length(dist) ;
		if ( (taille > 0.) && (coord.y < 62.) )
			{ force.xyz += -K1 * (taille - 2./1000.)* normalize(dist); }

		dist = pos.xyz - pos2B.xyz ; 
		taille = length(dist) ;
		if ( (taille > 0.) && (coord.y >  2.) )
			{ force.xyz += -K1 * (taille - 2./1000.)* normalize(dist); }

	// autres forces
		vec4 normal = texture2DRect(texture_normal, coord*64.) -vec4(0.5);
		normal = normalize(normal);
	//	float force_wind = abs(dot(normalize(vec3(wind.xy,0.)),normal.xyz));

		force.r += wind.x/100.;
		force.g += gravite/100.;
		force.b += wind.y/100.;

	// ajout d'une force sinusoidal perpendiculaire a la direction du vent
		float W_sin  = cos(-t + f*0.01*gl_TexCoord[0].s);
		W_sin *= 91.- gl_TexCoord[0].s;
		W_sin /= 91.;
		W_sin *= 91.- gl_TexCoord[0].s;
		W_sin /= 91.;

		W_sin *= gl_TexCoord[0].t + 50.;
		W_sin /= 63.;
		W_sin *= gl_TexCoord[0].t + 50.;
		W_sin /= 63.;

		force.x += W*(-wind.y*W_sin);
		force.z += W*( wind.x*W_sin);

	// ajout d'un pseuo buit
		float W_noiseX = cos(t + 0.353 * coord.t + 0.0234434* coord.s + 345.2342);
		W_noiseX = cos(123456.35345 * W_noiseX + 234.23);
		float W_noiseY = cos(-2.2 * t  + 6235.457456 * W_noiseX + 567.456);
		float W_noiseZ = cos(5.*t + 8976457.457 * W_noiseX + 3464.54);
		W_noiseX = cos(323344.64345 * W_noiseX + 567.45);

		force.x += 0.00001 * N * W_noiseX;
		force.y += 0.00001 * N * W_noiseY;
		force.z += 0.00001 * N * W_noiseZ;


	// preparation de la sortie
		force = min(max(force,vec4(-0.5)),vec4(0.5));
			// min et max pour virer les +inf
		force += vec4(0.5,0.5,0.5,1.);
			// ajout de l'ofset pour etre entre 0 et 1
		force.a = 1.;
			// on vire la 4eme composante au cas ou.
	}
	gl_FragColor = mix(force,vec4(0.5,0.5,0.5,1.),init);
}
