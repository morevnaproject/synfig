/* === S Y N F I G ========================================================= */
/*!	\file trgt_ffmpeg.cpp
**	\brief ppm Target Module
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

#define SYNFIG_TARGET

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <ETL/stringf>
#include "trgt_ffmpeg.h"
#include <stdio.h>
#include <sys/types.h>
#if HAVE_SYS_WAIT_H
 #include <sys/wait.h>
#endif
#if HAVE_IO_H
 #include <io.h>
#endif
#if HAVE_PROCESS_H
 #include <process.h>
#endif
#if HAVE_FCNTL_H
 #include <fcntl.h>
#endif
#include <unistd.h>
#include <algorithm>
#include <functional>
#include <ETL/clock>

#endif

/* === M A C R O S ========================================================= */

using namespace synfig;
using namespace std;
using namespace etl;

#define RGB_SIZE	3

#if defined(HAVE_FORK) && defined(HAVE_PIPE) && defined(HAVE_WAITPID)
 #define UNIX_PIPE_TO_PROCESSES
#else
 #define WIN32_PIPE_TO_PROCESSES
#endif

/* === G L O B A L S ======================================================= */

SYNFIG_TARGET_INIT(ffmpeg_trgt);
SYNFIG_TARGET_SET_NAME(ffmpeg_trgt,"ffmpeg");
SYNFIG_TARGET_SET_EXT(ffmpeg_trgt,"mpg");
SYNFIG_TARGET_SET_VERSION(ffmpeg_trgt,"0.1");
SYNFIG_TARGET_SET_CVS_ID(ffmpeg_trgt,"$Id$");

/* === M E T H O D S ======================================================= */

ffmpeg_trgt::ffmpeg_trgt(const char *Filename)
{
	pid=-1;
	file=NULL;
	filename=Filename;
	multi_image=false;
	buffer=NULL;
	color_buffer=0;
	target_format_ = PF_RGB | PF_8BITS;
	set_remove_alpha();
}

ffmpeg_trgt::~ffmpeg_trgt()
{
	if(file)
	{
		etl::yield();
		sleep(1);
#if defined(WIN32_PIPE_TO_PROCESSES)
		pclose(file);
#elif defined(UNIX_PIPE_TO_PROCESSES)
		fclose(file);
		int status;
		waitpid(pid,&status,0);
#endif
	}
	file=NULL;
	delete [] buffer;
	delete [] color_buffer;
}

bool
ffmpeg_trgt::set_rend_desc(RendDesc *given_desc)
{
	//given_desc->set_pixel_format(PF_RGB);

	// Make sure that the width and height
	// are multiples of 8
	given_desc->set_w((given_desc->get_w()+4)/8*8);
	given_desc->set_h((given_desc->get_h()+4)/8*8);

	/*
	// Valid framerates:
	// 23.976, 24, 25, 29.97, 30, 50 ,59.94, 60
	float fps=given_desc->get_frame_rate();
	if(fps <24.0)
		given_desc->set_frame_rate(23.976);
	if(fps>=24.0 && fps <25.0)
		given_desc->set_frame_rate(24);
	if(fps>=25.0 && fps <29.97)
		given_desc->set_frame_rate(25);
	if(fps>=29.97 && fps <30.0)
		given_desc->set_frame_rate(29.97);
	if(fps>=29.97 && fps <30.0)
		given_desc->set_frame_rate(29.97);
	if(fps>=30.0 && fps <50.0)
		given_desc->set_frame_rate(30.0);
	if(fps>=50.0 && fps <59.94)
		given_desc->set_frame_rate(50);
	if(fps>=59.94)
		given_desc->set_frame_rate(59.94);
    */

	desc=*given_desc;

	return true;
}

