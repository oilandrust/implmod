

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
#include <HMesh/obj_save.h>
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

//#define USEBVH



int FREQUENCY = 1000; //For FPS sampling


enum EditorMode{
	VIEW,
	IMPLICIT,
	MAXMODE
};


namespace
{
	//VIEW PARAMETERS
	GLViewController *view_ctrl;       // This is the class used to help us navigate with the mouse (virtual trackball)
	float _near;
	int WINX = 800, WINY = 800;        // Predefine window dimensions
	DisplayText displayText;


	//DATA STRUCTURES
	Skeleton skeleton;                 //The skeleton
	vector<Skeleton*> bones;           //separate list of all the bones
	SkeletalImplicit implicitFuntion;  //The implicit function attached to the skeleton
	AABBLite globalAABB;
	Manifold mani;                     //The mesh tesselated from the skeletal implicit function

	GLuint checkBoardTexId =0;
				
	//INPUT HANDLING MODE
	vector<IEditor*> editors;
	MainEditor* mainEditor;
	SurfaceEditor* surfEditor;
	int currentEditor = 0;

	int editMode = VIEW;

	int oldx = 0;
	int oldy = 0;

	//DISPLAY OPTIONS
	int do_wire = false;			   // Whether or not to use wireframe
	bool create_display_list = true;   // used to check whether we need to redo the display list (for efficient rendering).
	bool showTesselatedMesh = false;
	bool drawWordref = true;
	bool renderRaytraced = true;
	
	//SHADER MANAGEMENT
	GLuint prog_traceRaster = 0;
	const string shader_path = "shaders";
	bool reloadShader = true;
	bool canRayTrace = true;
	
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
bool CompileSuccessful(int obj) {
	int status;
	glGetShaderiv(obj, GL_COMPILE_STATUS, &status);
	return status == GL_TRUE;
}

bool LinkSuccessful(int obj) {
	int status;
	glGetProgramiv(obj, GL_LINK_STATUS, &status);
	return status == GL_TRUE;
}

void cleanUp(){
	delete view_ctrl;
	delete mainEditor;
	for(size_t i = 0 ; i < editors.size(); i++)
		delete editors[i];

	_CrtDumpMemoryLeaks();

}

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
		
		vs = create_glsl_shader(GL_VERTEX_SHADER, shader_path, "raytraceBall.vert");
		fs = create_glsl_shader(GL_FRAGMENT_SHADER, shader_path, "raytraceBall.frag");
		
		if(CompileSuccessful(vs)) glAttachShader(prog_traceRaster, vs);
		else{
			cout<<"Failed to compile vertex shader"<<endl;
			canRayTrace = false;
		}
		if(CompileSuccessful(fs)) glAttachShader(prog_traceRaster, fs);
		else{
			cout<<"Failed to compile fragment shader"<<endl;
			canRayTrace = false;
		}

		glLinkProgram(prog_traceRaster);
		if(!LinkSuccessful(prog_traceRaster)){
			cout<<"Failed to link shader programs"<<endl;
			canRayTrace = false;
		}else
			canRayTrace = true;

		print_glsl_program_log(prog_traceRaster);
	}
	Vec3f cameraPos;
	view_ctrl->get_view_param(cameraPos,Vec3f(),Vec3f());

	glUseProgram(prog_traceRaster);

	glUniform3fv(glGetUniformLocation(prog_traceRaster,"cameraPos"), 1, cameraPos.get());
	glUniform1i(glGetUniformLocation(prog_traceRaster,"checkTexture"), 1);
	glUniform1i(glGetUniformLocation(prog_traceRaster,"permTexture"), 2);

	//METABALLS
	glUniform1i(glGetUniformLocation(prog_traceRaster,"nbOfBalls"), implicitFuntion.spheres.size());
	glUniform3fv(glGetUniformLocation(prog_traceRaster,"p1s"), implicitFuntion.spheres.size(), implicitFuntion.spheres[0].get());
	glUniform1fv(glGetUniformLocation(prog_traceRaster,"radius"), implicitFuntion.sRadius.size(), &implicitFuntion.sRadius[0]);

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
	glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);

	if(drawWordref)
		drawAxis(1);


	/************************Render Solid Meshes*******************/
	if(showTesselatedMesh){

		glPolygonMode(GL_FRONT, GL_FILL);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glEnable(GL_DEPTH_TEST);

		if(create_display_list){
			glNewList(1, GL_COMPILE);
			draw(mani);
			glEndList();
			create_display_list = false;
		}
		glCallList(1);
	}
	
	/*********************Render WireFrame Meshes on top in red**************/
	glDepthFunc(GL_LEQUAL);
	//glEnable(GL_DEPTH_TEST);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor3f(1, 0, 0);
	glPolygonOffset(-2, -2);

	if(showTesselatedMesh && do_wire)
		glCallList(1);

	glPolygonOffset(0, 0);
	glDisable(GL_POLYGON_OFFSET_LINE);
	glDepthFunc(GL_LESS);
	


	/*********************Render WireFrame GUISMOS**************/
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glColor3f(1,0,0);

	if(editMode == IMPLICIT){
		for(size_t i = 0 ; i < implicitFuntion.primitives.size(); i++){
			if(surfEditor->selectedPrimitive == implicitFuntion.primitives[i])
				glColor4f(1,0,0,1);
			else glColor4f(.7f,.7f,.7f,1);
				implicitFuntion.primitives[i]->renderGuizmo();
		}
	}

	/*********************Render Ray Traced**************/
	if(renderRaytraced && canRayTrace){	
		glPolygonMode(GL_FRONT, GL_FILL);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);	

		enableRayTraceNoBvh();
		drawFullBox(globalAABB.bottomCorner,globalAABB.topCorner);
		
		glUseProgram(0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_BUFFER_EXT, 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	/*****************************/
	


    glPopMatrix();
	displayText.draw();

    glutSwapBuffers();
}

