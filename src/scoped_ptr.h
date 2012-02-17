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

#ifndef MINESEEKER_SCOPED_PTR_H_
#define MINESEEKER_SCOPED_PTR_H_

#include "glog/logging.h"

// A Boost-like scoped_ptr class. Holds a pointer and guarantees that this
// pointer is deleted when the scoped_ptr object is destroyed. Moreover, the
// scoped_ptr object is always initialized either with NULL, or with a
// user-provided pointer.
//
// Typical usage:
// scoped_ptr<MyClass> ptr(new MyClass());
// ptr->SomeMethodOfMyClass();
template<typename T>
class scoped_ptr {
 public:
  typedef T element_type;

  // Initializes the scoped_ptr object with NULL pointer.
  scoped_ptr() : ptr_(NULL) {}
  // Initializes the scoped_ptr object with the given pointer.
  explicit scoped_ptr(T* ptr) : ptr_(ptr) {}
  ~scoped_ptr() {
    reset(NULL);
  }

  T* get() const { return ptr_; }

  // Releases the pointer from the scoped_ptr object without deleting it;
  // returns the released pointer.
  T* release() {
    T* ptr = ptr_;
    ptr_ = NULL;
    return ptr;
  }

  // Updates the object in the scoped_ptr. Deletes the old object before
  // replacing it with the new one.
  void reset(T* new_ptr) {
    enum { TYPE_MUST_BE_COPLETE = sizeof(T) };
    delete ptr_;
    ptr_ = new_ptr;
  }

  T* operator->() const {
    DCHECK(ptr_ != NULL);
    return ptr_;
  }

  T& operator*() const {
    DCHECK(ptr_ != NULL);
    return *ptr_;
  }

  bool operator==(T* ptr) const { return ptr == ptr_; }
  bool operator!=(T* ptr) const { return ptr != ptr_; }

 private:
  T* ptr_;

  // By the semantics of scoped_ptr, comparing two scoped_ptr's does not make
  // sense, as each pointer must be owned by at most one scoped_ptr.
  bool operator==(const scoped_ptr& other);
  bool operator!=(const scoped_ptr& other);
};

#endif  // MINESEEKER_SCOPED_PTR_H_
