uniform vec3 cameraPos;
uniform float time;
varying vec3 worldPos;



const vec3 lightPos = vec3(2,1,0);

//RAY TRACING CONSTANTS
const int maxSteps = 200;
const float ds = 0.01;

//DIFFERENTIATION steplenght
const float Du = 0.0001; //potentiel
const float Dun = 1; //noise

const float PI = 3.14159265358979323846264;

//0.01 for two spheres
const float threshold = 0.002;

///////////////////// PERLIN NOISE  /*///////////////////////////////////////////
uniform sampler2D permTexture;

#define ONE 0.00390625
#define ONEHALF 0.001953125

float fade(float t) {
  // return t*t*(3.0-2.0*t); // Old fade, yields discontinuous second derivative
  return t*t*t*(t*(t*6.0-15.0)+10.0); // Improved fade, yields C2-continuous noise
}
float noise(vec3 P)
{
	P = 10*P;
  vec3 Pi = ONE*floor(P)+ONEHALF; // Integer part, scaled so +1 moves one texel
                                  // and offset 1/2 texel to sample texel centers
  vec3 Pf = fract(P);     // Fractional part for interpolation

  // Noise contributions from (x=0, y=0), z=0 and z=1
  float perm00 = texture2D(permTexture, Pi.xy).a ;
  vec3  grad000 = texture2D(permTexture, vec2(perm00, Pi.z)).rgb * 4.0 - 1.0;
  float n000 = dot(grad000, Pf);
  vec3  grad001 = texture2D(permTexture, vec2(perm00, Pi.z + ONE)).rgb * 4.0 - 1.0;
  float n001 = dot(grad001, Pf - vec3(0.0, 0.0, 1.0));

  // Noise contributions from (x=0, y=1), z=0 and z=1
  float perm01 = texture2D(permTexture, Pi.xy + vec2(0.0, ONE)).a ;
  vec3  grad010 = texture2D(permTexture, vec2(perm01, Pi.z)).rgb * 4.0 - 1.0;
  float n010 = dot(grad010, Pf - vec3(0.0, 1.0, 0.0));
  vec3  grad011 = texture2D(permTexture, vec2(perm01, Pi.z + ONE)).rgb * 4.0 - 1.0;
  float n011 = dot(grad011, Pf - vec3(0.0, 1.0, 1.0));

  // Noise contributions from (x=1, y=0), z=0 and z=1
  float perm10 = texture2D(permTexture, Pi.xy + vec2(ONE, 0.0)).a ;
  vec3  grad100 = texture2D(permTexture, vec2(perm10, Pi.z)).rgb * 4.0 - 1.0;
  float n100 = dot(grad100, Pf - vec3(1.0, 0.0, 0.0));
  vec3  grad101 = texture2D(permTexture, vec2(perm10, Pi.z + ONE)).rgb * 4.0 - 1.0;
  float n101 = dot(grad101, Pf - vec3(1.0, 0.0, 1.0));

  // Noise contributions from (x=1, y=1), z=0 and z=1
  float perm11 = texture2D(permTexture, Pi.xy + vec2(ONE, ONE)).a ;
  vec3  grad110 = texture2D(permTexture, vec2(perm11, Pi.z)).rgb * 4.0 - 1.0;
  float n110 = dot(grad110, Pf - vec3(1.0, 1.0, 0.0));
  vec3  grad111 = texture2D(permTexture, vec2(perm11, Pi.z + ONE)).rgb * 4.0 - 1.0;
  float n111 = dot(grad111, Pf - vec3(1.0, 1.0, 1.0));

  // Blend contributions along x
  vec4 n_x = mix(vec4(n000, n001, n010, n011),
                 vec4(n100, n101, n110, n111), fade(Pf.x));

  // Blend contributions along y
  vec2 n_xy = mix(n_x.xy, n_x.zw, fade(Pf.y));

  // Blend contributions along z
  float n_xyz = mix(n_xy.x, n_xy.y, fade(Pf.z));

  // We're done, return the final noise value.
  return n_xyz;
}
/********************************************************************************************************/

float sqr(float x){
	return x*x;
}

