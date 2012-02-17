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

#ifndef MINESEEKER_MINESEEKER_H_
#define MINESEEKER_MINESEEKER_H_

#include "common.h"

namespace mineseeker {

class MineSweeper;

// Contains information about the state of a single field in the mine seeker.
// Keeps track whether the field was already uncovered and the number of
// possible configurations of mines in the neighborhood of the field.
class MineSeekerField {
 public:
  // The state of a mine field from the viewpoint of the mine seeker.
  enum MineSeekerFieldState {
    // The field was not visited nor proven to contain a mine yet.
    HIDDEN,
    // The field was proven to contain a mine.
    MINE,
    // The field was uncovered and did not contain a mine.
    UNCOVERED,
  };

  static const int kNumPossibleConfigurations;

  MineSeekerField();

  // Returns true if this field may contain a mine, i.e. it was not uncovered
  // yet, or it was already proven to contain a mine.
  bool IsPossibleMine() const { return state_ != MINE; }

  int NumberOfActiveConfigurations() const;
  void RemoveConfiguration(int configuration);

  MineSeekerFieldState state() const { return state_; }
  const vector<bool>& configurations() const { return configurations_; }

 private:
  void ResetConfigurations();

  MineSeekerFieldState state_;
  vector<bool> configurations_;
};

class MineSeeker {
 public:
  MineSeeker(const MineSweeper& mine_sweeper);

  const MineSeekerField& FieldAtPosition(int x, int y) const;
  void Solve();

  bool is_dead() const { return is_dead_; }
  const MineSweeper& mine_sweeper() const { return mine_sweeper_; }
 private:
  typedef vector<vector<MineSeekerField> > MineSeekerState;

  bool ConfigurationFitsWithSingleField(int configuration,
                                        int x,
                                        int x,
                                        int cx,
                                        int cy) const;
  // Tests if configuration can be placed at the position (x, y) with respect to
  // the knowledge about the other fields.
  bool ConfigurationFitsAt(int configuration, int x, int y) const;

  // Tests if the current state of the seeker allows for a mine at the position
  // (x, y). The state allows the mine if it has already proved that the field
  // contains the mine or the field was not uncovered yet.
  // Returns false if the coordinates are outside the mine field.
  bool IsPossibleMineAt(int x, int y) const;

  void ResetState();

  // Reference to the mine field on which the mine seeker works.
  const MineSweeper& mine_sweeper_;
  // The state of the mine seeker.
  MineSeekerState state_;
  // Keeps trace of whether the mineseeker stepped on a mine when uncovering a
  // new field.
  bool is_dead_;
};

}  // namespace mineseeker

#endif  // MINESEEKER_MINESEEKER_H_
