
uniform vec3 cameraPos;
uniform int nbOfMetaTubes;
uniform int nbOfBalls;

//IMPLICIT FUNTIONS
uniform vec3 p1s[100];
uniform vec3 p2s[100];
uniform vec3 MetaTubeOrtho[10];
uniform vec3 BoneWorldPos[10];

uniform float alpha;

uniform mat3 MetaTubeMat[10];

uniform float radius[100];
uniform int boneID;

varying mat4 bmat;
varying vec3 worldPos;


const vec3 lightPos = vec3(2,1,0);

//RAY TRACING CONSTANTS
const int maxSteps = 50;
const float ds = 0.005;

uniform sampler2D checkTexture;

//DIFFERENTIATION steplenght
const float Du = 0.0001; //potentiel
const float Dun = 1.0; //noise

//CUBE STYLE
const float gridSize = 0.02;

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

vec2 getUV(vec3 p){
	vec3 a = vec3(0,0,0);//p1s[boneID];
	vec3 b = vec3(0,0,2);//p2s[boneID];
	
	
	vec3 n = b - a;
	float lengh = length(n);
	n = n/lengh;
	vec3 ap = p - a;
	float Lambdap = dot( ap , n );

	vec3 projected = a+Lambdap*n;
	float u = Lambdap/lengh;
	float c = dot(normalize(p-projected),vec3(1,0,0));//MetaTubeOrtho[boneID]);
	float s = dot( cross(normalize(p-projected),vec3(1,0,0)), u);
	float v;
	if( s > 0 )
		v =  0.5*acos( c )/3.141592;
	else
		v = 0.5+0.5*(PI-acos( c ))/PI;
	
	return vec2(u,v);
}




float sqr(float x){
	return x*x;
}

