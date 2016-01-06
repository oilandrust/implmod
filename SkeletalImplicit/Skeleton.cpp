#include "Skeleton.h"
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "RayCastUtils.h"
#include "DrawUtils.h"
#include "MetaPrimitive.h"
#include "Metaball.h"
#include "MetaTube.h"
#include <fstream>
#include <iostream>


GLUquadric* Skeleton::_sphere = NULL;

Skeleton::Skeleton():
position(0),
worldPosition(0),
length(1),
parent(NULL),
orientation(orientation.identity_Quatf()),
worldOrientation(orientation.identity_Quatf()){
	if(!_sphere)
		_sphere=gluNewQuadric();
}
Skeleton::~Skeleton(){
	if(this->parent)
		this->parent->removeChild(this);
	for(unsigned int i = 0; i < this->children.size(); i++ )
		delete this->children[i];
	for(unsigned int i = 0; i < this->primitives.size(); i++)
		delete this->primitives[i];
}
void Skeleton::clear(){
	for(unsigned int i = 0; i < this->children.size(); i++ )
		delete this->children[i];
	this->children.clear();
	this->length = 0;
	this->position = Vec3f(0);
	this->orientation = this->orientation.identity_Quatf();
	for(unsigned int i = 0; i < this->primitives.size(); i++ )
		delete this->primitives[i];
	this->primitives.clear();
	this->_updateWorldPosition();
	this->_updateWorldOrientation();
}
void Skeleton::addChild(Skeleton* child){
	this->children.push_back(child);
	child->parent = this;
	child->_updateWorldPosition();
	child->_updateWorldOrientation();
}
void Skeleton::removeChild(Skeleton* child){
	vector<Skeleton*>::iterator it = this->children.begin();
	for(; it!=this->children.end(); ++it ){
		if(*it=child)
			break;
	}
	this->children.erase(it);
}

void Skeleton::addPrimitive(MetaPrimitive* primitive){
	this->primitives.push_back(primitive);
	primitive->skeleton = this;
	primitive->_updateWorldGeometry();
	primitive->_boneLengthChanged();
}

void Skeleton::addNode(const Vec3f& position){
	Skeleton* child = new Skeleton();
	child->position = position;
	addChild(child);
}

void Skeleton::rotate(const Vec3f& axis, float angle){
	orientation.make_rot(angle,axis);
	_updateWorldOrientation();
	for(unsigned int i = 0; i < children.size(); i++ ){
		children[i]->_updateWorldPosition();
		children[i]->_updateWorldOrientation();
	}
	for(unsigned int i = 0; i < primitives.size(); i++ ){
		primitives[i]->_updateWorldGeometry();
	}
}

void Skeleton::setPose(const Quatf& pose){
	orientation = pose;
	_updateWorldOrientation();
	for(unsigned int i = 0; i < children.size(); i++ ){
		children[i]->_updateWorldPosition();
		children[i]->_updateWorldOrientation();
	}
	for(unsigned int i = 0; i < primitives.size(); i++ ){
		primitives[i]->_updateWorldGeometry();
	}
}

void Skeleton::setWorldTarget(const Vec3f& wt){
	Vec3f target;
	if(this->parent)
		target = this->parent->worldOrientation.apply( wt-this->worldPosition );
	else
		target = wt-this->worldPosition;

	this->length = CGLA::length(target);
	this->orientation.make_rot(normalize(target),Vec3f(0,0,1));

	_updateWorldOrientation();
	for(unsigned int i = 0; i < primitives.size(); i++ ){
		primitives[i]->_boneLengthChanged();
	}
}

int Skeleton::getChildIndex(Skeleton* child){
	for(unsigned int i = 0; i < this->children.size(); i++){
		if(this->children[i] == child)
			return (int)i;
	}
	return -1;
}
void Skeleton::rotate(const Vec3f& s, const Vec3f& t){

	Vec3f oldAxis;
	//orientation.get_rot(oldAngle,oldAxis);
	oldAxis = orientation.inverse().apply(Vec3f(0,0,1));
	Vec3f target = normalize(t);//orientation.inverse().apply(-normalize(t));
	Vec3f newUp = normalize(t);
	Vec3f fwd(1,0,0);
	Vec3f right = cross(newUp,fwd);
	Mat3x3f rot(fwd,right,newUp);
	orientation.make_rot(rot);
	_updateWorldOrientation();
	_updateWorldPosition();
	for(unsigned int i = 0; i < children.size(); i++ ){
		children[i]->_updateWorldPosition();
		children[i]->_updateWorldOrientation();
	}
	for(unsigned int i = 0; i < primitives.size(); i++ ){
		primitives[i]->_updateWorldGeometry();
	}
}
void Skeleton::translate(const Vec3f& translation){
	position += translation;
	_updateWorldPosition();
	for(unsigned int i = 0; i < primitives.size(); i++ ){
		primitives[i]->_updateWorldGeometry();
	}
}
void Skeleton::setLength(float l){
	length = l;
	for(unsigned int i = 0; i < children.size(); i++ ){
		children[i]->_updateWorldPosition();
	}
	for(unsigned int i = 0; i < primitives.size(); i++ ){
		primitives[i]->_boneLengthChanged();
	}
}

