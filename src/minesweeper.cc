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
#include <sstream>

#include "glog/logging.h"
#include "gtest/gtest.h"
#include "scoped_ptr.h"

namespace mineseeker {

const int MineSweeper::kMineInField = -1;

MineSweeper::MineSweeper(int width, int height)
    : width_(width),
      height_(height),
      is_closed_(false) {
  ResetMinefield(width_, height_);
}

void MineSweeper::CloseMineField() {
  is_closed_ = true;
  for (int x = 0; x < width_; ++x) {
    for (int y = 0; y < height_; ++y) {
      if (mine_field_[x][y] == kMineInField) {
        IncreaseNeighborMineCounts(x, y);
      }
    }
  }
}

void MineSweeper::PrintMineCountsToString(string* out) const {
  CHECK_NOTNULL(out);
  std::stringstream buffer(std::stringstream::out);
  for (int y = 0; y < height_; ++y) {
    for (int x = 0; x < width_; ++x) {
      buffer << NumberOfMinesAroundField(x, y) << " ";
    }
    buffer << std::endl;
  }
  *out = buffer.str();
}

void MineSweeper::IncreaseNeighborMineCounts(int x, int y) {
  for (int i = -1; i < 2; ++i) {
    for (int j = -1; j < 2; ++j) {
      if (i != 0 || j != 0) {
        IncreaseMineCount(x + i, y + j);
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

bool MineSweeper::IsMine(int x, int y) const {
  return kMineInField == NumberOfMinesAroundField(x, y);
}

MineSweeper* MineSweeper::LoadFromString(const string& input) {
  std::istringstream in(input);
  int width = 0;
  int height = 0;
  in >> width >> height;
  if (width <= 0) {
    LOG(ERROR) << "Invalid width: " << width;
    return NULL;
  }
  if (height <= 0) {
    LOG(ERROR) << "Invalid height: " << height;
    return NULL;
  }

  scoped_ptr<MineSweeper> mine_sweeper(new MineSweeper(width, height));

  int num_mines = 0;
  in >> num_mines;
  if (num_mines <= 0) {
    LOG(ERROR) << "Invalid number of mines: " << num_mines;
    return NULL;
  }

  for (int i = 0; i < num_mines; ++i) {
    int x = -1;
    int y = -1;
    in >> x >> y;
    if (x < 0 || x >= width) {
      LOG(ERROR) << "Invalid X posistion of a mine: " << x;
      return NULL;
    }
    if (y < 0 || y >= height) {
      LOG(ERROR) << "Invalid Y position of a mine: " << y;
      return NULL;
    }
    mine_sweeper->SetMine(x, y, true);
  }

  mine_sweeper->CloseMineField();
  return mine_sweeper.release();
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

int MineSweeper::NumberOfMinesAroundField(int x, int y) const {
  CHECK_GE(x, 0);
  CHECK_LT(x, width_);
  CHECK_GE(y, 0);
  CHECK_LT(y, height_);
  return mine_field_[x][y];
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

void MineSweeper::SetMine(int x, int y, bool is_mine) {
  CHECK_GE(x, 0);
  CHECK_LT(x, width_);
  CHECK_GE(y, 0);
  CHECK_LT(y, height_);
  mine_field_[x][y] = is_mine ? kMineInField : 0;
}

}  // namespace mineseeker
