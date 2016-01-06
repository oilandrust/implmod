#ifndef METAPRIMITIVE_H
#define METAPRIMITIVE_H

#include <CGLA/Vec3f.h>
#include <CGLA/Quatf.h>
#include <Geometry/polygonizer.h>
#include "BVH.h"

using namespace std;
using namespace CGLA;

struct RayCastHit;
struct MyRay;
class Skeleton;

class MetaPrimitive: public Geometry::ImplicitFunction{
	public:
		static const string type;
		virtual const string getType()const{return type;}

		AABB aabb;

		bool needsUpdate;
		unsigned int id;

		MetaPrimitive();
		virtual ~MetaPrimitive();

		virtual float eval(const Vec3f& p){return 0;}
		virtual void _updateWorldGeometry(){ needsUpdate = true; }
		virtual void renderGuizmo(){}
		virtual void renderBoundingMesh()const{}
		virtual void _boneLengthChanged(){ needsUpdate = true; }
		inline float eval(float x, float y, float z){return eval(Vec3f(x,y,z));}
		virtual bool rayCast(const MyRay&, RayCastHit& hit)const{return false;}

		Skeleton* skeleton;

		friend ostream &operator<<(ostream &stream, const MetaPrimitive& ob);
		friend ifstream &operator>>(ifstream &stream, MetaPrimitive& ob);

		virtual void write(ostream &stream)const{};
		virtual void read(ifstream &ifs){};

};


		

#endif
