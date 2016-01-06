uniform vec3 cameraPos;
uniform int nbOfMetaTubes;
uniform int nbOfBalls;
uniform int nbOfNodes;
uniform sampler2D procTex;
uniform sampler2D checkTexture;

#extension EXT_gpu_shader4 : enable
uniform samplerBuffer tboSampler;

//IMPLICIT FUNTIONS
uniform vec3 p1s[30];
uniform vec3 p2s[30];
uniform float radius[30];

//Bones 1 primitve in p1s -> same index bone
uniform int boneIds[30];
uniform mat4 boneTrans[30];
uniform mat4 boneRestTrans[30];

//Texture Projectors
uniform vec3 proj1s[5];
uniform vec3 proj2s[5];
uniform vec3 orthos[5];
uniform float rads[5];

//Stack
int OpenList[10];
int top = 0;

//Globab variables for root finding opti
mat2x4 sphrs = mat2x4(-1);
int nbSphInNode = 0;
vec4 nodeS1 = vec4(-1,-1,-1,-1);
vec4 nodeS2 = vec4(-1,-1,-1,-1);
vec4 nodeT1 = vec4(-1,-1,-1,-1);
vec4 nodeT2 = vec4(-1,-1,-1,-1);
vec3 posPoint = vec3(1,0,0);
vec3 negPoint = vec3(0,1,0);
vec3 hitPtsInNode[8];

varying vec3 worldPos;

const vec3 lightPos = vec3(2,1,0);

//RAY TRACING CONSTANTS
const int maxSteps = 10;
const float ds = 0.01;

vec3 debugVec;

//DIFFERENTIATION steplenght
const float Du = 0.001; //potentiel
const float DuInv = 0.5/Du;
const float Dun = 1.0; //noise

//CUBE STYLE
const float gridSize = 0.01;

const float PI = 3.14159265358979323846264;

const float threshold = 0.5;


bool getCylinderUV(vec3 p, int projector, out vec2 uv){
	vec3 a = proj1s[projector];//p1s[boneID];
	vec3 b = proj2s[projector];//p2s[boneID];
	
	vec3 n = b - a;
	float lengh = length(n);
	n = n/lengh;
	vec3 ap = p - a;
	float Lambdap = dot( ap , n );
	
	if(Lambdap < 0)
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

	return r2 <= sqradius?sqr(1.0-r2/sqradius):0;

}

