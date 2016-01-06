

#define _CRTDBG_MAP_ALLOC
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

#include "Skeleton.h"
#include "RayCastUtils.h"
#include "DrawUtils.h"
#include "MetaPrimitive.h"
#include "Metaball.h"
#include "MetaTube.h"
#include "BVH.h"
#include "BvhUtils.h"
#include "DisplayText.h"
#include "SkeletalImplicit.h"
#include "Animation.h"
#include "TextureProjector.h"
#include "Input.h"

#include "IEditor.h"
#include "SkeletonEditor.h"
#include "SurfaceEditor.h"
#include "TextureEditor.h"
#include "AnimationEditor.h"
#include "MainEditor.h"

#include <boost/filesystem.hpp>
using namespace boost::filesystem;

#include <sstream>

#include <queue>

#include <math.h>



using namespace std;
using namespace HMesh;
using namespace Geometry;
using namespace GLGraphics;
using namespace Util;
using namespace CGLA;

#define USEBVH



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
	DisplayText displayText;


	//DATA STRUCTURES
	Skeleton skeleton;                 //The skeleton
	vector<Skeleton*> bones;           //separate list of all the bones
	vector<AABB> globalAabbs;
	SkeletalImplicit implicitFuntion;  //The implicit function attached to the skeleton
	Manifold mani;                     //The mesh tesselated from the skeletal implicit function
	CompactBvh* cmpBvh;
	Animation* animation = 0;

	int boneIds[100];
	Mat4x4f boneTrans[25];
	Mat4x4f boneRestTrans[20];
	Vec3f restWPos[5];

	GLuint checkBoardTexId =0;
				
	//DEBUG
	GLUquadric* debug_sphere = NULL;
	Vec3f debug_position = Vec3f(0);
	Vec3f sphre_position = Vec3f(1,1,1);


	//Test
	MetaTube* testSectionTube;
	Section sections[3][2];
	Section tubeSection;
	Plan sectionPlan;
	bool lockCenter = false;
	AABBLite bigAABB;
	AABBLite restrictedAABB;
	AABBLite axisAbs[3];
	

	
	//INPUT HANDLING MODE
	vector<IEditor*> editors;
	MainEditor* mainEditor;
	AnimationEditor* animEditor;
	SkeletonEditor* skeletonEditor;
	SurfaceEditor* surfEditor;
	TextureEditor* texEditor;
	int currentEditor = 0;

	int editMode = VIEW;

	int oldx = 0;
	int oldy = 0;

	//DISPLAY OPTIONS
	int do_wire = false;			   // Whether or not to use wireframe
	bool create_display_list = true;   // used to check whether we need to redo the display list (for efficient rendering).
	bool showSkeleton = true;
	bool showTesselatedMesh = false;
	bool drawWordref = true;
	bool renderRaytraced = false;
	bool showBoundingMeshes = false;
	bool renderBVH = false;
	int bvhLevel = 0;
	bool renderPrimitiveAABB = false;
	bool playAnimation = false;
	
	//SHADER MANAGEMENT
	GLuint prog_trace = 0;
	GLuint prog_traceRaster = 0;
	const string shader_path = "shaders";
	bool reloadShader = true;
	bool useBVH = false;
	GLuint boneMatAttibId = 0;
	GLuint boneIdAttribId = 0;

	//Shared memory for bvh
	unsigned int BVH_IMAGE_SIZE = 1024;
	unsigned int BVH_IMAGE_WIDTH = 1024;
	unsigned int BVH_IMAGE_HEIGHT = 1024;
	//const int BVH_CHANNEL_COUNT = 4;
	const GLenum BVH_PIXEL_FORMAT = GL_RGBA32F;
	//const int BVH_DATA_SIZE = BVH_IMAGE_WIDTH * BVH_IMAGE_HEIGHT * BVH_CHANNEL_COUNT;
	GLuint bvhPboIds[2];                   // IDs of PBO
	GLuint bvhTexId;                   // ID of texture
	Vec4f* bvhTexData = 0;             // pointer to texture buffer
	unsigned int numberOfNodes = 0;

	//NOISE FUNTION UTILS
	GLuint permTextureID;
	int perm[256]= {151,160,137,91,90,15,
	  131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
	  190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
	  88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
	  77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
	  102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
	  135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
	  5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
	  223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
	  129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
	  251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
	  49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
	  138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180};
	int grad3[16][3] = {{0,1,1},{0,1,-1},{0,-1,1},{0,-1,-1},
					   {1,0,1},{1,0,-1},{-1,0,1},{-1,0,-1},
					   {1,1,0},{1,-1,0},{-1,1,0},{-1,-1,0}, // 12 cube edges
					   {1,0,-1},{-1,0,-1},{0,-1,1},{0,1,1}}; // 4 more to make 16
	
};
void createEmpty();
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

/** Sample an implicit function on a regular 3D grid and polygonize the 0 level isosurface. */

void polygonize(Manifold & m, ImplicitFunction & imp, int N,
                const Vec3f& _p0, const Vec3f& _p7)
{
	Vec3f p0 = _p0; // lower, left, close corner of volume.
	Vec3f p7 = _p7; // Upper, right, far corner of volume
    RGrid<float> grid(Vec3i(N,N,N)); // Voxel grid on which to sample 
    Vec3f d = p7-p0; // Diagonal

	// Sample the function passed as `imp' on the regular grid	
    cout << "Sampling" << endl;
    for(int i=0;i<N;++i)
        for(int j=0;j<N;++j)
            for(int k=0;k<N;++k)
                grid[Vec3i(i,j,k)] = imp.eval(d[0]*(i/float(N))+p0[0],
                                              d[1]*(j/float(N))+p0[1],
                                              d[2]*(k/float(N))+p0[2]);
	
	// Polygonize by passing the grid to a voxel grid polygonizer.
    cout << "Polygonizing" << endl;
    mani.clear();
    cuberille_polygonize(grid, mani, 0.0, true);

	for (VertexIter vi = mani.vertices_begin(); vi != mani.vertices_end(); vi++){
		vi->pos = vi->pos/N*d - d/2;
	}
	
	// Triangulate the polygonal mesh and remove caps and needles.
	// caps are triangles with one angle close to 180 degrees.
	// needles are triangles with one very short edge. (yes a needle can be a cap)
    cout << "Removing caps and needles ... " << endl;
	triangulate(mani);
    remove_caps_from_trimesh(mani, M_PI * 0.85);
    remove_needles_from_trimesh(mani, 1e-2);
	
}


void updateBVHTexture(){

	static int index = 0;
    int nextIndex = 0;                  // pbo index used for next frame

	index = 0;//(index + 1) % 2;
	nextIndex = 0;//(index + 1) % 2;

   // bind the texture and TBO
	glBindBuffer(GL_TEXTURE_BUFFER_EXT, bvhPboIds[0]);


    // map the buffer object into client's memory
    //glBufferData(GL_TEXTURE_BUFFER_EXT, BVH_IMAGE_SIZE*sizeof(Vec4f), 0, GL_STREAM_DRAW_ARB);
    Vec4f* ptr = (Vec4f*)glMapBuffer(GL_TEXTURE_BUFFER_EXT, GL_WRITE_ONLY_ARB);

    if(ptr)
    {
        // update data directly on the mapped buffer
		memcpy(ptr,cmpBvh->data,cmpBvh->dataSize*sizeof(Vec4f));
        //updateBVHPixelData(ptr, BVH_IMAGE_SIZE);
        glUnmapBuffer(GL_TEXTURE_BUFFER_EXT); // release pointer to mapping buffer
    }else
		cout<<"unable to map bvh data pointer"<<endl;

    // it is good idea to release PBOs with ID 0 after use.
    // Once bound with 0, all pixel operations behave normal ways.
    glBindBuffer(GL_TEXTURE_BUFFER_EXT, 0);
}


