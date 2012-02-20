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


TEST(MineSeekerFieldTest, TestPushTemporaryMine) {
  MineSeekerField field;

  const int kNumIterations = 10;
  for (int i = 0; i < kNumIterations; ++i) {
    EXPECT_EQ(i, field.temporary_status());
    EXPECT_TRUE(field.PushTemporaryMine());
    EXPECT_EQ(i + 1, field.temporary_status());
  }

  for (int i = kNumIterations; i > 0; --i) {
    EXPECT_EQ(i, field.temporary_status());
    field.PopTemporaryMine();
    EXPECT_EQ(i - 1, field.temporary_status());
  }
}

TEST(MineSeekerFieldTest, TestPushTemporaryClearArea) {
  MineSeekerField field;

  const int kNumIterations = 10;
  for (int i = 0; i < kNumIterations; ++i) {
    EXPECT_EQ(-i, field.temporary_status());
    EXPECT_TRUE(field.PushTemporaryClearArea());
    EXPECT_EQ(-i - 1, field.temporary_status());
  }

  for (int i = kNumIterations; i > 0; --i) {
    EXPECT_EQ(-i, field.temporary_status());
    field.PopTemporaryClearArea();
    EXPECT_EQ(-i + 1, field.temporary_status());
  }
}

TEST(MineSeekerFieldTest, TestPushTemporaryMineOnTemporaryClearArea) {
  MineSeekerField field;

  EXPECT_TRUE(field.PushTemporaryClearArea());
  EXPECT_EQ(-1, field.temporary_status());
  EXPECT_FALSE(field.PushTemporaryMine());
  EXPECT_EQ(0, field.temporary_status());
}

TEST(MineSeekerFieldTest, TestPushTemporaryClearAreaOnTemporaryMine) {
  MineSeekerField field;

  EXPECT_TRUE(field.PushTemporaryMine());
  EXPECT_EQ(1, field.temporary_status());
  EXPECT_FALSE(field.PushTemporaryClearArea());
  EXPECT_EQ(0, field.temporary_status());
}

// Base class for tests of the MineSeeker clas. Sets up a 30x20 minefield with
// a few mines to test the solver.
class MineSeekerTest : public testing::Test {
 protected:
  static const int kWidth = 30;
  static const int kHeight = 20;

