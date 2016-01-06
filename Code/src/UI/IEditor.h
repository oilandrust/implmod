#ifndef IEDITOR_H
#define IEDITOR_H

#include <map>
#include <vector>

#include <CGLA/Vec3f.h>
#include <CGLA/Mat3x3f.h>
#include <CGLA/Mat4x4f.h>
#include <GLGraphics/GLViewController.h>
#include <string>

class Skeleton;
class SkeletalImplicit;
class MetaPrimitive;
class DisplayText;
struct MyRay;
struct Mouse;

const std::string modelDirectory = "Models";

using namespace std;
using namespace CGLA;
using namespace GLGraphics;

class IEditor{
	
	public: 
		static int modeTextId;
		static int WINX;
		static int WINY;

		float _near;
		GLViewController *view_ctrl; 
		DisplayText* textDisplayer;

		Skeleton* rootSkeleton;
		SkeletalImplicit* implicitSurface;
		vector<Skeleton*>& bones;

	public:
		IEditor(vector<Skeleton*>& boneList):
			bones(boneList),
			_near(1),
			view_ctrl(0),
			textDisplayer(0),
			rootSkeleton(0),
			implicitSurface(0){
		}
		virtual ~IEditor(void){}

		virtual void initialize() = 0;

		virtual void open() = 0;
		virtual void close() = 0;
		virtual bool onMouseMotion(int x, int y) = 0;
		virtual void openModel(Skeleton* model, const string& ressourceName) = 0;
		virtual void saveModel(Skeleton* model, const string& ressourceName) = 0;
		virtual void dropModel() = 0;
		virtual bool onMouseButton(int button, int state, int x, int y) = 0;
		virtual void onKeyboard(unsigned char key, int x, int y) = 0;

		MyRay getRayFromScreePos(int xScreen, int yScreen);
		Vec3f getPosInViewPlane(int xScreen, int yScreen, Vec3f pointInPlane);

		MetaPrimitive* selectPrimitiveAt(int x,int y, const vector<MetaPrimitive*>& prims);
};

#endif