void enableRayTraceRaster()
{
	static GLuint vs = 0;
	static GLuint fs = 0;
	if(reloadShader){
		cout<<"Compiling shader raster"<<endl;
		reloadShader = false;
		
		glDeleteProgram(prog_traceRaster);
		glDeleteShader(vs);
		glDeleteShader(fs);
		
		prog_traceRaster = glCreateProgram();
		
		vs = create_glsl_shader(GL_VERTEX_SHADER, shader_path, "raytraceRaster.vert");
		fs = create_glsl_shader(GL_FRAGMENT_SHADER, shader_path, "raytraceRaster.frag");
		
		if(vs) glAttachShader(prog_traceRaster, vs);
		if(fs) glAttachShader(prog_traceRaster, fs);

		glLinkProgram(prog_traceRaster);
		print_glsl_program_log(prog_traceRaster);
	}
	Vec3f cameraPos;
	view_ctrl->get_view_param(cameraPos,Vec3f(),Vec3f());

	glUseProgram(prog_traceRaster);

	glUniform3fv(glGetUniformLocation(prog_traceRaster,"cameraPos"), 1, cameraPos.get());
	glUniform1i(glGetUniformLocation(prog_traceRaster,"permTexture"), 0);
	glUniform1i(glGetUniformLocation(prog_traceRaster,"checkTexture"), 0);

	//SURFACE
	glUniform1i(glGetUniformLocation(prog_traceRaster,"nbOfMetaTubes"), implicitFuntion.p1s.size());
	glUniform1i(glGetUniformLocation(prog_traceRaster,"nbOfBalls"), implicitFuntion.spheres.size());
	glUniform3fv(glGetUniformLocation(prog_traceRaster,"p1s"), implicitFuntion.p1s.size(), implicitFuntion.p1s[0].get());
	glUniform3fv(glGetUniformLocation(prog_traceRaster,"p1s")+implicitFuntion.p1s.size(), implicitFuntion.spheres.size(), implicitFuntion.spheres[0].get());
	glUniform3fv(glGetUniformLocation(prog_traceRaster,"p2s"), implicitFuntion.p1s.size(), implicitFuntion.p2s[0].get());
	glUniform1fv(glGetUniformLocation(prog_traceRaster,"radius"), implicitFuntion.radiuses.size(), &implicitFuntion.radiuses[0]);
	glUniform1fv(glGetUniformLocation(prog_traceRaster,"radius")+implicitFuntion.radiuses.size(), implicitFuntion.sRadius.size(), &implicitFuntion.sRadius[0]);

	//TRANSFORMATION MATRIXES FOR TEXTURING
	const int  nb = implicitFuntion.primitives.size();
	vector<Vec3f> orth;
	vector<Mat3x3f> mats;
	orth.reserve(nb);
	mats.reserve(nb);

	for(unsigned int i = 0 ; i < implicitFuntion.primitives.size(); i++){
		orth.push_back(implicitFuntion.primitives[i]->skeleton->worldPosition);
		mats.push_back(implicitFuntion.primitives[i]->skeleton->worldOrientation.inverse().get_Mat3x3f());
	}
	glUniform3fv(glGetUniformLocation(prog_traceRaster,"MetaTubeOrtho"), implicitFuntion.primitives.size(), restWPos[0].get());
	glUniform3fv(glGetUniformLocation(prog_traceRaster,"BoneWorldPos"), implicitFuntion.primitives.size(), orth[0].get());
	glUniformMatrix3fv(glGetUniformLocation(prog_traceRaster,"MetaTubeMat"), implicitFuntion.primitives.size(), false, mats[0].get());
	
	
	
	glUniform1f(glGetUniformLocation(prog_traceRaster,"alpha"), showSkeleton?0.5f:1);


	boneMatAttibId = glGetAttribLocation(prog_traceRaster,"boneWTrans");


	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, checkBoardTexId);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, permTextureID);
}

