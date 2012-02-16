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

#include "minesweeper.h"

#include <algorithm>

#include "glog/logging.h"
#include "gtest/gtest.h"

namespace mineseeker {

MineSweeper::MineSweeper(int width, int height)
    : width_(width),
      height_(height),
      is_closed_(false) {
  ResetMinefield(width_, height_);
}

void MineSweeper::CloseMineField() {
  for (int x = 0; x < width_; ++x) {
    for (int y = 0; y < height_; ++y) {
      if (mine_field_[x][y] == kMineInField) {
        IncreaseNeighborMineCounts(x, y);
      }
    }
  }
}

void MineSweeper::IncreaseNeighborMineCounts(int x, int y) {
  for (int i = -1; i < 2; ++i) {
    for (int j = -1; j < 2; ++j) {
      if (i != 0 && j != 0) {
        IncreaseNeighborMineCounts(x + i, y + j);
      }
    }
  }
}

void MineSweeper::IncreaseMineCount(int x, int y) {
  if (x >= 0 && y >= 0 && x < width_ && y < height_) {
    if (mine_field_[x][y] != kMineInField) {
      ++mine_field_[x][y];
    }
  }
}

int MineSweeper::NumberOfMines() const {
  int num_mines = 0;
  for (int x = 0; x < width_; ++x) {
    int mines_in_column = std::count(mine_field_[x].begin(),
                                     mine_field_[x].end(),
                                     kMineInField);
    num_mines += mines_in_column;
  }
  return num_mines;
}

void MineSweeper::ResetMinefield(int width, int height) {
  CHECK_GT(width, 0);
  CHECK_GT(height, 0);
  width_ = width;
  height_ = height;
  mine_field_.resize(width);
  for (int i = 0; i < width_; ++i) {
    mine_field_[i].clear();
    mine_field_[i].resize(height_, 0);
  }
}

}  // namespace mineseeker
