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

#include <iostream>
#include "common.h"
#include "glog/logging.h"
#include "minesweeper.h"
#include "mineseeker.h"
#include "scoped_ptr.h"

namespace mineseeker {

void ReadStdinToString(string* out) {
  CHECK_NOTNULL(out);
  out->clear();
  const int kBufferSize = 4096;
  char buffer[kBufferSize];
  while (std::cin) {
    std::cin.read(buffer, kBufferSize);
    out->append(buffer, std::cin.gcount());
  }
}

bool RunSolverOnStdin() {
  string input;
  ReadStdinToString(&input);

  scoped_ptr<MineSweeper> mine_sweeper(MineSweeper::LoadFromString(input));
  if (!mine_sweeper.get()) {
    return false;
  }

  scoped_ptr<MineSeeker> mine_seeker(new MineSeeker(*mine_sweeper));
  if (mine_seeker->Solve()) {
    LOG(INFO) << "Hooray!";
  } else {
    LOG(INFO) << "Did not finish, booo!";
  }
  
  string output;
  mine_seeker->DebugString(&output);
  std::cout << output;

  return true;
}
}  // namespace mineseeker

int main(int argc, char* argv[]) {
  google::InitGoogleLogging("MineSeeker");
  if (mineseeker::RunSolverOnStdin()) {
    return 0;
  } else {
    return 1;
  }
}
