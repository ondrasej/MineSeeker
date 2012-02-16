// Copyright 2012 Ondrej Sykora
//
// This file is part of MineSeeker.
//
// MineSeeker is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option)
// any later version.
//
// MineSeeker is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along with
// MineSeeker. If not, see <http://www.gnu.org/licenses/>.

#ifndef MINESEEKER_COMMON_H_
#define MINESEEKER_COMMON_H_

#include <string>
#include <vector>

using std::string;
using std::vector;

// Determines the size of an array (that is defined as an array, not a pointer)
// within the scope of the use of the macro.
#define ARRAYSIZE(array) (sizeof(array) / sizeof(*array))

#endif  // MINESEEKER_COMMON_H_
