#include "Animation.h"
#include "Skeleton.h"
#include <fstream>
#include <iostream>


Animation::Animation():
m_playing(false){
}


Animation::~Animation(){
	vector<AnimationClip*>::iterator it;
    for ( it = clips.begin() ; it < clips.end(); ++it ){
        delete *it;
    }
}

void Animation::clear(){
	this->restPoseQuats.clear();
	this->restPoseMatrixes.clear();
	this->m_playing = false;
	clips.clear();
	weights.clear();
	startTimes.clear();
	blendedRotations.clear();
	clipsmap.clear();
}

void Animation::addClip(AnimationClip* clip){
	clipsmap.insert( pair<string,AnimationClip*>(clip->name,clip) );
	clips.push_back(clipsmap[clip->name]);
	weights.push_back(1);
	startTimes.push_back(0);
}
void Animation::removeClip(AnimationClip* clip){
}

void Animation::play(const string& clip){

	this->m_playing = true;
	//cout<<"play "<<clipsmap[clip]->name<<endl;
	clipsmap[clip]->playing = true;
}

void Animation::play(){
	
	this->m_playing = true;
	for(size_t i = 0; i < clips.size(); i++){
		if( !clips[i]->playing){
			clips[i]->start();
			clips[i]->playing = true;
		}
	}
}
void Animation::stop(){
	this->m_playing = false;
	for(size_t i = 0; i < clips.size(); i++){
		clips[i]->playing = false;
		this->setModelToRestPose();
	}
}
void Animation::pause(){
	this->m_playing = false;
}

void Animation::fadeIn(const string& clip){
	//cout<<"fade in "<<clipsmap[clip]->name<<endl;
	map<string,AnimationClip*>::iterator it = clipsmap.find(clip);
	if(it != clipsmap.end()){
		(*it).second->fade = 1;
	}
}
void Animation::fadeOut(const string& clip){
	//cout<<"fade out "<<clipsmap[clip]->name<<endl;
	map<string,AnimationClip*>::iterator it = clipsmap.find(clip);
	if(it != clipsmap.end()){
		(*it).second->fade = -1;
	}
}

AnimationClip* Animation::getClip(size_t i){
	assert(this->clips.size() > i);
	return this->clips[i];
}

void Animation::animate(unsigned int t){
	

	if(this->m_playing){
		for(size_t i = 0; i < clips.size(); i++){

			if( clips[i]->playing  && clips[i]->keyFrameCount() > 0){

				const KeyFrame*const prevKey = clips[i]->currentKey();
				const KeyFrame*const nextKey = clips[i]->nextKey();

				unsigned int animTime = t - startTimes[i];
			
				for(size_t j = 0; j < clips[i]->boneCount(); j++){
					Quatf interpolated;
					if(prevKey->rotations[j] != nextKey->rotations[j])
						 interpolated = slerp(prevKey->rotations[j],nextKey->rotations[j],(float)(animTime - prevKey->time)/(float)(nextKey->time - prevKey->time));
					else
						interpolated = prevKey->rotations[j];
					blendedRotations[clips[i]->getBone(j)] = interpolated;
				}
				
				if( animTime > nextKey->time ){
					clips[i]->moveToNextKeyInterval();
					if(animTime >= clips[i]->length)
						startTimes[i] = t;
				}
			}
		}

		map<Skeleton*,Quatf>::iterator it = blendedRotations.begin();
		for(; it != blendedRotations.end(); ++it){
			(*it).first->setPose((*it).second);
			//(*it).second = (*it).second.identity_Quatf();
		}
	}
}
void Animation::setModelToRestPose(){
	if(!this->clips.empty()){
		for(size_t i = 0; i < this->clips[0]->boneCount(); i++){
			this->clips[0]->getBone(i)->orientation = this->restPoseQuats[i];
			this->clips[0]->getBone(i)->_updateWorldOrientation();
		}
	}

}

void Animation::saveRestPose(const vector<Skeleton*>& modelBonnes){
	assert(!this->clips.empty());
	assert(modelBonnes.size() == this->clips[0]->boneCount());

	this->restPoseQuats.clear();
	this->restPoseMatrixes.clear();

	this->restPoseQuats.reserve(modelBonnes.size());
	this->restPoseMatrixes.reserve(modelBonnes.size());
	
	for(size_t i = 0; i < modelBonnes.size(); i++){
		this->restPoseQuats.push_back(modelBonnes[i]->orientation);
		this->restPoseMatrixes.push_back(modelBonnes[i]->worldOrientation.get_Mat4x4f());
		this->restPoseMatrixes[i][3] = Vec4f(modelBonnes[i]->worldPosition,1);
	}
}

void Animation::save(const string& filemame){
	std::ofstream ofs(filemame.c_str(),ios::out);
	
	ofs << this->clips.size() <<endl;
	
	//SAVE EACH CLIP
	for(size_t i = 0; i < this->clips.size(); i++){

		AnimationClip* clip = this->clips[i];
		size_t kFrameCount = clip->keyFrameCount();
		ofs     << clip->boneCount() <<
			" " << kFrameCount << endl;

		//SAVE EACH KEY FRAME
		AnimationClip::keyFrameIterator kIt = clip->keyIteratorBegin();
		for(; kIt != clip->keyIteratorEnd(); ++kIt){
			ofs << kIt->time <<" ";
			for(size_t b = 0; b < clip->boneCount(); b++){
				ofs     << kIt->rotations[b].qv[0] <<
					" " << kIt->rotations[b].qv[1] <<
					" " << kIt->rotations[b].qv[2] <<
					" " << kIt->rotations[b].qw    <<
					endl;
			}
		}
	}

	ofs.close();

}
void load(Animation& anim, std::ifstream& ifs){
	int clipCount = 0;
	ifs >> clipCount;

	//LOAD EACH CLIP
	for(int i = 0; i < clipCount; i++){
		AnimationClip* clip = new AnimationClip();

		int keyFrameCount = 0;
		int boneCount = 0;

		ifs >> boneCount;
		ifs >> keyFrameCount;
		
		//LOAD EACH KEY FRAME
		for(int k = 0; k < keyFrameCount; k++){
			
			KeyFrame key;
			ifs >> key.time;
			
			key.rotations.reserve(boneCount);
			//LOAD EACH KEY QUATERNIONS
			for(size_t b = 0; b < boneCount; b++){
				Quatf quat;
				ifs >> quat.qv[0];
				ifs >> quat.qv[1]; 
				ifs >> quat.qv[2];
				ifs >> quat.qw;
				key.rotations.push_back(quat);
			}
			clip->pushKeyFrame(key);
		}
		anim.addClip(clip);
	}

}