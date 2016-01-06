#ifndef ANIMATIONEDITOR_H
#define ANIMATIONEDITOR_H

#include "IEditor.h"
#include <vector>
#include <CGLA/Mat4x4f.h>

class Skeleton;
class SkeletalImplicit;
class Animation;
class AnimationClip;
class KeyFrame;

using namespace std;
//using namespace CGLA;

class AnimationEditor : public IEditor{
	private:
		bool movingBone;
		AnimationClip* clip;
		Quatf* restPose;
		int nbOfBones;
		
		KeyFrame* currentFrame;
		int currentFrameId;
		int keyFrameCountId;
		int selecteBoneId;
	public:
		Skeleton* selectedBone;
		Mat4x4f* restPoseTrans;
	public:
		Animation* animationPlayer;

	public:

		AnimationEditor(vector<Skeleton*>& boneList);
		virtual ~AnimationEditor();

		virtual void initialize();

		virtual void open();
		virtual void close();

		virtual void openModel(Skeleton* model, const string& ressourceName);
		virtual void saveModel(Skeleton* model, const string& ressourceName);
		virtual void dropModel();
		
		virtual bool onMouseMotion(int x, int y);
		virtual bool onMouseButton(int button, int state, int x, int y);
		virtual void onKeyboard(unsigned char key, int x, int y);

		void createNewAnimationClip(Skeleton* skl);

		//Pose handling
		void setToRestPose();
		void setToPose(KeyFrame* key);
		void saveCurrentPose();

		//Bone selection and orientation
		Skeleton* selectBoneAt(int x, int y);
		void orientBoneTo(int x, int y);
};

#endif