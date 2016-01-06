
#include <stdlib.h>

#include <GL/glew.h>
#include <GLGraphics/gel_glut.h>

#include <iostream>
#include <CGLA/eigensolution.h>
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

#include <LinAlg/Matrix.h>
#include <LinAlg/Vector.h>
#include <LinAlg/LapackFunc.h>

#include <math.h>

using namespace std;
using namespace HMesh;
using namespace Geometry;
using namespace GLGraphics;
using namespace Util;
using namespace LinAlg;
using namespace CGLA;


namespace
{
	const string shader_path = "shaders";
	GLViewController *view_ctrl;       // This is the class used to help us navigate with the mouse (virtual trackball)
	int WINX = 800, WINY = 800;        // Predefine window dimensions
	int do_wire = false;			   // Whether or not to use wireframe
	bool create_display_list = true;   // used to check whether we need to redo the display list (for efficient rendering).
	float _near;
	bool drawSphere;
	GLUquadricObj* debug_sphere;
	bool reloadShader = true;
	GLuint permTextureID;
};

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

void drawAxis(double scale)
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushMatrix();
	glUseProgram(0);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_LINE_SMOOTH);
    glLineWidth(2);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glScaled(scale,scale,scale);
    glBegin(GL_LINES);
    glColor3ub(255,0,0);
    glVertex3i(0,0,0);
    glVertex3i(1,0,0);
    glColor3ub(0,255,0);
    glVertex3i(0,0,0);
    glVertex3i(0,1,0);
    glColor3ub(0,0,255);
    glVertex3i(0,0,0);
    glVertex3i(0,0,1);
    glEnd();
    glPopMatrix();
    glPopAttrib();
	glEnable(GL_DEPTH_TEST);
}

void drawCube(){
	glBegin(GL_QUAD_STRIP);
	glVertex3f(-1,1,-1);
	glVertex3f(-1,1,1);
	glVertex3f(1,1,-1);
	glVertex3f(1,1,1);

	glVertex3f(1,-1,-1);
	glVertex3f(1,-1,1);

	glVertex3f(-1,-1,-1);
	glVertex3f(-1,-1,1);

	glVertex3f(-1,1,-1);
	glVertex3f(-1,1,1);
	glEnd();

	glBegin(GL_QUADS);
	glVertex3f(-1,-1,-1);
	glVertex3f(-1,1,-1);
	glVertex3f(1,1,-1);
	glVertex3f(1,-1,-1);

	glVertex3f(-1,-1,1);
	glVertex3f(1,-1,1);
	glVertex3f(1,1,1);
	glVertex3f(-1,1,1);

	glEnd();
}

void enableRayTrace()
{
	static GLuint prog_trace = 0;
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
	}
	Vec3f cameraPos;
	view_ctrl->get_view_param(cameraPos,Vec3f(),Vec3f());
	glUseProgram(prog_trace);
	glUniform3fv(glGetUniformLocation(prog_trace,"cameraPos"), 1, cameraPos.get());
	glUniform1i(glGetUniformLocation(prog_trace,"permTexture"), 0);
	static float time = 0;
	time += 1;
	glUniform1f(glGetUniformLocation(prog_trace,"time"), time);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, permTextureID);
	
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPushMatrix();	
    view_ctrl->set_gl_modelview();

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glColor3f(1,1,1);
	enableRayTrace();
	drawCube();

    if(do_wire) {
		glUseProgram(0);
        glDepthFunc(GL_LEQUAL);
        glEnable(GL_POLYGON_OFFSET_LINE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glColor3f(1, 0, 0);
        glPolygonOffset(-2, -2);

		if(drawSphere){
			glDisable(GL_DEPTH_TEST);
			glUseProgram(0);
			gluSphere(debug_sphere,1,20,20);
			glEnable(GL_DEPTH_TEST);
		}
		drawCube();	

        glPolygonOffset(0, 0);
        glDisable(GL_POLYGON_OFFSET_LINE);
        glDepthFunc(GL_LESS);
        glPolygonMode(GL_FRONT, GL_FILL);
	}
/*
	glDisable(GL_DEPTH_TEST);
	drawAxis(1);
	glEnable(GL_DEPTH_TEST);
*/	
    glPopMatrix();
    glutSwapBuffers();
}

void animate()
{
    view_ctrl->try_spin();
    glutPostRedisplay();
}


void reshape(int W, int H)
{
    view_ctrl->reshape(W, H);
}


void mouse(int button, int state, int x, int y)
{
    Vec2i pos(x, y);
    if(state == GLUT_DOWN) {
        if(button == GLUT_LEFT_BUTTON)
            view_ctrl->grab_ball(ROTATE_ACTION, pos);
        else if(button == GLUT_MIDDLE_BUTTON)
            view_ctrl->grab_ball(ZOOM_ACTION, pos);
		else if(button == GLUT_RIGHT_BUTTON){
			view_ctrl->grab_ball(PAN_ACTION, pos);
		}
    }
    else if(state == GLUT_UP)
        view_ctrl->release_ball();
}

void motion(int x, int y)
{
    Vec2i pos(x, y);
    view_ctrl->roll_ball(Vec2i(x, y));
}


void keyboard(unsigned char key, int x, int y)
{
    switch (key) {
		case '\033':
			cout << "Bye" << endl;
			exit(0);
			break;
		case 'l':
			cout << "Showing lines" << endl;
			do_wire = !do_wire;
			break;
		case 'r':
			reloadShader = true;		
			break;
		case 's':
			drawSphere = !drawSphere;		
			break;

    }
    create_display_list = true;
}

void initPermTexture(GLuint *texID)
{
  char *pixels;
  int i,j;
  
  glGenTextures(1, texID); // Generate a unique texture ID
  glBindTexture(GL_TEXTURE_2D, *texID); // Bind the texture to texture unit 0

  pixels = (char*)malloc( 256*256*4 );
  for(i = 0; i<256; i++)
    for(j = 0; j<256; j++) {
      int offset = (i*256+j)*4;
      char value = perm[(j+perm[i]) & 0xFF];
      pixels[offset] = grad3[value & 0x0F][0] * 64 + 64;   // Gradient x
      pixels[offset+1] = grad3[value & 0x0F][1] * 64 + 64; // Gradient y
      pixels[offset+2] = grad3[value & 0x0F][2] * 64 + 64; // Gradient z
      pixels[offset+3] = value;                     // Permuted index
    }
  
  // GLFW texture loading functions won't work here - we need GL_NEAREST lookup.
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
}


int main(int argc, char **argv)
{
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(WINX, WINY);
    glutInit(&argc, argv);
    glutCreateWindow("RayTraceImplicit");
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutIdleFunc(animate);

    Vec3f c(0, 0, 0);
    float r = 1;
    view_ctrl = new GLViewController(WINX, WINY, c, 1.5*r);
	_near = 1.5f*0.01f;

	debug_sphere = gluNewQuadric();
	
	glewInit();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClearColor(1.0f, 1.0f, 1.0f, 0.f);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    //glEnable(GL_LIGHT0);
    //glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
    //glEnable(GL_NORMALIZE);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	initPermTexture(&permTextureID);
	
    glutMainLoop();
}
