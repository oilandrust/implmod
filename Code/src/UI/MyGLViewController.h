#ifndef __GLGRAPHICS_MYMyGLViewController_H__
#define __GLGRAPHICS_MYMyGLViewController_H__

#include <fstream>
#include "MyQuatTrackBall.h"

namespace GLGraphics
{
	/** The MyGLViewController is a more high level component than a trackball.
		The idea behind MyGLViewController is to handle setting up the projection
		and changing the viewport when the window is reshaped. Basically the raw
		mouse position and related info is sent to the view controller which takes
		care of the rest.
		*/
	class MyGLViewController
	{
		float FOV_DEG;
		int WINX, WINY;
		float znear, zfar;
		float aspect;
		bool button_down;
		MyTrackBallAction last_action;
		bool spin;

		MyQuatTrackBall ball;
		

	public:

		/** Constructor which accepts the window dimensions as well as the world center and the 
			radius which should be construed as the distance to the observer */
		MyGLViewController(int _WINX, int _WINY,
										 const CGLA::Vec3f& _centre, float _rad);
										 
		/// Grab ball takes an action and a mouse position.
		void grab_ball(MyTrackBallAction action, const CGLA::Vec2i& pos);
		
		/// Roll virtual trackball (pass just mouse position).
		void roll_ball(const CGLA::Vec2i& pos);
		
		/// Release the virtual trackball
		void release_ball();
		
		/// Try to spind the trackball - called from idle.
		bool try_spin();
				
		/// Setup GL modelview matrix.
		void set_gl_modelview();

		/// Reset projection. Called initially, when window size has changed or when user zooms. 
		void reset_projection();

		/// Reshape window.
		void reshape(int W, int H);
		
		/// Set near and far planes.
		void set_near_and_far();
		
		/// Set centre of ball.
		void set_centre(const CGLA::Vec3f& c)
		{
			ball.set_centre(c);
		}
		
		/// Set eye distance.
		void set_eye_dist(float rad)
		{
			ball.set_eye_dist(rad);
			set_near_and_far();
		}

		/// Returns eye distance
		float get_eye_dist() const
		{
			return ball.get_eye_dist();
		}

		/// Get viewing parameters: eye, centre, up
		void get_view_param(CGLA::Vec3f& e, CGLA::Vec3f& c, CGLA::Vec3f& u) const
		{
			ball.get_view_param(e,c,u);
		}

		/// Load trackball from stream
		bool load(std::ifstream&);
		
		/// Save trackball to stream.
		bool save(std::ofstream&) const;
	};
	
}

#endif
