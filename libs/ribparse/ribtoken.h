// Aqsis
// Copyright (C) 1997 - 2007, Paul C. Gregory
//
// Contact: pgregory@aqsis.org
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
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
 * \brief A RIB token class.
 * \author Chris Foster  [chris42f (at) gmail (dot) com]
 */

#ifndef RIBTOKEN_H_INCLUDED
#define RIBTOKEN_H_INCLUDED

#include <string>
#include <iostream>

#include <aqsis/math/math.h>

namespace Aqsis {

/** \brief A class encapsulating a RIB token
 */
class RibToken
{
	public:
		/// Rib token types
		enum Type
		{
			ARRAY_BEGIN,
			ARRAY_END,
			STRING,
			INTEGER,
			FLOAT,
			REQUEST,
			ERROR,
			ENDOFFILE
		};

		//--------------------------------------------------
		/// \name Constructors
		//@{
		/// Construct a token of the given type (default ERROR)
		RibToken(Type type = ERROR);
		/// Construct a INTEGER token from the given integer value
		RibToken(int intVal);
		/// Construct a FLOAT token from the given float value
		RibToken(float floatVal);
		/// Construct a token with a string as the token value
		RibToken(Type type, const std::string& strVal);
		//@}

		/// Assign to the given type
		RibToken& operator=(Type type);
		RibToken& operator=(int i);
		RibToken& operator=(float f);
		/// Set the token to the error type, with the given error message
		void error(const char* message);

		/** Equality operator (mostly for testing purposes)
		 *
		 * \return true if the token type and data are the same.
		 */
		bool operator==(const RibToken& rhs) const;
		/// Format the token to an output stream.
		friend std::ostream& operator<<(std::ostream& outStream, const RibToken& tok);

		//--------------------------------------------------
		/// \name accessors
		//@{
		/// Get the token type.
		Type type() const;
		/// Get an integer value from the token
		int intVal() const;
		/// Get a float value from the token
		float floatVal() const;
		/// Get a string value from the token
		const std::string& stringVal() const;
		//@}

	private:
		friend class RibTokenizer;

		/// Token type
		Type m_type;

		/// Integer value for token
		int m_intVal;
		/// Float value for token
		float m_floatVal;
		/// String value for token
		std::string m_strVal;
		// note: boost::variant was tried for the mutually exclusive values
		// above, but turned out to have a performance hit of about 7% of total
		// lexer time for a simple test which read tokens from a large RIB and
		// sent them to stdout with operator<<.  (g++-4.1, boost-1.34.1)
};



//==============================================================================
// Implementation details
//==============================================================================
// RibToken implementation
inline RibToken::RibToken(RibToken::Type type)
		: m_type(type),
		m_intVal(0),
		m_floatVal(0),
		m_strVal()
{ }

inline RibToken::RibToken(int intVal)
		: m_type(INTEGER),
		m_intVal(intVal),
		m_floatVal(0),
		m_strVal()
{}

inline RibToken::RibToken(float floatVal)
		: m_type(FLOAT),
		m_intVal(0),
		m_floatVal(floatVal),
		m_strVal()
{}

inline RibToken::RibToken(RibToken::Type type, const std::string& strVal)
		: m_type(type),
		m_intVal(0),
		m_floatVal(0),
		m_strVal(strVal)
{
	assert(type == STRING || type == REQUEST || type == ERROR);
}

inline RibToken& RibToken::operator=(Type type)
{
	m_type = type;
	return *this;
}

inline RibToken& RibToken::operator=(int i)
{
	m_type = INTEGER;
	m_intVal = i;
	return *this;
}

inline RibToken& RibToken::operator=(float f)
{
	m_type = FLOAT;
	m_floatVal = f;
	return *this;
}

inline void RibToken::error(const char* message)
{
	m_type = ERROR;
	m_strVal = message;
}

inline bool RibToken::operator==(const RibToken& rhs) const
{
	if(m_type != rhs.m_type)
		return false;
	switch(m_type)
	{
		case ARRAY_BEGIN:
		case ARRAY_END:
		case ENDOFFILE:
		case ERROR:
		default:
			return true;
		case INTEGER:
			return m_intVal == rhs.m_intVal;
		case FLOAT:
			return isClose(m_floatVal, rhs.m_floatVal);
		case STRING:
		case REQUEST:
			return m_strVal == rhs.m_strVal;
	}
}

inline RibToken::Type RibToken::type() const
{
	return m_type;
}

inline int RibToken::intVal() const
{
	assert(m_type == INTEGER);
	return m_intVal;
}

inline float RibToken::floatVal() const
{
	assert(m_type == FLOAT);
	return m_floatVal;
}

inline const std::string& RibToken::stringVal() const
{
	assert(m_type == STRING || m_type == REQUEST || m_type == ERROR);
	return m_strVal;
}

inline std::ostream& operator<<(std::ostream& outStream, const RibToken& tok)
{
	static const char* tokenNames[] = {
		"ARRAY_BEGIN",
		"ARRAY_END",
		"STRING",
		"INTEGER",
		"FLOAT",
		"REQUEST",
		"ERROR",
		"ENDOFFILE"
	};
	outStream << tokenNames[tok.m_type];
	switch(tok.m_type)
	{
		case RibToken::ARRAY_BEGIN:
		case RibToken::ARRAY_END:
		case RibToken::ENDOFFILE:
			break;
		case RibToken::INTEGER:
			outStream << ": " << tok.m_intVal;
			break;
		case RibToken::FLOAT:
			outStream << ": " << tok.m_floatVal;
			break;
		case RibToken::STRING:
			outStream << ": \"" << tok.m_strVal << "\"";
			break;
		case RibToken::REQUEST:
		case RibToken::ERROR:
			outStream << ": " << tok.m_strVal;
			break;
	}
	return outStream;
}

} // namespace Aqsis

#endif // RIBTOKEN_H_INCLUDED
