#version 120 
#extension GL_EXT_geometry_shader4: enable 

void main(void)
{
	//increment variable
	int i,j;
	float rnd1,rnd2;
	rnd1 = 0.0216767863321334264; // seed
	rnd2 = 0.0475785688678678345;
	for(j=0; j< 40; j++)
	{
		rnd1 = fract(124.321345312123465*rnd1) -0.5; // easy way to generated pseudo random number
		rnd2 = fract(5234.43532345435245*rnd2) -0.5;

		//draw 40 time the same geometry, but with small shift in it's position
		for(i=0; i< gl_VerticesIn; i++)
		{
			gl_FrontColor = gl_FrontColorIn[i];
			gl_TexCoord[0] = gl_TexCoordIn[i][0];
			gl_Position = gl_PositionIn[i]; // get position of the original point
			gl_Position.xy += vec2(rnd1,rnd2)/3.; // add small random
			// the geometry as already been transform in 2d, so we jut have to move it in X and Y
			EmitVertex();
		}
		EndPrimitive();	
	}
}
