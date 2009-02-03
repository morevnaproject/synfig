/* === S Y N F I G ========================================================= */
/*!	\file asyncrenderer.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

#ifndef __SYNFIG_ASYNCRENDERER_H
#define __SYNFIG_ASYNCRENDERER_H

/* === H E A D E R S ======================================================= */

#include <ETL/handle>
#include <sigc++/signal.h>
#include <sigc++/trackable.h>
#include <sigc++/connection.h>

#include <synfig/targets/target_scanline.h>
#include <synfig/targets/target_tile.h>
#include <synfig/surface.h>
#include <glibmm/main.h>
#include <ETL/ref_count>
#include <glibmm/thread.h>
#include <glibmm/dispatcher.h>

/* === M A C R O S ========================================================= */

// uncomment to use a single thread, and hopefully get more stability
// #define SINGLE_THREADED
#ifdef SINGLE_THREADED
#  define single_threaded()	App::single_threaded
#endif

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class AsyncRenderer : public etl::shared_object, public sigc::trackable
{
	sigc::signal<void> signal_finished_;
	sigc::signal<void> signal_success_;

	std::list<sigc::connection> activity_connection_list;

	//etl::handle<synfig::Target_Scanline> target_scanline;
	//etl::handle<synfig::Target_Tile> target_tile;
	etl::handle<synfig::Target> target;

	bool error;
	bool success;

	synfig::ProgressCallback *cb;

	sigc::signal<void> signal_stop_;

	Glib::Thread* render_thread;
	Glib::Dispatcher signal_done_;
	Glib::Mutex mutex;
	sigc::connection done_connection;

	/*
 --	** -- P A R E N T   M E M B E R S -----------------------------------------
	*/
public:

	AsyncRenderer(etl::handle<synfig::Target> target,synfig::ProgressCallback *cb=0);
	virtual ~AsyncRenderer();

	void start();
	void stop();
	void pause();
	void resume();
#ifdef SINGLE_THREADED
	void rendering_progress();
	bool updating;
#endif

	bool has_error()const { return error; }
	bool has_success()const { return success; }

	sigc::signal<void>& signal_finished() { return signal_finished_; }
	sigc::signal<void>& signal_success() { return signal_success_; }

private:

	void render_target();
	void start_();

	/*
 --	** -- C H I L D   M E M B E R S -------------------------------------------
	*/

protected:

};

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
