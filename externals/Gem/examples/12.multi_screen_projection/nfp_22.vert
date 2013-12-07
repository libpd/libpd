uniform vec2 Vtl,Vbl,Vtr,Vbr; // Vertex position
uniform vec2 Ttl,Tbl,Ttr,Tbr; // texture coordinate
uniform vec2 center;

varying vec2 pos;

void main()
{
    gl_TexCoord[0] = gl_MultiTexCoord0;

    vec4 position = gl_Vertex;
    position.xy += 1.;
    position.xy /= 2.;
    pos = position.xy;
    position.xy = pow(position.xy,center);

    vec2 tex_top = mix(Ttl,Ttr,pos.x);
    vec2 tex_bottom = mix(Tbl,Tbr,pos.x);
    gl_TexCoord[0].st = mix(tex_top,tex_bottom, pos.y);

    vec2 pos_top = mix(Vtl,Vtr,position.x);
    vec2 pos_bottom = mix(Vbl,Vbr,position.x);
    position.xy = mix(pos_top,pos_bottom, position.y);

    gl_Position = gl_ModelViewProjectionMatrix * position;
}
