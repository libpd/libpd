// Cyrille Henry 2010

#version 120 
#extension GL_EXT_geometry_shader4 : enable

void main(void)
{
	
	int i; //increment variable
	vec4 pos1,pos2, pos3, pos4, pos5; // tmp
	vec3 high;

	for(i=0; i< gl_VerticesIn; i += 3){ // for all triangles
		pos1 = gl_PositionIn[i];
		pos2 = gl_PositionIn[i+1];
		pos3 = gl_PositionIn[i+2];
		pos4 = (pos1 + pos2 + pos3) / 3.; // center of the triangle

		high = 10.*cross(pos2.xyz-pos1.xyz,pos3.xyz-pos2.xyz); 
			// hight and orientation of the piramide

		high = normalize(high);
		high *= length(pos2-pos1);
		pos5 = pos4 ;
        pos5.xyz += high;

		gl_Position = gl_ModelViewProjectionMatrix * pos1;
		EmitVertex();
		gl_Position = gl_ModelViewProjectionMatrix * pos4;
		EmitVertex();
		gl_Position = gl_ModelViewProjectionMatrix * pos5;
		EmitVertex();
		gl_Position = gl_ModelViewProjectionMatrix * pos2;
		EmitVertex();
		gl_Position = gl_ModelViewProjectionMatrix * pos4;
		EmitVertex();
		gl_Position = gl_ModelViewProjectionMatrix * pos5;
		EmitVertex();
		gl_Position = gl_ModelViewProjectionMatrix * pos3;
		EmitVertex();
		gl_Position = gl_ModelViewProjectionMatrix * pos4;
		EmitVertex();
		gl_Position = gl_ModelViewProjectionMatrix * pos5;
		EmitVertex();

		EndPrimitive();	
			// new primitive
	}

}

