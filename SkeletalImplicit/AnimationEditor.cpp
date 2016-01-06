#include "AnimationEditor.h"

#include "Skeleton.h"
#include "SkeletalImplicit.h"
#include "MetaTube.h"
#include "Metaball.h"
#include "Input.h"
#include "RayCastUtils.h"
#include "Animation.h"
#include "DisplayText.h"

#include <GLGraphics/gel_glut.h>
#include <sstream>
#include <iostream>


AnimationEditor::AnimationEditor(vector<Skeleton*>& boneList):
	IEditor(boneList),
	movingBone(false),
	clip(0),
	restPose(0),
	restPoseTrans(0),
	currentFrame(0),
	currentFrameId(0),
	keyFrameCountId(0),
	selectedBone(0),
	animationPlayer(0),
	nbOfBones(0){
}

AnimationEditor::~AnimationEditor(){
}

void AnimationEditor::initialize(){
	this->keyFrameCountId = this->textDisplayer->addText("");
}

void AnimationEditor::open(){

	//Check validity of the clip and rest pose

	cout<<"switch to animation editor"<<endl;
	this->textDisplayer->updateText(IEditor::modeTextId,"Animation editor mode");
	if(this->restPose){
		if( this->bones.size() != this->nbOfBones ){
			this->nbOfBones = this->bones.size();
			delete[] this->restPose;
			delete[] this->restPoseTrans;

			this->restPose = new Quatf[nbOfBones];
			this->restPoseTrans = new Mat4x4f[nbOfBones];
		}
		for(unsigned int i = 0; i < this->bones.size(); i++){
			this->restPose[i] = this->bones[i]->orientation;
			this->restPoseTrans[i] =  bones[i]->worldOrientation.get_Mat4x4f();
			this->restPoseTrans[i][3] = Vec4f(bones[i]->worldPosition,1);
		}
	}else
		cout<<"openning animation editor without proper initialization (call openModel())"<<endl;

	if(this->clip && this->clip->keyFrameCount > 0){
		std::stringstream out;
		out << this->clip->keyFrameCount;
		currentFrameId = 1;
		this->currentFrame = this->clip->first;
		this->textDisplayer->updateText(this->keyFrameCountId,"Key Frame 1 / "+out.str());
	}else
		this->textDisplayer->updateText(this->keyFrameCountId,"Key Frame  0 / 0");

}
void AnimationEditor::close(){
	this->textDisplayer->updateText(this->keyFrameCountId,"");
	setToRestPose();
	this->selectedBone = NULL;
}

void AnimationEditor::openModel(Skeleton* model, const string& ressourceName){

	std::ifstream ifs;
	string bModelName = ressourceName.substr(0,ressourceName.find("."));
	ifs.open(("Models/"+bModelName+".anim").c_str(),ios::in);
	if(!ifs.fail()){
		load(*this->animationPlayer,ifs);
		this->clip = this->animationPlayer->clips[0];
	}else{
		if(this->bones.size() >= 0)
			this->createNewAnimationClip(model);
		else
			cout<<"nothing to animate"<<endl;
	}
	if(this->bones.size() >= 0){
		this->nbOfBones = this->bones.size(); 
		this->restPose = new Quatf[this->bones.size()];
		this->restPoseTrans = new Mat4x4f[this->bones.size()];
		for(int i = 0; i < this->bones.size(); i++){
			this->restPose[i] = bones[i]->orientation;
			this->restPoseTrans[i] =  bones[i]->worldOrientation.get_Mat4x4f();
			this->restPoseTrans[i][3] = Vec4f(bones[i]->worldPosition,1);
		}
	}
}

void AnimationEditor::saveModel(Skeleton* model, const string& ressourceName){
	if(this->clip->keyFrameCount > 0){
		string bModelName = ressourceName.substr(0,ressourceName.find("."));
		save(*animationPlayer,("Models/"+bModelName+".anim").c_str());
	}
}

