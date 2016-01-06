#include "DrawUtils.h"

void drawCircle(const Vec3f& center, float radius){
	glBegin(GL_LINE_LOOP);
	float angle;
	for(int i = 0; i < 60; i++){
		angle = 12*i*M_PI/360;
		glVertex3fv( (center + radius*Vec3f(cosf(angle),sin(angle),0)).get() );
	}
	glEnd();
}

void drawAxis(double scale)
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushMatrix();
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

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
}

void drawFullBox(const Vec3f& bottom, const Vec3f& top){
	//front
	Vec3f bottomFrontRight = Vec3f(bottom[0],top[1],bottom[2]);
	Vec3f topFrontRight = Vec3f(bottom[0],top[1],top[2]);
	Vec3f topFrontLeft = Vec3f(bottom[0],bottom[1],top[2]);
	//back
	Vec3f bottomBackLeft = Vec3f(top[0],bottom[1],bottom[2]);
	Vec3f bottomBackRight = Vec3f(top[0],top[1],bottom[2]);
	Vec3f topBackLeft = Vec3f(top[0],bottom[1],top[2]);

	glBegin(GL_QUAD_STRIP);

		glVertex3f(bottom[0],top[1],bottom[2]);
		glVertex3f(bottom[0],top[1],top[2]);
		glVertex3f(top[0],top[1],bottom[2]);
		glVertex3f(top[0],top[1],top[2]);

		glVertex3f(top[0],bottom[1],bottom[2]);
		glVertex3f(top[0],bottom[1],top[2]);

		glVertex3f(bottom[0],bottom[1],bottom[2]);
		glVertex3f(bottom[0],bottom[1],top[2]);

		glVertex3f(bottom[0],top[1],bottom[2]);
		glVertex3f(bottom[0],top[1],top[2]);
	glEnd();

	glBegin(GL_QUADS);

		glVertex3f(bottom[0],bottom[1],bottom[2]);
		glVertex3f(bottom[0],top[1],bottom[2]);
		glVertex3f(top[0],top[1],bottom[2]);
		glVertex3f(top[0],bottom[1],bottom[2]);

		glVertex3f(bottom[0],bottom[1],top[2]);
		glVertex3f(top[0],bottom[1],top[2]);
		glVertex3f(top[0],top[1],top[2]);
		glVertex3f(bottom[0],top[1],top[2]);


	glEnd();

}

void drawBox(const Vec3f& bottom, const Vec3f& top){
	//front
	Vec3f bottomFrontRight = Vec3f(bottom[0],top[1],bottom[2]);
	Vec3f topFrontRight = Vec3f(bottom[0],top[1],top[2]);
	Vec3f topFrontLeft = Vec3f(bottom[0],bottom[1],top[2]);
	//back
	Vec3f bottomBackLeft = Vec3f(top[0],bottom[1],bottom[2]);
	Vec3f bottomBackRight = Vec3f(top[0],top[1],bottom[2]);
	Vec3f topBackLeft = Vec3f(top[0],bottom[1],top[2]);

	glBegin(GL_QUAD_STRIP);
	glVertex3fv( bottom.get() );
	glVertex3fv( bottomFrontRight.get() );
	glVertex3fv( topFrontLeft.get() );
	glVertex3fv( topFrontRight.get() );
	
	glVertex3fv(topBackLeft.get());
	glVertex3fv(top.get());

	glVertex3fv(bottomBackLeft.get());
	glVertex3fv(bottomBackRight.get());

	glVertex3fv(bottom.get());
	glVertex3fv(bottomFrontRight.get());
	glEnd();

}

