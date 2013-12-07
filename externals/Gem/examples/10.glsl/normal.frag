// Cyrille Henry 2008
#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect texture_mass;
varying vec2 coord;

vec2 test1(vec2 V)
{
	return(max(min(V,vec2(91.,63.)),vec2(0.)));
}

void main (void)
{
	vec4 color = vec4(1.,0.,0.,1.);

	if ( (coord.x < 91.) && (coord.y <  63.) )
	{
		vec4 posG = texture2DRect(texture_mass, test1(coord+vec2(-1., 0.))) ;
		vec4 posD = texture2DRect(texture_mass, test1(coord+vec2( 1., 0.))) ;
		vec4 posH = texture2DRect(texture_mass, test1(coord+vec2( 0.,-1.))) ;
		vec4 posB = texture2DRect(texture_mass, test1(coord+vec2( 0., 1.))) ;
	
		vec3 normal = cross((posG.xyz-posD.xyz),(posH.xyz-posB.xyz));
		color.xyz = normalize(normal);
		color.xyz = normal;

        vec3 DX = posG.xyz-posD.xyz;
        vec3 DY = posH.xyz-posB.xyz;

		color += vec4(1.);
		color *= 0.5;

	}
	color.a = 1.;
	gl_FragColor = color;

}
