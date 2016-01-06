#include "RigEditor.h"

#include "SklImp/Skeleton.h"
#include "SklImp/SkeletalImplicit.h"
#include "SklImp/MetaTube.h"
#include "SklImp/Metaball.h"

#include "Input.h"
#include "DisplayText.h"

#include "Utils/RayCastUtils.h"

#include <GLGraphics/gel_glut.h>


RigEditor::RigEditor(vector<Skeleton*>& boneList):
	IEditor(boneList),
	movingBone(false),
	selectedBone(0){
}

RigEditor::~RigEditor(){
}

void RigEditor::initialize(){
}

void RigEditor::open(){
	cout<<"switch to skeleton editor"<<endl;
	this->textDisplayer->updateText(IEditor::modeTextId,"Bone editor mode");
}
void RigEditor::close(){
	this->selectedBone = NULL;
}

void RigEditor::openModel(Skeleton* model, const string& ressourceName){
}

void RigEditor::saveModel(Skeleton* model, const string& ressourceName){
}

void RigEditor::dropModel(){
	movingBone = false;
	selectedBone = this->rootSkeleton;
}

bool RigEditor::onMouseMotion(int x, int y){
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
bool RigEditor::onMouseButton(int button, int state, int x, int y){
	if(state == GLUT_DOWN) {
		if(button == GLUT_RIGHT_BUTTON){ //BONE SELECTION
			if(this->selectedBone && glutGetModifiers() & GLUT_ACTIVE_CTRL){
				vector<MetaPrimitive*> prims = this->rootSkeleton->primitives;
				prims.insert(prims.end(),this->selectedBone->primitives.begin(),this->selectedBone->primitives.end());

				MetaPrimitive* selectedPrim = this->selectPrimitiveAt(x,y,prims);
				if(selectedPrim != 0){
					Metaball* selectedBall = static_cast<Metaball*>(selectedPrim);
					Vec3f saveWorldPos = selectedBall->worldC;
					
					if(glutGetModifiers() & GLUT_ACTIVE_SHIFT){//REMOVE FROM BONE AND reattach to the root
						if(selectedBall->skeleton == this->selectedBone){
							this->selectedBone->removePrimitive(selectedBall);
							this->rootSkeleton->addPrimitive(selectedBall);
						}
					}else//ADD TO BONE
						this->selectedBone->addPrimitive(selectedBall);

					selectedBall->setWorldPosition(saveWorldPos);

					return true;
				}
			}else{
				Skeleton* boneAtxy = 0;
				boneAtxy = selectBoneAt(x,y);
				if(boneAtxy){
					this->selectedBone = boneAtxy;
					return true;
				}
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
void RigEditor::onKeyboard(unsigned char key, int x, int y){
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

void RigEditor::removeBone(Skeleton* bone){
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
		for(size_t i = 0; i < node->children.size(); i++){
			openList.push_back(node->children[i]);				
		}
		for(size_t i = 0; i < node->primitives.size(); i++){
			prims.push_back(node->primitives[i]);				
		}
	}
	cout<<"remove "<<closedList.size()<<" bones"<<endl;
	for(size_t i = 0; i < closedList.size(); i++){
		vector<Skeleton*>::iterator it = remove(bones.begin(),bones.end(),closedList[i]);
		bones.erase(it,bones.end());
	}
	for(size_t i = 0; i < prims.size(); i++){
		cout<<"rm prim "<<i<<endl;
		this->implicitSurface->removePrimitive(prims[i]);
	}
	delete bone;
}

void RigEditor::addNewBoneTo(Skeleton* parent){
	Skeleton* newSkeleton = new Skeleton();
	newSkeleton->name = "new";

	parent->addChild(newSkeleton);
	this->implicitSurface->addBone(newSkeleton);

	this->selectedBone = newSkeleton;
	
	this->bones.push_back(newSkeleton);
	this->movingBone = true;
}

Skeleton* RigEditor::selectBoneAt(int x, int y){
	MyRay ray = getRayFromScreePos(x,y);
	//debug_position = ray.origine + 2*ray.direction;
	RayCastHit hit;
	for(size_t i = 0 ; i < this->bones.size(); i++){
		if(this->bones[i]->rayCast(ray,hit)){
			return bones[i];
		}
	}
	return NULL;
}

void RigEditor::orientBoneTo(int x, int y){
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

void RigEditor::setBoneToTarget(int x, int y){
	Vec3f intersect = getPosInViewPlane(x,y,this->selectedBone->worldPosition);
	this->selectedBone->setWorldTarget(intersect);
}

void RigEditor::scaleBone(int x, int dy){
	Vec3f boneAxis = this->selectedBone->worldOrientation.inverse().apply(Vec3f(0,0,1));
	this->selectedBone->setLength(this->selectedBone->length + 0.0005f*dy);
}
