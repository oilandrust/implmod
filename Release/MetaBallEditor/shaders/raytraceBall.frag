
//TO CONSTRUCT THE RAYS
uniform vec3 cameraPos;
varying vec3 worldPos;

//IMPLICIT FUNTIONS
uniform vec3 p1s[200];
uniform float radius[200];
uniform int nbOfMetaTubes;
uniform int nbOfBalls;
const float threshold = 0.5;

//LIGHT
const vec3 lightPos = vec3(2,1,0);

//TEXTURE PROJECTORS
uniform vec3 proj1s[5];
uniform vec3 proj2s[5];
uniform vec3 orthos[5];
uniform float rads[5];
uniform int nbProjectors;
uniform sampler2D checkTexture;

//RAY TRACING CONSTANTS
const int maxSteps = 10;
const float ds = 0.01;

//***************GLOBAL VARIABLES FOR RAYTRACE OPTI
//A point inside the surface for bisection
vec3 posPoint;
//Record of points along the ray that are positive
vec3 posPoints[10];
int nbHits = 0;
//The metaballs that are contributing along the ray
int ballsAlongRay[50];
int bInRay;



//DIFFERENTIATION steplenght
const float Du = 0.0001; //potentiel
const float DuInv = 0.5/Du;
const float Dun = 1.0; //noise

//CUBE STYLE
const float gridSize = 0.03;
const float PI = 3.14159265358979323846264;


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
	P = 10.0*P;
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

float sqr(in float x){
	return x*x;
}

float cube(in float x){
	return x*x*x;
}

vec4 gekkoNoise(in vec3 pos, in vec3 normal){
	float noise1 = (noise(0.1*pos)+0.5);
	float noise2 = (noise(0.5*pos)+0.5);
	float noise3 = (noise(0.1*pos)+0.5);

	return mix(	vec4(noise1,0.5*noise2,0.1*noise3,1.0),
					vec4(0.1*noise2+0.2*noise3,0.25*noise3+0.5*noise2,0.0,1.0),
					(-normal.z+1.0)/1.5);
}
float gekkoDispNoize(in vec3 pos){
	return 0.00*noise(2.0*pos)+ 0.05*noise(pos);
}

float implicitSphere(in vec3 pos, in vec3 center, in float sqradius){
	float r2 = dot(pos-center,pos-center);
	if(r2 <= sqradius)
		return sqr(1.0-r2/sqradius);
	else
		return 0.0;
}

float implicitFunction(in vec3 pos){
	
	float primitiveValue =  0.0;

	for(int i = 0; i < nbOfBalls; i++){
		primitiveValue += implicitSphere(pos,p1s[i+nbOfMetaTubes],radius[i+nbOfMetaTubes]);
	}
	
	float noiseDispValue = 0;//gekkoDispNoize(pos);
	return primitiveValue - threshold + noiseDispValue;
}

float implicitFunctionOnRay(in vec3 pos){
	
	float primitiveValue =  0.0;

	int index;
	for(int i = 0; i < bInRay; i++){
		index = ballsAlongRay[i]+nbOfMetaTubes;
		primitiveValue += implicitSphere(pos,p1s[index],radius[index]);//
	}
	
	float noiseDispValue = 0;//gekkoDispNoize(pos);
	return primitiveValue - threshold + noiseDispValue;
}

vec3 implicitNormal(in vec3 pos){
	float dfdx = DuInv * ( implicitFunctionOnRay(pos + vec3(Du,0.0,0.0)) -
						  implicitFunctionOnRay(pos + vec3(-Du,0.0,0.0)) )
						;
				  
	float dfdy = DuInv * ( implicitFunctionOnRay(pos + vec3(0.0,Du,0.0)) -
				   implicitFunctionOnRay(pos + vec3(0.0,-Du,0.0)) )
				  ;

	float dfdz = DuInv * ( implicitFunctionOnRay(pos + vec3(0.0,0.0,Du)) -
				   implicitFunctionOnRay(pos + vec3(0.0,0.0,-Du)) )
				  ;

	return normalize(vec3(dfdx,dfdy,dfdz));
}




vec4 phongShade(in vec3 pos, in vec3 normal){
	vec4 color = vec4(.3,0.3,0.3,1.0);
	vec4 constcolor = color;//gekkoNoise(pos,normal);

	// Compute dot product of normal and light source direction
	vec3 lightDir = normalize(pos-cameraPos);
	float d = dot(normal,lightDir);
	if(d>0.0)
	{
		//Diffuse
		color += d*vec4(1.0,1.0,1.0,1.0)*constcolor;
		vec3 h = -normalize(-pos-lightDir);
		float s = dot(normal, h);
		//Specular
		if(s>0.0)
			color += pow(s, 20.0)*
				vec4(1.0,1.0,1.0,1.0)*vec4(0.4,0.7,0.3,1.0);
	}
	color.a = 1;
	return color;
}



bool getClosestPosPoint(vec3 origine, vec3 dir, int sphereIndex){
	
	vec3 c = p1s[sphereIndex+nbOfMetaTubes];//ray2.origine;
	posPoint = origine + dot(c-origine,dir)*dir;
	vec3 v = posPoint-c;
	return dot(v,v) <= radius[sphereIndex+nbOfMetaTubes];
}

void main()
{
	vec4 color = vec4(1,1.0,1.0,0.0);
	//Construct ray from eye to fragment
	vec3 rayDir = normalize( worldPos - cameraPos );
	vec3 ray = worldPos;

	//Find first intersection
	int steps = 0;	
	bInRay = 0;

	//ptoject the center of each metaball THAT INTERSECT THE RAY on the ray
	//And store the metaballs that intersect the ray
	nbHits = 0;
	for(int i = 0; i < nbOfBalls; i++){
		if(getClosestPosPoint(ray,rayDir,i)){
			ballsAlongRay[bInRay] = i;
			bInRay++;
			if(implicitFunction(posPoint) > 0){
				posPoints[nbHits] = posPoint;
				nbHits++;
			}
		}
	}

	if(nbHits > 0){
		//find the closest projected center whose value is positive
		float minDist = 10000000; 
		int minIndex = -1;
		for(int i = 0; i < nbHits; i++){
			vec3 v = ray - posPoints[i];
			float dist = dot(v,v);
			if(dist < minDist){
				minIndex = i;
				minDist = dist;
			}
		}

		if(minIndex >= 0){
			vec3 low = ray;
			vec3 hi = posPoints[minIndex];
			vec3 mid = 0.5*(low+hi);	
			
			
			steps = 0;
			float vmid = implicitFunctionOnRay(mid);

			//bisection
			float eps =  0.01;
			while( steps < maxSteps && (vmid > eps || vmid < - eps)){
				steps++;
				mid = 0.5*(low+hi);
				vmid = implicitFunctionOnRay(mid);
				if( vmid < 0.0 )
					low = mid;
				else
					hi = mid;
			}
			ray = mid;


			//find the normal
			vec3 norm = implicitNormal(ray);
			color = phongShade(ray,norm);	
			gl_FragDepth = length(ray - cameraPos)/100;
		}
			
	}else{
		gl_FragDepth = -100000;
	}
	
	gl_FragColor = color;//vec4(floor(worldPos/gridSize)*vec3(gridSize),1);//vec4((float)steps/maxSteps);//bmat[3];//color;//vec4(worldPos,1);//vec4(-implicitFunction(worldPos));////
}
