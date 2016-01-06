#include "MetaTube.h"
#include "Skeleton.h"
#include "DrawUtils.h"
#include "RayCastUtils.h"
#include "BVH.h"
#include <fstream>
#include <iostream>


GLUquadric* MetaTube::_cylinder = NULL;

const string MetaTube::type = "MetaTube";


MetaTube::MetaTube():p1(0),p2(0,0,1),r1(0.3f),r2(0.3f){
	this->aabb.primitives.insert(this);
	if(!_cylinder)
		_cylinder=gluNewQuadric();
}

MetaTube::MetaTube(const Vec3f& a, const Vec3f& b, float ra, float rb)
:p1(a),p2(b),r1(ra),r2(rb){
	this->aabb.primitives.insert(this);
	if(!_cylinder)
		_cylinder=gluNewQuadric();
}

MetaTube::~MetaTube(){
}

float MetaTube::eval(const Vec3f& p){
	float segLengh = length(worldP1 - worldP2);
	Vec3f u = (worldP2- worldP1)/segLengh;

	Vec3f ap = p - worldP1;
	float Lambdap = dot( ap , u );

	float R2 = 0;

	if(Lambdap >= 0){
		if(Lambdap <= segLengh) //Projection of P on the line is in the segment
			R2 = sqr_length(ap - Lambdap*u); 
		else
			R2 = sqr_length(p-worldP2);
	}else
		R2 = sqr_length(p - worldP1);

	float value = R2 <= sqr<float>(r1) ? sqr<float>(1 - R2/sqr<float>(r1)): 0;	
	return value;
}

void MetaTube::renderGuizmo(){
	glPushMatrix();
		glTranslatef(skeleton->worldPosition[0],skeleton->worldPosition[1],skeleton->worldPosition[2]);
		glMultMatrixf(skeleton->worldOrientation.get_Mat4x4f().get());
		
		drawCircle(p1,r1);
		drawCircle(p2,r2);
		

	glPopMatrix();
	/*
		    glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glPushMatrix();
	glTranslatef(this->worldP1[0],this->worldP1[1],this->worldP1[2]);
	gluSphere( _cylinder , this->r1 , 10 , 10 );
	glPopMatrix();

	glPushMatrix();
	glTranslatef(this->worldP2[0],this->worldP2[1],this->worldP2[2]);
	gluSphere( _cylinder , this->r1 , 10 , 10 );
	glPopMatrix();

	glPushMatrix();
	glTranslatef(skeleton->worldPosition[0],skeleton->worldPosition[1],skeleton->worldPosition[2]);
		glMultMatrixf(skeleton->worldOrientation.get_Mat4x4f().get());
		
	
	gluCylinder(_cylinder,this->r1,this->r1,length(worldP2-worldP1),10,10);
	glPopMatrix();

			glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		*/

}

void MetaTube::renderBoundingMesh()const{

	glPushMatrix();
		glTranslatef(skeleton->worldPosition[0],skeleton->worldPosition[1],skeleton->worldPosition[2]);
		glMultMatrixf(skeleton->worldOrientation.get_Mat4x4f().get());

		float height = length(p2-p1);


			glBegin(GL_QUAD_STRIP);
			glVertex3f(-r1,r1,-r1);
			glVertex3f(-r1,r1,height+r1);
			glVertex3f(r1,r1,-r1);
			glVertex3f(r1,r1,height+r1);

			glVertex3f(r1,-r1,-r1);
			glVertex3f(r1,-r1,height+r1);

			glVertex3f(-r1,-r1,-r1);
			glVertex3f(-r1,-r1,height+r1);

			glVertex3f(-r1,r1,-r1);
			glVertex3f(-r1,r1,height+r1);
			glEnd();

			glBegin(GL_QUADS);
			glVertex3f(-r1,-r1,-r1);
			glVertex3f(-r1,r1,-r1);
			glVertex3f(r1,r1,-r1);
			glVertex3f(r1,-r1,-r1);

			glVertex3f(-r1,-r1,height+r1);
			glVertex3f(r1,-r1,height+r1);
			glVertex3f(r1,r1,height+r1);
			glVertex3f(-r1,r1,height+r1);

			glEnd();
	
		//gluCylinder( _cylinder,r1, r2, length(p2-p1), 10, 10);

	glPopMatrix();
}

void MetaTube::_boneLengthChanged(){
	p2 = Vec3f(0,0,skeleton->length);
	MetaPrimitive::_boneLengthChanged();
	_updateWorldGeometry();
}
void MetaTube::_updateWorldGeometry(){
	MetaPrimitive::_updateWorldGeometry();
	worldP1 = skeleton->worldOrientation.inverse().apply( p1 ) + skeleton->worldPosition;
	worldP2 = skeleton->worldOrientation.inverse().apply( p2 ) + skeleton->worldPosition;

	float minx, miny, minz, maxx, maxy, maxz;
	minx = min(worldP1[0],worldP2[0]);
	miny = min(worldP1[1],worldP2[1]);
	minz = min(worldP1[2],worldP2[2]);
	maxx = max(worldP1[0],worldP2[0]);
	maxy = max(worldP1[1],worldP2[1]);
	maxz = max(worldP1[2],worldP2[2]);

	this->aabb.bottomCorner = Vec3f(minx,miny,minz)-Vec3f(r1);
	this->aabb.topCorner = Vec3f(maxx,maxy,maxz)+Vec3f(r1);

}

bool MetaTube::rayCast(const MyRay& ray, RayCastHit& hit)const{
	Vec3f delta = worldP1 - ray.origine;
	
	Vec3f boneAxis = normalize(worldP2-worldP1);
	float lengt = length(worldP2-worldP1);

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

	return R2 <= sqr<float>(r1);	

}

void MetaTube::write(ostream& os)const{
	os << "t" <<
		" " << p1[0] <<
		" " << p1[1] <<
		" " << p1[2] <<
		" " << p2[0] <<
		" " << p2[1] <<
		" " << p2[2] <<
		" " << r1    <<
		" " << r2    << endl;
}
void MetaTube::read(ifstream &ifs){
	ifs >> p1[0];
	ifs >> p1[1];
	ifs >> p1[2];
	ifs >> p2[0];
	ifs >> p2[1];
	ifs >> p2[2];
	ifs >> r1;
	ifs >> r2;
}
