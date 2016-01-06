#include "HyperTexture.h"
#include "Utils/DrawUtils.h"
#include "Utils/RayCastUtils.h"


HyperTexture::HyperTexture(){
	orientation = orientation.identity_Quatf();
}

HyperTexture::~HyperTexture()
{
}


bool HyperTexture::rayCast(const MyRay& ray, RayCastHit& hit)const{

	Vec3f worldCenter = position;

	//the box's axis
	Vec3f slabs[3];
	slabs[0] = this->orientation.apply(Vec3f(1,0,0));
	slabs[1] = this->orientation.apply(Vec3f(0,1,0));
	slabs[2] = this->orientation.apply(Vec3f(0,0,1));

	//the box halfhedges
	float he[3];
	he[0] = this->length;
	he[1] = this->length;
	he[2] = this->length;

	float tmin = - 1000000000;
	float tmax = 10000000000;

	Vec3f p = worldCenter - ray.origine;

	for(int i = 0; i < 3; i++){
		float e = dot(slabs[i],p);
		float f = dot(slabs[i],ray.direction);

		if( fabsf(f) > 0 ){
			float finv = 1 / f;
			float t1 = ( e + he[i] ) * finv;
			float t2 = ( e - he[i] ) * finv;

			if( t1 > t2 ){
				float tmp = t1;
				t1 = t2;
				t2 = tmp;
			}

			if( t1 > tmin )
				tmin = t1;
			if( t2 < tmax )
				tmax = t2;

			if( tmin > tmax )
				return false;
			if( tmax < 0 )
				return false;

		}else if( ( -e - he[i] > 0 ) || ( -e + he[i] < 0 ) )
			return false;
	}

	return true;	

}


void HyperTexture::renderGuizmo(){

	glPushMatrix();
	glTranslatef(this->position[0],this->position[1],this->position[2]);
	glMultMatrixf(this->orientation.get_Mat4x4f().get());

	drawBox( Vec3f(-this->length), Vec3f(this->length) );

	glPopMatrix();
}
