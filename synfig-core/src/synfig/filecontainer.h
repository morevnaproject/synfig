/* === S Y N F I G ========================================================= */
/*!	\file filecontainer.h
**	\brief FileContainer
**
**	$Id$
**
**	\legal
**	......... ... 2013 Ivan Mahonin
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

#ifndef __SYNFIG_FILECONTAINER_H
#define __SYNFIG_FILECONTAINER_H

/* === H E A D E R S ======================================================= */

#include <list>
#include "filesystem.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{

	class FileContainer : public FileSystem
	{
	public:
		class ReadStream : public FileSystem::ReadStream
		{
		protected:
			friend FileContainer;
			ReadStream(Handle file_system);
		public:
			virtual ~ReadStream();
			virtual size_t read(void *buffer, size_t size);
		};

		class WriteStream : public FileSystem::WriteStream
		{
		protected:
			friend FileContainer;
			WriteStream(Handle file_system);
		public:
			virtual ~WriteStream();
			virtual size_t write(void *buffer, size_t size);
		};

	protected:
		bool stream_opened_;
		bool stream_valid_;

	public:
		FileContainer();
		virtual ~FileContainer();

		virtual bool create(const std::string &container_filename) = 0;
		virtual bool open(const std::string &container_filename) = 0;
		virtual void close() = 0;
		virtual bool is_opened() = 0;

		virtual bool is_file(const std::string &filename) = 0;
		virtual bool is_directory(const std::string &filename) = 0;

		virtual bool directory_create(const std::string &dirname) = 0;
		virtual bool directory_scan(const std::string &dirname, std::list< std::string > &out_files) = 0;

		virtual bool file_open_read_whole_container();
		virtual bool file_open_read(const std::string &filename) = 0;
		virtual bool file_open_write(const std::string &filename) = 0;
		virtual void file_close();

		virtual bool file_is_opened_for_read() = 0;
		virtual bool file_is_opened_for_write() = 0;

		virtual size_t file_read(void *buffer, size_t size) = 0;
		virtual size_t file_write(const void *buffer, size_t size) = 0;

		inline bool file_is_opened()
		{
			return file_is_opened_for_read() || file_is_opened_for_write();
		}

		inline bool is_exists(const std::string filename) { return is_file(filename) || is_directory(filename); }

		ReadStreamHandle get_read_stream_whole_container();
		virtual ReadStreamHandle get_read_stream(const std::string &filename);
		virtual WriteStreamHandle get_write_stream(const std::string &filename);
	};

}

/* === E N D =============================================================== */

#endif
