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

#include <queue>
#include "common.h"
#include "gtest/gtest.h"

namespace mineseeker {

class MineSweeper;

// Contains information about the state of a single field in the mine seeker.
// Keeps track whether the field was already uncovered and the number of
// possible configurations of mines in the neighborhood of the field.
class MineSeekerField {
 public:
  // The state of a mine field from the viewpoint of the mine seeker.
  enum State {
    // The field was not visited nor proven to contain a mine yet.
    HIDDEN = 0,
    // The field was proven to contain a mine.
    MINE = 1,
    // The field was uncovered and did not contain a mine.
    UNCOVERED = 2,
  };

  // The number of all possible configurations. This is equal to the number of
  // combinations of mines that can be around a given field.
  static const int kNumPossibleConfigurations;

  // Initializes a new mine seeker field. Marks the field as uncovered and
  // allows all configurations for the field.
  MineSeekerField();

  // Returns true if the configuration can be assigned to this field.
  bool IsPossibleConfiguration(int configuration) const {
    DCHECK_GE(configuration, 0);
    DCHECK_LT(configuration, kNumPossibleConfigurations);
    return configurations_[configuration];
  }
  // Returns true if this field may contain a mine, i.e. it was not uncovered
  // yet, or it was already proven to contain a mine.
  bool IsPossibleMine() const { return state_ != UNCOVERED; }
  // Returns true if this field is bound, i.e. a single configuration is
  // assigned to it.
  bool IsBound() const;

  // Returns the number of configuration that can be assigned to this field
  // (given its neighborhood).
  int NumberOfActiveConfigurations() const;
  // Disables the specified configuration.
  void RemoveConfiguration(int configuration);
  // Binds the field to a given configuration.
  void SetConfiguration(int configuration);

  // Returns the state of the field.
  State state() const { return state_; }
  // Changes the state of the field (but only updates the state, calling this
  // method does not run propagation to other fields).
  void set_state(State state) { state_ = state; }
  // Returns a bitmap with the possible configurations. For each configuration
  // ID, this bitmap contains true if the configuration can be assigned to this
  // field and false otherwise.
  const vector<bool>& configurations() const { return configurations_; }

  // TODO(ondrasej): Documentation for temporary statuses.
  int temporary_status() const { return temporary_status_; }
  void PopTemporaryMine() { --temporary_status_; }
  bool PushTemporaryMine() {
    bool result = temporary_status_ >= 0;
    ++temporary_status_;
    return result;
  }
  void PopTemporaryClearArea() { ++temporary_status_; }
  bool PushTemporaryClearArea() {
    bool result = temporary_status_ <= 0;
    --temporary_status_;
    return result;
  }
  void ResetTemporaryStatus() { temporary_status_ = 0; }
  
 private:
  // Resets the configurations - enables all configurations.
  void ResetConfigurations();

  int temporary_status_;

  State state_;
  vector<bool> configurations_;
};

// Contains coordinates of a field.
struct FieldCoordinate {
  int x;
  int y;
  
  FieldCoordinate(int x_coord, int y_coord)
      : x(x_coord), y(y_coord) {}
};

// Implements the mine seeking algorithm. Uses propagation and tree search to
// prove the fields contain mines or not.
class MineSeeker {
 public:
  explicit MineSeeker(const MineSweeper& mine_sweeper);

  // Tests if configuration can be placed at the position (x, y) with respect to
  // the knowledge about the other fields.
  bool ConfigurationFitsAt(int configuration, int x, int y) const;

  const MineSeekerField& FieldAtPosition(int x, int y) const;
  MineSeekerField::State StateAtPosition(int x, int y) const;

  // Tests if the current state of the seeker allows for a mine at the position
  // (x, y). The state allows the mine if it has already proved that the field
  // contains the mine or the field was not uncovered yet.
  // Returns false if the coordinates are outside the mine field.
  bool IsPossibleMineAt(int x, int y) const;

  // Returns true if the mine seeker finished either by finding all mines or
  // stepping on a mine.
  bool IsSolved() const;

  // Returns the number of mines around the given field. The field must be
  // uncovered for this method to return the number. For fields marked with
  // mines or hidden fields, this method returns -1.
  int NumberOfMinesAroundField(int x, int y) const;

