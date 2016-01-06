#define KEY_S 19
#define KEY_R 18
#define KEY_O 15
#define KEY_N 14
#define KEY_ENTER 13
#define KEY_TAB 9
#define KEY_SPACE 32
#define KEY_DEL 127

#define ROTATE_KEY 'r'//82 //r
#define SCALE_KEY 's'//83 //s
#define TRANSLATE_KEY 't'//84 //t
#define ADD_KEY 'a'
#define SNAP_KEY 'b'

#include <map>
using namespace std;

struct Mouse{
	int dx;
	int dy;
};

class Input{
	public:
		static std::map<unsigned char,bool> keyStates;
		static Mouse mouse;
		static bool SHIFT_DOWN;
		static bool CTRL_DOWN;
};

