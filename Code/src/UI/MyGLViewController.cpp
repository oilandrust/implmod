#include <GLGraphics/gel_glut.h>
#include "MyGLViewController.h"

using namespace std;
using namespace CGLA;

namespace GLGraphics
{
    
  void MyGLViewController::reset_projection()
  {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(FOV_DEG, aspect, znear, zfar);
    glMatrixMode(GL_MODELVIEW);
  }

  MyGLViewController::MyGLViewController(int _WINX, int _WINY, const CGLA::Vec3f& centre, float rad)
    : FOV_DEG(53),
      WINX(_WINX), WINY(_WINY), 
      aspect(WINX/WINY),
      button_down(false),
      spin(false),
      ball(centre, rad, WINX, WINY)
  {
    znear = 0.01f*rad;
    zfar  = 3*rad;
    reset_projection();
  }

  void MyGLViewController::grab_ball(MyTrackBallAction action, const CGLA::Vec2i& pos)
  {
    ball.grab_ball(action,pos);
    if(action==ZOOM_ACTION)
      set_near_and_far();

    spin = false;
    button_down = true;
    last_action = action;
  }

  void MyGLViewController::roll_ball(const CGLA::Vec2i& pos)
  {
    static Vec2i old_pos = pos;
    Vec2f dir = Vec2f(pos-old_pos);
    float len = dir.length();
    if (len < TINY)
      return;
    
    ball.roll_ball(pos);
    if(last_action==ZOOM_ACTION)
      set_near_and_far();
    
    spin = len>=1.1f;
    old_pos = pos;  
  }


  void MyGLViewController::release_ball()
  {
    ball.release_ball();
    if(last_action==ZOOM_ACTION)
      set_near_and_far();
  }

  bool MyGLViewController::try_spin()
  {
    if(spin && !ball.is_grabbed()) 
    {
      ball.do_spin();
      return true;
    }
    return false;
  }
  
  void MyGLViewController::set_gl_modelview()
  {
    ball.set_gl_modelview();
  }


  void MyGLViewController::reshape(int W, int H)
  {
    WINX = W;
    WINY = H;
    aspect = WINX/static_cast<float>(WINY);
    glViewport(0,0,WINX,WINY);
    reset_projection();
    ball.set_screen_window(WINX, WINY);
  }  

  void MyGLViewController::set_near_and_far()
  {  
    float rad = ball.get_eye_dist();
    znear = 0.01f*rad;
    zfar = 3*rad;
    reset_projection();
  }

  bool MyGLViewController::load(std::ifstream& ifs)
  {
    if(ifs)
    {
      ifs.read(reinterpret_cast<char*>(this), sizeof(MyGLViewController));
      reset_projection();
      ball.set_screen_window(WINX, WINY);
      return true;
    }
    return false;
  }
  bool MyGLViewController::save(std::ofstream& ofs) const
  {
    if(ofs)
    {
      ofs.write(reinterpret_cast<const char*>(this), sizeof(MyGLViewController));
      return true;
    }
    return false;
   }
}
