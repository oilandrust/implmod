#include "Animation.h"
#include "Skeleton.h"
#include <fstream>
#include <iostream>


Animation::Animation():restPose(0){
}


Animation::~Animation(){
	vector<AnimationClip*>::iterator it;
    for ( it = clips.begin() ; it < clips.end(); ++it ){
        delete * it;
    }
}

void Animation::clear(){
	this->restPose = NULL;
	playing = false;
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
	//cout<<"play "<<clipsmap[clip]->name<<endl;
	clipsmap[clip]->playing = true;
}
void Animation::fadeIn(const string& clip){
	//cout<<"fade in "<<clipsmap[clip]->name<<endl;
	map<string,AnimationClip*>::iterator it = clipsmap.begin();
	while((*it).first != clip && it != clipsmap.end())
		++it;
	if((*it).first == clip){
		(*it).second->fade = 1;
	}
}
void Animation::fadeOut(const string& clip){
	//cout<<"fade out "<<clipsmap[clip]->name<<endl;
		map<string,AnimationClip*>::iterator it = clipsmap.begin();
	while((*it).first != clip && it != clipsmap.end())
		++it;
	if((*it).first == clip)
		(*it).second->fade = -1;
}

void Animation::animate(unsigned int t){
	float sumWeights = 0;
	for(unsigned int i = 0; i < clips.size(); i++){

		if( clips[i]->playing  && clips[i]->keyFrameCount > 0){
	
			if( clips[i]->fade != 0 ){
				weights[i] += (float)clips[i]->fade/10;
				if( weights[i] > 1){
					clips[i]->fade = 0;
					weights[i] = 1;
				}
				if( weights[i] < 0){
					clips[i]->fade = 0;
					weights[i] = 0;
				}
			}
			
			sumWeights += weights[i];
			KeyFrame* prevKey = clips[i]->current;
			KeyFrame* nextKey = NULL;
			if( clips[i]->current->next )
				nextKey = clips[i]->current->next;
			else{
				nextKey = clips[i]->first;
			}

			unsigned int animTime = t - startTimes[i];

			if( weights[i] != 0){			
				for(int j = 0; j < clips[i]->nbBones; j++){
					Quatf interpolated;
					if(prevKey->rotations[j] != nextKey->rotations[j])
						 interpolated = slerp(prevKey->rotations[j],nextKey->rotations[j],(float)(animTime - prevKey->time)/(float)(nextKey->time - prevKey->time));
					else
						interpolated = prevKey->rotations[j];
					blendedRotations[clips[i]->bones[j]] = interpolated;
				}
			}

			if( animTime > nextKey->time ){
				clips[i]->current = nextKey;
				if(!prevKey->next){
					clips[i]->playing = clips[i]->loop;
					startTimes[i] = t;
				}
			}
		}
	}

	map<Skeleton*,Quatf>::iterator it = blendedRotations.begin();
	for(; it != blendedRotations.end(); ++it){
		(*it).first->setPose((*it).second);
		(*it).second = (*it).second.identity_Quatf();
	}
}

void save(Animation& anim, const string& filemame){
	std::ofstream ofs(filemame.c_str(),ios::out);
	
	ofs << anim.clips.size() <<endl;
	
	for(int i = 0; i < anim.clips.size(); i++){
		AnimationClip* clip = anim.clips[i];
		int kFrameCount = clip->keyFrameCount;
		ofs     << clip->name <<
			" " << clip->nbBones <<
			" " << kFrameCount << endl;
		KeyFrame* key = clip->first;
		for(int k = 0; k < kFrameCount; k++){
			ofs << key->time <<" ";
			for(int b = 0; b < clip->nbBones; b++){
				ofs     << key->rotations[b].qv[0] <<
					" " << key->rotations[b].qv[1] <<
					" " << key->rotations[b].qv[2] <<
					" " << key->rotations[b].qw    <<
					endl;
			}
			key = key->next;
		}
	}

	ofs.close();

}
void load(Animation& anim, std::ifstream& ifs){
	int clipCount = 0;
	ifs >> clipCount;

	for(int i = 0; i < clipCount; i++){
		AnimationClip* clip = new AnimationClip();

		ifs >> clip->name;
		ifs >> clip->nbBones;

		int keyFrameCount = 0;
		ifs >> keyFrameCount;
		
		for(int k = 0; k < keyFrameCount; k++){
			KeyFrame* key = new KeyFrame();
			ifs >> key->time;
			key->rotations = new Quatf[clip->nbBones];
			for(int b = 0; b < clip->nbBones; b++){
				ifs >> key->rotations[b].qv[0];
				ifs >> key->rotations[b].qv[1]; 
				ifs >> key->rotations[b].qv[2];
				ifs >> key->rotations[b].qw;
			}
			clip->pushKeyFrame(key);
		}
		anim.addClip(clip);
	}

}