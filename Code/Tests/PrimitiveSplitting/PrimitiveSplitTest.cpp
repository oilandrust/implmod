/**********************************************************************/
/**********************************************************************
Test Application to test splitting of tube primitives.




/**********************************************************************/
/**********************************************************************/

#include <stdlib.h>
#include <crtdbg.h>

#include <GL/glew.h>
#include <GLGraphics/gel_glut.h>

#include <iostream>
#include <fstream>

#include <CGLA/Vec2d.h>
#include <CGLA/Vec3d.h>
#include <CGLA/Mat3x3d.h>
#include <CGLA/Mat3x3f.h>
#include <CGLA/Mat2x2d.h>
#include <CGLA/Mat2x3d.h>
#include <Util/Timer.h>

#include <GLGraphics/GLViewController.h>
#include <GLGraphics/draw.h>
#include <GLGraphics/glsl_shader.h>
#include <Util/ArgExtracter.h>

#include <HMesh/VertexCirculator.h>
#include <HMesh/FaceCirculator.h>
#include <HMesh/x3d_save.h>
#include <HMesh/x3d_load.h>
#include <HMesh/build_manifold.h>
#include <HMesh/triangulate.h>
#include <HMesh/mesh_optimization.h>
#include <HMesh/caps_and_needles.h>
#include <HMesh/volume_polygonize.h>

#include <Geometry/RGrid.h>
#include <Geometry/Polygonizer.h>

#include <SOIL.h>

#include <SklImp/Skeleton.h>
#include <SklImp/MetaPrimitive.h>
#include <SklImp/Metaball.h>
#include <SklImp/MetaTube.h>
#include <SklImp/BVH.h>
#include <SklImp/BvhUtils.h>
#include <SklImp/SkeletalImplicit.h>
#include <SklImp/Animation.h>
#include <SklImp/TextureProjector.h>

#include <Utils/RayCastUtils.h>
#include <Utils/DrawUtils.h>

#include <UI/Input.h>



#include <sstream>
#include <queue>
#include <math.h>



using namespace std;
using namespace HMesh;
using namespace Geometry;
using namespace GLGraphics;
using namespace Util;
using namespace CGLA;

//#define USEBVH



int FREQUENCY = 1000; //For FPS sampling


enum EditorMode{
	VIEW,
	BONE,
	IMPLICIT,
	TEXTURING,
	ANIMATION,
	MAXMODE
};


namespace
{
	int MAXAABBCOUNT = 50;
	//VIEW PARAMETERS
	GLViewController *view_ctrl;       // This is the class used to help us navigate with the mouse (virtual trackball)
	float _near;
	int WINX = 800, WINY = 800;        // Predefine window dimensions
				
	//DEBUG
	GLUquadric* debug_sphere = NULL;
	Vec3f debug_position = Vec3f(0);
	Vec3f sphre_position = Vec3f(1,1,1);

	//Interactive movement variables
	bool movingBone;
	Quatf oldOrientation;
	Vec3f oldRotationTarget;
	Vec3f rotationTarget;

	//Test
	MetaTube* testSectionTube;
	Section sections[3][2];
	Section tubeSection;
	Plan sectionPlan;
	bool lockCenter = false;
	AABBLite bigAABB;
	AABBLite restrictedAABB;
	AABBLite axisAbs[3];
	
	int oldx = 0;
	int oldy = 0;

};

void preprocessAABBsESC(vector<AABB>& aabbs);

void drawSectionInPlan(const Section& section, const Plan& plan){
	glPushMatrix();
	
	glTranslatef(section.center[0],section.center[1],section.center[2]);
	//drawAxis(0.5f);

	glBegin(GL_QUADS);
	switch(plan.axis){
		case 0:
			glVertex3f(0,-section.r1,-section.r2);
			glVertex3f(0,-section.r1,section.r2);
			glVertex3f(0,section.r1,section.r2);
			glVertex3f(0,section.r1,-section.r2);
			break;
		case 1:
			glVertex3f(-section.r1,0,-section.r2);
			glVertex3f(-section.r1,0,section.r2);
			glVertex3f(section.r1,0,section.r2);
			glVertex3f(section.r1,0,-section.r2);
			break;
		case 2:
			glVertex3f(-section.r1,-section.r2,0);
			glVertex3f(-section.r1,section.r2,0);
			glVertex3f(section.r1,section.r2,0);
			glVertex3f(section.r1,-section.r2,0);
			break;
	}
	glEnd();


	glPopMatrix();
}

