#ifndef RAYCASTUTILS_H
#define RAYCASTUTILS_H

#include <CGLA/Vec3f.h>

using namespace CGLA;

class Skeleton;

struct RayCastHit{
	Skeleton* bone;
};

struct MyRay{
	Vec3f direction;
	Vec3f origine;
};

bool getRaySphereIntersection(const MyRay& ray, const Vec3f& center, float radius, Vec3f& outInter);

#endif