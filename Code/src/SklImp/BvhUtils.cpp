#include "BvhUtils.h"
#include "MetaBall.h"
#include "MetaPrimitive.h"

Vec2f v3DTov2D(Vec3f v3d,int axis){
	Vec2f v2d;
	switch(axis){ //Layout : splix (y,z), spliy (x,z), spliz (x,y)
		case 0:
			v2d = Vec2f(v3d[1],v3d[2]);
			break;
		case 1:
			v2d = Vec2f(v3d[0],v3d[2]);
			break;
		case 2:
			v2d = Vec2f(v3d[0],v3d[1]);
			break;
	}
	return v2d;
}

Vec3f v2DTov3D(Vec2f v2d, int axis){
	Vec3f v3d;
	switch(axis){
		case 0:
			v3d[0] = 0;
			v3d[1] = v2d[0];
			v3d[2] = v2d[1];
			break;
		case 1:
			v3d[0] = v2d[0];
			v3d[1] = 0;
			v3d[2] = v2d[1];
			break;
		case 2:
			v3d[0] = v2d[0];
			v3d[1] = v2d[1];
			v3d[2] = 0;			
			break;
	}
	return v3d;
}

Section3D sec3d(const Section& sec, int axis){
	Section3D section;
	Vec3f halfDiag = v2DTov3D(Vec2f(sec.r1,sec.r2),axis);
	section.minv = sec.center - halfDiag;
	section.maxv = sec.center + halfDiag;
	return section;
}

AABBLite aabbFromSec3ds(const Section3D& sec1, const Section3D& sec2){
	AABBLite aabb;
	aabb.bottomCorner[0] = min(sec1.minv[0],sec2.minv[0]);
	aabb.bottomCorner[1] = min(sec1.minv[1],sec2.minv[1]);
	aabb.bottomCorner[2] = min(sec1.minv[2],sec2.minv[2]);

	aabb.topCorner[0] = max(sec1.maxv[0],sec2.maxv[0]);
	aabb.topCorner[1] = max(sec1.maxv[1],sec2.maxv[1]);
	aabb.topCorner[2] = max(sec1.maxv[2],sec2.maxv[2]);

	return aabb;
}

void Intersection(AABBLite& aabb,const AABBLite& newAABB){
	aabb.bottomCorner[0] = max(aabb.bottomCorner[0],newAABB.bottomCorner[0]);
	aabb.bottomCorner[1] = max(aabb.bottomCorner[1],newAABB.bottomCorner[1]);
	aabb.bottomCorner[2] = max(aabb.bottomCorner[2],newAABB.bottomCorner[2]);

	aabb.topCorner[0] = min(aabb.topCorner[0],newAABB.topCorner[0]);
	aabb.topCorner[1] = min(aabb.topCorner[1],newAABB.topCorner[1]);
	aabb.topCorner[2] = min(aabb.topCorner[2],newAABB.topCorner[2]);
}

void Intersection(AABBLite& aabb,const AABB& newAABB){
	aabb.bottomCorner[0] = max(aabb.bottomCorner[0],newAABB.bottomCorner[0]);
	aabb.bottomCorner[1] = max(aabb.bottomCorner[1],newAABB.bottomCorner[1]);
	aabb.bottomCorner[2] = max(aabb.bottomCorner[2],newAABB.bottomCorner[2]);

	aabb.topCorner[0] = min(aabb.topCorner[0],newAABB.topCorner[0]);
	aabb.topCorner[1] = min(aabb.topCorner[1],newAABB.topCorner[1]);
	aabb.topCorner[2] = min(aabb.topCorner[2],newAABB.topCorner[2]);
}

AABBLite Restriction(MetaPrimitive* prim, const AABBLite& ab){
	AABBLite axisAbs[3];
	if(prim->getType() == "MetaTube"){
		MetaTube* tube = (MetaTube*)prim;
		Section sections[3][2];

		bool inter[3][2];
		
		for(int axis = 0; axis < 3; axis++){
			inter[axis][0] = getMetaTubeSectionBounds(tube, ab.bottomCorner[axis], axis, sections[axis][0]);
			inter[axis][1] = getMetaTubeSectionBounds(tube, ab.topCorner[axis], axis, sections[axis][1]);
			
			axisAbs[axis] = aabbFromSec3ds(sec3d(sections[axis][0],axis),sec3d(sections[axis][1],axis));
		}
		Intersection(axisAbs[0],axisAbs[1]);
		Intersection(axisAbs[0],axisAbs[2]);
		return axisAbs[0];
	
	}else{
		AABBLite abl;
		abl.bottomCorner = prim->aabb.bottomCorner;
		abl.topCorner = prim->aabb.topCorner;
		return abl;
	}
	
}


void Union(AABBLite& aabb,const AABBLite& newAABB){
	if(newAABB.bottomCorner[0] < aabb.bottomCorner[0])
		aabb.bottomCorner[0] = newAABB.bottomCorner[0];
	if(newAABB.topCorner[0] > aabb.topCorner[0])
		aabb.topCorner[0] = newAABB.topCorner[0];
	if(newAABB.bottomCorner[1] < aabb.bottomCorner[1])
		aabb.bottomCorner[1] = newAABB.bottomCorner[1];
	if(newAABB.topCorner[1] > aabb.topCorner[1])
		aabb.topCorner[1] = newAABB.topCorner[1];
	if(newAABB.bottomCorner[2] < aabb.bottomCorner[2])
		aabb.bottomCorner[2] = newAABB.bottomCorner[2];
	if(newAABB.topCorner[2] > aabb.topCorner[2])
		aabb.topCorner[2] = newAABB.topCorner[2];
}

