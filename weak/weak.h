#pragma once

#include "sw_fwd.h"  // Forward declaration
// https://en.cppreference.com/w/cpp/memory/weak_ptr
template <typename T>
class WeakPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors
    BaseControlBlock* block_;
    T* ptr_;
    WeakPtr() : block_(nullptr), ptr_(nullptr){};

    WeakPtr(const WeakPtr& other) {
        block_ = other.block_;
        ptr_ = other.ptr_;
        if (other.block_) {
            block_->IncWeakCnt();
        }
    };
    WeakPtr(WeakPtr&& other) : block_(other.block_), ptr_(other.ptr_) {
        other.block_ = nullptr;
        other.ptr_ = nullptr;
    };

    // Demote `SharedPtr`
    // #2 from https://en.cppreference.com/w/cpp/memory/weak_ptr/weak_ptr
    WeakPtr(const SharedPtr<T>& other) : block_(other.block_), ptr_(other.ptr_) {
        if (block_) {
            block_->IncWeakCnt();
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    WeakPtr& operator=(const WeakPtr& other) {
        if (other.block_ != nullptr) {
            if (block_ != other.block_) {
                Reset();
                ptr_ = other.ptr_;
                block_ = other.block_;
                block_->IncSharedCnt();
            }
        } else {
            Reset();
        }
        return *this;
    };
    WeakPtr& operator=(WeakPtr&& other) {
        std::swap(ptr_, other.ptr_);
        std::swap(block_, other.block_);
        return *this;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~WeakPtr() {
        Reset();
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        if (block_) {
            block_->DecWeakCnt();
            if (!(block_->GetSharedCnt() || block_->GetWeakCnt())) {
                delete block_;
            }
            block_ = nullptr;
            ptr_ = nullptr;
        }
    };
    void Swap(WeakPtr& other) {
        std::swap(block_, other.block_);
        std::swap(ptr_, other.ptr_);
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    size_t UseCount() const {
        if (block_) {
            return block_->GetSharedCnt();
        }
        return 0;
    };
    bool Expired() const {
        if (block_) {
            return block_->GetSharedCnt() == 0;
        }
        return true;
    };
    SharedPtr<T> Lock() const {
        if (Expired()) {
            return SharedPtr<T>();
        }
        SharedPtr<T> new_ptr;
        new_ptr.block_ = block_;
        new_ptr.ptr_ = ptr_;
        block_->IncSharedCnt();
        return new_ptr;
    };
};
