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

const int MineSeekerTest::kMineX[] = { 1, 0, 10, 3, 20, 29 };
const int MineSeekerTest::kMineY[] = { 1, 0, 15, 8, 19, 0 };
const int MineSeekerTest::kNumMines = ARRAYSIZE(MineSeekerTest::kMineX);

TEST_F(MineSeekerTest, TestCreate) {
  MineSeeker mine_seeker(*mine_sweeper_);
  
  EXPECT_EQ(mine_sweeper_.get(), &mine_seeker.mine_sweeper());
  EXPECT_FALSE(mine_seeker.is_dead());

  for (int x = 0; x < kWidth; ++x) {
    for (int y = 0; y < kHeight; ++y) {
      const MineSeekerField& field = mine_seeker.FieldAtPosition(x, y);
      EXPECT_EQ(MineSeekerField::HIDDEN, field.state());
      EXPECT_TRUE(field.configurations()[0]);
      if (x > 0 && x < kWidth - 1 && y > 0 && y < kHeight - 1) {
        EXPECT_EQ(MineSeekerField::kNumPossibleConfigurations,
                  field.NumberOfActiveConfigurations());
      }
    }
  }
}

}  // namespace mineseeker
