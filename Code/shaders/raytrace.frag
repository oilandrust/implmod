uniform vec3 cameraPos;

//BVH
#extension EXT_gpu_shader4 : enable
uniform samplerBuffer tboSampler;
uniform int nbOfNodes;

//IMPLICIT FUNTIONS
uniform int nbOfBalls;
uniform vec3 p1s[100];
uniform float radius[100];

//Stack
int OpenList[10];
int top = 0;


//***************GLOBAL VARIABLES FOR RAYTRACE OPTI
//A point inside the surface for bisection
vec3 posPoint;
//Record of points along the ray that are positive
vec3 posPoints[16];
int nbHits = 0;
//The metaballs that are contributing along the ray
int ballsInNode[16];
int nbBallsInNode = 0;

varying mat4 bmat;
varying vec3 worldPos;

const vec3 lightPos = vec3(2,1,0);

//RAY TRACING CONSTANTS
const int maxSteps = 500;
const float ds = 0.01;


//DIFFERENTIATION steplenght
const float Du = 0.001; //potentiel
const float Dun = 1.0; //noise

//CUBE STYLE
const float gridSize = 0.03;

const float PI = 3.14159265358979323846264;

const float threshold = 0.5;


float sqr(in float x){
	return x*x;
}

float implicitSphere(vec3 pos, vec3 center, float sqradius){
	float r2 = dot(pos-center,pos-center);
	if(r2 <= sqradius)
		return sqr(1.0-r2/sqradius);
	else
		return 0.0;
}



vec4 nodeBottom(int node){
	return texelFetchBuffer(tboSampler, 2*node);
}
vec4 nodeTop(int node){
	return texelFetchBuffer(tboSampler, 2*node+1);
}


int intCompFromIndex(int i,vec4 vec){
	if( i == 0 )
		return int(vec.x);
	else if( i == 1 )
		return int(vec.y);
	else if( i == 2 )
		return int(vec.z);
	else if( i == 3 )
		return int(vec.w);
	
	return -1;
}

float implicitFunctionOfNode(vec3 pos, int node){
	float primitiveValue = 0;
	for(int i = 0;i < nbBallsInNode;i++){
		primitiveValue += implicitSphere(pos,p1s[ballsInNode[i]],radius[ballsInNode[i]]);
	}

	return primitiveValue - threshold;
}

