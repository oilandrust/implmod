#include "TextureProjector.h"
#include "Utils/DrawUtils.h"
#include "Utils/RayCastUtils.h"

GLUquadric* TextureProjector::_cylinder = NULL;

TextureProjector::TextureProjector()
:texId(0),position(0),radius(0.5f),length(0.5f){
	orientation = orientation.identity_Quatf();
	if(!_cylinder)
		_cylinder=gluNewQuadric();
}

TextureProjector::~TextureProjector()
{
}


bool TextureProjector::rayCast(const MyRay& ray, RayCastHit& hit)const{

	Vec3f boneAxis = this->orientation.apply(Vec3f(0,0,1));
	Vec3f worldP1 = this->position-0.5f*this->length*boneAxis;
	Vec3f worldP2 = this->position+0.5f*this->length*boneAxis;
	
	
	Vec3f delta = worldP1 - ray.origine;
	float lengt = this->length;

	float d1d2 = dot(boneAxis,ray.direction);
	float dd1 = dot(delta,boneAxis);
	float dd2 = dot(delta,ray.direction);

	float s = ( dd1*d1d2 - dd2 ) / ( d1d2*d1d2 - 1 );
	float t = s*d1d2 - dd1;

	if( abs(t) > lengt )
		return false;

	Vec3f point1 = worldP1 + t * boneAxis;
	Vec3f point2 = ray.origine + s * ray.direction;

	float R2 = 0;
	if(t >= 0){
	if(t <= lengt) //Projection of P on the line is in the segment
		R2 = sqr_length(point1 - point2); 
	else
		R2 = sqr_length(worldP2 - point2); 
	}else
		R2 = sqr_length(worldP1 - point2);

	return R2 <= sqr<float>(this->radius);	

}


void TextureProjector::renderGuizmo(){

	glPushMatrix();
	glTranslatef(this->position[0],this->position[1],this->position[2]);
	
	glMultMatrixf(this->orientation.get_Mat4x4f().get());

	glTranslatef(0,0,-0.5f*this->length);

	gluCylinder(_cylinder,this->radius,this->radius,this->length,10,2);
	glPopMatrix();
}
