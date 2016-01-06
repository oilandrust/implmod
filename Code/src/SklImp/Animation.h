#ifndef ANIMATION_H
#define ANIMATION_H

#include "CGLA\Vec3f.h"
#include "CGLA\Quatf.h"
#include <vector>
#include <map>
#include <list>

using namespace CGLA;
using namespace std;

class Skeleton;

class KeyFrame{
	public:
		unsigned int time;
		vector<Quatf> rotations;

		KeyFrame():time(0){}
		KeyFrame(const KeyFrame& other):time(other.time),rotations(other.rotations){}
		~KeyFrame(){}	
};
class AnimationClip{
	public:
		typedef list<KeyFrame>::iterator keyFrameIterator;

	protected:
		list<KeyFrame>::iterator curr;
		//key frames
		list<KeyFrame> keyFrames;
		//Bones
		vector<Skeleton*> bones;
		
	public:
		//Playing parameters
		int fade;
		bool loop;
		bool playing;
		float length;
		
		string name;

		AnimationClip():
		fade(0),
		playing(false),
		loop(true),
		length(0){
		}

		~AnimationClip(){
		}

		void initialize(const vector<Skeleton*>& skeleton){
			assert(this->bones.empty());
			assert(this->keyFrames.empty());

			this->bones = skeleton;
		}

		void clearTarget(){
			this->bones.clear();
		}

		void setTargetSkeleton(const vector<Skeleton*>& skeleton){
			assert(this->bones.empty());

			this->bones = skeleton;
		}

		void pushKeyFrame(const KeyFrame& key){

			KeyFrame newKey(key);
			if(key.time == 0){
				if(this->keyFrames.empty()){//Adding the first keyframe
					newKey.time = 0;
				}else{//Set an arbitrary time based on the last key's time
					newKey.time = this->keyFrames.back().time + 400;
				}
			}

			this->length = newKey.time;
			this->keyFrames.push_back(newKey);
		}

		inline void start(){
			this->curr = this->keyFrames.begin();
		}

		inline size_t keyFrameCount()const{
			return this->keyFrames.size();
		}
		inline KeyFrame* getKeyFrame(size_t i){
			if(i > this->keyFrames.size() - 1)
				return 0;

			keyFrameIterator it = this->keyIteratorBegin();
			advance(it,i);
			return &*it;
		}
		inline size_t boneCount()const{
			return this->bones.size();
		}
		inline Skeleton* const getBone(size_t i)const{
			if(i > this->bones.size() - 1)
				return 0;
			return this->bones[i];
		}
		inline KeyFrame const* const currentKey()const{
			return &*(this->curr);
		}
		inline KeyFrame const* const nextKey()const{
			list<KeyFrame>::iterator next = curr;
			next++;
			return (next == this->keyFrames.end()) ? &*(this->keyFrames.begin()) : &*next;
		}
		inline void moveToNextKeyInterval(){
			++(this->curr);
			if(this->curr == this->keyFrames.end()){
				playing = this->loop;
				this->curr = this->keyFrames.begin();
			}
		}

		inline keyFrameIterator keyIteratorBegin(){
			return this->keyFrames.begin();
		}
		inline keyFrameIterator keyIteratorEnd(){
			return this->keyFrames.end();
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
		void stop();
		void play(const string& clip);
		void pause();
		void fadeIn(const string& clip);
		void fadeOut(const string& clip);
		void clear();

		bool playing(){return this->m_playing;}
		AnimationClip* getClip(size_t i);
		void save(const string& filename);

		inline const vector<Quatf>& getRestPoseQuats()const{
			return this->restPoseQuats;
		}
		void setModelToRestPose();
		void saveRestPose(const vector<Skeleton*>& modelBonnes);
		
	protected:

		vector<AnimationClip*> clips;
		bool m_playing;
		vector<float> weights;
		vector<unsigned int> startTimes;
		map<Skeleton*,Quatf> blendedRotations;
		map<string,AnimationClip*> clipsmap;

		vector<Quatf> restPoseQuats;
		vector<Mat4x4f> restPoseMatrixes;
};


void load(Animation& skeleton,  std::ifstream& ifs);

#endif
