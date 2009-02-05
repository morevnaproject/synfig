/* === S Y N F I G ========================================================= */
/*!	\file target_tile.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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

#ifndef __SYNFIG_TARGET_TILE_H
#define __SYNFIG_TARGET_TILE_H

/* === H E A D E R S ======================================================= */

#include "../target.h"

/* === M A C R O S ========================================================= */

#define TILE_SIZE 120

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

/*!	\class Target_Tile
**	\brief Render-target
**	\todo writeme
*/
class Target_Tile : public Target
{
	int threads_;
	int tile_w_;
	int tile_h_;
	int curr_tile_;
	int curr_frame_;
	bool clipping_;
public:
	typedef etl::handle<Target_Tile> Handle;
	typedef etl::loose_handle<Target_Tile> LooseHandle;
	typedef etl::handle<const Target_Tile> ConstHandle;

	Target_Tile();

	//! Renders the canvas to the target
	virtual bool render(ProgressCallback *cb=NULL);

	//! Determines which tile needs to be rendered next.
	/*!	Most cases will not have to redefine this function.
	**	The default should be adequate in nearly all situations.
	**	\returns The number of tiles left to go <i>plus one</i>.
	**		This means that whenever this function returns zero,
	**		there are no more tiles to render and that any value
	**		in \a x or \a y should be disregarded. */
	virtual int next_tile(int& x, int& y);

	virtual int next_frame(Synfig_Time& time);

	//! Adds the tile at \a x , \a y contained in \a surface
	virtual bool add_tile(const synfig::Surface &surface, int x, int y)=0;

	virtual int total_tiles()const
	{
		// Width of the image(in tiles)
		const int tw(rend_desc().get_w()/tile_w_+(rend_desc().get_w()%tile_w_?1:0));
		const int th(rend_desc().get_h()/tile_h_+(rend_desc().get_h()%tile_h_?1:0));

		return tw*th;
	}

	//! Marks the start of a frame
	/*! \return \c true on success, \c false upon an error.
	**	\see end_frame(), start_scanline()
	*/
	virtual bool start_frame(ProgressCallback *cb=NULL)=0;

	//! Marks the end of a frame
	/*! \see start_frame() */
	virtual void end_frame()=0;

	void set_threads(int x) { threads_=x; }

	int get_threads()const { return threads_; }

	void set_tile_w(int w) { tile_w_=w; }

	int get_tile_w()const { return tile_w_; }

	void set_tile_h(int h) { tile_h_=h; }

	int get_tile_h()const { return tile_h_; }

	bool get_clipping()const { return clipping_; }

	void set_clipping(bool x) { clipping_=x; }

private:

	bool render_frame_(int quality, ProgressCallback *cb=0, RenderMethod method=SOFTWARE);

}; // END of class Target_Tile

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
