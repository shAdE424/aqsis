// hairgen procedural
// Copyright (C) 2008 Christopher J. Foster [chris42f (at) gmail (d0t) com]
//
// This software is licensed under the GPLv2 - see the file COPYING for details.

#ifndef HAIR_TYPES_H_INCLUDED
#define HAIR_TYPES_H_INCLUDED

#include <cstdlib>
#include <iosfwd>

#include <aqsis/math/vector3d.h>
#include "../../../libs/ribparse/ricxx.h"

namespace Ri = Aqsis::Ri;

/// Alias for the longwinded Aqsis::CqVector3D
typedef Aqsis::CqVector3D Vec3;

/// Uniform random numbers on [0,1).  Could use Aqsis::CqRandom instead after a cleanup.
inline float uRand()
{
	return float(std::rand())/RAND_MAX;
}

/// Error stream.
extern std::ostream& g_errStream;

#endif // HAIR_TYPES_H_INCLUDED