void drawSection3D(const Section3D& section){
	drawBox(section.minv,section.maxv);
}


void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPushMatrix();	
    view_ctrl->set_gl_modelview();
	glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);

	/*
	glPushMatrix();
	glTranslatef(debug_position[0],debug_position[1],debug_position[2]);
	gluSphere( debug_sphere , 0.03 , 20 , 20 );
	glPopMatrix();
	*/

	drawAxis(1);

	glPolygonMode(GL_FRONT, GL_FILL);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glColor4f(.7f,.7f,.7f,1);
	testSectionTube->renderSolidGuizmo();

	/*********************Render WireFrame GUISMOS**************/
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glColor3f(1,0,0);
	
	/*TEST*/
	drawSectionInPlan(tubeSection, sectionPlan);
	drawBox(testSectionTube->aabb.bottomCorner,testSectionTube->aabb.topCorner);

    glPopMatrix();
    glutSwapBuffers();
}

void animate()
{

	if(lockCenter)
		sectionPlan.position = 0.5f*(testSectionTube->worldP1[sectionPlan.axis]+testSectionTube->worldP2[sectionPlan.axis]);
	getMetaTubeSectionBounds(testSectionTube,sectionPlan.position,sectionPlan.axis,tubeSection);
	
	for(int axis = 0; axis < 3; axis++){
		getMetaTubeSectionBounds(testSectionTube, bigAABB.bottomCorner[axis], axis, sections[axis][0]);
		getMetaTubeSectionBounds(testSectionTube, bigAABB.topCorner[axis], axis, sections[axis][1]);

		axisAbs[axis] = aabbFromSec3ds(sec3d(sections[axis][0],axis),sec3d(sections[axis][1],axis));
	}


	restrictedAABB = Restriction(testSectionTube,bigAABB);

    view_ctrl->try_spin();
    glutPostRedisplay();
}


void reshape(int W, int H)
{
    view_ctrl->reshape(W, H);
}

