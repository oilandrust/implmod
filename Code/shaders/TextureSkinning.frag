
//TO CONSTRUCT THE RAYS
uniform vec3 cameraPos;
varying vec3 worldPos;

//IMPLICIT FUNTIONS
uniform vec3 p1s[5];
uniform vec3 p2s[5];
uniform float radius[5];
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
uniform bool doHyperTexture;

//RAY TRACING CONSTANTS
const int maxSteps = 50;
const float ds = 0.025;

//GLOBAL VARIABLES FOR RAYTRACE OPTI
vec3 posPoint;
vec3 posPoints[20];
int nbHits = 0;

//Bones 1 primitve in p1s -> same index bone
uniform mat4 boneInvTrans[30];
uniform mat4 boneRestTrans[30];

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

bool getCylinderUV(vec3 p, int projector, out vec2 uv){
	vec3 a = proj1s[projector];//p1s[boneID];
	vec3 b = proj2s[projector];//p2s[boneID];
	
	vec3 n = b - a;
	float lengh = length(n);
	n = n/lengh;
	vec3 ap = p - a;
	float Lambdap = dot( ap , n );
	
	if(Lambdap < 0 || Lambdap > lengh)
		return false;

	vec3 projected = a+Lambdap*n;
	vec3 pproj = p-projected;
	
	float dist = length(pproj);
	if(dist > rads[projector])
		return false;
		
	pproj = pproj/dist;
	
	uv.x = Lambdap/lengh;
	float c = dot(pproj,orthos[projector]);
	float s = dot( cross(pproj,orthos[projector]), n);

	if( s > 0.0 )
		uv.y =  0.5*acos( c )/3.141592;
	else
		uv.y = 0.5+0.5*(PI-acos( c ))/PI;
	
	return true;
}

vec2 getUV(vec3 p){
	vec2 uv = vec2(0);
	for(int i = 0; i < 3; i++){
		if(getCylinderUV(p,i,uv))
			break;
	}
	return uv;
}

vec4 textureColor(vec3 p, int projector){
	vec2 uv = vec2(0);
	if(getCylinderUV(p,projector,uv)){
		return texture2D(checkTexture, uv);
	}else
		return vec4(1,1,1,1);
}



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

float sqDistTosegment(in vec3 pos, in vec3 a, in vec3 b, in float sqradius){
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
		return r2;
	else
		return 0.0;
}

float DistToPoint(in vec3 pos, in vec3 center, in float sqradius){
	return dot(pos-center,pos-center);
}

float DistToSegment(in vec3 pos, in vec3 a, in vec3 b, in float sqradius){
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

	return r2;
}

float distanceToBone(in int boneID, in vec3 ray){
	float val = implicitMetaTube(ray,p1s[boneID],p2s[boneID],radius[boneID]);
	/*
	if(val < 0.01)
		return 0;
	else if(val>threshold-0.001)
		return 1;
	else 
	*/
		return val/threshold;
}
float sigmaDist(in vec3 pos){
	
	float primitiveValue =  10000000;
	for(int i = 0; i < nbOfMetaTubes; i++){
		primitiveValue = min(primitiveValue,implicitMetaTube(pos,p1s[i],p2s[i],radius[i]));//implicitSphere(pos,p1s[i],radius[i]*radius[i]);//
	}
	for(int i = 0; i < nbOfBalls; i++){
		primitiveValue = min(primitiveValue,implicitSphere(pos,p1s[i+nbOfMetaTubes],radius[i+nbOfMetaTubes]));//
	}
	
	return primitiveValue;//+ 0.5*(noise(2.0*pos))+ 0.1*(noise(4.0*pos)) 
}

