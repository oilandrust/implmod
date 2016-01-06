#ifndef SKELETALIMPLICIT_H
#define SKELETALIMPLICIT_H

#include <CGLA/Vec3f.h>
#include <Geometry/Polygonizer.h>
#include <vector>


using namespace CGLA;
using namespace std;

class MetaPrimitive;
class Skeleton;


//Extracts the implicit function from a skeleton
class SkeletalImplicit:public Geometry::ImplicitFunction {
	private:
		unsigned int nextSId;
		unsigned int nextTId;

	public:
		vector< MetaPrimitive* > primitives;
		//USED TO PASS MetaTubeS TO THE SHADER
		vector<Vec3f> spheres;
		vector<float> sRadius;
		vector<Vec3f> p1s;
		vector<Vec3f> p2s;
		vector<float> radiuses;
		vector<float> radiuses2;
		float T;

		SkeletalImplicit();
		SkeletalImplicit(Skeleton* skeleton, float threshold);

		void readSkeleton(Skeleton* skeleton);

		void update();

		void addBone(Skeleton* bone);
		void addPrimitive(MetaPrimitive* prim);
		void removePrimitive(MetaPrimitive* prim);

		float eval(float x, float y, float z);
		void clear();
};

#endif