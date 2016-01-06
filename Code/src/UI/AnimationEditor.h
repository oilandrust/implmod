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
	friend class Animation;

	private:
		//THE CLIP WE ARE EDITING
		AnimationClip* clip;
		//THE FRAME OF THE CLIP THAT WE ARE EDITING
		KeyFrame* currentFrame;
		//IT'S INDEX
		size_t currentFrameId;

		//TEXT ID FOR FRAME COUNT DISPLAY
		int keyFrameCountId;


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
	
	private:
		void createNewAnimationClip();

		//UI
		void updateKeyFrameCountDisplay(size_t current);

		//Pose handling
		void setToRestPose();
		void setToPose(KeyFrame* key);
		void saveCurrentPose();

		//Bone selection and orientation
		Skeleton* selectBoneAt(int x, int y);
		void orientBoneTo(int x, int y);
		void setBoneToTarget(int x, int y);
};

#endif