  // Runs the solver. Returns true if the game was successfully solved;
  // otherwise, returns false.
  bool Solve();

  // Marks the given field as a field with mine. Runs propagation on its
  // neighbors.
  void MarkAsMine(int x, int y);
  // Uncovers the given field. If the field did not contain a mine, returns true
  // and runs propagation on its neighbors. Otherwise, returns false and sets
  // is_dead to true.
  bool UncoverField(int x, int y);

  // Returns true if the mine seeker stepped on a mine.
  bool is_dead() const { return is_dead_; }
  // Returns the MineSweeper instance on which the game is played.
  const MineSweeper& mine_sweeper() const { return mine_sweeper_; }
  // Returns the number of times the solver requested a safe field.
  int safe_field_requests() const { return safe_field_requests_; }

  void DebugString(string* out) const;

 private:
  typedef vector<vector<MineSeekerField> > MineSeekerState;
  typedef vector<vector<int> > IntMatrix;
  typedef std::pair<FieldCoordinate, FieldCoordinate> CoordinatePair;

  // Checks that the given coordinates are valid. Uses CHECK_GE and CHECK_LT on
  // them.
  void CheckCoordinatesAreValid(int x, int y) const;

  // Tests if the given configuration can be assigned to the field at position
  // (x, y) with respect to its neighbor with relative coordinates (cx, cy).
  bool ConfigurationFitsWithSingleField(int configuration,
                                        int x,
                                        int x,
                                        int cx,
                                        int cy) const;

  // Selects a field with no mine that was not uncovered yet (for cases where
  // the solver gets stuck).
  bool GetSafeFieldCoordinates(FieldCoordinate* coordinates);

  void QueueFieldForUncover(int x, int y);
  void QueueNeighborsForUpdate(int x, int y);
  void QueueFieldForUpdate(int x, int y);
  void QueueFieldPairForUpdate(int x1, int y1, int x2, int y2);

  // Resets the state of the mine seeker.
  void ResetState();

  void ResetTemporaryStatuses();
  bool PushConfigurationAt(int configuration, int x, int y);
  void PopConfigurationAt(int configuration, int x, int y);

  // Performs a single step of the solution 
  bool SolveStep();

  // Updates the available configurations at the given position based on the
  // fields around the position.
  void UpdateConfigurationsAtPosition(int x, int y);

  // Uses the list of possible configurations at the given position to determine
  // fields in neighborhood that are certain to contain a mine or that are
  // certain to be clear. Updates the state of such fields and schedules their
  // neighbors for update.
  void UpdateNeighborsAtPosition(int x, int y);

  // Removes non-compatible configurations for a pair of neighboring fields.
  void UpdatePairConsistency(int x1, int y1, int x2, int y2);

  // The queues for fields that should be uncovered by the algorithm and fields
  // that should be updated (after something in their neighborhood changed). The
  // algorithm processes them asynchronously to avoid too deep recursion and to
  // give uncovering a higher priority.
  std::queue<FieldCoordinate> uncover_queue_;
  std::queue<FieldCoordinate> update_queue_;
  std::queue<CoordinatePair> pair_update_queue_;

  // Reference to the mine field on which the mine seeker works.
  const MineSweeper& mine_sweeper_;
  // The state of the mine seeker.
  MineSeekerState state_;
  // Keeps trace of whether the mineseeker stepped on a mine when uncovering a
  // new field.
  bool is_dead_;
  // The number of calls to GetSafeFieldCoordinates used while solving the
  // puzzle.
  int safe_field_requests_;

  FRIEND_TEST(MineSeekerTest, TestTemporaryStatus);
  FRIEND_TEST(MineSeekerTest, TestUpdateConfigurationsAtPoint);
  FRIEND_TEST(MineSeekerTest, TestUpdateNeighborsAtPoint);
  FRIEND_TEST(MineSeekerTest, TestUpdatePairConsistency);
  FRIEND_TEST(MineSeekerTest, TestUncoverFieldWithNoMine);
};

}  // namespace mineseeker

#endif  // MINESEEKER_MINESEEKER_H_
