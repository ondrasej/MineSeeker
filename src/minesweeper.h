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

#ifndef MINESEEKER_MINESWEEPER_H_
#define MINESEEKER_MINESWEEPER_H_

#include "common.h"

namespace mineseeker {

typedef vector<vector<int> > MineField;

// Implements the minefield for the minesweeper game. Keeps track of the number
// of mines in the neighbourhood of empty fields, and supports loading the mine
// field from a file.
class MineSweeper {
 public:
  MineSweeper(int width, int height);

  // Places or removes mine from the given position in the mine field. This
  // method only works before the mine field is closed for changes.
  void SetMine(int x, int y, bool is_mine);
  
  // Checks if at the position (x, y) is a mine.
  bool IsMine(int x, int y) const;

  // Loads the mine field from a file. Returns NULL if loading of the mine field
  // failed. Upon success, returns the minefield; the caller is responsible for
  // deleting the returned object.
  static MineSweeper* LoadFromFile(const string& file_name);

  // Closes the mine field. Updates the numbers of neighboring mines for each
  // field.
  void CloseMineField();

  // Returns the number of mines in the minefield.
  int NumberOfMines() const;

  // If true, the mine field is closed for modifications; no mines can be added
  // or removed from the mine field.
  bool is_closed() const { return is_closed_; }

  // The size of the mine field.
  int width() const { return width_; }
  int height() const { return height_; }
 private:
  // The constant used in mine_field_ for fields that contain a mine.
  static const int kMineInField;

  // Increases the number of mines reported at position x, y.
  void IncreaseMineCount(int x, int y);
  // Increases the number of mines in the neighborhood of this field.
  void IncreaseNeighborMineCounts(int x, int x);

  // Resizes the mine field and removes all mines.
  void ResetMinefield(int width, int height);

  // The mine field.
  int width_;
  int height_;
  MineField mine_field_;
  // Set to true if the mine field is closed for changes.
  bool is_closed_;
};

}  // namespace mineseeker

#endif  // MINESEEKER_MINESWEEPER_H_
