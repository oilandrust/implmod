#ifndef ANIMATION_H
#define ANIMATION_H

#include "CGLA\Vec3f.h"
#include "CGLA\Quatf.h"
#include <vector>
#include <map>

using namespace CGLA;
using namespace std;

class Skeleton;

class KeyFrame{
	public:
		unsigned int time;
		Quatf* rotations;
		KeyFrame* next;
		KeyFrame():next(0),rotations(0){}
		~KeyFrame(){
			if(this->next)
				delete this->next;
		}	
};
class AnimationClip{
	public:
		int fade;
		bool loop;
		bool playing;
		int keyFrameCount;
		string name;
		KeyFrame* first;
		KeyFrame* current;
		KeyFrame* last;
		int nbBones;
		Skeleton** bones;

		AnimationClip():
			first(0),
			current(0),
			bones(0),
			keyFrameCount(0),
			last(0)
		{}
		~AnimationClip(){
			delete this->first;
			delete[] this->bones;
		}

		void pushKeyFrame(KeyFrame* key){
			if(this->keyFrameCount == 0){
				this->first = this->last = this->current = key;
				key->time = 0;
			}else{
				this->last->next = key;
				key->time = this->last->time+400;
				this->last = key;
			}
			this->keyFrameCount++;
		}
};


class Animation{
	public:
		Animation();
		~Animation();

		void addClip(AnimationClip* clip);
		void removeClip(AnimationClip* clip);
		void animate(unsigned int t);
		void play();
		void play(const string& clip);
		void pause();
		void fadeIn(const string& clip);
		void fadeOut(const string& clip);
		void clear();

	public:
		vector<AnimationClip*> clips;
		Quatf* restPose;
	protected:	
		bool playing;
		vector<float> weights;
		vector<unsigned int> startTimes;
		map<Skeleton*,Quatf> blendedRotations;
		map<string,AnimationClip*> clipsmap;
};

void save(Animation& skeleton, const string& filename);
void load(Animation& skeleton,  std::ifstream& ifs);

#endif
