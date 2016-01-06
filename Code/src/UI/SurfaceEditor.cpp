#include "SurfaceEditor.h"

#include "SklImp/Skeleton.h"
#include "SklImp/SkeletalImplicit.h"
#include "SklImp/MetaTube.h"
#include "SklImp/Metaball.h"

#include "Input.h"
#include "DisplayText.h"

#include "Utils/RayCastUtils.h"

#include <GLGraphics/gel_glut.h>

const float SurfaceEditor::minRadius = 0.0001;


SurfaceEditor::SurfaceEditor(vector<Skeleton*>& boneList):IEditor(boneList),
selectedPrimitive(0),radiusOfNewBall(0.5f){
}

SurfaceEditor::~SurfaceEditor(){
}

void SurfaceEditor::initialize(){
}

void SurfaceEditor::open(){
	cout<<"switch to surface editor"<<endl;
	this->textDisplayer->updateText(IEditor::modeTextId,"Surface editor mode");
}
void SurfaceEditor::close(){
	this->selectedPrimitive = NULL;
}

void SurfaceEditor::openModel(Skeleton* model, const string& ressourceName){
}


void SurfaceEditor::saveModel(Skeleton* model, const string& ressourceName){
}

void SurfaceEditor::dropModel(){
	selectedPrimitive = NULL;

}

bool SurfaceEditor::onMouseMotion(int x, int y){
	if(this->selectedPrimitive){
		if(Input::keyStates[TRANSLATE_KEY] || Input::keyStates[ADD_KEY]){
			translatePrimitive(x,y);
			return true;
		}else if(Input::keyStates[SCALE_KEY]){
				scalePrimitive(x,y);
			return true;
		}
	}
	return false;
}
bool SurfaceEditor::onMouseButton(int button, int state, int x, int y){

	if(state == GLUT_DOWN) {
		/***************************************SELECTION******************//////////////
		if(button == GLUT_RIGHT_BUTTON){ 
			MetaPrimitive* primAtxy = NULL;
			primAtxy = selectPrimitiveAt(x,y,this->implicitSurface->primitives);
			if(primAtxy){
				this->selectedPrimitive = primAtxy;
				return true;
			}
		}
		
		if(button == GLUT_LEFT_BUTTON &&  (Input::keyStates[SCALE_KEY] || Input::keyStates[TRANSLATE_KEY]) ){ //Bone Movement

			if(this->selectedPrimitive){

				/***************************************SCALING******************//////////////
				if(Input::keyStates[SCALE_KEY]){
 					if(this->selectedPrimitive->getType() == "MetaBall"){
						Metaball* ball = static_cast<Metaball*>(selectedPrimitive);
						scaleOldPos = getPosInViewPlane(x,y,ball->worldC);
						if(length(scaleOldPos-ball->worldC) <= ball->r)
							this->movingPrimitive = true;
					}else if(this->selectedPrimitive->getType() == "MetaTube"){

						MetaTube* tube = static_cast<MetaTube*>(selectedPrimitive);

						MyRay ray = getRayFromScreePos(x,y);
						Vec3f inter;
						if(getRaySphereIntersection(ray,tube->worldP1,tube->r1,inter)){
							this->sideToScale = -1;
							scaleOldPos = getPosInViewPlane(x,y,tube->worldP1);
						}else if(getRaySphereIntersection(ray,tube->worldP2,tube->r2,inter)){
							this->sideToScale = 1;
							scaleOldPos = getPosInViewPlane(x,y,tube->worldP2);
						}else{
							this->sideToScale = 0;
							scaleOldPos = getPosInViewPlane(x,y,0.5f*(tube->worldP2+tube->worldP1));
						}
					}
				}

				/***************************************TRANSLATION******************//////////////
				else if(Input::keyStates[TRANSLATE_KEY]){
					if(this->selectedPrimitive->getType() == "MetaTube"){
						MetaTube* tube = static_cast<MetaTube*>(selectedPrimitive);

						MyRay ray = getRayFromScreePos(x,y);
						Vec3f inter;
						if(getRaySphereIntersection(ray,tube->worldP1,tube->r1,inter)){
							this->sideToScale = -1;
						}else if(getRaySphereIntersection(ray,tube->worldP2,tube->r2,inter)){
							this->sideToScale = 1;
						}else{
							this->sideToScale = 0;
						}
					}
				}



			}


			return true;
		}

		/***************************************ADD******************//////////////
		if( button == GLUT_LEFT_BUTTON && Input::keyStates[ADD_KEY] ){
			if(this->selectedPrimitive){
				movingPrimitive = true;
				addNewSphereTo(this->selectedPrimitive->skeleton);
				translatePrimitive(x,y);
			}else{
				movingPrimitive = true;
				addNewSphereTo(this->rootSkeleton);
				translatePrimitive(x,y);
			}
		}

	}else if(state == GLUT_UP && button == GLUT_RIGHT_BUTTON){
		this->movingPrimitive = false;
		return true;
	}
	return false;
}
void SurfaceEditor::onKeyboard(unsigned char key, int x, int y){
	if(glutGetModifiers() == GLUT_ACTIVE_CTRL)
		snap = true;
	else
		snap = false;

	   switch (key) {
			case KEY_DEL:
				if(this->selectedPrimitive){
					this->selectedPrimitive->skeleton->removePrimitive(this->selectedPrimitive);	
					this->implicitSurface->removePrimitive(this->selectedPrimitive);
					delete this->selectedPrimitive;
					this->selectedPrimitive = 0;
				}
				break;
	   }
}

