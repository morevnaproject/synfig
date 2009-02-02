/* === S Y N F I G ========================================================= */
/*!	\file target.h
**	\brief Target Class Implementation
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

#ifndef __SYNFIG_TARGET_H
#define __SYNFIG_TARGET_H

/* === H E A D E R S ======================================================= */

#include <sigc++/signal.h>
#include "string_decl.h"
#include <utility>
//#include <list>
#include <map>
#include <ETL/handle>
#include "renddesc.h"
//#include "general.h"
#include "color.h"
#include "canvas.h"

/* === M A C R O S ========================================================= */

//! \writeme
#define SYNFIG_TARGET_MODULE_EXT public: static const char name__[], version__[], ext__[],cvs_id__[]; static Target *create(const char *filename);

//! Sets the name of the target
#define SYNFIG_TARGET_SET_NAME(class,x) const char class::name__[]=x

//! \writeme
#define SYNFIG_TARGET_SET_EXT(class,x) const char class::ext__[]=x

//! Sets the version of the target
#define SYNFIG_TARGET_SET_VERSION(class,x) const char class::version__[]=x

//! Sets the CVS ID of the target
#define SYNFIG_TARGET_SET_CVS_ID(class,x) const char class::cvs_id__[]=x

//! \writeme
#define SYNFIG_TARGET_INIT(class) synfig::Target* class::create(const char *filename) { return new class(filename); }

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class Surface;
class RendDesc;
class Canvas;
class ProgressCallback;

//! Available rendering methods
enum RenderMethod {
	SOFTWARE = 0x100,		//!< Software rendering
	OPENGL					//!< OpenGL rendering
};

/*!	\class Target
**	\brief This class provides a skeleton in top of which rendering targets will be built.
**
**	When a scene needs rendering, the source will call Target render() method in order to
** redraw each child layer. Target_Tile and Target_Scanline are the main implementations
** of this class
*/
class Target : public etl::shared_object
{
public:
	typedef etl::handle<Target> Handle;
	typedef etl::loose_handle<Target> LooseHandle;
	typedef etl::handle<const Target> ConstHandle;

	/*
 -- ** -- S I G N A L S -------------------------------------------------------
	*/

private:

	sigc::signal<void> signal_progress_;

	/*
 -- ** -- S I G N A L   I N T E R F A C E -------------------------------------
	*/

public:

	sigc::signal<void>& signal_progress() { return signal_progress_; }

	/*
 --	** -- C O N S T R U C T O R S ---------------------------------------------
	*/

public:
	typedef Target* (*Factory)(const char *filename);

	//! A type for a map of targets, indexed by the name of the Target
	typedef std::map<String,std::pair<Factory,String> > Book;

	typedef std::map<String,String> ExtBook;

	//! Target Book, indexed by the target's name
	static Book* book_;

	//! Map of target names indexed by associated file extension
	static ExtBook* ext_book_;

	static Book& book();
	static ExtBook& ext_book();

private:
	//! Current Target quality
	int quality_;
	//! Current Target gamma
	Gamma gamma_;
	//! If this flag is set, Alpha will be set to 1.0f in the implemented Target
	bool remove_alpha;

	bool avoid_time_sync_;
	//! Rendering method used by the Target to generate images from data
	RenderMethod render_method_;

protected:

	Target();

	//! Adjusted Render description set by set_rend_desc()
	RendDesc desc;

	etl::handle<Canvas> canvas;

public:
	virtual ~Target() { }

	//! Returns Target rendering method
	RenderMethod get_render_method()const { return render_method_; }
	//! Sets Target rendering method
	void set_render_method(RenderMethod method) { render_method_ = method; }
	//! Returns Target quality
	int get_quality()const { return quality_; }
	//! Sets Target quality to \a q
	void set_quality(int q) { quality_=q; }

	void set_avoid_time_sync(bool x=true) { avoid_time_sync_=x; }

	bool get_avoid_time_sync()const { return avoid_time_sync_; }
	//! Returns Target remove_alpha flag
	bool get_remove_alpha()const { return remove_alpha; }
	//! Sets Target remove_alpha flag to \a x
	void set_remove_alpha(bool x=true) { remove_alpha=x; }
	//! Returns Target gamma
	Gamma &gamma() { return gamma_; }
	//! Returns Target gamma
	const Gamma &gamma()const { return gamma_; }

	virtual void set_canvas(etl::handle<Canvas> c);

	const etl::handle<Canvas> &get_canvas()const { return canvas; }
	//! Returns Target RendDesc	
	RendDesc &rend_desc() { return desc; }
	//! Returns Target RendDesc	
	const RendDesc &rend_desc()const { return desc; }

	//! Renders the canvas to the target
	virtual bool render(ProgressCallback *cb=NULL)=0;

	//! Sets the RendDesc for the Target to \a desc.
	/*!	If there are any parts of \a desc that the render target
	**	is not capable of doing, the render target will adjust
	**	\a desc to fit its needs.
	*/
	virtual bool set_rend_desc(RendDesc *d) { desc=*d; return true; }

	virtual bool init() { return true; }
	//! Initializes Target subsystems (Book, ExtBook and Gamma)
	static bool subsys_init();
	//! Removes Target subsystems
	static bool subsys_stop();

	//! Creates a new Target described by \a type, outputting to a file described by \a filename.
	static Handle create(const String &type, const String &filename);
}; // END of class Target

}; // END of namespace synfig

/* === E N D =============================================================== */

#include "canvas.h"
#endif
