#include "MainEditor.h"

#include "SklImp/Skeleton.h"
#include "SklImp/SkeletalImplicit.h"
#include "SklImp/MetaTube.h"
#include "SklImp/Metaball.h"
#include "SklImp/BVH.h"
#include "SklImp/Animation.h"

#include "Input.h"
#include "DisplayText.h"

#include "Utils/RayCastUtils.h"

#include <GLGraphics/gel_glut.h>


MainEditor::MainEditor(vector<Skeleton*>& boneList):
IEditor(boneList),
helpTextId(0),
modelNameTextId(0),
showhelp(true),
modelName("New model*"),
userWritesName(false),
userBrowsesDir(false),
animation(0),
bvh(0),
mani(0)
{
	std::ifstream ifs;
	ifs.open("Controls.txt",ios::in);
	if(!ifs.fail()){
		helpString = string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
	}
}

MainEditor::~MainEditor(void)
{
}

void MainEditor::addEditor(IEditor* editor){
	this->editors.push_back(editor);
}

void MainEditor::initialize(){
	modelNameTextId = this->textDisplayer->addText("New model*");
	this->textDisplayer->addText("\n\n");
	if(this->showhelp)
		this->helpTextId = this->textDisplayer->addText(helpString);
	else
		this->helpTextId = this->textDisplayer->addText("");
}

void MainEditor::open(){
	this->textDisplayer->updateText(IEditor::modeTextId,"View mode");
}
void MainEditor::close(){
	if(this->animation)
		this->animation->stop();
}
bool MainEditor::onMouseMotion(int x, int y){
	return false;
}

void MainEditor::openModel(Skeleton* model, const string& ressourceName){
	for(size_t i = 0; i < this->editors.size(); i++){
		this->editors[i]->openModel(model,ressourceName);
	}
}

void MainEditor::saveModel(Skeleton* model, const string& ressourceName){
	save(*this->rootSkeleton,modelDirectory+'/'+ressourceName);
	for(size_t i = 0; i < this->editors.size(); i++){
		this->editors[i]->saveModel(model,ressourceName);
	}
	if(this->mani){
		if(this->mani->no_vertices() > 0)
			obj_save("Models/"+this->getModelName()+".obj",*mani);
	}
}

void MainEditor::dropModel(){
	for(size_t i = 0; i < this->editors.size(); i++){
		this->editors[i]->dropModel();
	}
}

void MainEditor::createNewEmptyModel(){
	clearSkeleton();
	createEmpty();
	modelName = "New model*";
	this->textDisplayer->updateText(modelNameTextId,modelName);
	this->initializeSkeleton(this->rootSkeleton, this->implicitSurface);
	this->openModel(this->rootSkeleton,"None");
}

bool MainEditor::onMouseButton(int button, int state, int x, int y){
	return false;
}
void MainEditor::onKeyboard(unsigned char key, int x, int y){
	if(glutGetModifiers() == GLUT_ACTIVE_CTRL){
		if(key == KEY_S){//s
			if( modelName == "New model*"){
				modelName = "";
				this->textDisplayer->updateText(modelNameTextId,modelName);
				userWritesName = true;
			}else{
				std::string filename(modelName+".skl");
				this->saveModel(this->rootSkeleton, filename);
			}
			return;
		}else if(key == KEY_O){
			readModelsDirectory();
			if(!modelNames.empty()){
				userBrowsesDir = true;
			}
			string bModelName = modelNames[0].filename();
			bModelName = bModelName.substr(0,bModelName.find("."));
			this->textDisplayer->updateText(modelNameTextId,bModelName);
			return;
		}else if(key == KEY_N){
			clearSkeleton();
			createEmpty();
			modelName = "New model*";
			this->textDisplayer->updateText(modelNameTextId,modelName);
			this->initializeSkeleton(this->rootSkeleton, this->implicitSurface);
			this->dropModel();
			this->openModel(this->rootSkeleton,"None");
		}
	}
	if(userWritesName){
		if( (int)key == KEY_ENTER){//enter
			cout<<"enter "<<modelName<<endl;
			userWritesName = false;
			std::string filename(modelName+".skl");
			this->saveModel(this->rootSkeleton, filename);
			return;
		}
		modelName += key;
		this->textDisplayer->updateText(modelNameTextId,modelName);
		return;
		
	}
	

	static int modelIndex = 0;
	switch (key) {

		case 'h':
			showhelp = !showhelp;
			if(showhelp)
				this->textDisplayer->updateText(helpTextId,helpString);
			else
				this->textDisplayer->updateText(helpTextId,"");
			break;
		case GLUT_KEY_RIGHT:
			if(userBrowsesDir){
				string bModelName;
				modelIndex = (modelIndex+1)%modelNames.size();
				bModelName = modelNames[modelIndex].filename();
				bModelName = bModelName.substr(0,bModelName.find("."));
				this->textDisplayer->updateText(modelNameTextId,bModelName);
			}
			break;
		case GLUT_KEY_LEFT:
			if(userBrowsesDir){
				string bModelName;
				modelIndex = (modelIndex-1)%modelNames.size();
				bModelName = modelNames[modelIndex].filename();
				bModelName = bModelName.substr(0,bModelName.find("."));
				this->textDisplayer->updateText(modelNameTextId,bModelName);
			}
			break;
		case KEY_ENTER:
			if(userBrowsesDir){
				string bModelName;
				userBrowsesDir = false;
				clearSkeleton();
				
				this->loadAndInitializeSkeleton(this->rootSkeleton, this->implicitSurface, modelNames[modelIndex].filename());
				this->openModel(this->rootSkeleton,modelNames[modelIndex].filename());
				bModelName = modelNames[modelIndex].filename();
				modelName = bModelName.substr(0,bModelName.find("."));
				modelIndex = 0;
			}
	}

}