vec3 implicitNormalOfNode(vec3 pos,int node){
	float dfdx = ( implicitFunctionOfNode(pos + vec3(Du,0.0,0.0), node) -
				   implicitFunctionOfNode(pos + vec3(-Du,0.0,0.0), node) )
				  /(2.0*Du);
				  
	float dfdy = ( implicitFunctionOfNode(pos + vec3(0.0,Du,0.0), node) -
				   implicitFunctionOfNode(pos + vec3(0.0,-Du,0.0), node) )
				  /(2.0*Du);

	float dfdz = ( implicitFunctionOfNode(pos + vec3(0.0,0.0,Du), node) -
				   implicitFunctionOfNode(pos + vec3(0.0,0.0,-Du), node) )
				  /(2.0*Du);

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



bool RayIntestectsAABB(vec3 origine, vec3 dir, vec3 aabbBottom, vec3 aabbTop, out float dist){

	//return PointInsideAABB(origine,aabbBottom,aabbTop);
	//	return true;
	
	//the box halfhedges
	vec3 he = 0.5*(aabbTop-aabbBottom);
	
	vec3 slabs[3];
	slabs[0] = vec3(1,0,0);
	slabs[1] = vec3(0,1,0);
	slabs[2] = vec3(0,0,1);

	float tmin = - 10000000.0;
	float tmax = 10000000.0;

	vec3 worldCenter = 0.5*(aabbTop+aabbBottom);
	vec3 p = worldCenter - origine;
	
	vec3 mins = vec3(tmin);

	for(int i = 0; i < 3; i++){
		float e = dot(slabs[i],p);
		float f = dot(slabs[i],dir);

		if( abs(f) > 0.0 ){
			float finv = 1.0 / f;
			float t1 = ( e + he[i] ) * finv;
			float t2 = ( e - he[i] ) * finv;

			if( t1 > t2 ){
				float tmp = t1;
				t1 = t2;
				t2 = tmp;
			}

			if( t1 > tmin )
				tmin = t1;
			if( t2 < tmax )
				tmax = t2;

			if( tmin > tmax )
				return false;
			if( tmax < 0.0 )
				return false;
			
			dist = tmin;
			mins[i] = t1;

		}else if( ( -e - he[i] > 0.0 ) || ( -e + he[i] < 0.0 ) )
			return false;
	}
	//dist = max(mins[0],max(mins[1],mins[2]));
	return true;
}

vec3 getFirstRayBoxInter(vec3 ray, vec3 rayDir, int node){
	vec3 bottomC = nodeBottom(node).xyz;
	vec3 topC = nodeTop(node).xyz;
	
	float d;
	if(RayIntestectsAABB(ray, rayDir, bottomC, topC, d))
		return ray + rayDir*d;
	else
		return ray;
}

int child1(int node){
	return 2*node + 1;
}
int child2(int node){
	return 2*node + 2;
}

void push(int value){
	OpenList[top] = value;
	top++;
}
int pop(){
	top--;
	return OpenList[top];
}

bool getClosestPosPoint(vec3 origine, vec3 dir, int sphereIndex){
	
	vec3 c = p1s[sphereIndex];//ray2.origine;
	posPoint = origine + dot(c-origine,dir)*dir;
	vec3 v = posPoint-c;
	return dot(v,v) <= radius[sphereIndex];
}



bool rayIntersectsSurfaceInNode(vec3 origine, vec3 dir, int node){
	int leafIndex = int(nodeBottom(node).w);
	nbBallsInNode = int(nodeTop(node).w);

	vec4 b1 = nodeBottom(leafIndex);
	vec4 b2 = nodeTop(leafIndex);
	vec4 b3 = nodeTop(leafIndex+1);
	vec4 b4 = nodeTop(leafIndex+1);

	//LOAD THE METABALLS OF THE NODE
	for(int i = 0; i < nbBallsInNode; i++){
		if(i < 4)
			ballsInNode[i] = intCompFromIndex(i,b1);
		else if(i < 8)
			ballsInNode[i] = intCompFromIndex(i-4,b2);
		else if(i < 12)
			ballsInNode[i] = intCompFromIndex(i-8,b3);
		else if(i < 16)
			ballsInNode[i] = intCompFromIndex(i-12,b4);
	}
	
	//project the center of each metaball THAT INTERSECT THE RAY on the ray
	//And store the metaballs that intersect the ray
	nbHits = 0;
	for(int i = 0; i < nbBallsInNode; i++){
		int ballIndex = ballsInNode[i];
		if(getClosestPosPoint(origine,dir,ballIndex)){
			if(implicitFunctionOfNode(posPoint,node) > 0){
				posPoints[nbHits] = posPoint;
				nbHits++;
			}
		}
	}

	if(nbHits == 0)
		return false;

	float minDist = 10000000; 
	int minIndex = -1;
	for(int i = 0; i < nbHits; i++){
		vec3 v = origine - posPoints[i];
		float dist = dot(v,v);
		if(dist < minDist){
			minIndex = i;
			minDist = dist;
		}
	}

	posPoint = posPoints[minIndex];

	return true;
}

int RayIntestectsBVH(vec3 origine, vec3 dir){

	vec4 bottomC;
	vec4 topC;

	top = 0;
	int node = 0;
	int c1;
	int c2;
	
	push(node);
	bool c1Closer = false;

	while(top > 0){
		//First non expanded node
		node = pop();
		
		bottomC = nodeBottom(node);
		topC = nodeTop(node);
		if(bottomC.w != -1.0 || topC.w != -1.0){
			if(rayIntersectsSurfaceInNode(origine, dir, node))
			if(implicitFunctionOfNode(posPoint,node) > 0)
				return node;
		}else{
			//expand
			c1 = child1(node);
			c2 = child2(node);
			
			float d1;
			float d2;
			
			//Test c1
			bottomC = nodeBottom(c1);
			topC = nodeTop(c1);
			bool c1Intersects = RayIntestectsAABB(origine, dir, bottomC.xyz, topC.xyz, d1);

			//Test c2
			bottomC = nodeBottom(c2);
			topC = nodeTop(c2);
			bool c2Intersects = RayIntestectsAABB(origine, dir, bottomC.xyz, topC.xyz, d2);
	
				
			c1Closer = d1 <= d2;
			
			if( c1Intersects ){
				if( c2Intersects ){
					if( c1Closer ){
						push(c2);
						push(c1);
					}else{
						push(c1);
						push(c2);
					}
				}else{
					push(c1);
				}
			}else if( c2Intersects ){
				push(c2);
			}
		}
	}
	return -1;
}


void main()
{
	//Construct ray from eye to fragment
	vec3 rayDir = normalize( worldPos - cameraPos );
	vec3 ray = worldPos;//cameraPos;
	
	vec4 color = vec4(.3,0,0,0.8);

	int node = RayIntestectsBVH(ray, rayDir);
	if(node != -1){
		ray = getFirstRayBoxInter(ray, rayDir, node);

		vec3 low = ray;
		vec3 hi = posPoint;
		vec3 mid = 0.5*(low+hi);	

		int steps = 0;
		float vmid = implicitFunctionOfNode(mid,node);

		//bisection
		float eps =  0.01;
		while( steps < maxSteps && (vmid > eps || vmid < - eps)){
			steps++;
			mid = 0.5*(low+hi);
			vmid = implicitFunctionOfNode(mid,node);
			if( vmid < 0.0 )
				low = mid;
			else
				hi = mid;
		}
		ray = mid;
		
		vec3 norm = implicitNormalOfNode(ray,node);
		color = phongShade(ray,norm);
	
	}	else color = vec4(.5,0,0,0.5);

	gl_FragColor = color;
}


