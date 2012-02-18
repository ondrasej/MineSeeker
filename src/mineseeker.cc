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

#include <algorithm>

#include "glog/logging.h"
#include "mineseeker.h"
#include "minesweeper.h"

namespace mineseeker {

const int MineSeekerField::kNumPossibleConfigurations = 256;

MineSeekerField::MineSeekerField()
    : state_(HIDDEN) {
  ResetConfigurations();
}

bool MineSeekerField::IsBound() const {
  // TODO(ondrasej): Optimize this (e.g. cache the number of active
  // configurations).
  return NumberOfActiveConfigurations() == 1;
}

int MineSeekerField::NumberOfActiveConfigurations() const {
  return std::count(configurations_.begin(), configurations_.end(), true);
}

void MineSeekerField::RemoveConfiguration(int configuration) {
  CHECK_GE(configuration, 0);
  CHECK_LT(configuration, kNumPossibleConfigurations);
  configurations_[configuration] = false;
}

void MineSeekerField::ResetConfigurations() {
  configurations_.clear();
  configurations_.resize(256, true);
}

void MineSeekerField::SetConfiguration(int configuration) {
  CHECK_GE(configuration, 0);
  CHECK_LT(configuration, kNumPossibleConfigurations);
  CHECK(configurations_[configuration]);
  std::fill(configurations_.begin(), configurations_.end(), false);
  configurations_[configuration] = true;
}

MineSeeker::MineSeeker(const MineSweeper& mine_sweeper)
    : mine_sweeper_(mine_sweeper),
      is_dead_(false) {
  CHECK(mine_sweeper_.is_closed());
  ResetState();
}

void MineSeeker::CheckCoordinatesAreValid(int x, int y) const {
  DCHECK_GE(x, 0);
  DCHECK_LT(x, mine_sweeper_.width());
  DCHECK_GE(y, 0);
  DCHECK_LT(y, mine_sweeper_.height());
}

namespace {
// Translates the relative position of the mine to the bit index in the ID of
// the configuration.
// TODO(ondrasej): Better explanation of the encoding of configurations with
// bits.
const int kMinePositionToConfigurationBit[] = {
  0,  1, 2,
  3, -1, 4,
  5,  6, 7,
};

inline bool IsBitSet(int value, int bit) {
  return 0 != (value & (1 << bit));
}

// Checks if configuration has a mine at position (x, y). The positions are
// expected to numbers between -1 and 1, where (0, 0) is the center of the
// configuration (and contains the field to which the configuration is
// assigned).
bool ConfigurationHasMineAt(int configuration, int x, int y) {
  const int linear_index = x + 1 + (y + 1) * 3;
  const int bit = kMinePositionToConfigurationBit[linear_index];
  return IsBitSet(configuration, bit);
}

int NumberOfMinesInConfiguration(int configuration) {
  int num_mines = 0;
  // TODO(ondrasej): A more efficient implementation...
  for (int i = 0; i < 8; ++i) {
    if ((configuration & (1 << i)) != 0) {
      ++num_mines;
    }
  }
  return num_mines;
}
}  // namespace

bool MineSeeker::ConfigurationFitsWithSingleField(int configuration,
                                                  int x,
                                                  int y,
                                                  int cx,
                                                  int cy) const {
  const bool configuration_has_mine = ConfigurationHasMineAt(configuration,
                                                             cx, cy);
  const MineSeekerField::State state = StateAtPosition(x + cx, y + cy);
  return state == MineSeekerField::HIDDEN
      || configuration_has_mine == (state == MineSeekerField::MINE);
}

bool MineSeeker::ConfigurationFitsAt(int configuration, int x, int y) const {
  DCHECK_GE(configuration, 0);
  DCHECK_LT(configuration, MineSeekerField::kNumPossibleConfigurations);
  CheckCoordinatesAreValid(x, y);

  int num_mines_around = NumberOfMinesAroundField(x, y);
  if (num_mines_around >= 0) {
    if (num_mines_around != NumberOfMinesInConfiguration(configuration)) {
      return false;
    }
  }

  for (int cx = -1; cx <= 1; ++cx) {
    for (int cy = -1; cy <= 1; ++cy) {
      if (cx != 0 || cy != 0) {
        if (!ConfigurationFitsWithSingleField(configuration, x, y, cx, cy)) {
          return false;
        }
      }
    }
  }
  return true;
}

const MineSeekerField& MineSeeker::FieldAtPosition(int x, int y) const {
  CheckCoordinatesAreValid(x, y);
  return state_[x][y];
}

MineSeekerField::State MineSeeker::StateAtPosition(int x, int y) const {
  if (x < 0
      || y < 0
      || x >= mine_sweeper_.width()
      || y >= mine_sweeper_.height()) {
    return MineSeekerField::UNCOVERED;
  }
  return state_[x][y].state();
}

bool MineSeeker::IsPossibleMineAt(int x, int y) const {
  if (x < 0
      || y < 0
      || x >= mine_sweeper_.width()
      || y >= mine_sweeper_.height()) {
    return false;
  }
  return state_[x][y].IsPossibleMine();
}

bool MineSeeker::IsSolved() const {
  if (is_dead_) {
    return true;
  }
  for (int x = 0; x < mine_sweeper_.width(); ++x) {
    for (int y = 0; y < mine_sweeper_.height(); ++y) {
      if (MineSeekerField::HIDDEN == StateAtPosition(x, y)) {
        return false;
      }
    }
  }
  return true;
}

void MineSeeker::MarkAsMine(int x, int y) {
  if (x < 0
      || y < 0
      || x >= mine_sweeper_.width()
      || y >= mine_sweeper_.height()) {
    return;
  }
  CHECK_EQ(MineSeekerField::HIDDEN, StateAtPosition(x, y));
  const MineSeekerField::State state = StateAtPosition(x, y);
  switch (state) {
    case MineSeekerField::HIDDEN:
      state_[x][y].set_state(MineSeekerField::MINE);
      // TODO(ondrasej): Queue the neighbors for update
    case MineSeekerField::MINE:
      break;
    default:
      LOG(FATAL) << "Invalid field state: " << state;
  }
}

int MineSeeker::NumberOfMinesAroundField(int x, int y) const {
  if (StateAtPosition(x, y) == MineSeekerField::UNCOVERED) {
    return mine_sweeper_.NumberOfMinesAroundField(x, y);
  } else {
    return -1;
  }
}

void MineSeeker::QueueFieldForUncover(int x, int y) {
  if (StateAtPosition(x, y) == MineSeekerField::HIDDEN) {
    uncover_queue_.push(FieldCoordinate(x, y));
  }
}

void MineSeeker::QueueFieldForUpdate(int x, int y) {
  if (StateAtPosition(x, y) == MineSeekerField::UNCOVERED
      && x >= 0 && x < mine_sweeper_.width()
      && y >= 0 && y < mine_sweeper_.height()
      && NumberOfMinesAroundField(x, y) > 0) {
    update_queue_.push(FieldCoordinate(x, y));
  }
}

void MineSeeker::ResetState() {
  state_.clear();
  state_.resize(mine_sweeper_.width());
  for (int i = 0; i < state_.size(); ++i) {
    state_[i].resize(mine_sweeper_.height());
  }

  // Filter possible configurations for the border.
  for (int x = 0; x < mine_sweeper_.width(); ++x) {
    UpdateConfigurationsAtPosition(x, 0);
    UpdateConfigurationsAtPosition(x, mine_sweeper_.height() - 1);
  }
  for (int y = 1; y < mine_sweeper_.height() - 1; ++y) {
    UpdateConfigurationsAtPosition(0, y);
    UpdateConfigurationsAtPosition(mine_sweeper_.width() - 1, y);
  }
}

bool MineSeeker::Solve() {
  while (!IsSolved()) {
    SolveStep();
  }
  return !is_dead();
}

void MineSeeker::SolveStep() {
  if (!uncover_queue_.empty()) {

  } else if (!update_queue_.empty()) {
    
  }
}

bool MineSeeker::UncoverField(int x, int y) {
  CheckCoordinatesAreValid(x, y);

  MineSeekerField* const field = &state_[x][y];
  CHECK_EQ(MineSeekerField::HIDDEN, field->state());

  if (mine_sweeper_.IsMine(x, y)) {
    // The seeker stepped on a mine and is dead. Kaboom!
    field->set_state(MineSeekerField::MINE);
    is_dead_ = true;
    return false;
  }
  
  field->set_state(MineSeekerField::UNCOVERED);
  int num_mines_around = mine_sweeper_.NumberOfMinesAroundField(x, y);

  if (num_mines_around == 0) {
    for (int i = -1; i <= 1; ++i) {
      for (int j = -1; j <= 1; ++j) {
        if (i != 0 || j != 0) {
          QueueFieldForUncover(x + i, x + j);
        }
      }
    }
  } else {
    UpdateConfigurationsAtPosition(x, y);
  }

  return true;
}

void MineSeeker::UpdateConfigurationsAtPosition(int x, int y) {
  CheckCoordinatesAreValid(x, y);

  bool changed_configurations = false;
  MineSeekerField* const field = &state_[x][y];
  for (int configuration = 0;
       configuration < MineSeekerField::kNumPossibleConfigurations;
       ++configuration) {
    if (field->IsPossibleConfiguration(configuration)) {
      if (!ConfigurationFitsAt(configuration, x, y)) {
        field->RemoveConfiguration(configuration);
        changed_configurations = true;
      }
    }
  }

  if (changed_configurations) {
    // TODO(ondrasej): Check if we proved out some new mines or clear fields;
    // check consistency with neighbor fields.
  }
}

namespace {
const int kMineRelativePositionX[] = { -1, 0, 1, -1, 1, -1, 0, 1 };
const int kMineRelativePositionY[] = { -1, -1, -1, 0, 0, 1, 1, 1 };
}

void MineSeeker::UpdateNeighborsAtPosition(int x, int y) {
  CheckCoordinatesAreValid(x, y);

  // Find fields in the neighborhood that are certain not to contain a mine.
  // Uses the encoding of the mines in the ID of the configuration to accumulate
  // the possible positions of mines in empty_fields_in_neighborhood; after the
  // loop is finished, empty_fields_in_neighborhood contains 1's at bits
  // corresponding to places proved not to contain mines. At the same time, it
  // accumulates the fields that are certain to contain mines in
  // mines_in_neighborhood the same way.
  int empty_fields_in_neighborhood = 0xFF;
  int mines_in_neighborhood = 0xFF;
  const MineSeekerField& field = state_[x][y];
  for (int configuration = 0;
       configuration < MineSeekerField::kNumPossibleConfigurations;
       ++configuration) {
    if (field.IsPossibleConfiguration(configuration)) {
      mines_in_neighborhood &= configuration;
      empty_fields_in_neighborhood &= (~configuration);
    }
  }
  // Uncover the fields that are certain not to contain a mine, mark fields with
  // mines as such.
  for (int bit = 0; bit < 8; ++bit) {
    const int updated_field_x = x + kMineRelativePositionX[bit];
    const int updated_field_y = y + kMineRelativePositionY[bit];
    if (IsBitSet(bit, empty_fields_in_neighborhood)) {
      QueueFieldForUncover(updated_field_x, updated_field_y);
    }
    if (IsBitSet(bit, mines_in_neighborhood)) {
      MarkAsMine(updated_field_x, updated_field_y);
    }
  }
  
}

}  // namespace mineseeker