  // The positions of the mines in the test minefield.
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

const int MineSeekerTest::kMineX[] = { 1, 0, 10, 3, 20, 29, 15, 15, 15, 9, 9, 10, 11 };
const int MineSeekerTest::kMineY[] = { 1, 0, 15, 8, 19, 0, 0, 1, 2, 19, 17, 17, 17 };
const int MineSeekerTest::kNumMines = ARRAYSIZE(MineSeekerTest::kMineX);

// Tests creating a new minefield. Examines that the state of the seeker allows
// for all possible configurations of the mines.
TEST_F(MineSeekerTest, TestCreate) {
  MineSeeker mine_seeker(*mine_sweeper_);
  
  EXPECT_EQ(mine_sweeper_.get(), &mine_seeker.mine_sweeper());
  EXPECT_FALSE(mine_seeker.is_dead());

  // Check that the configuration with no mines at all can occur at all
  // positions on the minefield and that the state allows for all possible
  // configurations.
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

  // There are eight possible configurations for the fields in the corner. Check
  // the number of possible configurations for the corners of the minefield.
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

  // Checks the number of possible configurations on the border of the minefield
  // (but not in the corners).
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

// Tests that in a minefield with all fields hidden, the configuration with no
// mines is allowed for all fields.
TEST_F(MineSeekerTest, TestConfigurationWithNoMinesFits) {
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
// Checks that the allowed configurations for the given field are exactly those
// provided in the list.
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

// Checks the allowed configurations for the left-top and bottom right corners
// of the minefield. All fields are hidden, but only a limited subset of
// configurations are allowed in the corners as there can be no mines outside
// the minefield.
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

// Tests marking a field with a mine.
TEST_F(MineSeekerTest, TestMarkAsMine) {
  MineSeeker mine_seeker(*mine_sweeper_);

  EXPECT_EQ(MineSeekerField::HIDDEN, mine_seeker.StateAtPosition(0, 0));
  EXPECT_TRUE(mine_seeker.IsPossibleMineAt(0, 0));
  mine_seeker.MarkAsMine(0, 0);
  EXPECT_EQ(MineSeekerField::MINE, mine_seeker.StateAtPosition(0, 0));
  EXPECT_TRUE(mine_seeker.IsPossibleMineAt(0, 0));
}

// Tests running the solver on a simple problem that can be solved by
// single field consistency.
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

// Tests uncovering a fields with no mine (one with no mines in the neighborhood
// and two with a mine). Checks that the neighbor fields will get queued for
// updating and uncovering.
TEST_F(MineSeekerTest, TestUncoverFieldWithNoMine) {
  MineSeeker mine_seeker(*mine_sweeper_);

  EXPECT_EQ(MineSeekerField::HIDDEN, mine_seeker.StateAtPosition(1, 0));
  EXPECT_EQ(-1, mine_seeker.NumberOfMinesAroundField(1, 0));
  EXPECT_FALSE(mine_seeker.is_dead());
  EXPECT_TRUE(mine_seeker.UncoverField(1, 0));
  EXPECT_FALSE(mine_seeker.is_dead());
  EXPECT_EQ(2, mine_seeker.NumberOfMinesAroundField(1, 0));
  EXPECT_EQ(0, mine_seeker.update_queue_.size());

  EXPECT_TRUE(mine_seeker.UncoverField(2, 0));
  EXPECT_FALSE(mine_seeker.is_dead());
  EXPECT_EQ(1, mine_seeker.update_queue_.size());
  EXPECT_EQ(1, mine_seeker.NumberOfMinesAroundField(2, 0));
  EXPECT_EQ(0, mine_seeker.uncover_queue_.size());

  EXPECT_EQ(MineSeekerField::HIDDEN, mine_seeker.StateAtPosition(10, 10));
  EXPECT_EQ(-1, mine_seeker.NumberOfMinesAroundField(10, 10));
  EXPECT_FALSE(mine_seeker.is_dead());
  EXPECT_TRUE(mine_seeker.UncoverField(10, 10));
  EXPECT_FALSE(mine_seeker.is_dead());
  EXPECT_EQ(0, mine_seeker.NumberOfMinesAroundField(10, 10));
  EXPECT_EQ(1, mine_seeker.update_queue_.size());
  EXPECT_EQ(8, mine_seeker.uncover_queue_.size());
}

// Tests updating the available configurations at the given point after marking
// one of its neighbors as a mine.
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

// Tests updating the neighbors based on available configurations at a given
// field.
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

TEST_F(MineSeekerTest, TestTemporaryStatus) {
  MineSeeker mine_seeker(*mine_sweeper_);

  for (int x = 0; x < 3; ++x) {
    for (int y = 0; y < 3; ++y) {
      EXPECT_EQ(0, mine_seeker.FieldAtPosition(x, y).temporary_status());
    }
  }
  const int kConfiguration1 = 7;
  EXPECT_TRUE(mine_seeker.PushConfigurationAt(kConfiguration1, 1, 1));
  const int kExpectedStatusesAfterPush[] = { 1, 1, 1, -1, 0, -1, -1, -1, -1 };
  const int* expected_status_pos = kExpectedStatusesAfterPush;
  for (int y = 0; y < 3; ++y) {
    for (int x = 0; x < 3; ++x) {
      EXPECT_EQ(*expected_status_pos,
                mine_seeker.FieldAtPosition(x, y).temporary_status())
          << "Invalid temporary status at " << x << " " << y;
      ++expected_status_pos;
    }
  }
  const int kConfiguration2 = 87;
  EXPECT_FALSE(mine_seeker.PushConfigurationAt(kConfiguration2, 1, 1));
  mine_seeker.PopConfigurationAt(kConfiguration2, 1, 1);

  EXPECT_FALSE(mine_seeker.PushConfigurationAt(kConfiguration1, 1, 2));
  mine_seeker.PopConfigurationAt(kConfiguration1, 1, 2);

  mine_seeker.PopConfigurationAt(kConfiguration1, 1, 1);

  for (int x = 0; x < kWidth; ++x) {
    for (int y = 0; y < kHeight; ++y) {
      EXPECT_EQ(0, mine_seeker.FieldAtPosition(x, y).temporary_status())
          << "Invalid temporary status at " << x << " " << y;
    }
  }
}

TEST_F(MineSeekerTest, TestUpdatePairConsistency) {
  MineSeeker mine_seeker(*mine_sweeper_);

  mine_seeker.UncoverField(0, 2);
  mine_seeker.UncoverField(1, 2);
  mine_seeker.UpdateConfigurationsAtPosition(0, 2);
  mine_seeker.UpdateConfigurationsAtPosition(1, 2);

  {
    const int kExpectedNumConfigurationsOnBorder = 4;
    EXPECT_EQ(kExpectedNumConfigurationsOnBorder,
              mine_seeker.FieldAtPosition(0, 2).NumberOfActiveConfigurations());
    const int kExpectedNumConfigurationsInside = 7;
    EXPECT_EQ(kExpectedNumConfigurationsInside,
              mine_seeker.FieldAtPosition(1, 2).NumberOfActiveConfigurations());

    mine_seeker.UpdatePairConsistency(1, 2, 0, 2);
    const int kExpectedNumConfigurationsAfterUpdate = 4;
    EXPECT_EQ(kExpectedNumConfigurationsAfterUpdate,
              mine_seeker.FieldAtPosition(0, 2).NumberOfActiveConfigurations());
    EXPECT_EQ(kExpectedNumConfigurationsAfterUpdate,
              mine_seeker.FieldAtPosition(1, 2).NumberOfActiveConfigurations());

    EXPECT_EQ(3, mine_seeker.uncover_queue_.size());
    mine_seeker.UpdateNeighborsAtPosition(1, 2);
    EXPECT_EQ(6, mine_seeker.uncover_queue_.size());
  }

  mine_seeker.UncoverField(10, 19);
  mine_seeker.UncoverField(10, 18);
  mine_seeker.UpdateConfigurationsAtPosition(10, 19);
  mine_seeker.UpdateConfigurationsAtPosition(10, 18);

  {
    const int kExpectedNumConfigurationsOnBorder = 4;
    EXPECT_EQ(
        kExpectedNumConfigurationsOnBorder,
        mine_seeker.FieldAtPosition(10, 19).NumberOfActiveConfigurations());
    const int kExpectedNumConfigurationsInside = 35;
    EXPECT_EQ(
        kExpectedNumConfigurationsInside,
        mine_seeker.FieldAtPosition(10, 18).NumberOfActiveConfigurations());
    mine_seeker.UpdatePairConsistency(10, 18, 10, 19);

    const int kExpectedNumConfigurationsAfterUpdate = 4;
    EXPECT_EQ(
        kExpectedNumConfigurationsAfterUpdate,
        mine_seeker.FieldAtPosition(10, 19).NumberOfActiveConfigurations());
    EXPECT_EQ(
        kExpectedNumConfigurationsAfterUpdate,
        mine_seeker.FieldAtPosition(10, 18).NumberOfActiveConfigurations());
  }
}

}  // namespace mineseeker
