#pragma once

#include "sw_fwd.h"  // Forward declaration

#include <cstddef>  // std::nullptr_t

// https://en.cppreference.com/w/cpp/memory/shared_ptr
template <typename T>
class SharedPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    SharedPtr() : block_(nullptr), ptr_(nullptr) {
    }
    SharedPtr(std::nullptr_t) : block_(nullptr), ptr_(nullptr){};
    explicit SharedPtr(T* ptr) : block_(nullptr), ptr_(ptr) {
        block_ = new ControlBlockPtr<T>(ptr);
    };
    template <typename Y>
    SharedPtr(Y* ptr) {
        ptr_ = ptr;
        block_ = new ControlBlockPtr<Y>(ptr);
    }

    SharedPtr(const SharedPtr& other) : block_(other.block_), ptr_(other.ptr_) {
        if (block_) {
            block_->IncSharedCnt();
        }
    };
    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other) {
        ptr_ = other.ptr_;
        block_ = other.block_;
        if (block_) {
            block_->IncSharedCnt();
        }
    }
    template <typename Y>
    SharedPtr(SharedPtr<Y>&& other) {
        ptr_ = other.ptr_;
        block_ = other.block_;
        other.block_ = nullptr;
        other.ptr_ = nullptr;
    }

    SharedPtr(SharedPtr&& other) : block_(other.block_), ptr_(other.ptr_) {
        other.block_ = nullptr;
        other.ptr_ = nullptr;
    };

    // Aliasing constructor
    // #8 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other, T* ptr) {
        ptr_ = ptr;
        block_ = other.block_;
        if (block_) {
            block_->IncSharedCnt();
        }
    }

    // Promote `WeakPtr`
    // #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    explicit SharedPtr(const WeakPtr<T>& other) {
        if (other.Expired()) {
            throw BadWeakPtr();
        }
        block_ = other.block_;
        ptr_ = other.ptr_;
        block_->IncSharedCnt();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    SharedPtr& operator=(const SharedPtr& other) {
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
    template <typename Y>
    SharedPtr& operator=(const SharedPtr<Y>& other) {
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
    }
    SharedPtr& operator=(SharedPtr&& other) {
        std::swap(ptr_, other.ptr_);
        std::swap(block_, other.block_);
        return *this;
    };
    template <typename Y>
    SharedPtr& operator=(SharedPtr<Y>&& other) {
        if (other.block_ != nullptr) {
            if (block_ != other.block_) {
                Reset();
                ptr_ = other.ptr_;
                block_ = other.block_;
                block_->IncSharedCnt();
                other.Reset();
            }
        } else {
            Reset();
        }
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~SharedPtr() {
        Reset();
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        if (block_ != nullptr) {
            block_->DecSharedCnt();
        }
        ptr_ = nullptr;
        block_ = nullptr;
    };
    void Reset(T* ptr) {
        if (block_ != nullptr) {
            block_->DecSharedCnt();
        }
        ptr_ = ptr;
        block_ = new ControlBlockPtr<T>(ptr);
    };
    template <typename Y>
    void Reset(Y* ptr) {
        if (block_ != nullptr) {
            block_->DecSharedCnt();
        }
        ptr_ = ptr;
        block_ = new ControlBlockPtr<Y>(ptr);
    };
    void Swap(SharedPtr& other) {
        std::swap(ptr_, other.ptr_);
        std::swap(block_, other.block_);
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return ptr_;
    };
    T& operator*() const {
        return *ptr_;
    };
    T* operator->() const {
        return ptr_;
    };
    size_t UseCount() const {
        if (block_) {
            return block_->GetSharedCnt();
        }
        return 0;
    };
    explicit operator bool() const {
        if (ptr_) {
            return true;
        }
        return false;
    };

    BaseControlBlock* block_;
    T* ptr_;
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right) {
    return left.ptr == right.ptr_;
}

// Allocate memory only once
template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    SharedPtr<T> new_shared;
    ControlBlockObj<T>* new_block = new ControlBlockObj<T>(std::forward<Args>(args)...);
    new_shared.block_ = new_block;
    new_shared.ptr_ = new_block->GetPtr();
    return new_shared;
};
