/* === S Y N F I G ========================================================= */
/*!	\file preview.h
**	\brief Previews an animation
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**
**	This package is free software; you can redistribute it and/or
**	modify it under the terms of the GNU General Public License as
**	published by the Free Software Foundation; either version 2 of
**	the License, or (at your option) any later version.
**
**	This package is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
**	General Public License for more details.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_PREVIEW_H
#define __SYNFIG_PREVIEW_H

/* === H E A D E R S ======================================================= */
#include <ETL/handle>
#include <ETL/clock> /* indirectly includes winnt.h on WIN32 - needs to be included before gtkmm headers, which fix this */

#include <gtkmm/drawingarea.h>
#include <gtkmm/table.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/image.h>
#include <gdkmm/pixbuf.h>
#include <gtkmm/dialog.h>
#include <gtkmm/scrollbar.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/canvasview.h>
#include <gtkmm/tooltips.h>

#include <synfig/synfig_time.h>
#include <synfig/vector.h>
#include <synfig/general.h>
#include <synfig/renddesc.h>
#include <synfig/canvas.h>

#include "widgets/widget_sound.h"

#include <vector>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {
class AsyncRenderer;

class Preview : public sigc::trackable, public etl::shared_object
{
public:
	struct FlipbookElem
	{
		float						t;
		Glib::RefPtr<Gdk::Pixbuf>	buf; //at whatever resolution they are rendered at (resized at run time)
	};

	etl::handle<studio::AsyncRenderer>	renderer;

	sigc::signal<void, Preview *>	signal_destroyed_;	//so things can reference us without fear

	typedef std::vector<FlipbookElem>	 FlipBook;
private:

	FlipBook			frames;

	studio::CanvasView::LooseHandle	canvasview;

	//synfig::RendDesc		description; //for rendering the preview...
	float	zoom,fps;
	float	begintime,endtime;
	bool 	overbegin,overend;
	int		quality;

	float	global_fps;

	//expose the frame information etc.
	class Preview_Target;
	void frame_finish(const Preview_Target *);

	sigc::signal0<void>	sig_changed;

public:

	Preview(const studio::CanvasView::LooseHandle &h = studio::CanvasView::LooseHandle(),
				float zoom = 0.5f, float fps = 15);
	~Preview();

	float 	get_zoom() const {return zoom;}
	void	set_zoom(float z){zoom = z;}

	float 	get_fps() const {return fps;}
	void	set_fps(float f){fps = f;}

	float 	get_global_fps() const {return global_fps;}
	void	set_global_fps(float f){global_fps = f;}

	float	get_begintime() const
	{
		if(overbegin)
			return begintime;
		else if(canvasview)
			return get_canvas()->rend_desc().get_time_start();
		else return -1;
	}

	float	get_endtime() const
	{
		if(overend)
			return endtime;
		else if(canvasview)
			return get_canvas()->rend_desc().get_time_end();
		else return -1;
	}

	void	set_begintime(float t)	{begintime = t;}
	void	set_endtime(float t) 	{endtime = t;}

	bool get_overbegin() const {return overbegin;}
	void set_overbegin(bool b) {overbegin = b;}

	bool get_overend() const {return overend;}
	void set_overend(bool b) {overend = b;}

	int		get_quality() const {return quality;}
	void	set_quality(int i)	{quality = i;}

	synfig::Canvas::Handle	get_canvas() const {return canvasview->get_canvas();}
	studio::CanvasView::Handle	get_canvasview() const {return canvasview;}

	void set_canvasview(const studio::CanvasView::LooseHandle &h);

	//signal interface
	sigc::signal<void, Preview *> &	signal_destroyed() { return signal_destroyed_; }
	//sigc::signal<void, const synfig::RendDesc &>	&signal_desc_change() {return signal_desc_change_;}

	//functions for exposing iterators through the preview
	FlipBook::iterator	begin() 	{return frames.begin();}
	FlipBook::iterator	end() 		{return frames.end();}

	FlipBook::const_iterator	begin() const {return frames.begin();}
	FlipBook::const_iterator	end() const	  {return frames.end();}

	void clear() {frames.clear();}

	unsigned int				numframes() const  {return frames.size();}

	void render();

	sigc::signal0<void>	&signal_changed() { return sig_changed; }
};

class Widget_Preview : public Gtk::Table
{
	Gtk::DrawingArea	draw_area;
	Gtk::Adjustment 	adj_time_scrub; //the adjustment for the managed scrollbar
	Gtk::HScrollbar		scr_time_scrub;
	Gtk::ToggleButton	b_loop;
	Gtk::Tooltips		tooltips;

	//Glib::RefPtr<Gdk::GC>		gc_area;
	Glib::RefPtr<Gdk::Pixbuf>	currentbuf;
	int							currentindex;
	//double						timeupdate;
	double						timedisp;
	double						audiotime;

	//sound stuff
	etl::handle<AudioContainer>	audio;
	sigc::connection	scrstartcon;
	sigc::connection	scrstopcon;
	sigc::connection	scrubcon;

	//preview encapsulation
	etl::handle<Preview>	preview;
	sigc::connection		prevchanged;

	Widget_Sound			disp_sound;
	Gtk::Adjustment			adj_sound;

	Gtk::Label				l_lasttime;

	//only for internal stuff, doesn't set anything
	bool 	playing;
	bool	singleframe;

	//for accurate time tracking
	etl::clock	timer;

	//int		curindex; //for later
	sigc::connection	timecon;

	void slider_move(); //later to be a time_slider that's cooler
	bool play_update();
	void play_stop();
	//bool play_frameupdate();
	void update();

	void scrub_updated(double t);

	void repreview();

	void whenupdated();

	void eraseall();

	bool scroll_move_event(GdkEvent *);
	void disconnect_preview(Preview *);

	bool redraw(GdkEventExpose *heh = 0);
	void preview_draw();

	sigc::signal<void,float>	signal_play_;
	sigc::signal<void>			signal_stop_;
	sigc::signal<void,float>	signal_seek_;

public:

	Widget_Preview();
	~Widget_Preview();

	//sets a signal to identify disconnection (so we don't hold onto it)...
	void set_preview(etl::handle<Preview> prev);
	void set_audioprofile(etl::handle<AudioProfile> p);
	void set_audio(etl::handle<AudioContainer> a);

	void clear();

	void play();
	void stop();
	void seek(float t);

	void stoprender();

	sigc::signal<void,float>	&signal_play() {return signal_play_;}
	sigc::signal<void>	&signal_stop() {return signal_stop_;}
	sigc::signal<void,float>	&signal_seek() {return signal_seek_;}

	bool get_loop_flag() const {return b_loop.get_active();}
	void set_loop_flag(bool b) {return b_loop.set_active(b);}
};

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
