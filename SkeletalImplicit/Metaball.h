#ifndef METABALL_H
#define METABALL_H

#include "MetaPrimitive.h"
#include <CGLA/Vec3f.h>
#include <CGLA/Quatf.h>
#include <vector>

#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>

using namespace std;
using namespace CGLA;
class Skeleton;

class Metaball: public MetaPrimitive{
	protected:
		static GLUquadric *_sphere;
	public:
		static const string type;
		virtual const string getType()const{return type;}

		Metaball();
		Metaball(const Vec3f& pos, float radius);
		~Metaball();

		/****GEOMETRY*****/
		Vec3f c;
		Vec3f worldC;
		float r;

		void setWorldPosition(const Vec3f& wp);

		float eval(const Vec3f& p);	
		void renderGuizmo();
		void renderBoundingMesh()const;
		bool rayCast(const MyRay&, RayCastHit& hit)const;

		void _boneLengthChanged();
		void _updateWorldGeometry();

		void write(ostream &stream)const;
		void read(ifstream &ifs);
};



#endif
