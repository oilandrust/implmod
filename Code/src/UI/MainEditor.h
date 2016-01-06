#ifndef MAINEDITOR_H
#define MAINEDITOR_H

#include "IEditor.h"
#include <boost/filesystem.hpp>
#include <string>
using namespace boost::filesystem;

#include <HMesh/Manifold.h>
#include <HMesh/obj_save.h>
using namespace HMesh;

class CompactBvh;
class Animation;

class MainEditor: public IEditor{
	private:

		//text ids
		int helpTextId;
		int modelNameTextId;
		string helpString;
		bool showhelp;
		string modelName;
		vector<path> modelNames;
		bool userWritesName;
		bool userBrowsesDir;

		Animation* animation;
		

		vector<IEditor*> editors;

	public:
		Manifold* mani;
		CompactBvh* bvh;

		MainEditor(vector<Skeleton*>& boneList);
		~MainEditor();

		virtual void initialize();

		virtual void open();
		virtual void close();

		virtual void openModel(Skeleton* model, const string& ressourceName);
		virtual void saveModel(Skeleton* model, const string& ressourceName);
		virtual void dropModel();

		virtual bool onMouseMotion(int x, int y);
		virtual bool onMouseButton(int button, int state, int x, int y);
		virtual void onKeyboard(unsigned char key, int x, int y);

		void addEditor(IEditor* editor);
		void setAnimation(Animation* anim){this->animation = anim;}
		string getModelName(){
			return this->modelName;
		}
		
		void createNewEmptyModel();

	private:
		void readModelsDirectory();
		void loadAndInitializeSkeleton(Skeleton* outSkel, SkeletalImplicit* implicit,const string& filename);
		void initializeSkeleton(Skeleton* outSkel, SkeletalImplicit* implicit);
		void clearSkeleton();
		void createEmpty();
		
};

#endif