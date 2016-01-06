#ifndef SKELETON_H
#define SKELETON_H

#include <iostream>

#include <CGLA/Vec3f.h>
#include <CGLA/Quatf.h>
#include <vector>

using namespace std;
using namespace CGLA;

class MetaPrimitive;
class GLUquadric;

struct RayCastHit;
struct MyRay;

class Skeleton{
	protected:
		static GLUquadric *_sphere;
		
	public:
		float length;
		Vec3f position;
		Vec3f worldPosition;
		Quatf worldOrientation;
		Quatf orientation;

		Skeleton* parent;
		vector<Skeleton*> children;
		vector<MetaPrimitive*> primitives;
		string name;

		unsigned int id;

	public:
		Skeleton();
		~Skeleton();

		void clear();

		void rotate(const Vec3f& axis, float angle);
		void rotate(const Vec3f& s, const Vec3f& t);
		void setPose(const Quatf& pose);
		void translate(const Vec3f& translation);
		void setLength(float l);
		void setWorldTarget(const Vec3f& wt);
		
		void _updateWorldPosition();
		void _updateWorldOrientation();

		bool rayCast(const MyRay&, RayCastHit& hit)const;

		void addChild(Skeleton* child);
		void removeChild(Skeleton* child);
		void addNode(const Vec3f& position);

		int getChildIndex(Skeleton* child);
		
		void addPrimitive(MetaPrimitive* primitive);

		void render()const;
		void renderWorldSpace(bool guysmo = false)const;


};

void save(Skeleton& skeleton, const string& filename);
void load(Skeleton& skeleton,  std::ifstream& ifs);


#endif