#ifndef BVHUTILS_H
#define BVHUTILS_H

#include <CGLA/Vec3f.h>
#include <CGLA/Vec2f.h>

#include "Metatube.h"
#include "BVH.h"

using namespace CGLA;

struct Section{
	Vec3f center;
	float r1;
	float r2;
};

struct Section3D{
	Vec3f minv;
	Vec3f maxv;
};

struct Plan{
	int axis;
	float position;
};


Vec2f v3DTov2D(Vec3f v3d,int axis);
Vec3f v2DTov3D(Vec2f v2d, int axis);

Section3D sec3d(const Section& sec,int axis);
AABBLite aabbFromSec3ds(const Section3D& sec1, const Section3D& sec2);

void Union(AABBLite& aabb,const AABBLite& newAABB);
void Union(AABBLite& aabb,const AABB& newAABB);

void Intersection(AABBLite& aabb,const AABBLite& newAABB);
void Intersection(AABBLite& aabb,const AABB& newAABB);

AABBLite Restriction(MetaPrimitive* prim, const AABBLite& ab);

bool getMetaTubeSectionBounds(MetaTube* tube, float pos, int axis, Section& outSection);

#endif