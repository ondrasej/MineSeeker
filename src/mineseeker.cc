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

MineSeekerField::MineSeekerField()
    : state_(HIDDEN) {
  ResetConfigurations();
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

MineSeeker::MineSeeker(const MineSweeper& mine_sweeper)
    : mine_sweeper_(mine_sweeper) {
  CHECK(mine_sweeper_.is_closed());
  ResetState();
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

inline bool IsBitSet(int bit, int value) {
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
}  // namespace

bool MineSeeker::ConfigurationFitsWithSingleField(int configuration,
                                                  int x,
                                                  int y,
                                                  int cx,
                                                  int cy) const {
  const bool configuration_has_mine = ConfigurationHasMineAt(configuration,
                                                             cx, cy);
  const bool state_allows_mine = IsPossibleMineAt(x + cx, y + cy);
  return configuration_has_mine == state_allows_mine;
}

bool MineSeeker::ConfigurationFitsAt(int configuration, int x, int y) const {
  DCHECK_GE(configuration, 0);
  DCHECK_LT(configuration, MineSeekerField::kNumPossibleConfigurations);
  DCHECK_GE(x, 0);
  DCHECK_LT(x, mine_sweeper_.width());
  DCHECK_GE(y, 0);
  DCHECK_LT(y, mine_sweeper_.height());
  for (int cx = -1; cx <= 1; ++cx) {
    for (int cy = -1; cy <= 1; ++cy) {
      if (cx != 0 || cy != 0) {
        if (!ConfigurationFitsWithSingleField(configuration, x, y, cx, cy))
          return false;
      }
    }
  }
  return true;
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

void MineSeeker::ResetState() {
  state_.clear();
  state_.resize(mine_sweeper_.width());
  for (int i = 0; i < state_.size(); ++i) {
    state_[i].resize(mine_sweeper_.height());
  }
}

void MineSeeker::Solve() {
}

}  // namespace mineseeker
