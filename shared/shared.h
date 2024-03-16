#pragma once

#include "sw_fwd.h"  // Forward declaration

#include <cstddef>  // std::nullptr_t
struct BaseControlBlock {
    virtual size_t GetSharedCnt() = 0;
    virtual void IncSharedCnt() = 0;
    virtual void DecSharedCnt() = 0;
    virtual ~BaseControlBlock() = default;
};
template <typename U>
struct ControlBlockObj : public BaseControlBlock {
    size_t shared_counter;
    U object;
    template <typename... Args>
    ControlBlockObj(Args&&... args) : object(std::forward<Args>(args)...), shared_counter(1) {
    }
    size_t GetSharedCnt() override {
        return shared_counter;
    }
    void IncSharedCnt() override {
        ++shared_counter;
    }
    void DecSharedCnt() override {
        --shared_counter;
    }
    U* GetPtr() {
        return &object;
    }
};
template <typename U>
struct ControlBlockPtr : public BaseControlBlock {
    size_t counter;
    U* ptr;
    ControlBlockPtr(U* inptr) : ptr(inptr), counter(1) {
    }
    size_t GetSharedCnt() override {
        return counter;
    }
    void IncSharedCnt() override {
        ++counter;
    }
    void DecSharedCnt() override {
        --counter;
    }
    ~ControlBlockPtr() override {
        delete ptr;
        ptr = nullptr;
    }
};
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
    explicit SharedPtr(const WeakPtr<T>& other) : ptr_(nullptr), block_(nullptr){};

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
            if (UseCount() == 1) {
                delete block_;
            } else {
                block_->DecSharedCnt();
            }
            block_ = nullptr;
            ptr_ = nullptr;
        }
    };
    void Reset(T* ptr) {
        if (block_ != nullptr) {
            if (UseCount() == 1) {
                delete block_;
            } else {
                block_->DecSharedCnt();
            }
            block_ = nullptr;
            ptr_ = nullptr;
        }
        ptr_ = ptr;
        block_ = new ControlBlockPtr<T>(ptr);
    };
    template <typename Y>
    void Reset(Y* ptr) {
        if (block_ != nullptr) {
            if (UseCount() == 1) {
                delete block_;
            } else {
                block_->DecSharedCnt();
            }
            block_ = nullptr;
            ptr_ = nullptr;
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

// Look for usage examples in tests
template <typename T>
class EnableSharedFromThis {
public:
    SharedPtr<T> SharedFromThis();
    SharedPtr<const T> SharedFromThis() const;

    WeakPtr<T> WeakFromThis() noexcept {};
    WeakPtr<const T> WeakFromThis() const noexcept {};
};