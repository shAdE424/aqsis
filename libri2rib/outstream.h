// -*- C++ -*-
// Aqsis
// Copyright � 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.com
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

/** \file
 *  \brief Fstream and Gzip output
 *  \author Lionel J. Lacour (intuition01@online.fr)
 */

#ifndef RI2RIB_OUTSTREAM_H
#define RI2RIB_OUTSTREAM_H 1

#include <fstream>
#include <string>
#include <zlib.h>
#include "aqsis.h"


START_NAMESPACE( libri2rib )

class CqStream
{
	public:
		virtual CqStream & operator<<( int i ) = 0;
		virtual CqStream & operator<<( float f ) = 0;
		virtual CqStream & operator<<( std::string s ) = 0;
		virtual CqStream & operator<<( char c ) = 0;

		CqStream()
		{}
		virtual ~CqStream()
		{}

		virtual void openFile( const char * ) = 0;
		virtual void closeFile() = 0;
};


class CqStreamStd : public CqStream
{
	private:
		std::ofstream out;
	public:
		CqStream & operator<<( int i )
		{
			out << i; return *this;
		}
		CqStream & operator<<( float f )
		{
			out << f; return *this;
		}
		CqStream & operator<<( std::string s )
		{
			out << s; return *this;
		}
		CqStream & operator<<( char c )
		{
			out << c; return *this;
		}

		CqStreamStd()
		{}
		~CqStreamStd()
		{}

		void openFile( const char * );
		void closeFile();
};

class CqStreamGzip : public CqStream
{
	private:
		gzFile gzf;
		void error();
	public:
		CqStream & operator<<( int i );
		CqStream & operator<<( float f );
		CqStream & operator<<( std::string s );
		CqStream & operator<<( char c );

		CqStreamGzip()
		{}
		~CqStreamGzip()
		{}

		void openFile( const char * );
		void closeFile();
};

END_NAMESPACE( libri2rib )
#endif
