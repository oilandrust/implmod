#include "RayCastUtils.h"

#include <iostream>
using namespace std;

bool getRaySphereIntersection(const MyRay& ray, const Vec3f& center, float radius, Vec3f& outInter){
	Vec3f worldC = center;
	float lambda = (dot(worldC,ray.direction)-dot(ray.origine,ray.direction))/dot(ray.direction,ray.direction);

	Vec3f projectedCenter = ray.origine + lambda*ray.direction;

	if(sqr_length( projectedCenter - worldC ) <= sqr<float>(radius)){
		float c = sqrtf( sqr<float>(radius) - sqr_length( projectedCenter - worldC ) );
		outInter = projectedCenter - c*ray.direction;
		return true;
	}
	return false;
}