MyRay getRayFromScreePos(int xScreen, int yScreen){
	float fovy = 53;
	float aspect = (float)(WINX/WINY);
	float Hw = 2 * _near * tanf(M_PI*fovy/180);
	float Ww = aspect * Hw;
	
	float xWorld = (float)xScreen / WINX - 0.5;
	float yWorld = (1 - (float)yScreen / WINY ) - 0.5;

	//coordinate of the mouse in the view plane in the object coordinate of the camera
	xWorld *= -Ww/2.5;
	yWorld *= Hw/2.5;
	
	//direction in object space
	Vec3f dir = normalize(Vec3f(xWorld,yWorld,_near));

	//Get Camera frame
	Vec3f eye;
	Vec3f center;
	Vec3f up;
	view_ctrl->get_view_param(eye,center,up);
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

void mouseViewMode(int button, int state, int x, int y){
	Vec2i pos(x, y);
	if(button == GLUT_LEFT_BUTTON)
		view_ctrl->grab_ball(ROTATE_ACTION, pos);
	else if(button == GLUT_MIDDLE_BUTTON)
		view_ctrl->grab_ball(ZOOM_ACTION, pos);
	else if(button == GLUT_RIGHT_BUTTON)
		view_ctrl->grab_ball(PAN_ACTION, pos);
}

void mouse(int button, int state, int x, int y){
	if(state == GLUT_DOWN && Input::keyStates[ROTATE_KEY]){
		Vec3f interSphere;
		MyRay ray = getRayFromScreePos(x,y);
	
		if(getRaySphereIntersection(ray,0.5f*(testSectionTube->worldP1+testSectionTube->worldP2),0.5f,interSphere) ){
			rotationTarget = normalize(interSphere - 0.5f*(testSectionTube->worldP1+testSectionTube->worldP2));
			oldRotationTarget = rotationTarget;
			oldOrientation = testSectionTube->skeleton->orientation;
		}				
	}else if(state == GLUT_DOWN) {
		mouseViewMode(button,state,x,y);
	}else if(state == GLUT_UP){
	    view_ctrl->release_ball();
	}
}



void motion(int x, int y)
{
	Input::mouse.dx = x - oldx;
	Input::mouse.dy = y - oldy;

	oldx = x;
	oldy = y;

	if(Input::keyStates[TRANSLATE_KEY]){
	}else if(Input::keyStates[ROTATE_KEY]){
		Vec3f interSphere;
		MyRay ray = getRayFromScreePos(x,y);

		if(getRaySphereIntersection(ray, 0.5f*(testSectionTube->worldP1+testSectionTube->worldP2),0.5f,interSphere) ){

			Vec3f oldObjectTarget = oldOrientation.apply(oldRotationTarget);

			Vec3f rotationTarget = normalize(interSphere - 0.5f*(testSectionTube->worldP1+testSectionTube->worldP2));
			Vec3f objectNewTarget = oldOrientation.apply(rotationTarget);

			Quatf rot;
			rot.make_rot(objectNewTarget,oldObjectTarget);

			testSectionTube->skeleton->orientation = 
				rot * oldOrientation;
			testSectionTube->skeleton->_updateWorldOrientation();
		}
	}else
		view_ctrl->roll_ball(Vec2i(x, y));
}

void passiveMotion(int x, int y){
	Input::mouse.dx = x - oldx;
	Input::mouse.dy = y - oldy;

	oldx = x;
	oldy = y;
}



void keyboard(unsigned char key, int x, int y)
{
	Input::keyStates[key] = true;
	Vec3f c;


   switch (key) {
		case '\033':
			cout << "Bye" << endl;
			exit(0);
			break;
		case 'a':
			//renderPrimitiveAABB = !renderPrimitiveAABB;
			break;
		case 'b':
			break;
		case 'l':
			lockCenter = !lockCenter;
			break;
		case 'm':
	
			break;
		case 'p':

			break;
		case 'r':
			break;
		case 's':
		
			break;
		case 't':
		
			break;
		case 'w':

			break;
		case KEY_SPACE:
			break;
    }
}
  
void keyboardUp(unsigned char key, int x, int y)
{
	Input::keyStates[key] = false;
}
void specialKeyboard(int key, int x, int y)
{
    switch (key) {
		case GLUT_KEY_UP:
	
			sectionPlan.position += 0.01f;
	
			break;
		case GLUT_KEY_DOWN:
			
			sectionPlan.position -= 0.01f;
		
			break;
		case GLUT_KEY_RIGHT:
			sectionPlan.axis = (sectionPlan.axis+1)%3;
			sectionPlan.position = tubeSection.center[sectionPlan.axis];
			break;
    }
}





int main(int argc, char **argv)
{
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(WINX, WINY);
    glutInit(&argc, argv);
    glutCreateWindow("SkeletalImplicit");
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardUp);
	glutSpecialFunc(specialKeyboard);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
	glutPassiveMotionFunc(passiveMotion);
    glutIdleFunc(animate);

	debug_sphere=gluNewQuadric();

	//*********************************Create the input and control objects
	Input::keyStates.insert(pair<unsigned char,bool>('r',false));
	Input::keyStates.insert(pair<unsigned char,bool>('s',false));
	Input::keyStates.insert(pair<unsigned char,bool>('t',false));

	
	Vec3f c(0, 0, 0);
    float r = 1;
    view_ctrl = new GLViewController(WINX, WINY, c, 1.5*r);
	_near = 1.5f*0.01f;


	Skeleton* theSkeleton = new Skeleton();
	testSectionTube = new MetaTube();
	theSkeleton->addPrimitive(testSectionTube);
	testSectionTube->p1 = Vec3f(0,0,-1);
	testSectionTube->_updateWorldGeometry();

	glewInit();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClearColor(1.0f, 1.0f, 1.0f, 0.f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
    glEnable(GL_NORMALIZE);

	glutMainLoop();

	return 0;
}
