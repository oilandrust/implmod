#ifndef MAINEDITOR_H
#define MAINEDITOR_H

#include "IEditor.h"
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

class CompactBvh;

class MainEditor: public IEditor{
	private:
		bool ctrlDown;

		//text ids
		int helpTextId;
		int modelNameTextId;
		string helpString;
		bool showhelp;
		string modelName;
		vector<path> modelNames;
		bool userWritesName;
		bool userBrowsesDir;

		vector<IEditor*> editors;

	public:
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

		void readModelsDirectory();
		void loadAndInitializeSkeleton(Skeleton* outSkel, SkeletalImplicit* implicit,const string& filename);
		void initializeSkeleton(Skeleton* outSkel, SkeletalImplicit* implicit);
		void clearSkeleton();
		void createEmpty();
};

#endif