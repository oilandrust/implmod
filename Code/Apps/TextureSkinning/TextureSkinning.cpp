

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

#include <SklImp/Skeleton.h>
#include <SklImp/MetaPrimitive.h>
#include <SklImp/Metaball.h>
#include <SklImp/MetaTube.h>
#include <SklImp/BVH.h>
#include <SklImp/BvhUtils.h>
#include <SklImp/SkeletalImplicit.h>
#include <SklImp/Animation.h>
#include <SklImp/TextureProjector.h>
#include <SklImp/SkeletalImplicitModel.h>

#include <Utils/RayCastUtils.h>
#include <Utils/DrawUtils.h>

#include <UI/DisplayText.h>
#include <UI/Input.h>
#include <UI/IEditor.h>
#include <UI/SkeletonEditor.h>
#include <UI/SurfaceEditor.h>
#include <UI/TextureEditor.h>
#include <UI/AnimationEditor.h>
#include <UI/MainEditor.h>

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

int FREQUENCY = 1000; //For FPS sampling



namespace
{
	int MAXAABBCOUNT = 50;
	//VIEW PARAMETERS
	GLViewController *view_ctrl;       // This is the class used to help us navigate with the mouse (virtual trackball)
	float _near;
	int WINX = 800, WINY = 800;        // Predefine window dimensions

	bool playAnimation = false;
	//DATA STRUCTURES
	SkeletalImplicit implicitFuntion;  //The implicit function attached to the skeleton
	Skeleton skeleton;                 //The skeleton
	vector<Skeleton*> bones; 

	Animation* animation = 0;


	int boneIds[100];
	Mat4x4f boneInvTrans[25];
	Mat4x4f boneRestTrans[20];

	GLuint checkBoardTexId =0;
				

	int oldx = 0;
	int oldy = 0;
	
	//SHADER MANAGEMENT
	GLuint prog_trace = 0;
	GLuint prog_traceRaster = 0;
	const string shader_path = "shaders";
	bool reloadShader = true;
	GLuint boneMatAttibId = 0;
	GLuint boneIdAttribId = 0;



	//NOISE FUNTION UTILS
	bool doHyperTexture = false;
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

void enableRayTraceNoBvh(){
	static GLuint vs = 0;
	static GLuint fs = 0;
	if(reloadShader)
	{
		reloadShader = false;
		
		glDeleteProgram(prog_traceRaster);
		glDeleteShader(vs);
		glDeleteShader(fs);
		
		prog_traceRaster = glCreateProgram();
		
		vs = create_glsl_shader(GL_VERTEX_SHADER, shader_path, "TextureSkinning.vert");
		fs = create_glsl_shader(GL_FRAGMENT_SHADER, shader_path, "TextureSkinning.frag");
		
		if(vs) glAttachShader(prog_traceRaster, vs);
		if(fs) glAttachShader(prog_traceRaster, fs);

		glLinkProgram(prog_traceRaster);
		print_glsl_program_log(prog_traceRaster);
	}
	Vec3f cameraPos;
	view_ctrl->get_view_param(cameraPos,Vec3f(),Vec3f());

	glUseProgram(prog_traceRaster);

	glUniform3fv(glGetUniformLocation(prog_traceRaster,"cameraPos"), 1, cameraPos.get());
	glUniform1i(glGetUniformLocation(prog_traceRaster,"checkTexture"), 1);
	glUniform1i(glGetUniformLocation(prog_traceRaster,"permTexture"), 2);
	glUniform1i(glGetUniformLocation(prog_traceRaster,"doHyperTexture"), doHyperTexture?1:0);

	//PRIMITIVES
	glUniform1i(glGetUniformLocation(prog_traceRaster,"nbOfMetaTubes"), implicitFuntion.p1s.size());
	glUniform1i(glGetUniformLocation(prog_traceRaster,"nbOfBalls"), implicitFuntion.spheres.size());
	glUniform3fv(glGetUniformLocation(prog_traceRaster,"p1s"), implicitFuntion.p1s.size(), implicitFuntion.p1s[0].get());
	glUniform3fv(glGetUniformLocation(prog_traceRaster,"p1s")+implicitFuntion.p1s.size(), implicitFuntion.spheres.size(), implicitFuntion.spheres[0].get());
	glUniform3fv(glGetUniformLocation(prog_traceRaster,"p2s"), implicitFuntion.p2s.size(), implicitFuntion.p2s[0].get());
	glUniform1fv(glGetUniformLocation(prog_traceRaster,"radius"), implicitFuntion.radiuses.size(), &implicitFuntion.radiuses[0]);
	glUniform1fv(glGetUniformLocation(prog_traceRaster,"radius")+implicitFuntion.radiuses.size(), implicitFuntion.sRadius.size(), &implicitFuntion.sRadius[0]);

	glUniform1iv(glGetUniformLocation(prog_traceRaster,"boneIds"), implicitFuntion.p1s.size()+implicitFuntion.spheres.size(), &boneIds[0]);
	// SKINNING TRANSFORMS
	size_t nbOfBones = bones.size();
	for(size_t i = 0 ; i < nbOfBones-1; i++){
		boneInvTrans[i] = bones[i+1]->worldOrientation.inverse().get_Mat4x4f();
		boneInvTrans[i][3] = Vec4f(-bones[i+1]->worldPosition,1);
	}
	glUniformMatrix4fv(glGetUniformLocation(prog_traceRaster,"boneInvTrans"), bones.size(), false, boneInvTrans[0].get());
	glUniformMatrix4fv(glGetUniformLocation(prog_traceRaster,"boneRestTrans"), bones.size(), false, boneRestTrans[0].get());

	// TEXTURE PROJECTORS
	vector<Vec3f> proj1s;
	vector<Vec3f> proj2s;
	vector<Vec3f> orthos;
	vector<float> rads;
	
	proj1s.push_back(Vec3f(0));
	proj2s.push_back(Vec3f(0,0,1.5f));
	orthos.push_back(Vec3f(1,0,0));
	rads.push_back(0.5f);
	
	glUniform3fv(glGetUniformLocation(prog_traceRaster,"proj1s"), proj1s.size(), proj1s[0].get());
	glUniform3fv(glGetUniformLocation(prog_traceRaster,"proj2s"), proj1s.size(), proj2s[0].get());
	glUniform3fv(glGetUniformLocation(prog_traceRaster,"orthos"), proj1s.size(), orthos[0].get());
	glUniform1fv(glGetUniformLocation(prog_traceRaster,"rads"), proj1s.size(), &rads[0]);
	glUniform1i(glGetUniformLocation(prog_traceRaster,"nbProjectors"), 1);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, checkBoardTexId);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, permTextureID);
}



