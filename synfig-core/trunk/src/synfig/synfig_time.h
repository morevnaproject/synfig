/* === S Y N F I G ========================================================= */
/*!	\file time.h
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

#ifndef __SYNFIG_TIME_H
#define __SYNFIG_TIME_H

/* === H E A D E R S ======================================================= */

#include "string_decl.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

/*!	\class Synfig_Time
**	\todo writeme
**	\see Synfig_TimeFormat, time_to_string(), string_to_time()
*/
class Synfig_Time
{
public:
	typedef double value_type;

	/*!	\enum Format
	**	\todo writeme
	**	\see Synfig_Time, get_string() */
	enum Format
	{
		FORMAT_NORMAL=0,		//!< Represents the default method of printing the time
		FORMAT_NOSPACES=(1<<0),	//!< Remove any whitespace
		FORMAT_FULL=(1<<1),		//!< Do not remove units that have "zero" value
		FORMAT_VIDEO=(1<<2),	//!< Use the HH:MM:SS.FF format
		FORMAT_FRAMES=(1<<3),	//!< Use the FF format (frames only)

		FORMAT_END=(1<<4)		//!< \internal Not used
	}; // END of enum Format

private:
	value_type value_;

	static value_type epsilon_() { return static_cast<value_type>(0.0005); }

public:
	Synfig_Time() { }

	Synfig_Time(const value_type &x):value_(x) { }

	Synfig_Time(int x):value_(x) { }

	Synfig_Time(int hour, int minute, float second):value_(static_cast<value_type>(second+hour*3600+minute*60)) { }

	//! Constructs Synfig_Time from the given string.
	/*!	\note If the string references frames, then the
	**	frame rate (\a fps) should be provided from the
	**	correct source. (Which is most likely the RendDesc
	**	of the current Canvas)
	**	The frame count will be ignored if the
	**	FPS is not given. */
	Synfig_Time(const String &string, float fps=0);

	//! Marks the exclusive negative boundary of time
	static const Synfig_Time begin() { return static_cast<synfig::Synfig_Time>(-32767.0f*512.0f); }

	//! Marks the exclusive positive boundary of time
	static const Synfig_Time end() { return static_cast<synfig::Synfig_Time>(32767.0f*512.0f); }

	//! Marks zero time
	static const Synfig_Time zero() { return static_cast<synfig::Synfig_Time>(0); }

	//! The amount of allowable error in calculations
	static const Synfig_Time epsilon() { return static_cast<synfig::Synfig_Time>(epsilon_()); }

	//! Returns a string describing the current time value
	/*!	\see Format */
	String get_string(float fps=0, Synfig_Time::Format format=FORMAT_NORMAL)const;

#ifdef _DEBUG
	const char *c_str()const;
#endif

	//! \writeme
	bool is_valid()const;

	//! Rounds time to the nearest frame for the given frame rate, \a fps
	Synfig_Time round(float fps)const;

	bool is_equal(const Synfig_Time& rhs)const { return (value_>rhs.value_)?value_-rhs.value_<=epsilon_():rhs.value_-value_<=epsilon_(); }
	bool is_less_than(const Synfig_Time& rhs)const { return rhs.value_-value_ > epsilon_(); }
	bool is_more_than(const Synfig_Time& rhs)const { return value_-rhs.value_ > epsilon_(); }

	operator double()const { return value_; }

	template<typename U> bool operator<(const U& rhs)const { return value_<rhs; }
	template<typename U> bool operator>(const U& rhs)const { return value_>rhs; }
	template<typename U> bool operator<=(const U& rhs)const { return value_<=rhs; }
	template<typename U> bool operator>=(const U& rhs)const { return value_>=rhs; }
	template<typename U> bool operator==(const U& rhs)const { return value_==rhs; }
	template<typename U> bool operator!=(const U& rhs)const { return value_!=rhs; }

#if 0
	bool operator<(const Synfig_Time& rhs)const { return value_<rhs.value_; }
	bool operator>(const Synfig_Time& rhs)const { return value_>rhs.value_; }
	bool operator<=(const Synfig_Time& rhs)const { return value_<=rhs.value_; }
	bool operator>=(const Synfig_Time& rhs)const { return value_>=rhs.value_; }
	bool operator==(const Synfig_Time& rhs)const { return value_==rhs.value_; }
	bool operator!=(const Synfig_Time& rhs)const { return value_!=rhs.value_; }
#else
	bool operator<(const Synfig_Time& rhs)const { return is_less_than(rhs); }
	bool operator>(const Synfig_Time& rhs)const { return is_more_than(rhs); }
	bool operator<=(const Synfig_Time& rhs)const { return is_less_than(rhs)||is_equal(rhs); }
	bool operator>=(const Synfig_Time& rhs)const { return is_more_than(rhs)||is_equal(rhs); }
	bool operator==(const Synfig_Time& rhs)const { return is_equal(rhs); }
	bool operator!=(const Synfig_Time& rhs)const { return !is_equal(rhs); }
#endif

	template<typename U> const Synfig_Time& operator+=(const U &rhs) { value_+=static_cast<value_type>(rhs); return *this; }
	template<typename U> const Synfig_Time& operator-=(const U &rhs) { value_-=static_cast<value_type>(rhs); return *this; }
	template<typename U> const Synfig_Time& operator*=(const U &rhs) { value_*=static_cast<value_type>(rhs); return *this; }
	template<typename U> const Synfig_Time& operator/=(const U &rhs) { value_/=static_cast<value_type>(rhs); return *this; }

	template<typename U> Synfig_Time operator+(const U &rhs)const { return value_+static_cast<value_type>(rhs); }
	template<typename U> Synfig_Time operator-(const U &rhs)const { return value_-static_cast<value_type>(rhs); }
	template<typename U> Synfig_Time operator*(const U &rhs)const { return value_*static_cast<value_type>(rhs); }
	template<typename U> Synfig_Time operator/(const U &rhs)const { return value_/static_cast<value_type>(rhs); }

	Synfig_Time operator-()const { return -value_; }
}; // END of class Synfig_Time

//! This operator allows the combining of Synfig_Time::Format flags using the '|' operator
/*!	\see Synfig_Time::Format, Synfig_Time::get_string() */
inline Synfig_Time::Format operator|(Synfig_Time::Format lhs, Synfig_Time::Format rhs)
{ return static_cast<Synfig_Time::Format>((int)lhs|(int)rhs); }

//! This operator is for checking Synfig_Time::Format flags.
/*! Don't think of it as "less then or equal to", but think of it
**	like an arrow. Is \a rhs inside of \a lhs ?
**	\see Synfig_Time::Format, Synfig_Time::get_string() */
inline bool operator<=(Synfig_Time::Format lhs, Synfig_Time::Format rhs)
{ return (static_cast<int>(lhs) & static_cast<int>(rhs))==static_cast<int>(rhs); }

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
