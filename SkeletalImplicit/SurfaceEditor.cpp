#include "SurfaceEditor.h"

#include "Skeleton.h"
#include "SkeletalImplicit.h"
#include "MetaTube.h"
#include "Metaball.h"
#include "Input.h"
#include "RayCastUtils.h"
#include "DisplayText.h"

#include <GLGraphics/gel_glut.h>


SurfaceEditor::SurfaceEditor(vector<Skeleton*>& boneList):IEditor(boneList),
selectedPrimitive(0){
}

SurfaceEditor::~SurfaceEditor(){
}

void SurfaceEditor::initialize(){
}

void SurfaceEditor::open(){
	cout<<"switch to surface editor"<<endl;
	this->textDisplayer->updateText(IEditor::modeTextId,"Primirive editor mode");
}
void SurfaceEditor::close(){
	this->selectedPrimitive = NULL;
}

void SurfaceEditor::openModel(Skeleton* model, const string& ressourceName){
}


void SurfaceEditor::saveModel(Skeleton* model, const string& ressourceName){
}

void SurfaceEditor::dropModel(){
	movingPrimitive = false;
	selectedPrimitive = NULL;

}

bool SurfaceEditor::onMouseMotion(int x, int y){
	if(this->selectedPrimitive && this->movingPrimitive){
		if(Input::keyStates[TRANSLATE_KEY]){
			
			translatePrimitive(x,y);
			return true;
		}else if(Input::keyStates[SCALE_KEY]){
			scalePrimitive(x,Input::mouse.dy);
			return true;
		}
	}
	return false;
}
bool SurfaceEditor::onMouseButton(int button, int state, int x, int y){
	if(state == GLUT_DOWN) {
		if(button == GLUT_RIGHT_BUTTON){ //BONE SELECTION
			MetaPrimitive* primAtxy = NULL;
			primAtxy = selectPrimitiveAt(x,y);
			if(primAtxy){
				this->selectedPrimitive = primAtxy;
				return true;
			}
		}
		if(button == GLUT_LEFT_BUTTON && (Input::keyStates[TRANSLATE_KEY] || Input::keyStates[SCALE_KEY])){ //Bone Movement
			this->movingPrimitive = true;
			return true;
		}
	}else if(state == GLUT_UP && button == GLUT_RIGHT_BUTTON){
		this->movingPrimitive = false;
		return true;
	}
	return false;
}
void SurfaceEditor::onKeyboard(unsigned char key, int x, int y){
	   switch (key) {
			case 'b':
				if(this->selectedPrimitive)
					addNewSphereTo(this->selectedPrimitive->skeleton);
				break;
			case KEY_DEL:
				if(this->selectedPrimitive)
					delete this->selectedPrimitive;
				break;
	   }
}

void SurfaceEditor::addNewSphereTo(Skeleton* parent){
	Metaball* newMeta = new Metaball();
	parent->addPrimitive(newMeta);
	cout<<"hest"<<endl;
	this->implicitSurface->addPrimitive(newMeta);

	selectedPrimitive = newMeta;
}

MetaPrimitive* SurfaceEditor::selectPrimitiveAt(int x,int y){
	MyRay ray = getRayFromScreePos(x,y);
	//debug_position = ray.origine + 2*ray.direction;
	RayCastHit hit;
	for(unsigned int i = 0 ; i < this->implicitSurface->primitives.size(); i++){
		if(this->implicitSurface->primitives[i]->rayCast(ray,hit)){
			return this->implicitSurface->primitives[i];
		}
	}
	return NULL;
}
void SurfaceEditor::scalePrimitive(int x, int dy){
	if(this->selectedPrimitive->getType() == "MetaBall"){
		Metaball* ball = static_cast<Metaball*>(selectedPrimitive);
		ball->r -= 0.0005f*dy;
		ball->_updateWorldGeometry();
		ball->needsUpdate = true;
	}else if(this->selectedPrimitive->getType() == "MetaTube"){
		MetaTube* tube = static_cast<MetaTube*>(selectedPrimitive);
		tube->r1-=0.0005f*dy;
		tube->r2-=0.0005f*dy;
		tube->needsUpdate = true;
	}
}
void SurfaceEditor::translatePrimitive(int x, int y){
	if(this->selectedPrimitive->getType() == "MetaBall"){
		Metaball* ball = static_cast<Metaball*>(selectedPrimitive);
		Vec3f targetWorldPosition = getPosInViewPlane(x,y,ball->worldC);
		ball->setWorldPosition(targetWorldPosition);
		ball->needsUpdate = true;
	}
}