float implicitSphere(vec3 pos, vec3 center, float sqradius){
	float r2 = dot(pos-center,pos-center);
	if(r2 <= sqradius)
		return sqr(sqradius-r2);
	else
		return 0.0;
}
float implicitTube(vec3 pos, vec3 a, vec3 b, float sqradius){
	float segLengh = length(b - a);
	vec3 u = (b - a)/segLengh;
	vec3 ap = pos - a;
	float Lambdap = dot( ap , u );

	float r2 = 0.0;

	if(Lambdap >= 0.0){
		if(Lambdap <= segLengh){ //Projection of P on the line is in the segment, compute the distance
			vec3 projected = ap - Lambdap*u;
			r2 = dot(projected,projected);
		}
		else{ //P closer to b, compute the distance
			vec3 sphere2Pos = pos-b;
			r2 = dot(sphere2Pos,sphere2Pos);
		}
	}else //P closer to a
		r2 = dot(ap,ap);

	if(r2 <= sqradius)
		return sqr(sqradius-r2);
	else
		return 0.0;
}
float implicitFunction(vec3 pos){
	float primitiveValue =  0;
	
	for(int i = 0; i < 8; i++){
		float l1 = 0.6;
		float l2 = 0.9;
		primitiveValue +=	implicitTube(pos,l1*vec3(cos(i*PI/4),l1*sin(i*PI/4),0),vec3(0,0.0,0.3),0.05);
		primitiveValue +=	implicitTube(pos,l2*vec3(cos(i*PI/4),l2*sin(i*PI/4),-0.8),l1*vec3(cos(i*PI/4),l1*sin(i*PI/4),0),0.05);
	}
	
	primitiveValue +=	implicitSphere(pos,vec3(0,0,0.5),0.1);
		
	return primitiveValue + 0.0001*noise(pos) - threshold;
}
vec3 implicitSphereNormal(vec3 pos){
	float dfdx = ( implicitFunction(pos + vec3(Du,0.0,0.0)) -
				   implicitFunction(pos + vec3(-Du,0.0,0.0)) )
				  /(2.0*Du);
				  
	float dfdy = ( implicitFunction(pos + vec3(0.0,Du,0.0)) -
				   implicitFunction(pos + vec3(0.0,-Du,0.0)) )
				  /(2.0*Du);

	float dfdz = ( implicitFunction(pos + vec3(0.0,0.0,Du)) -
				   implicitFunction(pos + vec3(0.0,0.0,-Du)) )
				  /(2.0*Du);

	return normalize(vec3(dfdx,dfdy,dfdz));
}

vec3 noiseNormal(vec3 pos){
	float dfdx = ( noise(10*(pos + vec3(Dun,0.0,0.0))) -
				   noise(10*(pos + vec3(-Dun,0.0,0.0))) )
				  /(2.0*Dun);
				  
	float dfdy = ( noise(10*(pos + vec3(0.0,Dun,0.0))) -
				   noise(pos + vec3(0.0,-Dun,0.0)) )
				  /(2.0*Dun);

	float dfdz = ( noise(10*(pos + vec3(0.0,0.0,Dun))) -
				   noise(10*(pos + vec3(0.0,0.0,-Dun))) )
				  /(2.0*Dun);

	return vec3(dfdx,dfdy,dfdz);
}

vec4 phongShade(vec3 pos, vec3 normal){
	vec4 color = vec4(.7,.7,.7,.7);
	vec4 constcolor;
	float noise1 = (noise(pos)+0.5);
	float noise2 = (noise(2*pos)+0.5);
	float noise3 = (noise(3*pos)+0.5);
	//Ambient
	//if(normal.z > 0.4)
	//	constcolor = color = vec4(noise1,0.5*noise2,noise3,0);
	//else
	//	constcolor = color = vec4(0.4*noise2,noise3,0,0);
	constcolor = color = mix(vec4(noise1,0.5*noise2,0.2*noise3,0),vec4(0.4*noise2,noise3,0,0),(normal.z+1)/2);
	

	// Compute dot product of normal and light source direction
	vec3 lightDir = normalize(pos-cameraPos);
	float d = dot(normal,lightDir);
	if(d>0.0)
	{
		//Diffuse
		color += d*gl_LightSource[0].diffuse*constcolor;
		vec3 h = -normalize(-pos-lightDir);
		float s = dot(normal, h);
		//Specular
		if(s>0.0)
			color += pow(s, 50.0)*
				gl_LightSource[0].specular*vec4(0.7,0.7,0.7,1);
	}
	
	return color;
}

void main()
{
	//Construct ray from eye to fragment
	vec3 rayDir = normalize( worldPos - cameraPos );
	vec3 ray = worldPos;//cameraPos;

	//Find first intersection
	int steps = 0;	
	bool done = false; 
	bool positive = implicitFunction(ray) >= 0.0;
	while( !done && steps < maxSteps ){
		ray += ds*rayDir;
		float value = implicitFunction(ray);
		if(positive && value <= 0.0)
			done = true;
		else if(!positive && value >= 0.0)
			done = true;
		steps++;
	}
	
	//Shade
	vec4 color = vec4(0);
	if( !done )
		color = vec4(1);
	else{
		//find the normal
		vec3 norm = normalize(implicitSphereNormal(ray));
		color = phongShade(ray,norm);
		
		//Reflection
		/*
		vec3 reflRayDir = reflect(rayDir,norm);
		vec3 reflRay = ray;//cameraPos;
		//Find first intersection
		int steps2 = 0;	
		bool done2 = false; 
		reflRay += 5*ds*reflRayDir;
		bool positive2 = implicitFunction(reflRay) >= 0.0;
		while( !done2 && steps2 < maxSteps ){
			reflRay += ds*reflRayDir;
			float value = implicitFunction(reflRay);
			if(positive2 && value <= 0.0)
				done2 = true;
			else if(!positive2 && value >= 0.0)
				done2 = true;
			steps2++;
		}
		if(done2){
			vec3 normal = normalize(implicitSphereNormal(reflRay));
			color = mix(color,phongShade(reflRay,normal),.8);
		}
		*/
	
	}
	
	gl_FragColor = color;////vec4(worldPos,1);//
}
