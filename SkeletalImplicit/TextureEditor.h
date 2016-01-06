#ifndef TEXTUREEDITOR_H
#define TEXTUREEDITOR_H

#include "IEditor.h"

class TextureProjector;

class TextureEditor : public IEditor{

	private:
		bool movingProj;
		Quatf oldOrientation;
		

	public:
		Vec3f oldRotationTarget;
		Vec3f rotationTarget;
		TextureProjector* selectedProjector;
		vector<TextureProjector*> texProjectors;

	public:
		TextureEditor(vector<Skeleton*>& boneList);
		virtual ~TextureEditor();

		virtual void initialize();
		virtual void open();
		virtual void close();

		virtual void openModel(Skeleton* model, const string& ressourceName);
		virtual void saveModel(Skeleton* model, const string& ressourceName);
		virtual void dropModel();

		virtual bool onMouseMotion(int x, int y);
		virtual bool onMouseButton(int button, int state, int x, int y);
		virtual void onKeyboard(unsigned char key, int x, int y);

		
		TextureProjector* selectProjectorAt(int x, int y);
};

#endif
