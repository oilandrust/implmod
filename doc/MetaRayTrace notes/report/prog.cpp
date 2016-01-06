uniform sampler2D tex;

varying vec3 norm, position;

void main()
{
    vec4 color = gl_LightSource[0].ambient*vec4(0.2,0.2,0.2,0);
    

    vec3 normal = normalize(norm);
    // If the normal points away from the viewer, flip it.
	if(dot(normal,-position)<0.0) 
		normal = -normal;
    
	// Compute dot product of normal and light source direction
	float d = dot(normal,normalize(gl_LightSource[0].position.xyz));
	vec3 h = normalize(gl_LightSource[0].halfVector.xyz);
		
	if(d>0.0)
	{
		color += d*gl_LightSource[0].diffuse*gl_FrontMaterial.diffuse;
		float s = dot(normal, h);
		if(s>0.0)
		{
			color += pow(s, gl_FrontMaterial.shininess)*
				gl_LightSource[0].specular*gl_FrontMaterial.specular;
		}
	}
	
	// Simply multiply color by texture.
	gl_FragColor = color* texture2D(tex, gl_TexCoord[0].xy);
}
