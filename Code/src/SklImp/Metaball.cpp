#include "Metaball.h"
#include "Skeleton.h"
#include "Utils/DrawUtils.h"
#include "Utils/RayCastUtils.h"
#include "BVH.h"
#include <fstream>
#include <iostream>

GLUquadric* Metaball::_sphere = NULL;
const string Metaball::type = "MetaBall";

Metaball::Metaball():c(0),r(0.5),worldC(0){
	this->aabb.primitives.insert(this);
	if(!_sphere)
		_sphere=gluNewQuadric();
}
Metaball::Metaball(const Vec3f& pos, float radius):c(pos),r(radius){
	this->aabb.primitives.insert(this);
	if(!_sphere)
		_sphere=gluNewQuadric();
}

Metaball::~Metaball(){
}


float Metaball::eval(const Vec3f& p){
	float r2 = sqr_length(p-worldC);
	float R2 = sqr<float>(r);

	float value = r2 <= R2 ? sqr<float>(1 - r2/R2): 0;		
	return value;
}

void Metaball::renderGuizmo()const{
	assert(this->skeleton != 0);
	glPushMatrix();
		glTranslatef(skeleton->worldPosition[0],skeleton->worldPosition[1],skeleton->worldPosition[2]);
		glMultMatrixf(skeleton->worldOrientation.get_Mat4x4f().get());
		
		glTranslatef(c[0],c[1],c[2]);
		drawCircle(Vec3f(0),r);
		glRotatef(45,1,0,0);
		drawCircle(Vec3f(0),r);
		glRotatef(45,1,0,0);
		drawCircle(Vec3f(0),r);
		

	glPopMatrix();
}


void Metaball::renderBoundingMesh()const{
	glPushMatrix();
		glTranslatef(skeleton->worldPosition[0],skeleton->worldPosition[1],skeleton->worldPosition[2]);
		glMultMatrixf(skeleton->worldOrientation.get_Mat4x4f().get());
		drawFullBox(c+Vec3f(-r),c+Vec3f(r));
	glPopMatrix();
	/*
	glPushMatrix();
		
			glBegin(GL_QUAD_STRIP);
			glVertex3fv( (c+Vec3f(-r,r,-r)).get());
			glVertex3fv((c+Vec3f(-r,r,r)).get());
			glVertex3fv((c+Vec3f(r,r,-r)).get());
			glVertex3fv((c+Vec3f(r,r,r)).get());

			glVertex3fv((c+Vec3f(r,-r,-r)).get());
			glVertex3fv((c+Vec3f(r,-r,r)).get());

			glVertex3fv((c+Vec3f(-r,-r,-r)).get());
			glVertex3fv((c+Vec3f(-r,-r,r)).get());

			glVertex3fv((c+Vec3f(-r,r,-r)).get());
			glVertex3fv((c+Vec3f(-r,r,r)).get());
			glEnd();

			glBegin(GL_QUADS);
			glVertex3fv((c+Vec3f(-r,-r,-r)).get());
			glVertex3fv((c+Vec3f(-r,r,-r)).get());
			glVertex3fv((c+Vec3f(r,r,-r)).get());
			glVertex3fv((c+Vec3f(r,-r,-r)).get());

			glVertex3fv((c+Vec3f(-r,-r,r)).get());
			glVertex3fv((c+Vec3f(r,-r,r)).get());
			glVertex3fv((c+Vec3f(r,r,r)).get());
			glVertex3fv((c+Vec3f(-r,r,r)).get());

			glEnd();
			
	glPopMatrix();
	*/
}

void Metaball::_boneLengthChanged(){
	//p2 = Vec3f(0,0,skeleton->length);
	//_updateWorldGeometry();
}
void Metaball::_updateWorldGeometry(){
	MetaPrimitive::_updateWorldGeometry();
	worldC = skeleton->worldOrientation.inverse().apply( c ) + skeleton->worldPosition;
	this->aabb.bottomCorner = worldC - Vec3f(r);
	this->aabb.topCorner = worldC + Vec3f(r); 
}

void Metaball::setWorldPosition(const Vec3f& wp){
	this->c = this->skeleton->worldOrientation.apply( wp-this->skeleton->worldPosition );
	this->_updateWorldGeometry();
}

bool Metaball::rayCast(const MyRay& ray, RayCastHit& hit)const{
	float lambda = (dot(worldC,ray.direction)-dot(ray.origine,ray.direction))/dot(ray.direction,ray.direction);

	Vec3f projectedCenter = ray.origine + lambda*ray.direction;

	if(sqr_length( projectedCenter - worldC ) <= sqr<float>(r)){
		return true;
	}
	return false;
}

void Metaball::write(ostream &os)const{
	os << "s" <<
		" " << c[0] <<
		" " << c[1] <<
		" " << c[2] <<
		" " << r    << endl;
}

void Metaball::read(ifstream &ifs){
	ifs >> c[0];
	ifs >> c[1];
	ifs >> c[2];
	ifs >> r;
}