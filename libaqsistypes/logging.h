#ifndef LOGGING_H
#define LOGGING_H

// Copyright � 2003, Timothy M. Shead
//
// Contact: tshead@k-3d.com
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include <iosfwd>

// iostream-compatible manipulators - use these
// at the beginning of a message to indicate its priority, e.g.
//
// std::cerr << info << "Informational message" << std::endl;
// std::cerr << critical << "Critical message" << std::endl;

std::ostream& emergency(std::ostream&);
std::ostream& alert(std::ostream&);
std::ostream& critical(std::ostream&);
std::ostream& error(std::ostream&);
std::ostream& warning(std::ostream&);
std::ostream& notice(std::ostream&);
std::ostream& info(std::ostream&);
std::ostream& debug(std::ostream&);

#endif // !LOGGING_H