float distanceToBone(int boneID, vec3 ray){
	float val = implicitMetaTube(ray,p1s[boneID],p2s[boneID],radius[boneID]);

	if(val < 0.0001)
		return 0.0;
	else if(val>0.49)
		return 1.0;
	else 

	return val/threshold;
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

float implicitFunctionOfNode(vec3 pos){
	
	float primitiveValue =  0.0;
	int i;

	for(i = 0; i < 4; i++){
		int t = intCompFromIndex(i,nodeT1);
		if( 0 <= t )
			primitiveValue += implicitMetaTube(pos,p1s[t],p2s[t],radius[t]);
	}
	for(i = 0; i < 4; i++){
		int t = intCompFromIndex(i,nodeT2);
		if( 0 <= t )
			primitiveValue += implicitMetaTube(pos,p1s[t],p2s[t],radius[t]);
	}

	for(i = 0;i < 4;i++){
		int s = intCompFromIndex(i,nodeS1);
		if( 0 <= s ){
			primitiveValue += implicitSphere(pos,p1s[s+nbOfMetaTubes],radius[s+nbOfMetaTubes]);
		}
	}
	for(i = 0;i < 4;i++){
		int s = intCompFromIndex(i,nodeS2);
		if( 0 <= s ){
			primitiveValue += implicitSphere(pos,p1s[s+nbOfMetaTubes],radius[s+nbOfMetaTubes]);
		}
	}

	for(int i = 0; i < nbSphInNode; i++){
		int off = i%4;
		int s = sphrs[off][i-off];
		primitiveValue += implicitSphere(pos,p1s[s+nbOfMetaTubes],radius[s+nbOfMetaTubes]);
	}
	
	return primitiveValue - threshold;
}

vec3 implicitNormalOfNode(vec3 pos,int node){
	float dfdx = DuInv* ( implicitFunctionOfNode(pos + vec3(Du,0.0,0.0)) -
				   implicitFunctionOfNode(pos + vec3(-Du,0.0,0.0)) )
				  ;
				  
	float dfdy = DuInv* ( implicitFunctionOfNode(pos + vec3(0.0,Du,0.0)) -
				   implicitFunctionOfNode(pos + vec3(0.0,-Du,0.0)) )
				  ;

	float dfdz = DuInv* ( implicitFunctionOfNode(pos + vec3(0.0,0.0,Du)) -
				   implicitFunctionOfNode(pos + vec3(0.0,0.0,-Du)) )
				  ;

	return normalize(vec3(dfdx,dfdy,dfdz));
}



vec4 phongShade(vec3 pos, vec3 normal){
	vec4 color = vec4(.5,0.5,0.5,1);
	vec4 constcolor;
	//vec3 pos = floor(pose/gridSize)*vec3(gridSize)+vec3(gridSize/2);
		
	//float noise1 = (noise(pos)+0.5);
	//float noise2 = (noise(2.0*pos)+0.5);
	//float noise3 = (noise(3.0*pos)+0.5);

	constcolor =  color;//mix(vec4(noise1,0.5*noise2,0.2*noise3,1.0),vec4(0.4*noise2,noise3,0.0,1.0),(-normal.z+1.0)/1.5);
	
	// Compute dot product of normal and light source direction
	vec3 lightDir = -normalize(pos-cameraPos);
	float d = dot(normal,lightDir);
	if(d>0.0)
	{
		//Diffuse
		color += d*gl_LightSource[0].diffuse*constcolor;

		vec3 h = -normalize(-pos-lightDir);
		float s = dot(normal, h);
		//Specular
		//if(s>0.0)
		//	color += pow(s, 50.0)*
		//		gl_LightSource[0].specular*vec4(0.7,0.7,0.7,1);
			
	}
	//color.a = 0.7;
	return color;
}

bool PointInsideAABB(vec3 origine, vec3 aabbBottom, vec3 aabbTop){

	if(origine.x < aabbBottom.x)
		return false;
	if(origine.x > aabbTop.x)
		return false;
	if(origine.y < aabbBottom.y)
		return false;
	if(origine.y > aabbTop.y)
		return false;
	if(origine.z < aabbBottom.z)
		return false;
	if(origine.z > aabbTop.z)
		return false;
		
	return true;
		
}

bool PointInsideNode(vec3 origine, int node){
	return PointInsideAABB(origine, nodeBottom(node), nodeTop(node));
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

bool getClosestPointOnRayFromSphere(vec3 origine, vec3 dir, int sphereIndex){
	
	vec3 c =p1s[sphereIndex+nbOfMetaTubes];//ray2.origine;
	posPoint = origine + dot(c-origine,dir)*dir;
	vec3 v = posPoint-c;
	return dot(v,v) <= radius[sphereIndex+nbOfMetaTubes];

}

bool getClosestPointOnRayFromTube(vec3 origine, vec3 dir, int tubeIndex){
	
	//Point where the function is likely to be positive if any
	vec3 pPos;
	vec3 hits[3];
	int hitc;

	//******************************************pPos on the plane paralell to tube axis and view oriented
	vec3 mid = 0.5f*(p1s[tubeIndex]+p2s[tubeIndex]); 
	vec3 tdir = normalize(p2s[tubeIndex]-p1s[tubeIndex]);
	//Plane most ortho to view and parallel to tube

	vec3 nPlane = normalize(dir-dot(tdir,dir)*tdir);
	float lambda = dot(mid-origine,nPlane)/dot(nPlane,dir);
	
	pPos = origine + lambda * dir;
	
	if( implicitMetaTube(pPos, p1s[tubeIndex], p2s[tubeIndex], radius[tubeIndex] ) > 0 ){
		posPoint = pPos;
		hits[0] = pPos;
		hitc++;
		//return true;
	}

	//******************************************pPos on the plane passing through p1 and view oriented

	nPlane = p1s[tubeIndex] - cameraPos;
	vec3 c = p1s[tubeIndex];//ray2.origine;
	lambda = dot(c-origine,nPlane)/dot(nPlane,dir);
	pPos = origine + lambda*dir;

	if(implicitMetaTube(pPos, p1s[tubeIndex], p2s[tubeIndex], radius[tubeIndex] ) > 0){
		posPoint = pPos;
		return true;
	}

	//******************************************pPos on the plane passing through p2 and view oriented
	/*
	nPlane = p2s[tubeIndex] - cameraPos;
	c = p2s[tubeIndex];//ray2.origine;
	lambda = dot(c-origine,nPlane)/dot(nPlane,dir);
	pPos = origine + lambda*dir;
	if(implicitMetaTube(pPos, p1s[tubeIndex], p2s[tubeIndex], radius[tubeIndex] ) > 0){
		posPoint = pPos;
		return true;
	}
	*/
	
		
	return false;
}

bool rayIntersectsSurfaceInNode(vec3 origine, vec3 dir, int leafIndex, int nbOfspheres){
	nbSphInNode = nbOfspheres;
	nodeS1 = nodeBottom(leafIndex);
	nodeS2 = nodeTop(leafIndex);
	nodeT1 = nodeBottom(leafIndex+1);
	nodeT2 = nodeTop(leafIndex+1);

	for(int i = 0; i < 4; i++){
		int t = intCompFromIndex(i,nodeT1);
		if( 0 <= t){
			if(getClosestPointOnRayFromTube(origine,dir,t))
				return true;
		}
	}

	for(int i = 0; i < 4; i++){
		int t = intCompFromIndex(i,nodeT2);
		if( 0 <= t){
			if(getClosestPointOnRayFromTube(origine,dir,t))
				return true;
		}
	}

	sphrs = mat2x4(nodeS1,nodeS2);
	for(int i = 0; i < nbSphInNode; i++){
		int off = i%4;
		int t = sphrs[off][i-off];
		if (getClosestPointOnRayFromSphere(origine,dir,t)){
			//if(implicitFunctionOfNode(posPoint)>0.0){
				return true;
			//}
		}
	}
	
	
	return false;
}

int RayIntestectsBVH2(vec3 origine, vec3 dir){

	vec4 bottomC;
	vec4 topC;
	int c1;
	int c2;
	
	bool c1Closer = false;
	int node = -1;
	
	while(top > 0){
		//First non expanded node
		node = pop();
		
		bottomC = nodeBottom(node);
		topC = nodeTop(node);
		if(bottomC.w != -1.0){
			if(rayIntersectsSurfaceInNode(origine, dir, int(bottomC.w),int(topC.w)))
			//if(implicitFunctionOfNode(posPoint) > 0)
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
	vec3 origine = ray;
	
	//Color for ray shot but not intersecting the surface
	//vec4 color = vec4(.3,0,0,0.5);
	vec4 color = vec4(1.0,1.0,1.0,0.0);
	
	//Initialize the stack
	top = 0;
	push(0);

	int node = RayIntestectsBVH2(ray, rayDir);

	int steps = 0;	
	

	//Bisection refinement if we found a sign alternance
	if(node != -1){
		color.a = 1;
		color.xyz = vec3(1,0,0);
		if(implicitFunctionOfNode(posPoint) > 0){
			color.xyz = vec3(0,1,0);
			ray = getFirstRayBoxInter(origine, rayDir, node);
			
			if(implicitFunctionOfNode(ray)<0){
				vec3 low = ray;
				vec3 mid = 0.5*low + 0.5*posPoint;	
				vec3 hi = posPoint;
		
				steps = 0;
		
				float vmid = implicitFunctionOfNode(mid);
			
				//bisection
				float eps =  0.01;
			
				while( steps < maxSteps && (vmid > eps || vmid < - eps)){
					steps++;
					mid = 0.5*low+0.5*hi;
					vmid = implicitFunctionOfNode(mid);
					if( vmid < 0.0 )
						low = mid;
					else
						hi = mid;
				}
				
				
				//find the normal
				vec3 norm = -implicitNormalOfNode(mid,node);
				//Shading
				//color = phongShade(mid,norm);
			}

		}	
	}
	
	
	gl_FragColor = color;//vec4(floor(worldPos/gridSize)*vec3(gridSize),1);//bmat[3];//color;//vec4(worldPos,1);//vec4(-implicitFunction(worldPos));////
}

/*
			vec3 restPosition = vec3(0,0,0);
			vec4 weights = vec4(0,0,0,0);	
			for(int i = 0; i < 4; i++){
				
				int t = intCompFromIndex(i,nodeT1);
				if( 0 <= t ){
					int bone = boneIds[t];
					//skin
					float w = distanceToBone(t, ray);
					weights[i] = w;
					//transform
					vec3 transformed = ray;
					//Back to object space
					transformed += boneTrans[bone][3].xyz;
					transformed = mat3(boneTrans[bone])*transformed;
					//to rest world space
					transformed = transpose(mat3(boneRestTrans[bone]))*transformed;
					transformed.xyz += boneRestTrans[bone][3].xyz;
					//sum
					restPosition += w*transformed.xyz;
				}
			}
			//color.xyz = vec3(interval);
			color.a = 1.0;
			color *= texture2D(checkTexture, getUV(restPosition));//
*/