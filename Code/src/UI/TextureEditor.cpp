#include "TextureEditor.h"

#include "SklImp/TextureProjector.h"
#include "SklImp/HyperTexture.h"
#include "SklImp/Skeleton.h"
#include "SklImp/SkeletalImplicit.h"
#include "SklImp/MetaTube.h"
#include "SklImp/Metaball.h"

#include "Input.h"
#include "DisplayText.h"

#include "Utils/RayCastUtils.h"

#include <GLGraphics/gel_glut.h>

TextureEditor::TextureEditor(vector<Skeleton*>& boneList):IEditor(boneList),
selectedProjector(0){
}

TextureEditor::~TextureEditor(){
	for(size_t i = 0; i < texProjectors.size(); i++){
		delete texProjectors[i];
	}
	texProjectors.clear();
}

void TextureEditor::initialize(){
}

void TextureEditor::open(){
	cout<<"switch to texture editor"<<endl;
	this->textDisplayer->updateText(IEditor::modeTextId,"Texture editor mode");
}
void TextureEditor::close(){
	this->selectedProjector = NULL;
}

void TextureEditor::openModel(Skeleton* model, const string& ressourceName){
}

void TextureEditor::saveModel(Skeleton* model, const string& ressourceName){
}

void TextureEditor::dropModel(){
	movingProj = false;
	this->selectedProjector = NULL;
	for(size_t i = 0; i < texProjectors.size(); i++){
		delete texProjectors[i];
	}
	texProjectors.clear();
}

bool TextureEditor::onMouseMotion(int x, int y){
	if(this->selectedProjector && this->movingProj){
		if(Input::keyStates[SCALE_KEY]){
			this->selectedProjector->length += 0.0005f*Input::mouse.dy;
			if( this->selectedProjector->length < 0 )
				this->selectedProjector->length = 0.001;
			this->selectedProjector->radius += 0.0005f*Input::mouse.dx;
			if( this->selectedProjector->radius < 0 )
				this->selectedProjector->radius = 0.001;
			return true;
		}else if(Input::keyStates[TRANSLATE_KEY]){
			this->selectedProjector->position = getPosInViewPlane(x,y,this->selectedProjector->position);
			return true;
		}else if(Input::keyStates[ROTATE_KEY]){
			Vec3f interSphere;
			MyRay ray = this->getRayFromScreePos(x,y);
	
			if(getRaySphereIntersection(ray,this->selectedProjector->position,0.5f,interSphere) ){

				Vec3f oldObjectTarget = oldOrientation.apply(oldRotationTarget);

				Vec3f rotationTarget = normalize(interSphere - this->selectedProjector->position);
				Vec3f objectNewTarget = oldOrientation.apply(rotationTarget);

				Quatf rot;
				rot.make_rot(objectNewTarget,oldObjectTarget);
	
				this->selectedProjector->orientation = 
					rot * oldOrientation;
			}else{
			}
	
			return true;
		}
	}
	return false;
}
bool TextureEditor::onMouseButton(int button, int state, int x, int y){
	if(state == GLUT_DOWN) {
		if(button == GLUT_RIGHT_BUTTON){ //BONE SELECTION
			TextureProjector* projAtxy = NULL;
			projAtxy = this->selectProjectorAt(x,y);
			if(projAtxy){
				this->selectedProjector = projAtxy;
				return true;
			}
		}
		if(button == GLUT_LEFT_BUTTON && 
			(  Input::keyStates[ROTATE_KEY] 
			|| Input::keyStates[SCALE_KEY]
			|| Input::keyStates[ROTATE_KEY]) ){ //Bone Movement
			this->movingProj = true;

			if(Input::keyStates[ROTATE_KEY] && this->selectedProjector){
				Vec3f interSphere;
				MyRay ray = this->getRayFromScreePos(x,y);
			
				if(getRaySphereIntersection(ray,this->selectedProjector->position,0.5f,interSphere) ){
					this->rotationTarget = normalize(interSphere - this->selectedProjector->position);
					oldRotationTarget = this->rotationTarget;
					oldOrientation = this->selectedProjector->orientation;
				}				
			}
			return true;
		}
	}else if(state == GLUT_UP && button == GLUT_RIGHT_BUTTON){
		this->movingProj = false;
		return false;
	}
	return false;
}
void TextureEditor::onKeyboard(unsigned char key, int x, int y){
	switch (key) {
		case 'c':
			selectedProjector = new TextureProjector();
			texProjectors.push_back(selectedProjector);
			break;
		case 'h':
			selectedProjector = new HyperTexture();
			texProjectors.push_back(selectedProjector);
			break;
		case 127:
			if(this->selectedProjector){
				//removeBone(this->selectedBone);
				selectedProjector = NULL;
			}
    }
}

TextureProjector* TextureEditor::selectProjectorAt(int x, int y){
	MyRay ray = getRayFromScreePos(x,y);
	//debug_position = ray.origine + 2*ray.direction;
	RayCastHit hit;
	for(size_t i = 0 ; i < this->texProjectors.size(); i++){
		if(this->texProjectors[i]->rayCast(ray,hit)){
			return texProjectors[i];
		}
	}
	return NULL;
}