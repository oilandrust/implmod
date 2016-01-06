
uniform vec3 cameraPos;
uniform int nbOfMetaTubes;
uniform int nbOfBalls;

//IMPLICIT FUNTIONS
uniform vec3 p1s[100];
uniform vec3 p2s[100];
uniform vec3 MetaTubeOrtho[100];

uniform float radius[100];

varying vec3 worldPos;

const vec3 lightPos = vec3(2,1,0);

//RAY TRACING CONSTANTS
const int maxSteps = 50;
const float ds = 0.005;

//GLOBAL VARIABLES FOR RAYTRACE OPTI
vec3 posPoint;
vec3 posPoints[20];
int nbHits = 0;

uniform sampler2D checkTexture;

//DIFFERENTIATION steplenght
const float Du = 0.0001; //potentiel
const float DuInv = 0.5/Du;
const float Dun = 1.0; //noise

//CUBE STYLE
const float gridSize = 0.03;

const float PI = 3.14159265358979323846264;

const float threshold = 0.5;

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

float implicitSphere(in vec3 pos, in vec3 center, in float sqradius){
	float r2 = dot(pos-center,pos-center);
	if(r2 <= sqradius)
		return sqr(1.0-r2/sqradius);
	else
		return 0.0;
}
float implicitMetaTube(in vec3 pos, in vec3 a, in vec3 b, in float sqradius){
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
		return sqr(1.0-r2/sqradius);
	else
		return 0.0;
}

float implicitFunction(in vec3 pos){
	
	float primitiveValue =  0.0;
	for(int i = 0; i < nbOfMetaTubes; i++){
		primitiveValue += implicitMetaTube(pos,p1s[i],p2s[i],radius[i]);//implicitSphere(pos,p1s[i],radius[i]*radius[i]);//
	}
	for(int i = 0; i < nbOfBalls; i++){
		primitiveValue += implicitSphere(pos,p1s[i+nbOfMetaTubes],radius[i+nbOfMetaTubes]);//
	}
	
	return primitiveValue - threshold;//+ 0.5*(noise(2.0*pos))+ 0.1*(noise(4.0*pos)) 
}

float dist(in vec3 pos){
	
	float primitiveValue =  0.0;
	for(int i = 0; i < nbOfMetaTubes; i++){
		primitiveValue += implicitMetaTube(pos,p1s[i],p2s[i],radius[i]);//implicitSphere(pos,p1s[i],radius[i]*radius[i]);//
	}
	for(int i = 0; i < nbOfBalls; i++){
		primitiveValue += implicitSphere(pos,p1s[i+nbOfMetaTubes],radius[i+nbOfMetaTubes]);//
	}
	
	return primitiveValue;//+ 0.5*(noise(2.0*pos))+ 0.1*(noise(4.0*pos)) 
}

vec3 implicitNormal(in vec3 pos){
	float dfdx = DuInv* ( implicitFunction(pos + vec3(Du,0.0,0.0)) -
						  implicitFunction(pos + vec3(-Du,0.0,0.0)) )
						;
				  
	float dfdy = DuInv* ( implicitFunction(pos + vec3(0.0,Du,0.0)) -
				   implicitFunction(pos + vec3(0.0,-Du,0.0)) )
				  ;

	float dfdz = DuInv* ( implicitFunction(pos + vec3(0.0,0.0,Du)) -
				   implicitFunction(pos + vec3(0.0,0.0,-Du)) )
				  ;

	return normalize(vec3(dfdx,dfdy,dfdz));
}


vec4 phongShade(in vec3 pos, in vec3 normal){
	vec4 color = vec4(.5,0.5,0.5,1.0);
	vec4 constcolor;
	constcolor = color ;
	
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
				gl_LightSource[0].specular*vec4(0.7,0.7,0.7,1.0);
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
	vec4 color = vec4(0.5,0.0,0.0,0.5);
	//Construct ray from eye to fragment
	vec3 rayDir = normalize( worldPos - cameraPos );
	vec3 ray = worldPos;//cameraPos;

	//ptoject the center of each metaball THAT INTERSECT THE RAY on the ray
	nbHits = 0;
	for(int i = 0; i < nbOfBalls; i++){
		if(getClosestPosPoint(ray,rayDir,i)){
			if(implicitFunction(posPoint) > 0){
				posPoints[nbHits] = posPoint;
				nbHits++;
			}
		}
	}

	if(nbHits > 0){
		//Find first intersection
		int steps = 0;	

		float value = implicitFunction(ray);
		while( value < 0 && steps < maxSteps ){
			value = implicitFunction(ray);
			float dist = 0.5*abs(value);
			ray += dist*rayDir;
			steps++;
		}
		

		if( value > 0 ){
			//find the normal
			vec3 norm = implicitNormal(ray);
			color = phongShade(ray,norm);	
		}
	}
	//color.xyz = vec3(float(steps)/maxSteps);
	gl_FragColor = color;//vec4(floor(worldPos/gridSize)*vec3(gridSize),1);//vec4((float)steps/maxSteps);//bmat[3];//color;//vec4(worldPos,1);//vec4(-implicitFunction(worldPos));////
}
