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

namespace mineseeker {

TEST(MineSweeperTest, TestCreate) {
  const int kWidth = 30;
  const int kHeight = 20;
  MineSweeper mine_sweeper(kWidth, kHeight);
  EXPECT_EQ(kWidth, mine_sweeper.width());
  EXPECT_EQ(kHeight, mine_sweeper.height());

  EXPECT_EQ(0, mine_sweeper.NumberOfMines());
  for (int x = 0; x < kWidth; ++x) {
    for (int y = 0; y < kHeight; ++y) {
      EXPECT_FALSE(mine_sweeper.IsMine(x, y));
    }
  }
}

TEST(MineSweeperTest, TestSetMine) {
  const int kWidth = 30;
  const int kHeight = 20;

  const int kMineX[] = { 1, 0, 10, 3, 20, 29 };
  const int kMineY[] = { 1, 0, 15, 8, 19, 0 };
  const int kNumMines = ARRAYSIZE(kMineX);
  CHECK_EQ(kNumMines, ARRAYSIZE(kMineY));

  MineSweeper mine_sweeper(kWidth, kHeight);
  EXPECT_EQ(0, mine_sweeper.NumberOfMines());

  for (int i = 0; i < kNumMines; ++i) {
    const int x = kMineX[i];
    const int y = kMineY[i];

    const int kExpectedNumMinesPreSet = i;
    EXPECT_EQ(kExpectedNumMinesPreSet, mine_sweeper.NumberOfMines());

    EXPECT_FALSE(mine_sweeper.IsMine(x, y));
    mine_sweeper.SetMine(x, y, true);
    EXPECT_TRUE(mine_sweeper.IsMine(x, y));

    const int kExpectedNumMinesPostSet = i + 1;
    EXPECT_EQ(kExpectedNumMinesPostSet, mine_sweeper.NumberOfMines());
  }
}

}  // namespace mineseeker
