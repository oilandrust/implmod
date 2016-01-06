#include "IEditor.h"
#include "RayCastUtils.h"

int IEditor::modeTextId = 0;

MyRay IEditor::getRayFromScreePos(int xScreen, int yScreen){
	float fovy = 53;
	float aspect = (float)(this->WINX/this->WINY);
	float Hw = 2 * _near * tanf(M_PI*fovy/180);
	float Ww = aspect * Hw;
	
	float xWorld = (float)xScreen / this->WINX - 0.5;
	float yWorld = (1 - (float)yScreen / this->WINY ) - 0.5;

	//coordinate of the mouse in the view plane in the object coordinate of the camera
	xWorld *= -Ww/2.5;
	yWorld *= Hw/2.5;
	
	
	//direction in object space
	Vec3f dir = normalize(Vec3f(xWorld,yWorld,_near));

	//Get Camera frame
	Vec3f eye;
	Vec3f center;
	Vec3f up;
	this->view_ctrl->get_view_param(eye,center,up);
	Vec3f forward = normalize(center-eye);
	Vec3f right = cross(up,forward);

	//rotate to have direction in world space
	Mat3x3f rotMat = Mat3x3f(right,up,forward);
	Mat3x3f invRot;
	transpose(rotMat,invRot);
	dir =  invRot * dir;

	MyRay ray = {dir,eye};
	
	return ray;
}

Vec3f IEditor::getPosInViewPlane(int xScreen, int yScreen, Vec3f pointInPlane){
	MyRay ray = this->getRayFromScreePos(xScreen,yScreen);
	Vec3f eye;
	Vec3f center;
	Vec3f up;
	this->view_ctrl->get_view_param(eye,center,up);
	Vec3f norm = normalize(center-eye);

	Vec3f p = pointInPlane;
	Vec3f o = ray.origine;

	float s = -dot(norm,o-p)/dot(norm,ray.direction);
	return o + s*ray.direction;
}

