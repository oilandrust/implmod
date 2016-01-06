#ifndef SKELETALIMPLICITMODEL_H
#define SKELETALIMPLICITMODEL_H

#include <vector>

using namespace std;

class Skeleton;
class TextureProjector;

class SkeletalImplicitModel{
	public:
		Skeleton* rootBone;
		vector<TextureProjector*> texProjectors;
	public:

		SkeletalImplicitModel();
		~SkeletalImplicitModel();
};

#endif