void enableRayTrace()
{
	static GLuint vs = 0;
	static GLuint fs = 0;
	if(reloadShader)
	{
		reloadShader = false;
		
		glDeleteProgram(prog_trace);
		glDeleteShader(vs);
		glDeleteShader(fs);
		
		prog_trace = glCreateProgram();
		
		vs = create_glsl_shader(GL_VERTEX_SHADER, shader_path, "raytrace.vert");
		fs = create_glsl_shader(GL_FRAGMENT_SHADER, shader_path, "raytrace.frag");
		
		if(vs) glAttachShader(prog_trace, vs);
		if(fs) glAttachShader(prog_trace, fs);

		glLinkProgram(prog_trace);
		print_glsl_program_log(prog_trace);
/*
		globalAabbs.clear();
		for(unsigned int i = 0; i < implicitFuntion.primitives.size(); i++){
		globalAabbs.push_back(implicitFuntion.primitives[i]->aabb);
		}
		/*
		delete bvh;
		bvh = new BVH();
		bvh->initialize(globalAabbs);
*/
		
	}
	cmpBvh->clear();	
	vector<AABBLite> aabbsl;
	for(unsigned int i = 0; i < implicitFuntion.primitives.size(); i++){
		AABBLite ab;
		ab.bottomCorner = implicitFuntion.primitives[i]->aabb.bottomCorner;
		ab.topCorner = implicitFuntion.primitives[i]->aabb.topCorner;
		ab.primitive = implicitFuntion.primitives[i];
		aabbsl.push_back(ab);
	}
	if(aabbsl.size()>0)
	cmpBvh->initializeSplit(0,aabbsl);
	updateBVHTexture();


	Vec3f cameraPos;
	view_ctrl->get_view_param(cameraPos,Vec3f(),Vec3f());

	glUseProgram(prog_trace);

	glUniform3fv(glGetUniformLocation(prog_trace,"cameraPos"), 1, cameraPos.get());
	glUniform1i(glGetUniformLocation(prog_trace,"checkTexture"), 1);
	glUniform1i(glGetUniformLocation(prog_trace,"permTexture"), 2);


	//PRIMITIVES
	glUniform1i(glGetUniformLocation(prog_trace,"nbOfMetaTubes"), implicitFuntion.p1s.size());
	glUniform1i(glGetUniformLocation(prog_trace,"nbOfBalls"), implicitFuntion.spheres.size());
	glUniform1i(glGetUniformLocation(prog_trace,"nbOfNodes"), cmpBvh->maxNbOfNodes);
	glUniform3fv(glGetUniformLocation(prog_trace,"p1s"), implicitFuntion.p1s.size(), implicitFuntion.p1s[0].get());
	glUniform3fv(glGetUniformLocation(prog_trace,"p1s")+implicitFuntion.p1s.size(), implicitFuntion.spheres.size(), implicitFuntion.spheres[0].get());
	glUniform3fv(glGetUniformLocation(prog_trace,"p2s"), implicitFuntion.p2s.size(), implicitFuntion.p2s[0].get());
	glUniform1fv(glGetUniformLocation(prog_trace,"radius"), implicitFuntion.radiuses.size(), &implicitFuntion.radiuses[0]);
	glUniform1fv(glGetUniformLocation(prog_trace,"radius")+implicitFuntion.radiuses.size(), implicitFuntion.sRadius.size(), &implicitFuntion.sRadius[0]);
	
	glUniform1iv(glGetUniformLocation(prog_trace,"boneIds"), implicitFuntion.p1s.size()+implicitFuntion.spheres.size(), &boneIds[0]);

	// BVH
	glUniform1i(glGetUniformLocation(prog_trace, "tboSampler"), 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_BUFFER_EXT, bvhTexId);	
	
	// SKINNING TRANSFORMS
	int nbOfBones = bones.size();
	for(unsigned int i = 0 ; i < nbOfBones; i++){
		boneTrans[i] = bones[i]->worldOrientation.inverse().get_Mat4x4f();
		boneTrans[i][3] = Vec4f(-bones[i]->worldPosition,1);
	}
	glUniformMatrix4fv(glGetUniformLocation(prog_trace,"boneTrans"), bones.size(), false, boneTrans[0].get());
	glUniformMatrix4fv(glGetUniformLocation(prog_trace,"boneRestTrans"), bones.size(), false, animEditor->restPoseTrans[0].get());

	// TEXTURE PROJECTORS
	vector<Vec3f> proj1s;
	vector<Vec3f> proj2s;
	vector<Vec3f> orthos;
	vector<float> rads;
	for(unsigned int i = 0; i < texEditor->texProjectors.size(); i++){
		Vec3f axis = texEditor->texProjectors[i]->orientation.apply(Vec3f(0,0,1));
		proj1s.push_back(texEditor->texProjectors[i]->position - 0.5f*texEditor->texProjectors[i]->length*axis);
		proj2s.push_back(texEditor->texProjectors[i]->position + 0.5f*texEditor->texProjectors[i]->length*axis);
		orthos.push_back(texEditor->texProjectors[i]->orientation.apply(Vec3f(1,0,0)));
		rads.push_back(texEditor->texProjectors[i]->radius);
	}
	glUniform3fv(glGetUniformLocation(prog_trace,"proj1s"), proj1s.size(), proj1s[0].get());
	glUniform3fv(glGetUniformLocation(prog_trace,"proj2s"), proj1s.size(), proj2s[0].get());
	glUniform3fv(glGetUniformLocation(prog_trace,"orthos"), proj1s.size(), orthos[0].get());
	glUniform1fv(glGetUniformLocation(prog_trace,"rads"), proj1s.size(), &rads[0]);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, checkBoardTexId);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, permTextureID);

}

void drawScreenFillingQuad(){
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, WINX, 0, WINY, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glBegin(GL_QUADS);
	glVertex3f(0,0,0);
	glVertex3f(WINX,0,0);
	glVertex3f(WINX,WINY,0);
	glVertex3f(0,WINY,0);

	glEnd();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	
	glEnable(GL_DEPTH_TEST);
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
	if(drawWordref)
		drawAxis(1);

	/************************Render Solid Meshes********************/
	if(showSkeleton){
		glColor4f(.7f, .7f, .7f, 1);
		for(unsigned int i = 0 ; i < bones.size(); i++){
			bones[i]->renderWorldSpace();
		}
	}
	if(showTesselatedMesh){
		static bool first = true;
		if(first){
			glNewList(1, GL_COMPILE);
			draw(mani);
			glEndList();
			first = false;
		}
		glCallList(1);
	}
	/*********************Render WireFrame GUISMOS**************/
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glColor3f(1,0,0);
/*
	glPushMatrix();
	glTranslatef(sphre_position[0],sphre_position[1],sphre_position[2]);
	gluSphere( debug_sphere , 0.2f , 20 , 20 );
	glPopMatrix();
	*/
	
	/*TEST*/
	//drawSectionInPlan(tubeSection, sectionPlan);
	/*
	
	drawBox(bigAABB.bottomCorner,bigAABB.topCorner);
	drawBox(restrictedAABB.bottomCorner,restrictedAABB.topCorner);
	for(int axis = 0; axis < 3; axis++){
		Plan plan;
		plan.axis = axis;
		//plan.position = sections[axis][0].center[axis];
		//drawSectionInPlan(sections[axis][0], plan);
		//plan.position = sections[axis][1].center[axis];
		//drawSectionInPlan(sections[axis][1], plan);
		//drawSection3D(sec3d(sections[axis][0],axis));
		//drawSection3D(sec3d(sections[axis][1],axis));
		//drawBox(axisAbs[axis].bottomCorner,axisAbs[axis].top);
	}
	
	glColor4f(.7f,.7f,.7f,1);
	testSectionTube->renderGuizmo();

	/*...*/

	if(editMode == IMPLICIT){
		for(unsigned int i = 0 ; i < implicitFuntion.primitives.size(); i++){
			if(surfEditor->selectedPrimitive == implicitFuntion.primitives[i])
				glColor4f(1,0,0,1);
			else glColor4f(.7f,.7f,.7f,1);
			implicitFuntion.primitives[i]->renderGuizmo();
		}
	}
	

	if(editMode == TEXTURING){
		for(unsigned int i = 0; i < texEditor->texProjectors.size(); i++){
			if(texEditor->selectedProjector == texEditor->texProjectors[i]){
				glBegin(GL_LINES);
				glColor3f(1,0,0);
				glVertex3fv(texEditor->selectedProjector->position.get());
				glVertex3fv((texEditor->selectedProjector->position+0.2f*texEditor->oldRotationTarget).get());
				glColor3f(0,1,0);
				glVertex3fv(texEditor->selectedProjector->position.get());
				glVertex3fv((texEditor->selectedProjector->position+0.2f*texEditor->rotationTarget).get());
				glEnd();

				glColor4f(1,0,0,1);
			}
			else glColor4f(.7f,.7f,.7f,1);
			texEditor->texProjectors[i]->renderGuizmo();
		}
	}

	/*********************Render WireFrame Meshes on top in red**************/
	glDisable(GL_LIGHTING);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glColor3f(1, 0, 0);
	glPolygonOffset(-2, -2);


	if(editMode == BONE)
		if(skeletonEditor->selectedBone)
			skeletonEditor->selectedBone->renderWorldSpace(true);
	if(editMode == ANIMATION)
		if(animEditor->selectedBone)
			animEditor->selectedBone->renderWorldSpace(true);
	if(editMode == TEXTURING)
		if(texEditor->selectedProjector)
			texEditor->selectedProjector->renderGuizmo();
	if(editMode == IMPLICIT)
		if(surfEditor->selectedPrimitive)
			surfEditor->selectedPrimitive->renderGuizmo();
	if(showTesselatedMesh && do_wire)
		draw(mani);

	glPolygonOffset(0, 0);
	glDisable(GL_POLYGON_OFFSET_LINE);
	glDepthFunc(GL_LESS);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_LIGHTING);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	/*********************Render Ray Traced**************/
	if(renderRaytraced){
		#ifdef USEBVH
			enableRayTrace();
		#else
			enableRayTraceRaster();
		#endif
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		#ifdef USEBVH
			//drawFullBox(bvh->aabb.bottomCorner,bvh->aabb.topCorner);
			drawFullBox(Vec3f(cmpBvh->nodeBottom(0)),Vec3f(cmpBvh->nodeTop(0)));
		#else
			for(unsigned int i = 0 ; i < implicitFuntion.primitives.size(); i++){
				
				//Compute and sent to the vertex shader the world model matrix of the mesh
				Mat4x4f boneRot = implicitFuntion.primitives[i]->skeleton->worldOrientation.inverse().get_Mat4x4f();
				Mat4x4f boneWorldTrans =		translation_Mat4x4f(implicitFuntion.primitives[i]->skeleton->worldPosition)
											*	boneRot	;
				Mat4x4f boneWorldTransIvers;
				transpose(boneWorldTrans,boneWorldTransIvers);

				#ifdef USEBVH
					glBindAttribLocation(prog_trace,boneMatAttibId,"boneWTrans");
				#else
					glBindAttribLocation(prog_traceRaster,boneMatAttibId,"boneWTrans");
				#endif

					
				glVertexAttrib4fv(boneMatAttibId + 0, boneWorldTransIvers[0].get());
				glVertexAttrib4fv(boneMatAttibId + 1, boneWorldTransIvers[1].get());
				glVertexAttrib4fv(boneMatAttibId + 2, boneWorldTransIvers[2].get());
				glVertexAttrib4fv(boneMatAttibId + 3, boneWorldTransIvers[3].get());

				#ifndef USEBVH
				glUniform1i(glGetUniformLocation(prog_traceRaster,"boneID"),i);// static_cast<MetaTube*>(implicitFuntion.primitives[i])->id);
				#endif
				
				implicitFuntion.primitives[i]->renderBoundingMesh();
			}
		#endif
		glUseProgram(0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_BUFFER_EXT, 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, 0);

		glDisable(GL_TEXTURE_2D);
	}

	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	


	if(showBoundingMeshes)
	for(unsigned int i = 0 ; i < implicitFuntion.primitives.size(); i++){
	    glDisable(GL_LIGHTING);
        glDepthFunc(GL_LEQUAL);
        glEnable(GL_POLYGON_OFFSET_LINE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glColor3f(0, 0, 1);
        glPolygonOffset(-2, -2);

		implicitFuntion.primitives[i]->renderBoundingMesh();

		glPolygonOffset(0, 0);
		glDisable(GL_POLYGON_OFFSET_LINE);
		glDepthFunc(GL_LESS);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_LIGHTING);
	}else if(renderBVH){
		glColor3f(0,1,0);
	    glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		//bvh->render(bvhLevel);
		cmpBvh->render(bvhLevel);
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_LIGHTING);
	}
	if(renderPrimitiveAABB){
		glColor3f(1,0,0);
	    glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		for(unsigned int i = 0; i < globalAabbs.size(); i++){
			renderAABB(globalAabbs[i]);
		}

		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_LIGHTING);
	}
	glEnable(GL_DEPTH_TEST);

	
    glPopMatrix();
	displayText.draw();

    glutSwapBuffers();
}

