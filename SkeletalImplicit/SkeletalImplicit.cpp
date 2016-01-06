#include "SkeletalImplicit.h"
#include "skeleton.h"
#include "Metaprimitive.h"
#include "MetaTube.h"
#include "Metaball.h"


SkeletalImplicit::SkeletalImplicit(){}

SkeletalImplicit::SkeletalImplicit(Skeleton* skeleton, float threshold):T(threshold){
	readSkeleton(skeleton);
} 

void SkeletalImplicit::readSkeleton(Skeleton* skeleton){
	Skeleton* node;
	vector<Skeleton*> openList;
	openList.push_back(skeleton);
	
	//Store all primitives
	while( !openList.empty() ){	
		node = openList.back();
		openList.pop_back();
		
		if(node){
			primitives.insert(primitives.end(),node->primitives.begin(),node->primitives.end());
		}
		for(unsigned int i = 0; i < node->children.size(); i++){
			openList.push_back(node->children[i]);				
		}
	}

	//Store separate MetaTubes for shader
	for(unsigned int i = 0 ; i < primitives.size(); i++){
		if(primitives[i]->getType() == "MetaTube"){
			MetaTube* metaTube = static_cast<MetaTube*>(primitives[i]);
			metaTube->id = p1s.size();
			p1s.push_back(metaTube->worldP1);
			p2s.push_back(metaTube->worldP2);
			radiuses.push_back(metaTube->r1*metaTube->r1);
		}else if(primitives[i]->getType() == "MetaBall"){
			Metaball* ball = static_cast<Metaball*>(primitives[i]);
			ball->id = spheres.size();
			spheres.push_back(ball->worldC);
			sRadius.push_back(ball->r*ball->r);
		}
	}
}

void SkeletalImplicit::update(){
	for(unsigned int i = 0; i < primitives.size(); i++){
		if(primitives[i]->needsUpdate){
			if(primitives[i]->getType() == "MetaTube"){
				MetaTube* metaTube = static_cast<MetaTube*>(primitives[i]);
				p1s[metaTube->id] = metaTube->worldP1;
				p2s[metaTube->id] = metaTube->worldP2;
				radiuses[metaTube->id] = metaTube->r1*metaTube->r1;
			}else if(primitives[i]->getType() == "MetaBall"){
				Metaball* ball = static_cast<Metaball*>(primitives[i]);
				spheres[ball->id] = ball->worldC;
				sRadius[ball->id] = ball->r*ball->r;
			}
			primitives[i]->needsUpdate = false;
		}
	}
}

void SkeletalImplicit::addBone(Skeleton* bone){
	primitives.insert(primitives.end(),bone->primitives.begin(),bone->primitives.end());
	for(unsigned int i = 0 ; i < bone->primitives.size(); i++){
		if(bone->primitives[i]->getType() == "MetaTube"){
			MetaTube* metaTube = static_cast<MetaTube*>(bone->primitives[i]);
			cout<<"MetaTube"<<metaTube->worldP1<<metaTube->worldP2<<endl;
			metaTube->id = p1s.size();
			p1s.push_back(metaTube->worldP1);
			p2s.push_back(metaTube->worldP2);
			radiuses.push_back(metaTube->r1*metaTube->r1);
		}else if(bone->primitives[i]->getType() == "MetaBall"){
			Metaball* ball = static_cast<Metaball*>(bone->primitives[i]);
			cout<<"ball"<<ball->worldC<<endl;
			ball->id = spheres.size();
			spheres.push_back(ball->worldC);
			sRadius.push_back(ball->r*ball->r);
		}
	}
}

void SkeletalImplicit::addPrimitive(MetaPrimitive* prim){
	primitives.push_back(prim);
	if(prim->getType() == "MetaTube"){
		MetaTube* metaTube = static_cast<MetaTube*>(prim);
		cout<<"MetaTube"<<metaTube->worldP1<<metaTube->worldP2<<endl;
		metaTube->id = p1s.size();
		p1s.push_back(metaTube->worldP1);
		p2s.push_back(metaTube->worldP2);
		radiuses.push_back(metaTube->r1*metaTube->r1);
	}else if(prim->getType() == "MetaBall"){
		Metaball* ball = static_cast<Metaball*>(prim);
		cout<<"ball"<<ball->worldC<<endl;
		ball->id = spheres.size();
		spheres.push_back(ball->worldC);
		sRadius.push_back(ball->r*ball->r);
	}
}

void SkeletalImplicit::removePrimitive(MetaPrimitive* prim){
	
	vector<MetaPrimitive*>::iterator it = this->primitives.begin();
	for(; it!=this->primitives.end(); ++it ){
		if(*it==prim)
			break;
	}
	this->primitives.erase(it);
	return;
	if(prim->getType() == "MetaTube"){
		
		MetaTube* metaTube = static_cast<MetaTube*>(prim);
		cout<<"remove MetaTube "<<metaTube->id<<endl;
		this->p1s.erase(this->p1s.begin()+metaTube->id);
		this->p2s.erase(this->p2s.begin()+metaTube->id);
		this->radiuses.erase(this->radiuses.begin()+metaTube->id);

		
	}else if(prim->getType() == "MetaBall"){

		Metaball* ball = static_cast<Metaball*>(prim);

		this->spheres.erase(this->spheres.begin()+ball->id);
		this->sRadius.erase(this->sRadius.begin()+ball->id);
	}
}

float SkeletalImplicit::eval(float x, float y, float z) {
	float value = -T;
	for(unsigned int i = 0; i < primitives.size(); i++){
		value += primitives[i]->eval(x,y,z);
	}
	return value;
}

void SkeletalImplicit::clear(){
	this->primitives.clear();
	this->spheres.clear();
	this->sRadius.clear();
	this->p1s.clear();
	this->p2s.clear();
	this->radiuses.clear();
} 