void MainEditor::readModelsDirectory(){
	modelNames.clear();
	path p(modelDirectory);
	try{
		if (exists(p)){    // does p actually exist?
			if (is_regular_file(p))        // is p a regular file?   
				cout << p << " size is " << file_size(p) << '\n';
			else if (is_directory(p)){      // is p a directory?
				directory_iterator dit = directory_iterator(p);
				for(; dit != directory_iterator(); ++dit){
					if(is_regular_file(*dit)){
						if((*dit).filename().find(".skl") != string::npos)
							modelNames.push_back(*dit);
					}
				}
				//copy(directory_iterator(p), directory_iterator(), back_inserter(modelNames));
			}
		}else
			cout<<"Couldn'f find model directory"<<endl;
	}
	catch (const filesystem_error& ex){
		cout << ex.what() << '\n';
	}
}

void MainEditor::loadAndInitializeSkeleton(Skeleton* outSkel, SkeletalImplicit* implicit, const string& filename){
	std::ifstream ifs;
	ifs.open((modelDirectory+'/'+filename).c_str(),ios::in);
	if(ifs.fail()){
		cout << "Error: Unable to open "<<filename<<endl;
		return;
	}

	cout<<"loading model "<<filename<<endl;
	load(*outSkel,ifs);

	string bModelName = filename;
	bModelName = bModelName.substr(0,bModelName.find("."));
	modelName = bModelName;
	this->textDisplayer->updateText(modelNameTextId,bModelName);
	
	initializeSkeleton(outSkel,implicit);

	ifs.close();
	cout<<"model loaded"<<endl;
}

void MainEditor::initializeSkeleton(Skeleton* outSkel, SkeletalImplicit* implicit){
	
	cout<<"Extracting bones"<<endl;
	/****************************READ SKELETON TO EXTRACT ALL BONES***********/
	Skeleton* node;
	vector<Skeleton*> openList;
	openList.push_back(outSkel);
	
	while( !openList.empty() ){	
		node = openList.back();
		openList.pop_back();
		
		if(node){
			this->bones.push_back(node);
			node->id = bones.size()-1;
		}
		for(size_t i = 0; i < node->children.size(); i++){
			openList.push_back(node->children[i]);				
		}
	}

	//Initialize the implicit function
	cout<<"Initializing implicit function"<<endl;
	implicit->readSkeleton(outSkel);
	this->implicitSurface = implicit;
}

void MainEditor::clearSkeleton(){
	cout<<"clear skeleton"<<endl;
	this->rootSkeleton->clear();                 //The skeleton
	this->bones.clear();           //separate list of all the bones
	cout<<"clear surface"<<endl;
	this->implicitSurface->clear(); 
	cout<<"drop model"<<endl;
	this->dropModel();
}


void MainEditor::createEmpty(){
	this->rootSkeleton->translate(Vec3f(0,0,0.0f));
	this->rootSkeleton->setLength(0.0f);
	//skeleton.addPrimitive();
	this->rootSkeleton->name = "center";
}