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

#include <stdlib.h>
#include <iostream>
#include <set>
#include "gflags/gflags.h"
#include "glog/logging.h"

DEFINE_int32(width, 30, "The width of the mine field.");
DEFINE_int32(height, 16, "The height of the mine field.");
DEFINE_int32(mines, 99, "The number of mines on the minefield");

int main(int argc, char* argv[]) {
  if (FLAGS_width <= 0) {
    LOG(ERROR) << "Invalid width: " << FLAGS_width;
    return 1;
  }
  if (FLAGS_height <= 0) {
    LOG(ERROR) << "Invalid height: " << FLAGS_height;
    return 1;
  }
  if (FLAGS_mines <= 0) {
    LOG(ERROR) << "Invalid number of mines: " << FLAGS_mines;
    return 1;
  }
  if (FLAGS_mines > FLAGS_width * FLAGS_height) {
    LOG(ERROR) << "Too many mines: " << FLAGS_mines;
    return 1;
  }

  std::cout << FLAGS_width << " " << FLAGS_height << std::endl;
  std::cout << FLAGS_mines << std::endl;

  std::set<std::pair<int, int> > used_coordinates;

  for (int i = 0; i < FLAGS_mines; ++i) {
    for (;;) {
      const int x = rand() % FLAGS_width;
      const int y = rand() % FLAGS_height;
      const std::pair<int, int> coordinates = std::make_pair(x, y);
      if (0 == used_coordinates.count(coordinates)) {
        std::cout << x << " " << y << std::endl;
        used_coordinates.insert(coordinates);
        break;
      }
    }
  }

  return 0;
}