void AnimationEditor::dropModel(){
	delete[] this->restPose;
	delete[] this->restPoseTrans;
	this->restPose = NULL;
	this->restPoseTrans = NULL;

	this->movingBone = false;
	delete this->clip;
	this->clip = NULL;
	this->currentFrame = NULL;
	this->currentFrameId = 0;
	this->selectedBone = NULL;
	this->selecteBoneId = 0;

	this->animationPlayer->clear();
}

bool AnimationEditor::onMouseMotion(int x, int y){
	if(this->selectedBone && this->movingBone){
		if(Input::keyStates[ROTATE_KEY]){
			orientBoneTo(x,y);
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
			movingBone = true;
			return true;
		}
	}else if(state == GLUT_UP && button == GLUT_RIGHT_BUTTON){
		if(this->movingBone){
			this->movingBone = false;
		}
		return false;
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
			saveCurrentPose();
			KeyFrame* newKey = new KeyFrame();
			newKey->rotations = new Quatf[this->clip->nbBones];
			for(int i = 0; i < this->clip->nbBones; i++){
				newKey->rotations[i] = this->restPose[i];
			}
			newKey->time = 0;
			this->clip->pushKeyFrame(newKey);
			this->currentFrame = newKey;
			this->currentFrameId++;
			std::stringstream out;
			out<<currentFrameId<<" / "<<this->clip->keyFrameCount;
			this->textDisplayer->updateText(keyFrameCountId,"Key Frame "+out.str());

			this->setToPose(currentFrame);
			break;
			}
		case GLUT_KEY_RIGHT:{
			if(this->clip->keyFrameCount > 0){
				cout<<"from "<<currentFrameId<<endl;
				currentFrameId = (currentFrameId)%(this->clip->keyFrameCount)+1;
				cout<<"to "<<currentFrameId<<endl;
				currentFrame = currentFrame->next?currentFrame->next:this->clip->first;
				
				setToPose(currentFrame);

				std::stringstream out;
				out<<currentFrameId<<" / "<<this->clip->keyFrameCount;
				this->textDisplayer->updateText(keyFrameCountId,"Key Frame "+out.str());
			break;
			}
			}
		case GLUT_KEY_LEFT:
			//currentFrameId = (currentFrameId-1)%animClip->keyFrameCount;
			break;
	}
}





void AnimationEditor::createNewAnimationClip(Skeleton* skl){

	if(this->bones.size() > 0){
		this->clip = new AnimationClip();
		this->clip->name = "Empty";
		this->clip->nbBones = bones.size();
		this->clip->playing = true;
		this->clip->loop = true;
		this->clip->fade = 0;
		this->clip->bones = new Skeleton*[this->clip->nbBones];
		for(int i = 0; i < this->clip->nbBones; i++){
			this->clip->bones[i] = this->bones[i];
		}
		this->animationPlayer->addClip(this->clip);
	}
}

void AnimationEditor::setToRestPose(){
	if(this->restPose){
		for(unsigned int i = 0; i < this->bones.size(); i++){
			this->bones[i]->orientation = this->restPose[i];
		}
		this->rootSkeleton->_updateWorldOrientation();
		this->rootSkeleton->_updateWorldPosition();
	}
}
void AnimationEditor::setToPose(KeyFrame* key){
	if(key){
		for(unsigned int i = 0; i < this->bones.size(); i++){
			this->bones[i]->orientation = key->rotations[i];
		}
		this->rootSkeleton->_updateWorldOrientation();
		this->rootSkeleton->_updateWorldPosition();
	}
}
void AnimationEditor::saveCurrentPose(){
	if(currentFrame){
		for(unsigned int i = 0; i < this->bones.size(); i++){
			currentFrame->rotations[i] = this->bones[i]->orientation;
		}
	}
}
Skeleton* AnimationEditor::selectBoneAt(int x, int y){
	MyRay ray = getRayFromScreePos(x,y);
	//debug_position = ray.origine + 2*ray.direction;
	RayCastHit hit;
	for(unsigned int i = 0 ; i < this->bones.size(); i++){
		if(this->bones[i]->rayCast(ray,hit)){
			this->selecteBoneId = i;
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
	if(this->currentFrame){
		this->currentFrame->rotations[this->selecteBoneId] = this->selectedBone->orientation;
	}
}