float implicitSphere(vec3 pos, vec3 center, float sqradius){
	float r2 = dot(pos-center,pos-center);
	if(r2 <= sqradius)
		return sqr(1.0-r2/sqradius);
	else
		return 0.0;
}
float implicitMetaTube(vec3 pos, vec3 a, vec3 b, float sqradius){
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

float distanceToBone(int boneID, vec3 ray){
	float val = implicitMetaTube(ray,p1s[boneID],p2s[boneID],radius[boneID]);
	if(val < 0.0001)
		return 0;
	else if(val>0.49)
		return 1;
	else 
		return val/threshold;
}
float implicitFunction(vec3 pos){
	
	float primitiveValue =  0.0;
	
	for(int i = 0; i < nbOfMetaTubes; i++){
		primitiveValue += implicitMetaTube(pos,p1s[i],p2s[i],radius[i]);//implicitSphere(pos,p1s[i],radius[i]*radius[i]);//
	}
	for(int i = 0; i < nbOfBalls; i++){
		primitiveValue += implicitSphere(pos,p1s[i+nbOfMetaTubes],radius[i+nbOfMetaTubes]);//
	}
	/*
	vec3 restPosition = vec3(0,0,0);
	for(int i = 0; i < 2; i++){
		//skin
		float w = distanceToBone(i, pos);
		//transform
		vec3 transformed = pos;
		transformed -= MetaTubeOrtho[i];
		transformed = MetaTubeMat[i]*transformed;
		transformed += MetaTubeOrtho[i];
		//sum
		restPosition += w*transformed;
	}
	//color = vec4(distanceToBone(0, ray),distanceToBone(1, ray),0,1);
	//color *=  texture2D(checkTexture, getUV(restPosition));//
	float h = texture2D(checkTexture, getUV(restPosition));
*/
	return primitiveValue - threshold;//+ 0.5*(noise(2.0*pos))+ 0.1*(noise(4.0*pos)) 
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


vec4 phongShade(vec3 pos, vec3 normal){
	vec4 color = vec4(.5,0.5,0.5,1);
	vec4 constcolor;
	//vec3 pos = floor(pose/gridSize)*vec3(gridSize)+vec3(gridSize/2);
	/*	
	float noise1 = (noise(pos)+0.5);
	float noise2 = (noise(2.0*pos)+0.5);
	float noise3 = (noise(3.0*pos)+0.5);
*/
	constcolor = color ; //= mix(vec4(noise1,0.5*noise2,0.2*noise3,1.0),vec4(0.4*noise2,noise3,0.0,1.0),(-normal.z+1.0)/1.5);
	
	

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

bool getClosestPointOnRayFromTube(vec3 origine, vec3 dir, int tubeIndex, out vec3 point){
	
	vec3 delta = origine - p1s[tubeIndex];//ray2.origine;
	vec3 tdir = normalize(p2s[tubeIndex]-p1s[tubeIndex]);
	
	float d1d2 = dot(dir,tdir);
	
	float dd1 = dot(delta,dir);
	float dd2 = dot(delta,tdir);

	float s = ( dd1*d1d2 - dd2 ) / ( d1d2*d1d2 - 1 );
	float t = s*d1d2 - dd1;

	point = origine + t * dir;

	return true;
}


void main()
{
	//Construct ray from eye to fragment
	vec3 rayDir = normalize( worldPos - cameraPos );
	vec3 ray = worldPos;//cameraPos;
	
	int steps = 0;	
	vec3 old;

	vec3 closest;
	getClosestPointOnRayFromTube(ray,rayDir,boneID,closest);
	
	bool done = implicitFunction(closest)>0;
	
	if(done){
		old = ray;// - ds*rayDir;
		vec3 low = old;
		vec3 mid = 0.5*(old+closest);
		vec3 hi = closest;
		float vlow = implicitFunction(low);
		float vmid = implicitFunction(mid);
		float vhi = implicitFunction(hi);
		if(vlow>vhi){
			hi = old;
			low = closest; 
		}
			
		//bisection
		float eps =  0.0001;
		while( steps < maxSteps && (vmid > eps || vmid < - eps)){
			steps++;
			mid = 0.5*(low+hi);
			vmid = implicitFunction(mid);
			if( vmid < 0 )
				low = mid;
			else
				hi = mid;
		}
		ray = mid;
	}
	
	

	//Shade
	vec4 color = vec4(steps/maxSteps);	
	if( !done )
		color = vec4(steps/maxSteps);
	else{
		//find the normal
		vec3 norm = normalize(implicitSphereNormal(ray));
		//clamp normal for cube
		/*
		if(abs(norm.x) > abs(norm.y))
			if(abs(norm.x) > abs(norm.z))
				norm = normalize( vec3( norm.x,0,0) );
			else
				norm = normalize( vec3( 0,0,norm.z) );
		else if(abs(norm.y) > abs(norm.z))
				norm = normalize( vec3(0,norm.y,0)  );
			else
				norm = normalize( vec3( 0,0,norm.z) );
		*/
		
		color = phongShade(ray,norm);

	

		vec3 restPosition = vec3(0,0,0);	
		for(int i = 0; i < 4; i++){
			//skin
			float w = distanceToBone(i, ray);
			//transform
			vec3 transformed = ray;
			transformed -= BoneWorldPos[i];
			transformed = MetaTubeMat[i]*transformed;
			transformed += MetaTubeOrtho[i];
			//sum
			restPosition += w*transformed;
		}
		//color = vec4(distanceToBone(0, ray),distanceToBone(1, ray),0,1);
		color *=  texture2D(checkTexture, getUV(restPosition));//

		color.a = alpha;
	/*
		ray -= bmat[3].xyz;
		ray = MetaTubeMat[boneID]*ray;
		//ray += MetaTubeOrtho[boneID];
		color *=  texture2D(checkTexture, getUV(ray));//
		//color = vec4(ray,1);//
		*/
	
	}
	
	gl_FragColor = color;//vec4(floor(worldPos/gridSize)*vec3(gridSize),1);//vec4((float)steps/maxSteps);//bmat[3];//color;//vec4(worldPos,1);//vec4(-implicitFunction(worldPos));////
}
