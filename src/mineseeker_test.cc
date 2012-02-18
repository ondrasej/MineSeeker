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

#include "common.h"
#include "glog/logging.h"
#include "gtest/gtest.h"
#include "minesweeper.h"
#include "mineseeker.h"
#include "scoped_ptr.h"

namespace mineseeker {

class MineSeekerTest : public testing::Test {
 protected:
  static const int kWidth = 30;
  static const int kHeight = 20;

  static const int kMineX[];
  static const int kMineY[];
  static const int kNumMines;

  virtual void SetUp() {
    mine_sweeper_.reset(new MineSweeper(kWidth, kHeight));

    for (int i = 0; i < kNumMines; ++i) {
      const int x = kMineX[i];
      const int y = kMineY[i];
      mine_sweeper_->SetMine(x, y, true);
    }
    mine_sweeper_->CloseMineField();
  }

  scoped_ptr<MineSweeper> mine_sweeper_;
};

const int MineSeekerTest::kMineX[] = { 1, 0, 10, 3, 20, 29, 15, 15, 15 };
const int MineSeekerTest::kMineY[] = { 1, 0, 15, 8, 19, 0, 0, 1, 2 };
const int MineSeekerTest::kNumMines = ARRAYSIZE(MineSeekerTest::kMineX);

TEST_F(MineSeekerTest, TestCreate) {
  MineSeeker mine_seeker(*mine_sweeper_);
  
  EXPECT_EQ(mine_sweeper_.get(), &mine_seeker.mine_sweeper());
  EXPECT_FALSE(mine_seeker.is_dead());

  for (int x = 0; x < kWidth; ++x) {
    for (int y = 0; y < kHeight; ++y) {
      const MineSeekerField& field = mine_seeker.FieldAtPosition(x, y);
      EXPECT_EQ(MineSeekerField::HIDDEN, field.state());
      EXPECT_TRUE(field.configurations()[0])
          << "Configuration with no mines is not allowed at " << x << " " << y
          << std::endl
          << "Number of allowed configurations: "
          << field.NumberOfActiveConfigurations();
      if (x > 0 && x < kWidth - 1 && y > 0 && y < kHeight - 1) {
        EXPECT_EQ(MineSeekerField::kNumPossibleConfigurations,
                  field.NumberOfActiveConfigurations());
      }
    }
  }

  const int kCornerFieldX[] = { 0, 0, kWidth - 1, kWidth - 1 };
  const int kCornerFieldY[] = { 0, kHeight - 1, 0, kHeight - 1 };
  const int kNumCornerFields = ARRAYSIZE(kCornerFieldX);
  CHECK_EQ(kNumCornerFields, ARRAYSIZE(kCornerFieldY));
  const int kExpectedAllowedConfigurationsInCorner = 8;
  for (int i = 0; i < kNumCornerFields; ++i) {
    const int x = kCornerFieldX[i];
    const int y = kCornerFieldY[i];
    const MineSeekerField& field = mine_seeker.FieldAtPosition(x, y);
    EXPECT_EQ(kExpectedAllowedConfigurationsInCorner,
              field.NumberOfActiveConfigurations());
  }

  const int kExpectedAllowedConfigurationsOnEdge = 32;
  for (int x = 1; x < kWidth - 1; ++x) {
    const MineSeekerField& top_field = mine_seeker.FieldAtPosition(x, 0);
    EXPECT_EQ(kExpectedAllowedConfigurationsOnEdge,
              top_field.NumberOfActiveConfigurations());
    const MineSeekerField& bottom_field =
        mine_seeker.FieldAtPosition(x, kHeight - 1);
    EXPECT_EQ(kExpectedAllowedConfigurationsOnEdge,
              bottom_field.NumberOfActiveConfigurations());
  }
  for (int y = 1; y < kHeight - 1; ++y) {
    const MineSeekerField& left_field = mine_seeker.FieldAtPosition(0, y);
    EXPECT_EQ(kExpectedAllowedConfigurationsOnEdge,
              left_field.NumberOfActiveConfigurations());
    const MineSeekerField& right_field =
        mine_seeker.FieldAtPosition(kWidth - 1, y);
    EXPECT_EQ(kExpectedAllowedConfigurationsOnEdge,
              right_field.NumberOfActiveConfigurations());
  }
}

TEST_F(MineSeekerTest, TestConfigurationFitsAt) {
  MineSeeker mine_seeker(*mine_sweeper_);

  for (int x = 1; x < kWidth - 1; ++x) {
    for (int y = 1; y < kHeight - 1; ++y) {
      for (int configuration = 0;
           configuration < MineSeekerField::kNumPossibleConfigurations;
           ++configuration) {
        EXPECT_TRUE(mine_seeker.ConfigurationFitsAt(0, x, y));
      }
    }
  }
}

namespace {
void CheckAllowedConfigurationsForField(const MineSeekerField& field,
                                        const int* allowed_configurations,
                                        int num_allowed_configurations) {
  CHECK_NOTNULL(allowed_configurations);
  CHECK_GE(num_allowed_configurations, 0);
  std::set<int> configurations(
      allowed_configurations,
      allowed_configurations + num_allowed_configurations);
  for (int configuration = 0;
       configuration < MineSeekerField::kNumPossibleConfigurations;
       ++configuration) {
    const bool configuration_is_allowed =
        configurations.count(configuration) > 0;
    EXPECT_EQ(configuration_is_allowed,
              field.IsPossibleConfiguration(configuration));
  }
}
}  // namespace

TEST_F(MineSeekerTest, TestAllowedConfigurationsInCorners) {
  MineSeeker mine_seeker(*mine_sweeper_);

  const int kNumAllowedConfigurations = 8;

  const int kAllowedConfigurationsInLeftTopCorner[] =
      { 0, 16, 64, 80, 128, 144, 192, 208 };
  const MineSeekerField& top_left_field = mine_seeker.FieldAtPosition(0, 0);
  CheckAllowedConfigurationsForField(top_left_field,
                                     kAllowedConfigurationsInLeftTopCorner,
                                     kNumAllowedConfigurations);

  const int kAllowedConfigurationsInBottomRightCorner[] =
      { 0, 1, 2, 3, 8, 9, 10, 11 };
  const MineSeekerField& bottom_righ_field =
      mine_seeker.FieldAtPosition(kWidth - 1, kHeight - 1);
  CheckAllowedConfigurationsForField(bottom_righ_field,
                                     kAllowedConfigurationsInBottomRightCorner,
                                     kNumAllowedConfigurations);
}

TEST_F(MineSeekerTest, TestMarkAsMine) {
  MineSeeker mine_seeker(*mine_sweeper_);

  EXPECT_EQ(MineSeekerField::HIDDEN, mine_seeker.StateAtPosition(0, 0));
  EXPECT_TRUE(mine_seeker.IsPossibleMineAt(0, 0));
  mine_seeker.MarkAsMine(0, 0);
  EXPECT_EQ(MineSeekerField::MINE, mine_seeker.StateAtPosition(0, 0));
  EXPECT_TRUE(mine_seeker.IsPossibleMineAt(0, 0));
}

TEST_F(MineSeekerTest, TestSolve) {
  MineSeeker mine_seeker(*mine_sweeper_);

  mine_seeker.UncoverField(10, 10);
  EXPECT_TRUE(mine_seeker.Solve());

  string debug_output;
  mine_seeker.DebugString(&debug_output);
}

TEST_F(MineSeekerTest, TestUncoverFieldWithMine) {
  MineSeeker mine_seeker(*mine_sweeper_);
  
  EXPECT_EQ(MineSeekerField::HIDDEN, mine_seeker.StateAtPosition(0, 0));
  EXPECT_FALSE(mine_seeker.is_dead());
  EXPECT_FALSE(mine_seeker.UncoverField(0, 0));
  EXPECT_TRUE(mine_seeker.is_dead());
}

TEST_F(MineSeekerTest, TestUncoverFieldWithNoMine) {
  MineSeeker mine_seeker(*mine_sweeper_);

  EXPECT_EQ(MineSeekerField::HIDDEN, mine_seeker.StateAtPosition(1, 0));
  EXPECT_EQ(-1, mine_seeker.NumberOfMinesAroundField(1, 0));
  EXPECT_FALSE(mine_seeker.is_dead());
  EXPECT_TRUE(mine_seeker.UncoverField(1, 0));
  EXPECT_FALSE(mine_seeker.is_dead());
  EXPECT_EQ(2, mine_seeker.NumberOfMinesAroundField(1, 0));
  EXPECT_EQ(0, mine_seeker.update_queue_.size());

  EXPECT_EQ(MineSeekerField::HIDDEN, mine_seeker.StateAtPosition(10, 10));
  EXPECT_EQ(-1, mine_seeker.NumberOfMinesAroundField(10, 10));
  EXPECT_FALSE(mine_seeker.is_dead());
  EXPECT_TRUE(mine_seeker.UncoverField(10, 10));
  EXPECT_FALSE(mine_seeker.is_dead());
  EXPECT_EQ(0, mine_seeker.NumberOfMinesAroundField(10, 10));
  EXPECT_EQ(0, mine_seeker.update_queue_.size());
  EXPECT_EQ(8, mine_seeker.uncover_queue_.size());
}

TEST_F(MineSeekerTest, TestUpdateConfigurationsAtPoint) {
  MineSeeker mine_seeker(*mine_sweeper_);

  const int kPossibleConfigurationsWithNoMarkedMine[] =
      { 24, 40, 72, 136, 48, 80, 144, 96, 160, 192 };
  const int kNumPossibleConfigurationsWithNoMarkedMine =
      ARRAYSIZE(kPossibleConfigurationsWithNoMarkedMine);
  const std::set<int> possible_configurations_with_no_marked_mine(
      kPossibleConfigurationsWithNoMarkedMine,
      kPossibleConfigurationsWithNoMarkedMine +
      kNumPossibleConfigurationsWithNoMarkedMine);
  EXPECT_TRUE(mine_seeker.UncoverField(1, 0));
  EXPECT_EQ(2, mine_seeker.NumberOfMinesAroundField(1, 0));
  const MineSeekerField& field = mine_seeker.FieldAtPosition(1, 0);
  for (int configuration = 0;
       configuration < MineSeekerField::kNumPossibleConfigurations;
       ++configuration) {
    bool expected_is_possible_configuration =
        possible_configurations_with_no_marked_mine.count(configuration) > 0;
    EXPECT_EQ(expected_is_possible_configuration,
              field.IsPossibleConfiguration(configuration));
  }

  mine_seeker.MarkAsMine(0, 0);
  mine_seeker.UpdateConfigurationsAtPosition(1, 0);
  const int kPossibleConfigurationsWithMarkedMine[] =
      { 24, 40, 72, 136 };
  const int kNumPossibleConfigurationsWithMarkedMine =
      ARRAYSIZE(kPossibleConfigurationsWithMarkedMine);
  const std::set<int> possible_configurations_with_marked_mine(
      kPossibleConfigurationsWithMarkedMine,
      kPossibleConfigurationsWithMarkedMine +
      kNumPossibleConfigurationsWithMarkedMine);
  for (int configuration = 0;
       configuration < MineSeekerField::kNumPossibleConfigurations;
       ++configuration) {
    bool expected_is_possible_configuration =
        possible_configurations_with_marked_mine.count(configuration) > 0;
    EXPECT_EQ(expected_is_possible_configuration,
              field.IsPossibleConfiguration(configuration));
  }
}

TEST_F(MineSeekerTest, TestUpdateNeighborsAtPoint) {
  MineSeeker mine_seeker(*mine_sweeper_);

  const MineSeekerField& field = mine_seeker.FieldAtPosition(1, 0);
  EXPECT_TRUE(mine_seeker.UncoverField(1, 0));
  EXPECT_TRUE(mine_seeker.UncoverField(2, 0));
  EXPECT_TRUE(mine_seeker.UncoverField(2, 1));
  EXPECT_TRUE(mine_seeker.UncoverField(2, 2));
  EXPECT_TRUE(mine_seeker.UncoverField(0, 1));
  mine_seeker.UpdateConfigurationsAtPosition(1, 0);
  EXPECT_EQ(1, field.NumberOfActiveConfigurations());
  EXPECT_TRUE(field.IsBound());
}

}  // namespace mineseeker
