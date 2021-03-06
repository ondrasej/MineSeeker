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
    : temporary_status_(0),
      state_(HIDDEN) {
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
      is_dead_(false),
      safe_field_requests_(-1) {
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

void MineSeeker::DebugString(string* out) const {
  std::stringstream out_stream;
  out_stream << "Is dead: " << is_dead_ << std::endl;
  out_stream << "Safe spots: " << safe_field_requests_ << std::endl;
  for (int y = 0; y < mine_sweeper_.height(); ++y) {
    for (int x = 0; x < mine_sweeper_.width(); ++x) {
      switch (StateAtPosition(x, y)) {
        case MineSeekerField::HIDDEN:
          out_stream << '.';
          break;
        case MineSeekerField::MINE:
          out_stream << '*';
          break;
        case MineSeekerField::UNCOVERED:
          const int num_mines = NumberOfMinesAroundField(x, y);
          if (num_mines == 0) {
            out_stream << ' ';
          } else {
           out_stream << num_mines;
          }
          break;
      }
    }
    out_stream << std::endl;
  }
  *out = out_stream.str();
}

const MineSeekerField& MineSeeker::FieldAtPosition(int x, int y) const {
  CheckCoordinatesAreValid(x, y);
  return state_[x][y];
}

bool MineSeeker::GetSafeFieldCoordinates(FieldCoordinate* coordinates) {
  CHECK_NOTNULL(coordinates);
  LOG(INFO) << "Asking for a hint";
  ++safe_field_requests_;
  for (int x = 0; x < mine_sweeper_.width(); ++x) {
    for (int y = 0; y < mine_sweeper_.height(); ++y) {
      if (MineSeekerField::HIDDEN == StateAtPosition(x, y)
          && !mine_sweeper_.IsMine(x, y)
          && 0 == mine_sweeper_.NumberOfMinesAroundField(x, y)) {
        coordinates->x = x;
        coordinates->y = y;
        LOG(INFO) << "Got hint: " << x << " " << y;
        return true;
      }
    }
  }
  for (int x = 0; x < mine_sweeper_.width(); ++x) {
    for (int y = 0; y < mine_sweeper_.height(); ++y) {
      if (MineSeekerField::HIDDEN == StateAtPosition(x, y)
          && !mine_sweeper_.IsMine(x, y)) {
        coordinates->x = x;
        coordinates->y = y;
        LOG(INFO) << "Got hint: " << x << " " << y;
        return true;
      }
    }
  }
  LOG(INFO) << "No hint :(";
  return false;
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
  LOG(INFO) << "Found mine at " << x << " " << y;
  const MineSeekerField::State state = StateAtPosition(x, y);
  switch (state) {
    case MineSeekerField::HIDDEN:
      state_[x][y].set_state(MineSeekerField::MINE);
      QueueNeighborsForUpdate(x, y);
    case MineSeekerField::MINE:
      break;
    default:
      LOG(FATAL) << "Invalid field state: " << state;
  }

  string debug_output;
  DebugString(&debug_output);
  LOG(INFO) << debug_output;
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

void MineSeeker::QueueFieldPairForUpdate(int x1, int y1, int x2, int y2) {
  pair_update_queue_.push(std::make_pair(FieldCoordinate(x1, y1),
                                         FieldCoordinate(x2, y2)));
}

void MineSeeker::QueueNeighborsForUpdate(int x, int y) {
  for (int i = -1; i <= 1; ++i) {
    for (int j = -1; j <= 1; ++j) {
      if (i != 0 || j != 0) {
        QueueFieldForUpdate(x + i, y + j);
      }
    }
  }
  for (int i = -2; i <= 2; ++i) {
    for (int j = -2; j <= 2; ++j) {
      if (i != 0 || j != 0) {
        QueueFieldPairForUpdate(x, y, x + i, y + j);
        QueueFieldPairForUpdate(x + i, y + j, x, y);
      }
    }
  }
}

void MineSeeker::ResetTemporaryStatuses() {
  for (int x = 0; x < mine_sweeper_.width(); ++x) {
    for (int y = 0; y < mine_sweeper_.height(); ++y) {
      state_[x][y].ResetTemporaryStatus();
    }
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
  FieldCoordinate start_coordinates(-1, -1);
  if (!GetSafeFieldCoordinates(&start_coordinates)) {
    LOG(INFO) << "There is no safe start field";
    return false;
  }

  UncoverField(start_coordinates.x, start_coordinates.y);

  while (!IsSolved()) {
    if (!SolveStep()) {
      break;
    }
  }
  return IsSolved() && !is_dead();
}

bool MineSeeker::SolveStep() {
  if (!uncover_queue_.empty()) {
    const FieldCoordinate& coordinates = uncover_queue_.front();
    if (MineSeekerField::HIDDEN == StateAtPosition(coordinates.x,
                                                   coordinates.y)) {
      UncoverField(coordinates.x, coordinates.y);
    }
    uncover_queue_.pop();
    return true;
  } else if (!update_queue_.empty()) {
    const FieldCoordinate& coordinates = update_queue_.front();
    UpdateConfigurationsAtPosition(coordinates.x, coordinates.y);
    update_queue_.pop();
    return true;
  } else if (!pair_update_queue_.empty()) {
    const FieldCoordinate& first = pair_update_queue_.front().first;
    const FieldCoordinate& second = pair_update_queue_.front().second;
    UpdatePairConsistency(first.x, first.y, second.x, second.y);
    pair_update_queue_.pop();
    return true;
  } else {
    FieldCoordinate safe_spot(-1, -1);
    if (!GetSafeFieldCoordinates(&safe_spot)) {
      return false;
    }
    UncoverField(safe_spot.x, safe_spot.y);
    return true;
  }
  return false;
}

bool MineSeeker::UncoverField(int x, int y) {
  CheckCoordinatesAreValid(x, y);
  LOG(INFO) << "Uncovering field " << x << " " << y;

  MineSeekerField* const field = &state_[x][y];
  CHECK_EQ(MineSeekerField::HIDDEN, field->state());

  if (mine_sweeper_.IsMine(x, y)) {
    // The seeker stepped on a mine and is dead. Kaboom!
    LOG(INFO) << "Death on the position " << x << " " << y;
    field->set_state(MineSeekerField::MINE);
    is_dead_ = true;
    return false;
  }
  
  field->set_state(MineSeekerField::UNCOVERED);
  int num_mines_around = mine_sweeper_.NumberOfMinesAroundField(x, y);

  if (num_mines_around == 0) {
    // TODO(ondrasej): Refactor this code.
    field->SetConfiguration(0);
    for (int i = -1; i <= 1; ++i) {
      for (int j = -1; j <= 1; ++j) {
        if (i != 0 || j != 0) {
          QueueFieldForUncover(x + i, y + j);
        }
      }
    }
  } else {
    UpdateConfigurationsAtPosition(x, y);
  }
  QueueNeighborsForUpdate(x, y);

  string state;
  DebugString(&state);
  LOG(INFO) << state;

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

  for (int i = -2; i <= 2; ++i) {
    for (int j = -2; j <= 2; ++j) {
      if (i != 0 || j != 0) {
        QueueFieldPairForUpdate(x, y, x + i, y + j);
        QueueFieldPairForUpdate(x + i, y + j, x, y);
      }
    }
  }


  if (changed_configurations) {
    UpdateNeighborsAtPosition(x, y);
    // TODO(ondrasej): Check if we proved out some new mines or clear fields;
    // check consistency with neighbor fields.
  }
}

namespace {
// Coordinates for converting bits in the ID of a configuration to the relative
// coordinates of the field to which the bit refers.
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
  for (int configuration = 1;
       configuration < MineSeekerField::kNumPossibleConfigurations;
       ++configuration) {
    if (field.IsPossibleConfiguration(configuration)) {
      mines_in_neighborhood &= configuration;
      empty_fields_in_neighborhood &= (0xFF & ~configuration);
    }
  }
  // Uncover the fields that are certain not to contain a mine, mark fields with
  // mines as such.
  for (int bit = 0; bit < 8; ++bit) {
    const int updated_field_x = x + kMineRelativePositionX[bit];
    const int updated_field_y = y + kMineRelativePositionY[bit];
    if (IsBitSet(empty_fields_in_neighborhood, bit)) {
      QueueFieldForUncover(updated_field_x, updated_field_y);
    }
    if (IsBitSet(mines_in_neighborhood, bit)) {
      MarkAsMine(updated_field_x, updated_field_y);
    }
  }
}

void MineSeeker::PopConfigurationAt(int configuration, int x, int y) {
  for (int bit = 0; bit < 8; ++bit) {
    const int field_x = x + kMineRelativePositionX[bit];
    const int field_y = y + kMineRelativePositionY[bit];
    if (field_x >= 0 && field_y >= 0
        && field_x < mine_sweeper_.width()
        && field_y < mine_sweeper_.height()) {
      const bool configuration_has_a_mine = IsBitSet(configuration, bit);
      MineSeekerField* const field = &state_[field_x][field_y];
      if (configuration_has_a_mine) {
        field->PopTemporaryMine();
      } else {
        field->PopTemporaryClearArea();
      }
    }
  }
}

bool MineSeeker::PushConfigurationAt(int configuration, int x, int y) {
  bool configuration_was_ok = true;
  for (int bit = 0; bit < 8; ++bit) {
    const int field_x = x + kMineRelativePositionX[bit];
    const int field_y = y + kMineRelativePositionY[bit];
    if (field_x >= 0 && field_y >= 0
        && field_x < mine_sweeper_.width()
        && field_y < mine_sweeper_.height()) {
      const bool configuration_has_a_mine = IsBitSet(configuration, bit);
      MineSeekerField* const field = &state_[field_x][field_y];
      if (configuration_has_a_mine) {
        configuration_was_ok &= field->PushTemporaryMine();
      } else {
        configuration_was_ok &= field->PushTemporaryClearArea();
      }
    }
  }

  return configuration_was_ok;
}

void MineSeeker::UpdatePairConsistency(int x1, int y1, int x2, int y2) {
  CHECK_GE(x1 - x2, -2);
  CHECK_LE(x1 - x2, 2);
  CHECK_GE(y1 - y2, -2);
  CHECK_LE(y1 - y2, 2);

  if (x1 < 0 || x1 >= mine_sweeper_.width()
      || y1 < 0 || y1 >= mine_sweeper_.height()
      || MineSeekerField::UNCOVERED != StateAtPosition(x1, y1)
      || state_[x1][y1].IsBound()
      || x2 < 0 || x2 >= mine_sweeper_.width()
      || y2 < 0 || y2 >= mine_sweeper_.height()
      || MineSeekerField::UNCOVERED != StateAtPosition(x2, y2)) {
    return;
  }

  const vector<bool>& configurations1 = state_[x1][y1].configurations();
  const vector<bool>& configurations2 = state_[x2][y2].configurations();
  bool configurations_were_updated = false;
  for (int configuration1 = 0;
       configuration1 < MineSeekerField::kNumPossibleConfigurations;
       ++configuration1) {
    if (!configurations1[configuration1]) {
      continue;
    }
    CHECK(PushConfigurationAt(configuration1, x1, y1));
    bool found_matching_configuration = false;
    for (int configuration2 = 0;
         configuration2 < MineSeekerField::kNumPossibleConfigurations;
         ++configuration2) {
      if (!configurations2[configuration2]) {
        continue;
      }
      if (PushConfigurationAt(configuration2, x2, y2)) {
        found_matching_configuration = true;
      }
      PopConfigurationAt(configuration2, x2, y2);
      if (found_matching_configuration) {
        break;
      }
    }
    PopConfigurationAt(configuration1, x1, y1);
    if (!found_matching_configuration) {
      LOG(INFO) << "Removing configuration " << configuration1 << " at " << x1
          << " " << y1;
      state_[x1][y1].RemoveConfiguration(configuration1);
      configurations_were_updated = true;
    }
  }
  if (configurations_were_updated) {
    UpdateConfigurationsAtPosition(x1, y1);
    UpdateNeighborsAtPosition(x1, y1);
  }
}

}  // namespace mineseeker
