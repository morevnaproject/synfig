/* === S Y N F I G ========================================================= */
/*!	\file import.cpp
**	\brief Implementation of the "Import Image" layer
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
**
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "import.h"
#include <synfig/synfig_string.h>
#include <synfig/synfig_time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>
#include <synfig/canvas.h>

#endif

using namespace synfig;
using namespace std;
using namespace etl;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Import);
SYNFIG_LAYER_SET_NAME(Import,"import");
SYNFIG_LAYER_SET_LOCAL_NAME(Import,N_("Import Image"));
SYNFIG_LAYER_SET_CATEGORY(Import,N_("Other"));
SYNFIG_LAYER_SET_VERSION(Import,"0.1");
SYNFIG_LAYER_SET_CVS_ID(Import,"$Id$");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Import::Import()
{
	time_offset=0;
}

Import::~Import()
{
}

void
Import::on_canvas_set()
{
	if(get_canvas())set_param("filename",filename);
}

bool
Import::set_param(const String & param, const ValueBase &value)
{
	try{
	IMPORT(time_offset);
	if(param=="filename" && value.same_type_as(filename))
	{
		if(!get_canvas())
		{
			filename=value.get(filename);
			importer=0;
			surface.clear();
			return true;
		}

		String newfilename=value.get(string());
		String filename_with_path;

		// Get rid of any %20 crap
		{
			String::size_type n;
			while((n=newfilename.find("%20"))!=String::npos)
				newfilename.replace(n,3," ");
		}

		//if(get_canvas()->get_file_path()==dirname(newfilename))
		//{
		//	synfig::info("Image seems to be in local directory. Adjusting path...");
		//	newfilename=basename(newfilename);
		//}

#ifndef WIN32
		if(is_absolute_path(newfilename))
		{
			string curpath(cleanup_path(absolute_path(get_canvas()->get_file_path())));
			while(basename(curpath)==".")curpath=dirname(curpath);

			newfilename=relative_path(curpath,newfilename);
			synfig::info("basename(curpath)=%s, Path adjusted to %s",basename(curpath).c_str(),newfilename.c_str());
		}
#endif

		if(filename.empty())
			filename=newfilename;

		if(newfilename.empty())
		{
			filename=newfilename;
			importer=0;
			surface.clear();
			return true;
		}

		// If we are already loaded, don't reload
		if(filename==newfilename && importer)
		{
			synfig::warning(strprintf(_("Filename seems to already be set to \"%s\" (%s)"),filename.c_str(),newfilename.c_str()));
			return true;
		}

		assert(get_canvas());

		if(is_absolute_path(newfilename))
			filename_with_path=newfilename;
		else
			filename_with_path=get_canvas()->get_file_path()+ETL_DIRECTORY_SEPARATOR+newfilename;

		handle<Importer> newimporter;

		newimporter=Importer::open(absolute_path(filename_with_path));

		if(!newimporter)
		{
			newimporter=Importer::open(get_canvas()->get_file_path()+ETL_DIRECTORY_SEPARATOR+basename(newfilename));
			if(!newimporter)
			{
				synfig::error(strprintf("Unable to create an importer object with file \"%s\"",filename_with_path.c_str()));
				surface.clear();
				return false;
			}
		}

		surface.clear();
		if(!newimporter->get_frame(surface,Synfig_Time(0),trimmed,width,height,top,left))
		{
			synfig::warning(strprintf("Unable to get frame from \"%s\"",filename_with_path.c_str()));
		}

		importer=newimporter;
		filename=newfilename;
		abs_filename=absolute_path(filename_with_path);

		return true;
	}
	} catch(...) { set_amount(0); return false; }

	return Layer_Bitmap::set_param(param,value);
}

ValueBase
Import::get_param(const String & param)const
{
	EXPORT(time_offset);

	if(get_canvas())
	{
		if(param=="filename")
		{
			string curpath(cleanup_path(absolute_path(get_canvas()->get_file_path())));
			return relative_path(curpath,abs_filename);
		}
	}
	else
		EXPORT(filename);

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Bitmap::get_param(param);
}

Layer::Vocab
Import::get_param_vocab()const
{
	Layer::Vocab ret(Layer_Bitmap::get_param_vocab());

	ret.push_back(ParamDesc("filename")
		.set_local_name(_("Filename"))
		.set_description(_("File to import"))
		.set_hint("filename")
	);
	ret.push_back(ParamDesc("time_offset")
		.set_local_name(_("Time Offset"))
	);

	return ret;
}

void
Import::set_time(Context context, Synfig_Time time)const
{
	if(get_amount() && importer &&
	   importer->is_animated())
		importer->get_frame(surface,time+time_offset,trimmed,width,height,top,left);

	context.set_time(time);
}

void
Import::set_time(Context context, Synfig_Time time, const Point &pos)const
{
	if(get_amount() && importer &&
	   importer->is_animated())
		importer->get_frame(surface,time+time_offset,trimmed,width,height,top,left);

	context.set_time(time,pos);
}