void Skeleton::_updateWorldPosition(){
	//cout<<"wp"<<endl;
	if(parent)
		worldPosition = parent->worldOrientation.inverse().apply( Vec3f(0,0,parent->length) + position ) + parent->worldPosition;
	else
		worldPosition = position;
	for(unsigned int i = 0; i < children.size(); i++ )
		children[i]->_updateWorldPosition();
	for(unsigned int i = 0; i < primitives.size(); i++ )
		primitives[i]->_updateWorldGeometry();
	//cout<<"ewp"<<endl;
}
void Skeleton::_updateWorldOrientation(){
	//cout<<"wo"<<endl;
	if(parent)
		worldOrientation = orientation*parent->worldOrientation;
	else
		worldOrientation = orientation;
	for(unsigned int i = 0; i < children.size(); i++ ){
		children[i]->_updateWorldOrientation();
		children[i]->_updateWorldPosition();
	}
	for(unsigned int i = 0; i < primitives.size(); i++ )
		primitives[i]->_updateWorldGeometry();
	//cout<<"ewo"<<endl;
	
}


bool Skeleton::rayCast(const MyRay& ray, RayCastHit& hit)const{
	//float lambda = dot(position-ray.origine,ray.direction);
	float lambda = (dot(worldPosition,ray.direction)-dot(ray.origine,ray.direction))/dot(ray.direction,ray.direction);

	Vec3f projectedCenter = ray.origine + lambda*ray.direction;

	if(sqr_length( projectedCenter - worldPosition ) <= sqr<float>(0.1f)){
		return true;
	}else {//check MetaTube
	
		Vec3f delta = worldPosition - ray.origine;
		Vec3f boneAxis = worldOrientation.inverse().apply(Vec3f(0,0,1));

		float d1d2 = dot(boneAxis,ray.direction);
		float dd1 = dot(delta,boneAxis);
		float dd2 = dot(delta,ray.direction);

		float s = ( dd1*d1d2 - dd2 ) / ( d1d2*d1d2 - 1 );
		float t = s*d1d2 - dd1;

		if( abs(t) > length )
			return false;

		Vec3f point1 = worldPosition + t * boneAxis;
		Vec3f point2 = ray.origine + s * ray.direction;

		float R2 = 0;
		if(t >= 0){
		if(t <= length) //Projection of P on the line is in the segment
			R2 = sqr_length(point1 - point2); 
		else
			R2 = sqr_length(worldPosition + length*boneAxis - point2); 
		}else
			R2 = sqr_length(worldPosition - point2);

		return R2 <= sqr<float>(0.1f);	
	}

	return false;
}

void Skeleton::render()const{

	glPushMatrix();
	/*
	glPushMatrix();
	glTranslatef(position[0],position[1],position[2]);
	glMultMatrixf(orientation.get_Mat4x4f().get());
	*/

	if(parent){
		gluCylinder( _sphere,0.015, 0.01, length, 10, 10);
		gluSphere( _sphere , 0.025 , 20 , 20 );
	}
	drawAxis(0.3);

	glTranslatef(0,0,length);
	for(unsigned int i = 0; i < children.size(); i++ ){
		children[i]->render();
	}

	glPopMatrix();
}

void Skeleton::renderWorldSpace(bool guyzmo)const{

	glPushMatrix();
	glTranslatef(worldPosition[0],worldPosition[1],worldPosition[2]);
	glMultMatrixf(worldOrientation.get_Mat4x4f().get());
	
	if(abs(length) > 0)
		gluCylinder( _sphere,0.015, 0.01, length, 10, 10);
	gluSphere( _sphere , 0.025 , 10 , 10 );

	if(guyzmo)
	drawAxis(0.3f);
	

	glPopMatrix();
}

void save(Skeleton& skeleton, const string& filemame){
	Skeleton* node;
	vector<Skeleton*> openList;
	openList.push_back(&skeleton);

	std::ofstream ofs(filemame.c_str(),ios::out);
	
	while( !openList.empty() ){	
		node = openList.back();
		openList.pop_back();
		
		ofs    << node->name <<
			" "<< node->length <<
			" "<< node->position[0] <<
			" "<< node->position[1]	<<
			" "<< node->position[2]	<<
			" "<< node->orientation.qv[0] <<
			" "<< node->orientation.qv[1] <<
			" "<< node->orientation.qv[2] <<
			" "<< node->orientation.qw <<
			" "<< node->primitives.size() <<
			" "<< node->children.size() <<endl; 
		
		for(unsigned int i = 0; i < node->primitives.size(); i++){
			ofs << *(node->primitives[i]);				
		}
		for(unsigned int i = 0; i < node->children.size(); i++){
			openList.push_back(node->children[i]);				
		}
	}

	ofs.close();

}
void load(Skeleton& skeleton, std::ifstream& ifs){
	int nbOfChilds = 0;
	int nbOfPrimitives = 0;

	ifs >> skeleton.name
		>> skeleton.length
		>> skeleton.position[0]
		>> skeleton.position[1]
		>> skeleton.position[2]
		>> skeleton.orientation.qv[0]
		>> skeleton.orientation.qv[1]
		>> skeleton.orientation.qv[2]
		>> skeleton.orientation.qw
		>> nbOfPrimitives
		>> nbOfChilds;

	for(int i = 0; i < nbOfPrimitives; i++){
		char type;
		ifs >> type;
		MetaPrimitive* prim = NULL;
		switch(type){
			case 's':
				prim = new Metaball();
				ifs >> *prim;
				break;
			case 't':
				prim = new MetaTube();
				ifs >> *prim;
				break;
		}		
		if(prim)
			skeleton.addPrimitive(prim);
	}

	skeleton._updateWorldPosition();
	skeleton._updateWorldOrientation();

	for( int i = 0; i < nbOfChilds; i++){
		Skeleton* child = new Skeleton();
		load(*child,ifs);
		skeleton.addChild(child);
	}
}