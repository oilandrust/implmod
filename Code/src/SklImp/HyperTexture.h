#ifndef HYPERTEXTURE_H
#define HYPERTEXTURE_H

#include "TextureProjector.h"

class HyperTexture : public TextureProjector{

	public:
		HyperTexture();
		~HyperTexture();

		virtual bool rayCast(const MyRay& ray, RayCastHit& hit)const;
		virtual void renderGuizmo();
};

#endif
