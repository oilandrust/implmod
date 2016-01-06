#ifndef SURFACEEDITOR_H
#define SURFACEEDITOR_H

#include "IEditor.h"
#include <vector>

class Skeleton;
class SkeletalImplicit;
class MetaPrimitive;

using namespace std;
//using namespace CGLA;

class SurfaceEditor : public IEditor{
	private:
		bool movingPrimitive;
		float radiusOfNewBall;
		Vec3f scaleOldPos;
		int sideToScale;
		static const float minRadius;
		bool snap;

	public:
		MetaPrimitive* selectedPrimitive;

	public:
		SurfaceEditor(vector<Skeleton*>& boneList);
		virtual ~SurfaceEditor();

		virtual void initialize();
		virtual void open();
		virtual void close();

		virtual void openModel(Skeleton* model, const string& ressourceName);
		virtual void saveModel(Skeleton* model, const string& ressourceName);;
		virtual void dropModel();

		virtual bool onMouseMotion(int x, int y);
		virtual bool onMouseButton(int button, int state, int x, int y);
		virtual void onKeyboard(unsigned char key, int x, int y);

		void addNewSphereTo(Skeleton* parent);

		void setSpherePosition(int x, int y);

		void scalePrimitive(int x, int dy);
		void translatePrimitive(int x, int dy, bool snap = false);

};

#endif