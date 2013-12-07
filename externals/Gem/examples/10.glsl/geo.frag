void main()
{	
	vec4 tmp = gl_Color;
	tmp.a /= 10.;
	gl_FragColor = tmp;	
	// set color but alpha is 20 time less
}