float implicitFunction(in vec3 pos){
	
	float primitiveValue =  0.0;
	for(int i = 0; i < nbOfMetaTubes; i++){
		primitiveValue += implicitMetaTube(pos,p1s[i],p2s[i],radius[i]);//implicitSphere(pos,p1s[i],radius[i]*radius[i]);//
	}
	for(int i = 0; i < nbOfBalls; i++){
		primitiveValue += implicitSphere(pos,p1s[i+nbOfMetaTubes],radius[i+nbOfMetaTubes]);//
	}
	
	return primitiveValue - threshold;// + 0.025*noise(2.0*pos)+ 0.001*noise(4.0*pos);
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

vec3 implicitSphereNormal(in vec3 pos){
	float dfdx = DuInv * ( implicitFunction(pos + vec3(Du,0.0,0.0)) -
						  implicitFunction(pos + vec3(-Du,0.0,0.0)) )
						;
				  
	float dfdy = DuInv * ( implicitFunction(pos + vec3(0.0,Du,0.0)) -
				   implicitFunction(pos + vec3(0.0,-Du,0.0)) )
				  ;

	float dfdz = DuInv * ( implicitFunction(pos + vec3(0.0,0.0,Du)) -
				   implicitFunction(pos + vec3(0.0,0.0,-Du)) )
				  ;

	return normalize(vec3(dfdx,dfdy,dfdz));
}


vec4 phongShade(in vec3 pos, in vec3 normal){
	vec4 color = vec4(.5,0.5,0.5,1.0);
	vec4 constcolor = color;
	//vec3 pos = floor(pose/gridSize)*vec3(gridSize)+vec3(gridSize/2);
	
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


bool getClosestPointOnRayFromSphere(vec3 origine, vec3 dir, int sphereIndex){
	
	vec3 c =p1s[sphereIndex+nbOfMetaTubes];//ray2.origine;
	posPoint = origine + dot(c-origine,dir)*dir;
	vec3 v = posPoint-c;
	if(dot(v,v) <= radius[sphereIndex+nbOfMetaTubes])
		return true;
	return false;
}

bool getClosestPosPoint(vec3 origine, vec3 dir, int sphereIndex){
	
	vec3 c = p1s[sphereIndex+nbOfMetaTubes];//ray2.origine;
	posPoint = origine + dot(c-origine,dir)*dir;
	vec3 v = posPoint-c;
	return dot(v,v) <= radius[sphereIndex+nbOfMetaTubes];
}

void main()
{
	vec4 color = vec4(1.0,1.0,1.0,0.0);
	vec4 texColor;
	//Construct ray from eye to fragment
	vec3 rayDir = normalize( worldPos - cameraPos );
	vec3 ray = worldPos;//cameraPos;

	//Find first intersection
	int steps = 0;	
	bool done = false; 
	
	vec3 old;	
	float value = implicitFunction(ray);
	while( !done && steps < maxSteps ){
		value = implicitFunction(ray);
		if(value >= 0.0){
				done = true;
		}else{
			old = ray;
			ray += ds*rayDir;
			steps++;
		}
	}

	vec3 low = old;//getFirstRayBoxInter(ray, rayDir, node);
	vec3 mid = 0.5*(low+ray);	
	vec3 hi = ray;

	//bisection
	steps = 0;
	float eps =  0.001;
	float vmid = implicitFunction(mid);
	while( steps < maxSteps && (vmid > eps || vmid < - eps)){
		steps++;
		mid = 0.5*(low+hi);
		vmid = implicitFunction(mid);
		if( vmid < 0.0 )
			low = mid;
		else
			hi = mid;
	}

	ray = mid;
	if(value >= -0.001 ){
		//find the normal
		vec3 norm = implicitSphereNormal(ray);
		color = phongShade(ray,norm);	

		vec2 uv = vec2(0);

		vec3 restPosition = vec3(0,0,0);	

		vec4 we;
		for(int bone = 0; bone <  4; bone++){
			//skin
			float w = distanceToBone(bone, ray);
			we[bone] = w;
			//transform
			vec3 transformed = ray;
			//Back to object space
			transformed += boneInvTrans[bone][3].xyz;
			transformed = mat3(boneInvTrans[bone])*transformed;
			//to rest world space
			transformed = transpose(mat3(boneRestTrans[bone]))*transformed;
			transformed.xyz += boneRestTrans[bone][3].xyz;
			//sum
			restPosition += w*transformed.xyz;;
		}

		if(doHyperTexture){
			float noise1 = (noise(restPosition)+0.5);
			float noise2 = (noise(0.5*restPosition)+0.5);
			float noise3 = (noise(0.1*restPosition)+0.5);

			color *=vec4(noise1,noise2,noise3,1.0);
		}		else
			color *= textureColor(restPosition, 0);
	}

	gl_FragColor = color;//vec4(floor(worldPos/gridSize)*vec3(gridSize),1);//vec4((float)steps/maxSteps);//bmat[3];//color;//vec4(worldPos,1);//vec4(-implicitFunction(worldPos));////
}