void animate()
{
	/*
	if(lockCenter)
		sectionPlan.position = 0.5f*(testSectionTube->worldP1[sectionPlan.axis]+testSectionTube->worldP2[sectionPlan.axis]);
	getMetaTubeSectionBounds(testSectionTube,sectionPlan.position,sectionPlan.axis,tubeSection);
	
	for(int axis = 0; axis < 3; axis++){
		getMetaTubeSectionBounds(testSectionTube, bigAABB.bottomCorner[axis], axis, sections[axis][0]);
		getMetaTubeSectionBounds(testSectionTube, bigAABB.topCorner[axis], axis, sections[axis][1]);

		axisAbs[axis] = aabbFromSec3ds(sec3d(sections[axis][0],axis),sec3d(sections[axis][1],axis));
	}


	restrictedAABB = Restriction(testSectionTube,bigAABB);
	static unsigned int time = 0;
	if(playAnimation && animation){
		animation->animate(glutGet(GLUT_ELAPSED_TIME));
		time++;
	}
	*/

	implicitFuntion.update();
    view_ctrl->try_spin();
    glutPostRedisplay();
}


void reshape(int W, int H)
{
    view_ctrl->reshape(W, H);
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
	//MyRay ray = mainEditor->getRayFromScreePos(x,y);
	//getRaySphereIntersection(ray,sphre_position,0.2f,debug_position);

	bool editorTookInput = false;
	if(currentEditor >= 0)
		editorTookInput = editors[currentEditor]->onMouseButton(button,state,x,y);
	if(editorTookInput)return;

    if(state == GLUT_DOWN) {
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

	bool editorTookInput = false;
	if(currentEditor >= 0)
		editorTookInput = editors[currentEditor]->onMouseMotion(x,y);
	if(editorTookInput)return;

	view_ctrl->roll_ball(Vec2i(x, y));
}

void passiveMotion(int x, int y){
	Input::mouse.dx = x - oldx;
	Input::mouse.dy = y - oldy;

	oldx = x;
	oldy = y;
}



void clearSkeleton(){
	skeleton.clear();                 //The skeleton
	bones.clear();           //separate list of all the bones
	globalAabbs.clear();
	mani.clear();
	implicitFuntion.clear(); 

	//delete bvh;
	//bvh = NULL;
	cmpBvh->clear();

	delete animation;
	animation = NULL;
	/*
	delete restPose;
	restPose = NULL;
	delete animClip;
	animClip = NULL;
	*/
}

void initializeSkeleton(Skeleton& outSkel){


}
/*
void loadAndInitializeSkeleton(Skeleton& outSkel,const string& filename){
	std::ifstream ifs;
	ifs.open(("Models/"+filename).c_str(),ios::in);
	if(ifs.fail()){
		cout << "Error: Unable to open "<<filename<<endl;
		return;
	}

	cout<<"loading model "<<filename<<endl;
	load(outSkel,ifs);

	string bModelName = filename;
	bModelName = bModelName.substr(0,bModelName.find("."));
	modelName = bModelName;
	displayText.updateText(modelNameTextId,bModelName);
	
	initializeSkeleton(outSkel);
	ifs.close();
}
*/

  
void keyboard(unsigned char key, int x, int y)
{
	Input::keyStates[key] = true;
	Vec3f c;

	/******EDITOR INPUT HANDLING*******/
	if((int)key == KEY_TAB){//tab
		editMode = (editMode+1)%MAXMODE;
		playAnimation = false;
		editors[currentEditor]->close();
		currentEditor=(currentEditor+1)%editors.size();
		editors[currentEditor]->open();
	}
	editors[currentEditor]->onKeyboard(key,x,y);
	/******EDITOR INPUT HANDLING*******/

   switch (key) {
		case '\033':
			cout << "Bye" << endl;
			glBindBuffer(1, bvhPboIds[0]);
			glDeleteBuffers(1, &bvhPboIds[0]);

			exit(0);
			break;
		case 'a':
			//renderPrimitiveAABB = !renderPrimitiveAABB;
			break;
		case 'b':
			if(editMode == VIEW){
				if(showBoundingMeshes){
					showBoundingMeshes = false;
					renderBVH = true;
				}else if(renderBVH)
					renderBVH = false;
				else
					showBoundingMeshes = true;
			}
			break;
		case 'l':
			lockCenter = !lockCenter;
			cout << "Showing lines" << endl;
			do_wire = !do_wire;
			break;
		case 'm':
			mani.clear();
			implicitFuntion = SkeletalImplicit(&skeleton,0.5f);
			polygonize(mani, implicitFuntion, 100, Vec3f(cmpBvh->nodeBottom(0)),Vec3f(cmpBvh->nodeTop(0)));
			break;
		case 'p':
			showTesselatedMesh = !showTesselatedMesh;
			break;
		case 'r':
			if(editMode == VIEW)
				reloadShader = true;
			break;
		case 's':
			if(editMode == VIEW)
				showSkeleton = !showSkeleton;
			break;
		case 't':
			if(editMode == VIEW){
				renderRaytraced = !renderRaytraced;
			}
			break;
		case 'w':
			drawWordref = !drawWordref;
			break;
		case KEY_SPACE:
			if(editMode == VIEW){
				playAnimation =! playAnimation;
				if(!playAnimation)
					animEditor->setToRestPose();
			}
			break;
    }
    create_display_list = true;
}
  
void keyboardUp(unsigned char key, int x, int y)
{
	Input::keyStates[key] = false;
}
void specialKeyboard(int key, int x, int y)
{
	if(currentEditor >= 0)
		editors[currentEditor]->onKeyboard(key,x,y);

    switch (key) {
		case GLUT_KEY_UP:
			MAXAABBCOUNT++;
			/*
			globalAabbs.clear();
			for(unsigned int i = 0; i < implicitFuntion.primitives.size(); i++){
				globalAabbs.push_back(implicitFuntion.primitives[i]->aabb);
			}
			*/
			//preprocessAABBsESC(globalAabbs);

			sectionPlan.position += 0.01f;
			
			if(bvhLevel < MAX_BVH_D)
				bvhLevel++;
			//cmpBvh->print();
		
			break;
		case GLUT_KEY_DOWN:
			if(MAXAABBCOUNT > 2)
			MAXAABBCOUNT--;
			/*
			globalAabbs.clear();
			for(unsigned int i = 0; i < implicitFuntion.primitives.size(); i++){
				globalAabbs.push_back(implicitFuntion.primitives[i]->aabb);
			}
			*/
			//preprocessAABBsESC(globalAabbs);
			sectionPlan.position -= 0.01f;
			//cmpBvh->print();
			if(bvhLevel > 0)
				bvhLevel--;
			break;
		case GLUT_KEY_RIGHT:
			sectionPlan.axis = (sectionPlan.axis+1)%3;
			sectionPlan.position = tubeSection.center[sectionPlan.axis];
			break;
    }
    create_display_list = true;
}

void createTestGuy(){
	skeleton.translate(Vec3f(0,0,-0.5f));
	skeleton.setLength(0);
	//skeleton.addPrimitive(new MetaTube(Vec3f(0),Vec3f(0,0,0),0.1f,0.1f));
	skeleton.name = "center";

	//backBone
	Skeleton* backBone = new Skeleton();
	backBone->name = "backBone";
	backBone->addPrimitive(new MetaTube(Vec3f(0),Vec3f(0,0,backBone->length),0.1f,0.1f));
	skeleton.addChild(backBone);

	//head
	Skeleton* head = new Skeleton();
	head->name = "head";
	head->setLength(0.5f);
	head->addPrimitive(new MetaTube(Vec3f(0),Vec3f(0,0,head->length),0.1f,0.1f));
	head->addPrimitive(new Metaball(Vec3f(0,0,0.5f),0.25));
	backBone->addChild(head);

	//Left arm
	Skeleton* lArm = new Skeleton();
	lArm->name = "lArm";
	lArm->rotate(Vec3f(0,1,0),90+25);
	lArm->setLength(0.5f);
	lArm->addPrimitive(new MetaTube(Vec3f(0),Vec3f(0,0,lArm->length),0.1f,0.1f));
	backBone->addChild(lArm);
	//Right Arm
	Skeleton* rArm = new Skeleton();
	rArm->name = "rArm";
	rArm->rotate(Vec3f(0,1,0),-90-25);
	rArm->addPrimitive(new MetaTube(Vec3f(0),Vec3f(0,0,rArm->length),0.1f,0.1f));
	backBone->addChild(rArm);

	//Pelvis
	Skeleton* pelvis = new Skeleton();
	pelvis->name = "pelvis";
	pelvis->setLength(0.5f);
	pelvis->rotate(Vec3f(0,1,0),M_PI);
	pelvis->rotate(Vec3f(1,0,0),M_PI/6);
	pelvis->addPrimitive(new MetaTube(Vec3f(0),Vec3f(0,0,pelvis->length),0.1f,0.1f));
	skeleton.addChild(pelvis);
	//Left Leg

	Skeleton* lLeg = new Skeleton();
	lLeg->name = "lLeg";
	lLeg->rotate(Vec3f(0,1,0),M_PI/4);
	lLeg->setLength(0.5f);
	lLeg->addPrimitive(new MetaTube(Vec3f(0),Vec3f(0,0,lLeg->length),0.1f,0.1f));
	pelvis->addChild(lLeg);
	
	//Right Leg
	Skeleton* rLeg = new Skeleton();
	rLeg->name = "rLeg";
	rLeg->rotate(Vec3f(0,1,0),-M_PI/4);
	rLeg->addPrimitive(new MetaTube(Vec3f(0),Vec3f(0,0,rLeg->length),0.1f,0.1f));
	pelvis->addChild(rLeg);

}

void createTest(){
	skeleton.translate(Vec3f(0,0,-0.5f));
	skeleton.setLength(1);
	skeleton.addPrimitive(new MetaTube(Vec3f(0),Vec3f(0,0,skeleton.length),0.1f,0.1f));
	skeleton.name = "center";

	//backBone
	Skeleton* backBone = new Skeleton();
	backBone->name = "backBone";
	backBone->rotate(Vec3f(1,1,0),-M_PI/4);
	backBone->addPrimitive(new MetaTube(Vec3f(0),Vec3f(0,0,backBone->length),0.1f,0.1f));

	skeleton.addChild(backBone);
/*
	animation = new Animation();

	AnimationClip* lclip = new AnimationClip();
	lclip->name = "Test";
	lclip->playing = true;
	lclip->loop = true;
	lclip->nbBones = 2;
	lclip->fade = 0;
	lclip->bones = new Skeleton*[lclip->nbBones];
	lclip->bones[0] = &skeleton;
	lclip->bones[1] = backBone;

	KeyFrame* key0l = new KeyFrame();
	key0l->rotations = new Quatf[lclip->nbBones];
	//left leg
	key0l->rotations[0] = key0l->rotations[0].identity_Quatf();
	key0l->rotations[1] = key0l->rotations[1].identity_Quatf();
	key0l->time = 0;


	KeyFrame* key1l = new KeyFrame();
	key1l->rotations = new Quatf[lclip->nbBones];
	key1l->rotations[0].make_rot(90,Vec3f(1,0,0));
	key1l->rotations[1].make_rot(45,Vec3f(0,1,0));
	key1l->time = 250;

	KeyFrame* key2l = new KeyFrame();
	key2l->rotations = new Quatf[lclip->nbBones];
	key2l->rotations[0] = key0l->rotations[0].identity_Quatf();
	key2l->rotations[1] = key0l->rotations[1].identity_Quatf();
	key2l->time = 500;

	key0l->next = key1l;
	key1l->next = key2l;
	lclip->first = key0l;
	lclip->current = key0l;

	animation->addClip(lclip);
	*/
}

void createTexTest(){
	skeleton.translate(Vec3f(0,0,-0.5f));
	skeleton.setLength(1);
	skeleton.addPrimitive(new MetaTube(Vec3f(0),Vec3f(0,0,skeleton.length),0.1f,0.1f));
	skeleton.name = "center";
}



void createOctopus(){


	skeleton.translate(Vec3f(0,0,0.0f));
	skeleton.setLength(0.0f);
	//skeleton.addPrimitive();
	skeleton.name = "center";

		//head
	Skeleton* head = new Skeleton();
	head->name = "head";
	head->setLength(0.2f);
	head->addPrimitive(new MetaTube(Vec3f(0),Vec3f(0,0,head->length),0.1f,0.1f));
	head->addPrimitive(new Metaball(Vec3f(0,0,0.3f),0.25));

	skeleton.addChild(head);

	for(int i = 0; i < 8; i++){
			//Right Leg
		Skeleton* rLeg = new Skeleton();
		rLeg->name = "Leg";
		rLeg->rotate(Vec3f(0,1,0),i*M_PI/4);
		rLeg->setLength(0.6f);
		rLeg->addPrimitive(new MetaTube(Vec3f(0),Vec3f(0,0,rLeg->length),0.1f,0.1f));		

		Skeleton* Leg = new Skeleton();
		Leg->name = "Leg";
		Leg->rotate(Vec3f(0,1,0),i*M_PI/4);
		Leg->setLength(0.6f);
		Leg->addPrimitive(new MetaTube(Vec3f(0),Vec3f(0,0,rLeg->length),0.1f,0.1f));	
		rLeg->addChild(Leg);
			
		skeleton.addChild(rLeg);

	}

}

void createEmpty(){

	skeleton.translate(Vec3f(0,0,0.0f));
	skeleton.setLength(0.0f);
	//skeleton.addPrimitive();
	skeleton.name = "center";
}

unsigned int nextPowerOf2(unsigned int n){
   n |= (n >> 16);
   n |= (n >> 8);
   n |= (n >> 4);
   n |= (n >> 2);
   n |= (n >> 1);
   ++n;
   return n;
}


/*
Vec3f 2DTo3D(Vec2f 3d, int axis){
}
*/
/*
 * Splits ab1 in the axis direction and fills ab2 with the other part of the split
 */
void splitESCTest(int axis, AABB& ab1/*In out*/, AABB& ab2/*out*/){
	float mid = 0.5f*(ab1.topCorner[axis]+ab1.bottomCorner[axis]);
	Vec3f boxMiddle = 0.5f*(ab1.topCorner+ab1.bottomCorner);

	//Resize ab2 to ab1
	ab2.topCorner = ab1.topCorner;
	ab2.bottomCorner = ab1.bottomCorner;
	ab2.primitives.insert(ab1.primitives.begin(),ab1.primitives.end());

	//cut on the middle both aabbs
	ab1.topCorner[axis] = mid;
	ab2.bottomCorner[axis] = mid;

	//Get the primitive info and clip the segment against the aabb
	MetaTube* tube = static_cast<MetaTube*>((*(ab1.primitives.begin())));
	//MetaTube normal

	Vec3f sortP1;
	Vec3f sortP2;
	if(tube->worldP1[axis] <= tube->worldP2[axis]){
		sortP1 = tube->worldP1;
		sortP2 = tube->worldP2;
	}else{
		sortP1 = tube->worldP2;
		sortP2 = tube->worldP1;
	}

	Vec3f P1;
	Vec3f P2;
	Vec3f axisV(axis==0?1:0,axis==1?1:0,axis==2?1:0);
	Vec3f tn = normalize(tube->worldP2-tube->worldP1);

	// Project the segment points on the faces of the box
	if( ab1.bottomCorner[axis] < sortP1[axis] ){//ortogonal
		P1 = sortP1;
		P1[axis] = ab1.bottomCorner[axis];
	}else if( ab1.bottomCorner[axis] < sortP2[axis]){//on segment
		P1 = sortP1 + (dot(axisV,ab1.bottomCorner-sortP1)/dot(axisV,tn))*tn;
	}else{//orthogonal
		P1 = sortP2;
		P1[axis] = ab1.bottomCorner[axis];
	}
	if( ab2.topCorner[axis] < sortP1[axis] ){//ortogonal
		P2 = sortP1;
		P2[axis] = ab2.topCorner[axis];
	}else if( ab2.topCorner[axis] < sortP2[axis]){//on segment
		P2 = sortP2 + (dot(axisV,ab2.topCorner-sortP2)/dot(axisV,tn))*tn;
	}else{//orthogonal
		P2 = sortP2;
		P2[axis] = ab2.topCorner[axis];
	}
	
	//Section of the MetaTube with the plane at I
	//projected 
	float dntn = tn[axis];
	Vec3f tn0 = tn - dot(tn,Vec3f(0,1,0))*Vec3f(0,1,0);
	Vec3f tn1 = tn - dot(tn,Vec3f(1,0,0))*Vec3f(0,1,0);
	//Section great radiuses
	float sx = abs(tube->r1 / dntn);
	float sy = abs(tube->r1 / dntn);

	Vec3f I13D; 
	if(mid < sortP1[axis]){
		I13D = sortP1;
		I13D[axis] = mid;
		sx = sy = sqrtf(sqr<float>(tube->r1)-sqr<float>(mid - sortP1[axis]));
	}else if(mid < sortP2[axis]){
		I13D = boxMiddle;
		sx = tube->r1;//abs(MetaTube->r1 / dntn);//wrong
		sy = tube->r1;//abs(MetaTube->r1 / dntn);//wrond
	}else{
		I13D = sortP2;
		I13D[axis] = mid;
		sx = sy = sqrtf(sqr<float>(tube->r2)-sqr<float>(mid - sortP2[axis]));
	}
	debug_position = I13D;

	//centroid of the section
	Vec2f I1 = v3DTov2D(I13D,axis);
	//Bounds of the section
	Vec2f sBoundsMin;
	Vec2f sBoundsMax;
	sBoundsMin = I1 - Vec2f(sx,sy);//Vec2f(sx,sy);
	sBoundsMax = I1 + Vec2f(sx,sy);//Vec2f(sx,sy);

	//Clib ab1
	//Projection of P1 on the plane if P1 is in the box
	Vec2f p1 = v3DTov2D(P1,axis);
	//Bounds of the projection of p1
	Vec2f p1BoundsMin;
	Vec2f p1BoundsMax;

	if( P1[axis] > sortP2[axis]){
		sx = sy = sqrtf(sqr<float>(tube->r2)-sqr<float>(P1[axis] - sortP2[axis]));
	}else if( P1[axis] > sortP1[axis]){
		sx = tube->r1;//abs(MetaTube->r1 / dntn);//wrong
		sy = tube->r1;//abs(MetaTube->r1 / dntn);//wrond
	}else if( P1[axis] > sortP1[axis]-tube->r1+0.000001){
		sx = sy = sqrtf(sqr<float>(tube->r1)-sqr<float>(P1[axis] - sortP1[axis]));
	}else{
		sx = tube->r1;
		sy = tube->r1;
	}
/*
	if( P1[axis] <= sortP1[axis] - MetaTube->r1){
		sx = MetaTube->r1;
		sy = MetaTube->r1;
	}else if( P1[axis] < sortP1[axis]){ //Projection of the sphere
		sx = sy = sqrtf(sqr<float>(MetaTube->r1)-sqr<float>(P1[axis] - sortP1[axis]));
	}else if(P1[axis] < sortP2[axis]){ //Section of the intersection
		sx = MetaTube->r1;//abs(MetaTube->r1 / dntn);//wrong
		sy = MetaTube->r1;//abs(MetaTube->r1 / dntn);//wrond
	}else{
		sx = sy = sqrtf(sqr<float>(MetaTube->r2)-sqr<float>(P1[axis] - sortP2[axis]));
	}
	*/
	p1BoundsMin = p1 - Vec2f(sx,sy);//Vec2f(sx,sy);
	p1BoundsMax = p1 + Vec2f(sx,sy);//Vec2f(sx,sy);

	//Bounds of the two bounds
	float min0 = min(p1BoundsMin[0],sBoundsMin[0]);
	float min1 = min(p1BoundsMin[1],sBoundsMin[1]);
	float max0 = max(p1BoundsMax[0],sBoundsMax[0]);
	float max1 = max(p1BoundsMax[1],sBoundsMax[1]);

	//intersect with old aabb
	Vec2f ProjBottom = v3DTov2D(ab1.bottomCorner,axis);
	Vec2f ProjTop = v3DTov2D(ab1.topCorner,axis);
	min0 = max(min0, ProjBottom[0]);
	min1 = max(min1, ProjBottom[1]);
	max0 = min(max0, ProjTop[0]);
	max1 = min(max1, ProjTop[1]);

	switch(axis){ //Layout : splix (y,z), spliy (x,z), spliz (x,y)
	case 0:
		ab1.bottomCorner[1] = min0;
		ab1.bottomCorner[2] = min1;
		ab1.topCorner[1] = max0;
		ab1.topCorner[2] = max1;
		break;
	case 1:
		ab1.bottomCorner[0] = min0;
		ab1.bottomCorner[2] = min1;
		ab1.topCorner[0] = max0;
		ab1.topCorner[2] = max1;
		break;
	case 2:
		ab1.bottomCorner[0] = min0;
		ab1.bottomCorner[1] = min1;
		ab1.topCorner[0] = max0;
		ab1.topCorner[1] = max1;
		break;
	}

	//Clib ab2
	//Projection of P2 on the plane
	Vec2f p2 = v3DTov2D(P2,axis);
	//Bounds of the projection of p2
	Vec2f p2BoundsMin;
	Vec2f p2BoundsMax;
	if( P2[axis] < sortP1[axis]){ //Projection of the sphere
		sx = sy = sqrtf(sqr<float>(tube->r1)-sqr<float>(P2[axis] - sortP1[axis]));
	}else if(P2[axis] < sortP2[axis]){ //Section of the intersection
		sx = tube->r1;//abs(MetaTube->r1 / dntn);//wrong
		sy = tube->r1;//abs(MetaTube->r1 / dntn);//wrond
	}else if(P2[axis] < sortP2[axis] + tube->r2){
		sx = sy = sqrtf(sqr<float>(tube->r2)-sqr<float>(P2[axis] - sortP2[axis]));
	}else{
		sx = tube->r1;//abs(MetaTube->r1 / dntn);//wrong
		sy = tube->r1;//abs(MetaTube->r1 / dntn);//wrond
	}
	p2BoundsMin = p2 - Vec2f(sx,sy);//Vec2f(sx,sy);
	p2BoundsMax = p2 + Vec2f(sx,sy);//Vec2f(sx,sy);
	//Bounds of the two bounds
	min0 = min(p2BoundsMin[0],sBoundsMin[0]);
	min1 = min(p2BoundsMin[1],sBoundsMin[1]);
	max0 = max(p2BoundsMax[0],sBoundsMax[0]);
	max1 = max(p2BoundsMax[1],sBoundsMax[1]);

	//intersect with old aabb
	ProjBottom = v3DTov2D(ab2.bottomCorner,axis);
	ProjTop = v3DTov2D(ab2.topCorner,axis);
	min0 = max(min0, ProjBottom[0]);
	min1 = max(min1, ProjBottom[1]);
	max0 = min(max0, ProjTop[0]);
	max1 = min(max1, ProjTop[1]);


	switch(axis){ //Layout : splix (y,z), spliy (x,z), spliz (x,y)
		case 0:
			ab2.bottomCorner[1] = min0;
			ab2.bottomCorner[2] = min1;
			ab2.topCorner[1] = max0;
			ab2.topCorner[2] = max1;
			break;
		case 1:
			ab2.bottomCorner[0] = min0;
			ab2.bottomCorner[2] = min1;
			ab2.topCorner[0] = max0;
			ab2.topCorner[2] = max1;
			break;
		case 2:
			ab2.bottomCorner[0] = min0;
			ab2.bottomCorner[1] = min1;
			ab2.topCorner[0] = max0;
			ab2.topCorner[1] = max1;
			break;
	}
}



void preprocessAABBsESC(vector<AABB>& aabbs){
	list<AABB> ablist;
	ablist.insert(ablist.begin(),aabbs.begin(),aabbs.end());
	cout<<aabbs.size()<<endl;

	const float MINAABBSIZE = 0.01f;

	
	int i = 0;
	bool done = false;

	while(!done && ablist.size() <= MAXAABBCOUNT){
		done = true;
		list<AABB>::iterator it = ablist.begin();
		for(; it != ablist.end(); ++it){
			cout<<"curent "<<i<<" all "<<ablist.size()<<endl;
			//Decide in which direction to plit
			Vec3f size;
			size[0] = it->topCorner[0]-it->bottomCorner[0];
			size[1] = it->topCorner[1]-it->bottomCorner[1];
			size[2] = it->topCorner[2]-it->bottomCorner[2];

			int axis = 0;
			if( size[0] < size[1]){
				if(size[1] < size[2]){
					axis = 2;//splitz 
				}else{
					axis = 1;//splity
				}
			}else if(size[0] < size[2]){
				axis = 2;//splitz
			}else{
				axis = 0;//splitx
			}
			//axis = 1;
			//If box to big split it
			cout<<size[axis]<<endl;
			if(size[axis] > MINAABBSIZE){
				done = false;
				AABB ab2;

				splitESCTest(axis,*it,ab2);

				list<AABB>::const_iterator cit(it);
				++cit;
				it = ablist.insert(cit,ab2);
			}
		}
	}
/*
	while(it != ablist.end()&&int(ablist.size())<MAXAABBCOUNT){

		cout<<"curent "<<i<<" all "<<ablist.size()<<endl;
		//Decide in which direction to plit
		Vec3f size;
		size[0] = it->topCorner[0]-it->bottomCorner[0];
		size[1] = it->topCorner[1]-it->bottomCorner[1];
		size[2] = it->topCorner[2]-it->bottomCorner[2];

		int axis = 0;
		if( size[0] < size[1]){
			if(size[1] < size[2]){
				axis = 2;//splitz 
			}else{
				axis = 1;//splity
			}
		}else if(size[0] < size[2]){
			axis = 2;//splitz
		}else{
			axis = 0;//splitx
		}
		//axis = 1;
		//If box to big split it
		if(size[axis] > MINAABBSIZE){
			AABB ab2;

			cout<<"start split"<<endl;
			splitESCTest(axis,*it,ab2);
			cout<<"end split"<<endl;

			list<AABB>::const_iterator cit(it);
			++cit;
			it = ablist.insert(cit,ab2);

		}else{
			i++;
			++it;
		}
	}
	*/
	aabbs.clear();
	aabbs.reserve(ablist.size());
	aabbs.insert(aabbs.begin(),ablist.begin(),ablist.end());
	cout<<"aabbs size after preprocess "<<aabbs.size()<<endl;
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

	displayText.addFramerate();
	displayText.addText("\n\n");
	IEditor::modeTextId = displayText.addText("View mode");
	displayText.addText("\n\n");

	Vec3f c(0, 0, 0);
    float r = 1;
    view_ctrl = new GLViewController(WINX, WINY, c, 1.5*r);
	_near = 1.5f*0.01f;

	animation = new Animation();

	//*********************************Create the editors
	mainEditor = new MainEditor(bones);
	skeletonEditor = new SkeletonEditor(bones);
	surfEditor = new SurfaceEditor(bones);
	animEditor = new AnimationEditor(bones); 
	texEditor = new TextureEditor(bones);

	editors.push_back(mainEditor);
	editors.push_back(skeletonEditor);
	mainEditor->addEditor(skeletonEditor);
	editors.push_back(surfEditor);
	mainEditor->addEditor(surfEditor);
	editors.push_back(texEditor);
	mainEditor->addEditor(texEditor);
	editors.push_back(animEditor);
	mainEditor->addEditor(animEditor);

	animEditor->animationPlayer = animation;

	for(unsigned int i = 0; i < editors.size(); i++){
		//Set display and control objects pointers
		editors[i]->textDisplayer = &displayText;
		editors[i]->view_ctrl = view_ctrl;
		//Set data structures pointers
		editors[i]->implicitSurface = &implicitFuntion;
		editors[i]->rootSkeleton = &skeleton;
		editors[i]->initialize();
	}
	cmpBvh = new CompactBvh();
	mainEditor->bvh = cmpBvh;
	string name("Spine.skl");
	mainEditor->loadAndInitializeSkeleton(&skeleton, &implicitFuntion,name);
	mainEditor->openModel(&skeleton,name);
	
	currentEditor = 0;
	mainEditor->open();

	for(int i = 0; i < implicitFuntion.p1s.size() ; i++){
		boneIds[i] = implicitFuntion.primitives[i]->skeleton->id;
	}
	

	//*********************************Create the data structures for debugging and rendering
	vector<AABBLite> aabbsl;
	for(unsigned int i = 0; i < implicitFuntion.primitives.size(); i++){
		AABBLite ab;
		ab.bottomCorner = implicitFuntion.primitives[i]->aabb.bottomCorner;
		ab.topCorner = implicitFuntion.primitives[i]->aabb.topCorner;
		ab.primitive = implicitFuntion.primitives[i];
		aabbsl.push_back(ab);
	}
	if(aabbsl.size()>0)
	cmpBvh->initializeSplit(0,aabbsl);
	cmpBvh->print();


	
	testSectionTube = (MetaTube*)implicitFuntion.primitives[0];
	bigAABB.bottomCorner = Vec3f(0); 
	bigAABB.topCorner = Vec3f(0.5f); 
	/*
	sectionPlan.axis = 0;
	sectionPlan.position = testSectionTube->worldP1[0];
	getMetaTubeSectionBounds(testSectionTube,sectionPlan.position,sectionPlan.axis,tubeSection);
	*/

	for(int i = 0; i < implicitFuntion.primitives.size(); i++){
		cout<<boneIds[i]<<endl;
	}


	restWPos[0] = Vec3f(0,0,0);
	restWPos[1] = Vec3f(0,0,0.285f);
	restWPos[2] = Vec3f(0,0,0.285f+0.2565f);
	restWPos[2] = Vec3f(0,0,0.285f+0.2565f+0.219f);
	

	
	glewInit();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClearColor(1.0f, 1.0f, 1.0f, 0.f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
    glEnable(GL_NORMALIZE);


	BVH_IMAGE_SIZE = nextPowerOf2(3*powf(2,MAX_BVH_D+1)-1);
	cout<<"BVH_IMAGE_SIZE "<<endl;
	//BVH_IMAGE_WIDTH = nextPowerOf2(sqrtf(BVH_IMAGE_SIZE));
	//BVH_IMAGE_HEIGHT = BVH_IMAGE_SIZE/BVH_IMAGE_WIDTH;
	cout<<"Packed as :"<<endl;
	cout<<"BVH_IMAGE_WIDTH "<<endl;
	cout<<"BVH_IMAGE_HEIGHT "<<endl;

	//INIT bvh texture
	bvhTexData = new Vec4f[cmpBvh->dataSize];
	for(int i = 0; i < cmpBvh->dataSize;i++)
		bvhTexData[i] = Vec4f(-1);

	
	checkBoardTexId = SOIL_load_OGL_texture(string("Textures/CheckBoard.jpg").data(), 0, 0, SOIL_LOAD_RGB|SOIL_FLAG_INVERT_Y|SOIL_FLAG_MIPMAPS);

#ifdef USEBVH
    // create 2 pixel buffer objects, you need to delete them when program exits.
    //glBufferDataARB with NULL pointer reserves only memory space.
    glGenBuffers(1, &bvhPboIds[0]);
    glBindBuffer(GL_TEXTURE_BUFFER_EXT, bvhPboIds[0]);
	glBufferData(GL_TEXTURE_BUFFER_EXT, 2*BVH_IMAGE_SIZE*sizeof(Vec4f), 0, GL_DYNAMIC_DRAW);
	// init 2 texture objects
	cout<<"Intialize BVH texture"<<endl;
	glGenTextures(1, &bvhTexId);
    glBindTexture(GL_TEXTURE_BUFFER_EXT, bvhTexId);
	glTexBufferEXT(GL_TEXTURE_BUFFER_EXT, BVH_PIXEL_FORMAT, bvhPboIds[0]);
	glBindTexture(GL_TEXTURE_BUFFER_EXT, 0);
	cout<<"Intialize BVH texture done"<<endl;
#endif
	
    glutMainLoop();
	cout<<"exit"<<endl;

	_CrtDumpMemoryLeaks();

	return 0;
}
