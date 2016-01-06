#ifndef RIGEDITOR_H
#define RIGEDITOR_H

#include "IEditor.h"
#include <vector>

class Skeleton;
class SkeletalImplicit;

using namespace std;
//using namespace CGLA;

class RigEditor : public IEditor{
	private:
		bool movingBone;
	public:
		Skeleton* selectedBone;

	public:
		RigEditor(vector<Skeleton*>& boneList);
		virtual ~RigEditor();

		virtual void initialize();
		virtual void open();
		virtual void close();

		virtual void openModel(Skeleton* model, const string& ressourceName);
		virtual void saveModel(Skeleton* model, const string& ressourceName);
		virtual void dropModel();

		virtual bool onMouseMotion(int x, int y);
		virtual bool onMouseButton(int button, int state, int x, int y);
		virtual void onKeyboard(unsigned char key, int x, int y);


		void addNewBoneTo(Skeleton* parent);
		void removeBone(Skeleton* bone);
		Skeleton* selectBoneAt(int x, int y);
		void orientBoneTo(int x, int y);
		void setBoneToTarget(int x, int y);
		void scaleBone(int x, int dy);
		void createEmpty();
};

#endif