bool
ffmpeg_trgt::init()
{
	imagecount=desc.get_frame_start();
	if(desc.get_frame_end()-desc.get_frame_start()>0)
		multi_image=true;

#if defined(WIN32_PIPE_TO_PROCESSES)

	string command;

	if( filename.c_str()[0] == '-' )
			command=strprintf("ffmpeg -f image2pipe -vcodec ppm -an -r %f -i pipe: -loop -hq -title \"%s\" -vcodec mpeg1video -y -- \"%s\"\n",desc.get_frame_rate(),get_canvas()->get_name().c_str(),filename.c_str());
	else
			command=strprintf("ffmpeg -f image2pipe -vcodec ppm -an -r %f -i pipe: -loop -hq -title \"%s\" -vcodec mpeg1video -y \"%s\"\n",desc.get_frame_rate(),get_canvas()->get_name().c_str(),filename.c_str());

	file=popen(command.c_str(),POPEN_BINARY_WRITE_TYPE);

#elif defined(UNIX_PIPE_TO_PROCESSES)

	int p[2];

	if (pipe(p)) {
		synfig::error(_("Unable to open pipe to ffmpeg"));
		return false;
	};

	pid = fork();

	if (pid == -1) {
		synfig::error(_("Unable to open pipe to ffmpeg"));
		return false;
	}

	if (pid == 0){
		// Child process
		// Close pipeout, not needed
		close(p[1]);
		// Dup pipeout to stdin
		if( dup2( p[0], STDIN_FILENO ) == -1 ){
			synfig::error(_("Unable to open pipe to ffmpeg"));
			return false;
		}
		// Close the unneeded pipeout
		close(p[0]);
		if( filename.c_str()[0] == '-' )
			execlp("ffmpeg", "ffmpeg", "-f", "image2pipe", "-vcodec", "ppm", "-an", "-r", strprintf("%f", desc.get_frame_rate()).c_str(), "-i", "pipe:", "-loop", "-hq", "-title", get_canvas()->get_name().c_str(), "-vcodec", "mpeg1video", "-y", "--", filename.c_str(), (const char *)NULL);
		else
			execlp("ffmpeg", "ffmpeg", "-f", "image2pipe", "-vcodec", "ppm", "-an", "-r", strprintf("%f", desc.get_frame_rate()).c_str(), "-i", "pipe:", "-loop", "-hq", "-title", get_canvas()->get_name().c_str(), "-vcodec", "mpeg1video", "-y", filename.c_str(), (const char *)NULL);
		// We should never reach here unless the exec failed
		synfig::error(_("Unable to open pipe to ffmpeg"));
		return false;
	} else {
		// Parent process
		// Close pipein, not needed
		close(p[0]);
		// Save pipeout to file handle, will write to it later
		file = fdopen(p[1], "wb");
	}

#else
	#error There are no known APIs for creating child processes
#endif

	// etl::yield();

	if(!file)
	{
		synfig::error(_("Unable to open pipe to ffmpeg"));
		return false;
	}

	return true;
}

void
ffmpeg_trgt::end_frame()
{
	//fprintf(file, " ");
	fflush(file);
	imagecount++;
}

bool
ffmpeg_trgt::start_frame(synfig::ProgressCallback */*callback*/)
{
	int w=desc.get_w(),h=desc.get_h();

	if(!file)
		return false;

	fprintf(file, "P6\n");
	fprintf(file, "%d %d\n", w, h);
	fprintf(file, "%d\n", 255);

	delete [] buffer;
	buffer=new unsigned char[RGB_SIZE*w];
	delete [] color_buffer;
	color_buffer=new Color[w];

	return true;
}

Color *
ffmpeg_trgt::start_scanline(int /*scanline*/)
{
	return color_buffer;
}

bool
ffmpeg_trgt::end_scanline()
{
	if(!file)
		return false;

	convert_color_format(buffer, color_buffer, desc.get_w(), PF_RGB, gamma());

	if(!fwrite(buffer,1,desc.get_w()*RGB_SIZE,file))
		return false;

	return true;
}

unsigned char*
ffmpeg_trgt::start_scanline_rgba(int /*scanline*/)
{
	return buffer;
}

bool
ffmpeg_trgt::end_scanline_rgba()
{
	if(!file)
		return false;

	if(!fwrite(buffer,1,desc.get_w()*RGB_SIZE,file))
		return false;

	return true;
}