void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPushMatrix();	
    view_ctrl->set_gl_modelview();


	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);


	/*********UPDATE THE BVH*********/
	vector<AABBLite> aabbsl;
	aabbsl.reserve(implicitFuntion.primitives.size());
	for(size_t i = 0; i < implicitFuntion.primitives.size(); i++){
		AABBLite ab;
		ab.bottomCorner = implicitFuntion.primitives[i]->aabb.bottomCorner;
		ab.topCorner = implicitFuntion.primitives[i]->aabb.topCorner;
		aabbsl.push_back(ab);
	}
	AABBLite globalAABB = aabbOfaabbs(aabbsl);
	/*****************************/
	enableRayTraceNoBvh();
	drawFullBox(globalAABB.bottomCorner,globalAABB.topCorner);
	glUseProgram(0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_BUFFER_EXT, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);


    glPopMatrix();

    glutSwapBuffers();
}

void animate()
{
	if(playAnimation)
		animation->animate(glutGet(GLUT_ELAPSED_TIME));

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
		case 'h':
			doHyperTexture = !doHyperTexture;
			break;
		case 'r':
				reloadShader = true;
			break;
		case KEY_SPACE:
			playAnimation =! playAnimation;
			break;
    }
}
  
void keyboardUp(unsigned char key, int x, int y)
{
	Input::keyStates[key] = false;
}
void specialKeyboard(int key, int x, int y)
{

}


void initializeSkeleton(Skeleton* outSkel, SkeletalImplicit* implicit){
	
	cout<<"Extracting bones"<<endl;
	/****************************READ SKELETON TO EXTRACT ALL BONES***********/
	Skeleton* node;
	vector<Skeleton*> openList;
	openList.push_back(outSkel);
	
	while( !openList.empty() ){	
		node = openList.back();
		openList.pop_back();
		
		if(node){
			bones.push_back(node);
			node->id = bones.size()-1;
		}
		for(size_t i = 0; i < node->children.size(); i++){
			openList.push_back(node->children[i]);				
		}
	}

	//Initialize the implicit function
	cout<<"Initializing implicit function"<<endl;
	implicit->readSkeleton(outSkel);
}

void loadAndInitializeSkeleton(Skeleton* outSkel, SkeletalImplicit* implicit, const string& filename){
	std::ifstream ifs;
	ifs.open(("Models/"+filename).c_str(),ios::in);
	if(ifs.fail()){
		cout << "Error: Unable to open "<<filename<<endl;
		return;
	}

	cout<<"loading model "<<filename<<endl;
	load(*outSkel,ifs);

	
	initializeSkeleton(outSkel,implicit);

	ifs.close();
	cout<<"model loaded"<<endl;
}

void loadAnimation(const string& filename){
	std::ifstream ifs;
	string bModelName = filename.substr(0,filename.find("."));
	ifs.open(("Models/"+bModelName+".anim").c_str(),ios::in);
	if(!ifs.fail()){
		load(*animation,ifs);
	}else{
		cout << "Error: Unable to open animation "<<bModelName<<".anim"<<endl;
		return;
	}

	AnimationClip* clip = animation->getClip(0);
	clip->initialize(bones);
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


	//*********************************Create the input and control objects
	Input::keyStates.insert(pair<unsigned char,bool>('r',false));
	Input::keyStates.insert(pair<unsigned char,bool>('s',false));
	Input::keyStates.insert(pair<unsigned char,bool>('t',false));

	Vec3f c(0, 0, 0);
    float r = 1;
    view_ctrl = new GLViewController(WINX, WINY, c, 1.5*r);
	_near = 1.5f*0.01f;

	animation = new Animation();
	loadAndInitializeSkeleton(&skeleton,&implicitFuntion,"AnimatedSpine.skl");
	loadAnimation("AnimatedSpine.skl");
	animation->play();


	for(size_t i = 0; i < implicitFuntion.p1s.size() ; i++){
		boneIds[i] = implicitFuntion.primitives[i]->skeleton->id;
	}

	for(size_t i = 0; i < bones.size()-1; i++){
		boneRestTrans[i] = bones[i+1]->worldOrientation.get_Mat4x4f();
		boneRestTrans[i][3] = Vec4f(bones[i+1]->worldPosition,1);
	}
	
	
	glewInit();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClearColor(1.0f, 1.0f, 1.0f, 0.f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
    glEnable(GL_NORMALIZE);
	
	checkBoardTexId = SOIL_load_OGL_texture(string("Textures/CheckBoard.jpg").data(), 0, 0, SOIL_LOAD_RGB|SOIL_FLAG_INVERT_Y|SOIL_FLAG_MIPMAPS);

	
    glutMainLoop();
	cout<<"exit"<<endl;

	_CrtDumpMemoryLeaks();

	return 0;
}
