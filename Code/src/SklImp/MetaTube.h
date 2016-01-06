#ifndef METATUBE_H
#define METATUBE_H

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

class MetaTube : public MetaPrimitive{
	protected:
		static GLUquadric *_cylinder;

	public:
		static const string type;
		virtual const string getType()const{return type;}

		MetaTube();
		MetaTube(const Vec3f& a, const Vec3f& b, float ra, float rb);
		~MetaTube();

		/*****GEOMETRY*****/
		Vec3f p1;
		Vec3f p2;
		Vec3f worldP1;
		Vec3f worldP2;
		float r1;
		float r2;

		float eval(const Vec3f& p);	

		void renderGuizmo()const;
		void renderSolidGuizmo()const;
		void renderBoundingMesh()const;
		bool rayCast(const MyRay&, RayCastHit& hit)const;

		void setWorldPosition(const Vec3f& wp);
		void setWorldP1(const Vec3f& wp);
		void setWorldP2(const Vec3f& wp);

		void _boneLengthChanged();
		void _updateWorldGeometry();

		void write(ostream& stream)const;
		void read(ifstream &ifs);

};


#endif