#ifndef SINGLETON_H
#define SINGLETON_H

template <class T>
class Singleton{
	protected:
		static T* instance;
	public:
		static T* getInstance(){
			if(!instance)
				instance = new T();
			return instance;
		}	
};

#endif