void animate()
{
	implicitFuntion.update();
		//Update the AABB
	vector<AABBLite> aabbsl;
	aabbsl.reserve(implicitFuntion.primitives.size());
	for(size_t i = 0; i < implicitFuntion.primitives.size(); i++){
		AABBLite ab;
		ab.bottomCorner = implicitFuntion.primitives[i]->aabb.bottomCorner;
		ab.topCorner = implicitFuntion.primitives[i]->aabb.topCorner;
		aabbsl.push_back(ab);
	}
	globalAABB = aabbOfaabbs(aabbsl);

    view_ctrl->try_spin();
    glutPostRedisplay();
}


void reshape(int W, int H)
{
    view_ctrl->reshape(W, H);
	IEditor::WINX = W;
	IEditor::WINY = H;
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
	mani.clear();
	implicitFuntion.clear(); 
}
 
void keyboard(unsigned char key, int x, int y)
{
	Input::keyStates[key] = true;
	Vec3f c;

	/******EDITOR INPUT HANDLING*******/
	if((int)key == KEY_TAB){//tab
		editMode = (editMode+1)%MAXMODE;
		editors[currentEditor]->close();
		currentEditor=(currentEditor+1)%editors.size();
		editors[currentEditor]->open();
	}
	editors[currentEditor]->onKeyboard(key,x,y);
	/******EDITOR INPUT HANDLING*******/

   switch (key) {
		case '\033':
			//cleanUp();
			cout << "Bye" << endl;
			exit(0);
			break;
		case 'l':
			cout << "Showing lines" << endl;
			do_wire = !do_wire;
			break;
		case 'm':
			mani.clear();
			implicitFuntion = SkeletalImplicit(&skeleton,0.5f);
			polygonize(mani, implicitFuntion, 50, globalAABB.bottomCorner,globalAABB.topCorner);
			break;
		case 'p':
			showTesselatedMesh = !showTesselatedMesh;
			break;	
		case 'r':
			if(editMode == VIEW)
				reloadShader = true;
			break;
		case 't':
			if(editMode == VIEW){
				renderRaytraced = !renderRaytraced;
			}
			break;
		case 'w':
			drawWordref = !drawWordref;
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
}





int main(int argc, char **argv)
{
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(WINX, WINY);
    glutInit(&argc, argv);
    glutCreateWindow("MetaBallEditor");
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

	displayText.addFramerate();
	displayText.addText("\n\n");
	IEditor::modeTextId = displayText.addText("View mode");
	displayText.addText("\n\n");

	Vec3f c(0, 0, 0);
    float r = 1;
    view_ctrl = new GLViewController(WINX, WINY, c, 1.5*r);
	_near = 1.5f*0.01f;

	//*********************************Create the editors
	mainEditor = new MainEditor(bones);
	mainEditor->mani = &mani;
	surfEditor = new SurfaceEditor(bones);

	editors.push_back(mainEditor);
	editors.push_back(surfEditor);
	mainEditor->addEditor(surfEditor);
	
	for(size_t i = 0; i < editors.size(); i++){
		//Set display and control objects pointers
		editors[i]->textDisplayer = &displayText;
		editors[i]->view_ctrl = view_ctrl;
		//Set data structures pointers
		editors[i]->implicitSurface = &implicitFuntion;
		editors[i]->rootSkeleton = &skeleton;
		editors[i]->initialize();
	}
	
	currentEditor = 0;
	mainEditor->open();
	
	glewInit();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClearColor(1.0f, 1.0f, 1.0f, 0.f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
    glEnable(GL_NORMALIZE);
	glDisable(GL_TEXTURE_2D);

	glutMainLoop();

	cout<<"exit"<<endl;
	_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE );
   _CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDOUT );
   _CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_FILE );
   _CrtSetReportFile( _CRT_ERROR, _CRTDBG_FILE_STDOUT );
   _CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_FILE );
   _CrtSetReportFile( _CRT_ASSERT, _CRTDBG_FILE_STDOUT );

	

	return 0;
}
