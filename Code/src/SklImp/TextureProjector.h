#ifndef TEXTUREPROJECTOR_H
#define TEXTUREPROJECTOR_H

#include <CGLA/Vec3f.h>
#include <CGLA/Quatf.h>

#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>

struct RayCastHit;
struct MyRay;

using namespace CGLA;

class TextureProjector{
	protected:
		static GLUquadric *_cylinder;

	public:
		GLuint texId;
		Vec3f position;
		Quatf orientation;
		float radius;
		float length;

		TextureProjector();
		virtual ~TextureProjector();

		virtual bool rayCast(const MyRay& ray, RayCastHit& hit)const;
		virtual void renderGuizmo();
};

#endif