void SurfaceEditor::addNewSphereTo(Skeleton* parent){
	Metaball* newMeta = new Metaball();
	newMeta->r = this->radiusOfNewBall;
	parent->addPrimitive(newMeta);

	this->implicitSurface->addPrimitive(newMeta);

	selectedPrimitive = newMeta;
}


void SurfaceEditor::scalePrimitive(int x, int y){

	if(this->selectedPrimitive->getType() == "MetaBall"){
		Metaball* ball = static_cast<Metaball*>(selectedPrimitive);

		Vec3f mousePose = getPosInViewPlane(x,y,ball->worldC);
		float inc = length(mousePose-ball->worldC)-length(scaleOldPos-ball->worldC);
		ball->r += inc;
		if(ball->r < minRadius){
			ball->r -= inc;
		}
		scaleOldPos = mousePose;

		this->radiusOfNewBall = ball->r;
		ball->_updateWorldGeometry();
		ball->needsUpdate = true;

	}else if(this->selectedPrimitive->getType() == "MetaTube"){

		MetaTube* tube = static_cast<MetaTube*>(selectedPrimitive);

		float inc;
		Vec3f mousePose;
		Vec3f tubeMid;
		switch(this->sideToScale){
			case 0:
				tubeMid = 0.5f*(tube->worldP2+tube->worldP1);
				mousePose = getPosInViewPlane(x,y,tubeMid);
				inc = length(mousePose-tubeMid)-length(scaleOldPos-tubeMid);
				tube->r1 += inc;
				tube->r2 += inc;
				scaleOldPos = mousePose;
				break;
			case -1:
				mousePose = getPosInViewPlane(x,y,tube->worldP1);
				inc = length(mousePose-tube->worldP1)-length(scaleOldPos-tube->worldP1);
				tube->r1 += inc;
				scaleOldPos = mousePose;
				break;
			case 1:
				mousePose = getPosInViewPlane(x,y,tube->worldP2);
				inc = length(mousePose-tube->worldP2)-length(scaleOldPos-tube->worldP2);
				tube->r2 += inc;
				scaleOldPos = mousePose;
				break;
		}
		if(tube->r2 < minRadius){
			tube->r2 -= inc;
		}
		if(tube->r1 < minRadius){
			tube->r1 -= inc;
		}
		tube->_updateWorldGeometry();
		tube->needsUpdate = true;
	}
}
void SurfaceEditor::translatePrimitive(int x, int y, bool snap){
	if(this->selectedPrimitive->getType() == "MetaBall"){

		Metaball* ball = static_cast<Metaball*>(selectedPrimitive);
		Vec3f targetWorldPosition = getPosInViewPlane(x,y,ball->worldC);
		ball->setWorldPosition(targetWorldPosition);
		ball->needsUpdate = true;

	}else if(this->selectedPrimitive->getType() == "MetaTube"){

		MetaTube* tube = static_cast<MetaTube*>(selectedPrimitive);

		Vec3f targetWorldPosition;
		MyRay ray = getRayFromScreePos(x,y);

		switch(this->sideToScale){
			case -1:
				targetWorldPosition = getPosInViewPlane(x,y,tube->worldP1);
				break;
			case 0:
				targetWorldPosition = getPosInViewPlane(x,y,0.5f*(tube->worldP2+tube->worldP1));
				break;
			case 1:
				targetWorldPosition = getPosInViewPlane(x,y,tube->worldP2);
				break;
		}

		//PROJECT THE TARGET POSITION THE BONE AXIS
		if(Input::keyStates[SNAP_KEY]){
			Vec3f axis = tube->skeleton->worldOrientation.inverse().apply(Vec3f(0,0,1));
			Vec3f origine = tube->skeleton->worldPosition;

			targetWorldPosition = origine + dot(targetWorldPosition-origine,axis)*axis;
		}

		switch(this->sideToScale){
			case -1:
				tube->setWorldP1(targetWorldPosition);
				break;
			case 0:
				tube->setWorldPosition(targetWorldPosition);
				break;
			case 1:
				tube->setWorldP2(targetWorldPosition);
				break;
		}


		tube->_updateWorldGeometry();
		tube->needsUpdate = true;
	}
}

void SurfaceEditor::setSpherePosition(int x, int y){
	if(this->selectedPrimitive->getType() == "MetaBall"){
		Metaball* ball = static_cast<Metaball*>(selectedPrimitive);
		Vec3f targetWorldPosition = getPosInViewPlane(x,y,ball->worldC);
		ball->setWorldPosition(targetWorldPosition);
		ball->needsUpdate = true;
	}
}
