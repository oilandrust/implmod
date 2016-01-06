#include "AnimationEditor.h"

#include "SklImp/Skeleton.h"
#include "SklImp/SkeletalImplicit.h"
#include "SklImp/MetaTube.h"
#include "SklImp/Metaball.h"
#include "SklImp/Animation.h"

#include "Input.h"
#include "DisplayText.h"

#include "Utils/RayCastUtils.h"


#include <GLGraphics/gel_glut.h>
#include <sstream>
#include <iostream>


AnimationEditor::AnimationEditor(vector<Skeleton*>& boneList):
	IEditor(boneList),
	clip(0),
	restPoseTrans(0),
	currentFrame(0),
	currentFrameId(0),
	keyFrameCountId(-1),
	selectedBone(0),
	animationPlayer(0){
}

AnimationEditor::~AnimationEditor(){
}

void AnimationEditor::initialize(){
	this->keyFrameCountId = this->textDisplayer->addText("");
}

void AnimationEditor::open(){
	cout<<"switch to animation editor"<<endl;
	this->textDisplayer->updateText(IEditor::modeTextId,"Animation editor mode");

	//First time we open the model, create a new empty clip
	if(this->clip == 0 && this->bones.size() >= 0){
		this->createNewAnimationClip();
	}

	//Clip out of date, create a new one
	if( this->clip->boneCount() != this->bones.size() ){
		this->clip->clearTarget();
		this->clip->setTargetSkeleton(this->bones);
	}

	//Save Rest Position even if the the model is the same as the one pointed by the animation
	if(this->bones.size() >= 0){
		this->animationPlayer->saveRestPose(this->bones);
	}
	

	//Update the key frame count display
	if(this->clip->keyFrameCount() > 0){
		currentFrameId = 0;
		this->currentFrame = this->clip->getKeyFrame(0);
		updateKeyFrameCountDisplay(0);
	}else
		updateKeyFrameCountDisplay(-1);

}
void AnimationEditor::close(){
	saveCurrentPose();	
	this->textDisplayer->updateText(this->keyFrameCountId,"");
	setToRestPose();
	this->selectedBone = 0;
}

void AnimationEditor::openModel(Skeleton* model, const string& ressourceName){
	assert(this->animationPlayer != 0);

	if(ressourceName != "None"){
		std::ifstream ifs;
		string bModelName = ressourceName.substr(0,ressourceName.find("."));
		ifs.open((modelDirectory+'/'+bModelName+".anim").c_str(),ios::in);
		if(!ifs.fail()){
			load(*this->animationPlayer,ifs);
			this->clip = this->animationPlayer->getClip(0);
			this->clip->setTargetSkeleton(this->bones);

			this->animationPlayer->saveRestPose(this->bones);

			assert( this->clip->boneCount() == this->bones.size() );
			assert(this->clip->keyFrameCount() > 0);
		}
	}
}

void AnimationEditor::saveModel(Skeleton* model, const string& ressourceName){
	if(this->clip->keyFrameCount() > 0){
		string bModelName = ressourceName.substr(0,ressourceName.find("."));
		animationPlayer->save((modelDirectory+'/'+bModelName+".anim").c_str());
	}
}

void AnimationEditor::dropModel(){
	assert(this->animationPlayer != 0);

	delete this->clip;
	this->clip = NULL;
	this->currentFrame = NULL;
	this->currentFrameId = 0;
	this->selectedBone = NULL;

	this->animationPlayer->clear();
}