AABBLite aabbOfaabbs(const vector<AABBLite>& aabbs){
	AABBLite result;
	result.topCorner = Vec3f(-BIG);
	result.bottomCorner = Vec3f(BIG);

	for(size_t i = 0 ; i < aabbs.size(); i++){
		Union(result,aabbs[i]);
	}

	return result;
}


void Union(AABBLite& aabb,const AABB& newAABB){
	if(newAABB.bottomCorner[0] < aabb.bottomCorner[0])
		aabb.bottomCorner[0] = newAABB.bottomCorner[0];
	if(newAABB.topCorner[0] > aabb.topCorner[0])
		aabb.topCorner[0] = newAABB.topCorner[0];
	if(newAABB.bottomCorner[1] < aabb.bottomCorner[1])
		aabb.bottomCorner[1] = newAABB.bottomCorner[1];
	if(newAABB.topCorner[1] > aabb.topCorner[1])
		aabb.topCorner[1] = newAABB.topCorner[1];
	if(newAABB.bottomCorner[2] < aabb.bottomCorner[2])
		aabb.bottomCorner[2] = newAABB.bottomCorner[2];
	if(newAABB.topCorner[2] > aabb.topCorner[2])
		aabb.topCorner[2] = newAABB.topCorner[2];
}

bool getMetaTubeSectionBounds(MetaTube* tube, float pos, int axis, Section& outSection){

	float R1 = tube->r1;
	float R2 = tube->r2;

	Vec3f p1;
	Vec3f p2;
	if(tube->worldP1[axis] <= tube->worldP2[axis]){
		p1 = tube->worldP1;
		p2 = tube->worldP2;
	}else{
		p1 = tube->worldP2;
		p2 = tube->worldP1;
	}

	//normal of the plane
	Vec3f axisV(axis==0?1:0,axis==1?1:0,axis==2?1:0);
	//normal of the tube
	Vec3f t = normalize(p2-p1);
	//One point on the plane
	Vec3f p = pos*axisV;

	
	//Case plane parallel to tube axis 
	if( dot(t, axisV) == 0 ){
		float dist = pos-p1[axis];
		if( abs(dist) < R1 ){
			float r = sqrtf( sqr<float>(R1) - sqr<float>(dist) );
			Vec3f minv = v2DTov3D(Vec2f(-r,-r),axis)+p1;
			minv[axis] = pos;
			Vec3f maxv = v2DTov3D(Vec2f(r,r),axis)+p2;
			maxv[axis] = pos;

			outSection.center = 0.5f*(minv+maxv);
			Vec2f halfDiag = v3DTov2D(maxv-outSection.center,axis);
			outSection.r1 = halfDiag[0];
			outSection.r2 = halfDiag[1];
			return true;
		}else
			return false;
	}

	float cosA = abs(t[axis]);
	float sinA = length(cross(t,axisV));
	float greatAxis = R1/cosA;
	float tubesphere = R1*sinA;

	Vec2f tProj = v3DTov2D(t,axis);
	float tProjLen = length(tProj);
	float cosB = abs(tProj[0]/tProjLen);
	float sinB = abs(tProj[1]/tProjLen);
	//float tanB = tProj[1]/tProj[0];

	if( pos < p1[axis] - R1 ){ //Plane does not intersect the tube, project the sphere on the plane
		outSection.center = p1;
		outSection.center[axis] = pos;
		outSection.r1 = outSection.r2 = tube->r1;
		return true;
	}/*else if( pos <= p1[axis] - tubesphere ){
		//Sphere section
		outSection.center = p1;
		outSection.center[axis] = pos;
		float secRadius = sqrtf(sqr<float>(R1)-sqr<float>(pos-p1[axis]));
		outSection.r1 = outSection.r2 = secRadius;
		return true;
	}*//*else if( pos <= p1[axis] + tubesphere ){
		//Sphere/Tube
		outSection.center = p1;
		outSection.center[axis] = pos;

		outSection.r1 = 0.5f*(max(greatAxis*cosB,R1)+R1);
		outSection.r2 = 0.5f*(max(greatAxis*sinB,R1)+R1);
		
		Vec2f offset = Vec2f(outSection.r1,outSection.r1) - Vec2f(R1,R1);
		switch (axis){
			case 0:
				outSection.center[1] += offset[0];
				outSection.center[2] += offset[1];
				break;
			case 1:
				outSection.center[0] += offset[0];
				outSection.center[2] += offset[1];
				break;
			case 2:
				outSection.center[0] += offset[0];
				outSection.center[1] += offset[1];
				break;
		}

		return true;
	}*/else if( pos <= p2[axis] - tubesphere ){
		//Tube
		outSection.center = p1 + (dot(axisV,p-p1)/dot(axisV,t))*t;
		outSection.r1 = max(greatAxis*cosB,R1);
		outSection.r2 = max(greatAxis*sinB,R1);
		return true;
	}/*else if( pos <= p2[axis] + tubesphere ){
		//Sphere/Tube
		return true;
	}/*else if( pos <= p2[axis] + R2 ){
		//Sphere
		outSection.center = p2;
		outSection.center[axis] = pos;
		float secRadius = sqrtf(sqr<float>(R2)-sqr<float>(pos-p2[axis]));
		outSection.r1 = outSection.r2 = secRadius;
		return true;
	}*/else{
		outSection.center = p2;
		outSection.center[axis] = pos;
		outSection.r1 = outSection.r2 = tube->r2;
	}

	return false;

}