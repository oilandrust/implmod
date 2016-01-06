#ifndef DRAWUTILS_H
#define DRAWUTILS_H

#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <CGLA/Vec3f.h>

using namespace CGLA;

void drawCircle(const Vec3f& center, float radius);
void drawAxis(double scale);
void drawBox(const Vec3f& bottom, const Vec3f& top);
void drawFullBox(const Vec3f& bottom, const Vec3f& top);

#endif