#include "SkeletonEditor.h"

#include "Skeleton.h"
#include "SkeletalImplicit.h"
#include "MetaTube.h"
#include "Metaball.h"
#include "Input.h"
#include "RayCastUtils.h"
#include "DisplayText.h"

#include <GLGraphics/gel_glut.h>


SkeletonEditor::SkeletonEditor(vector<Skeleton*>& boneList):
	IEditor(boneList),
	movingBone(false),
	selectedBone(0){
}

SkeletonEditor::~SkeletonEditor(){
}

void SkeletonEditor::initialize(){
}

void SkeletonEditor::open(){
	cout<<"switch to skeleton editor"<<endl;
	this->textDisplayer->updateText(IEditor::modeTextId,"Bone editor mode");
}
void SkeletonEditor::close(){
	this->selectedBone = NULL;
}

void SkeletonEditor::openModel(Skeleton* model, const string& ressourceName){
}

void SkeletonEditor::saveModel(Skeleton* model, const string& ressourceName){
}

void SkeletonEditor::dropModel(){
	movingBone = false;
	selectedBone = this->rootSkeleton;
}

bool SkeletonEditor::onMouseMotion(int x, int y){
	if(this->selectedBone && this->movingBone){
		if(Input::keyStates[ROTATE_KEY]){
			orientBoneTo(x,y);
			return true;
		}else if(Input::keyStates[SCALE_KEY]){
			scaleBone(x,Input::mouse.dy);
			return true;
		}else if(Input::keyStates[TRANSLATE_KEY] || Input::keyStates[ADD_KEY]){
			setBoneToTarget(x,y);
			return true;
		}
	}
	return false;
}
bool SkeletonEditor::onMouseButton(int button, int state, int x, int y){
	if(state == GLUT_DOWN) {
		if(button == GLUT_RIGHT_BUTTON){ //BONE SELECTION
			Skeleton* boneAtxy = NULL;
			boneAtxy = selectBoneAt(x,y);
			if(boneAtxy){
				this->selectedBone = boneAtxy;
				return true;
			}
		}
		if(button == GLUT_LEFT_BUTTON && 
			(Input::keyStates[ROTATE_KEY] || 
			 Input::keyStates[SCALE_KEY]  ||
			 Input::keyStates[TRANSLATE_KEY])){ //Bone Movement
			this->movingBone = true;
			return true;
		}
		if(button == GLUT_LEFT_BUTTON && Input::keyStates[ADD_KEY]){ //Bone Movement
			if(this->selectedBone){
				addNewBoneTo(this->selectedBone);
				this->movingBone = true;
				setBoneToTarget(x,y);
			}
			return true;
		}
	}else if(state == GLUT_UP && button == GLUT_RIGHT_BUTTON){
		this->movingBone = false;
		return false;
	}
	return false;
}
void SkeletonEditor::onKeyboard(unsigned char key, int x, int y){
	switch (key) {
		case 'b':
			if(this->selectedBone)
				addNewBoneTo(this->selectedBone);
			break;
		case 'c':
			if(this->selectedBone){
				this->selectedBone->orientation = this->selectedBone->orientation.identity_Quatf();
				this->selectedBone->_updateWorldOrientation();
			}
			//this->selectedBone = this->rootSkeleton;
			break;
		case 127:
			if(this->selectedBone){
				removeBone(this->selectedBone);
				selectedBone = NULL;
			}
    }
}

void SkeletonEditor::removeBone(Skeleton* bone){
	Skeleton* node;
	vector<Skeleton*> openList;
	openList.push_back(bone);
	vector<Skeleton*> closedList;
	vector<MetaPrimitive*> prims;

	//Remove all references to the bones and primitives in the editor
	while( !openList.empty() ){	
		node = openList.back();
		openList.pop_back();
		closedList.push_back(node);
		for(unsigned int i = 0; i < node->children.size(); i++){
			openList.push_back(node->children[i]);				
		}
		for(unsigned int i = 0; i < node->primitives.size(); i++){
			prims.push_back(node->primitives[i]);				
		}
	}
	cout<<"remove "<<closedList.size()<<" bones"<<endl;
	for(unsigned int i = 0; i < closedList.size(); i++){
		vector<Skeleton*>::iterator it = this->bones.begin();
		for(; it!=this->bones.end(); ++it ){
			if(*it==closedList[i])
				break;
		}
		bones.erase(it);
	}
	for(unsigned int i = 0; i < prims.size(); i++){
		cout<<"rm prim "<<i<<endl;
		this->implicitSurface->removePrimitive(prims[i]);
	}
	delete bone;
}

void SkeletonEditor::addNewBoneTo(Skeleton* parent){
	Skeleton* newSkeleton = new Skeleton();
	newSkeleton->name = "new";
	newSkeleton->addPrimitive(new MetaTube(Vec3f(0),Vec3f(0,0,newSkeleton->length),0.1f,0.1f));		

	parent->addChild(newSkeleton);
	this->implicitSurface->addBone(newSkeleton);

	this->selectedBone = newSkeleton;
	
	this->bones.push_back(newSkeleton);
	this->movingBone = true;
}

Skeleton* SkeletonEditor::selectBoneAt(int x, int y){
	MyRay ray = getRayFromScreePos(x,y);
	//debug_position = ray.origine + 2*ray.direction;
	RayCastHit hit;
	for(unsigned int i = 0 ; i < this->bones.size(); i++){
		if(this->bones[i]->rayCast(ray,hit)){
			return bones[i];
		}
	}
	return NULL;
}
void SkeletonEditor::orientBoneTo(int x, int y){
	Vec3f intersect = getPosInViewPlane(x,y,this->selectedBone->worldPosition);

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

void SkeletonEditor::setBoneToTarget(int x, int y){
	Vec3f intersect = getPosInViewPlane(x,y,this->selectedBone->worldPosition);
	this->selectedBone->setWorldTarget(intersect);
}

void SkeletonEditor::scaleBone(int x, int dy){
	Vec3f boneAxis = this->selectedBone->worldOrientation.inverse().apply(Vec3f(0,0,1));
	this->selectedBone->setLength(this->selectedBone->length + 0.0005f*dy);
}