bool AnimationEditor::onMouseMotion(int x, int y){
	if(this->selectedBone){
		if(Input::keyStates[ROTATE_KEY]){
			orientBoneTo(x,y);
			return true;
		}else if(Input::keyStates[TRANSLATE_KEY]){
			setBoneToTarget(x,y);
			return true;
		}
	}
	return false;
}
bool AnimationEditor::onMouseButton(int button, int state, int x, int y){
	if(state == GLUT_DOWN){
		if(button == GLUT_RIGHT_BUTTON){ //BONE SELECTION
			Skeleton* boneAtxy = NULL;
			boneAtxy = selectBoneAt(x,y);
			if(boneAtxy){
				this->selectedBone = boneAtxy;
				return true;
			}
		}else if(button == GLUT_LEFT_BUTTON && (Input::keyStates[ROTATE_KEY])){ //Bone Movement
			return true;
		}
	}
	return false;
}
void AnimationEditor::onKeyboard(unsigned char key, int x, int y){
	if(glutGetModifiers() == GLUT_ACTIVE_CTRL){
		if(key == KEY_S){
			saveCurrentPose();
			return;
		}
		if(key == KEY_R){
			setToRestPose();
			return;
		}
	}

	switch (key) {
		case 'k':{
			assert(this->clip->boneCount() == this->bones.size());

			saveCurrentPose();

			//CREATE A NEW KEY FRAME INTITIALIZED AT THE REST POSE
			KeyFrame newKey;
			newKey.rotations = this->animationPlayer->getRestPoseQuats();

			//Add it in the clip
			this->clip->pushKeyFrame(newKey);

			//Get the last frame of the clip
			this->currentFrameId = clip->keyFrameCount()-1;
			this->currentFrame = clip->getKeyFrame(currentFrameId);

			//update the display
			updateKeyFrameCountDisplay(currentFrameId);
			
			//set the model in rest pose
			this->setToPose(currentFrame);
			
			break;
		}
		case GLUT_KEY_RIGHT:
		case GLUT_KEY_LEFT:{ //BROWSE KEY FRAME
			if(this->clip->keyFrameCount() > 0){
				//save the pose to the edited key
				saveCurrentPose();	

				if(key == GLUT_KEY_RIGHT)
					currentFrameId = (currentFrameId+1)%(this->clip->keyFrameCount());
				else
					currentFrameId = (currentFrameId-1)%(this->clip->keyFrameCount());

				//load the new key to edit
				currentFrame = this->clip->getKeyFrame(currentFrameId);
				setToPose(currentFrame);

				//update the key frame count display
				updateKeyFrameCountDisplay(currentFrameId);

				break;
			}
		}
	}
}


void AnimationEditor::createNewAnimationClip(){
	assert(this->clip == 0);
	if(this->bones.size() > 0){
		this->clip = new AnimationClip();
		this->clip->name = "Empty";
		this->clip->initialize(this->bones);
		this->animationPlayer->addClip(this->clip);
	}

}

void AnimationEditor::updateKeyFrameCountDisplay(size_t current){
	std::stringstream out;
	out<<current+1<<" / "<<this->clip->keyFrameCount();
	this->textDisplayer->updateText(this->keyFrameCountId,"Key Frame "+out.str());
}

void AnimationEditor::setToRestPose(){
	this->animationPlayer->setModelToRestPose();
}
void AnimationEditor::setToPose(KeyFrame* key){
	if(key){
		for(size_t i = 0; i < this->bones.size(); i++){
			this->bones[i]->orientation = key->rotations[i];
		}
		this->rootSkeleton->_updateWorldOrientation();
		this->rootSkeleton->_updateWorldPosition();
	}
}
void AnimationEditor::saveCurrentPose(){
	if(currentFrame){
		for(size_t i = 0; i < this->bones.size(); i++){
			currentFrame->rotations[i] = this->bones[i]->orientation;
		}
	}
}
Skeleton* AnimationEditor::selectBoneAt(int x, int y){
	MyRay ray = getRayFromScreePos(x,y);

	RayCastHit hit;
	for(size_t i = 0 ; i < this->bones.size(); i++){
		if(this->bones[i]->rayCast(ray,hit)){
			return bones[i];
		}
	}
	return NULL;
}
void AnimationEditor::orientBoneTo(int x, int y){
	MyRay ray = getRayFromScreePos(x,y);
	Vec3f eye;
	Vec3f center;
	Vec3f up;
	view_ctrl->get_view_param(eye,center,up);
	Vec3f norm = normalize(center-eye);

	Vec3f p = this->selectedBone->worldPosition;
	Vec3f o = ray.origine;

	float s = -dot(norm,o-p)/dot(norm,ray.direction);
	Vec3f intersect = o + s*ray.direction;

	Vec3f parentSpaceInter;
	if(!this->selectedBone->parent)
		parentSpaceInter = intersect;
	else{
		parentSpaceInter = intersect - this->selectedBone->parent->worldPosition;
		parentSpaceInter = this->selectedBone->parent->worldOrientation.apply(parentSpaceInter);
	}
	//debug_position = parentSpaceInter;
	this->selectedBone->rotate(Vec3f(0),parentSpaceInter);
}

void AnimationEditor::setBoneToTarget(int x, int y){
	float oldLengh = this->selectedBone->length;
	Vec3f intersect = getPosInViewPlane(x,y,this->selectedBone->worldPosition);
	this->selectedBone->setWorldTarget(intersect);
	this->selectedBone->setLength(oldLengh);
}