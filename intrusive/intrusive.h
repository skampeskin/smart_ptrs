#pragma once

#include <cstddef>  // for std::nullptr_t
#include <utility>  // for std::exchange / std::swap

class SimpleCounter {
public:
    size_t IncRef() {
        return ++count_;
    };
    size_t DecRef() {
        return --count_;
    };
    size_t RefCount() const {
        return count_;
    };

private:
    size_t count_ = 0;
};

struct DefaultDelete {
    template <typename T>
    static void Destroy(T* object) {
        delete object;
    }
};

template <typename Derived, typename Counter, typename Deleter>
class RefCounted {
public:
    // Increase reference counter.
    void IncRef() {
        counter_.IncRef();
    };
    RefCounted& operator=(const RefCounted& other) {
        return *this;
    };
    // Decrease reference counter.
    // Destroy object using Deleter when the last instance dies.
    void DecRef() {
        if (!counter_.DecRef()) {
            Deleter::Destroy(static_cast<Derived*>(this));
        }
    };

    // Get current counter value (the number of strong references).
    size_t RefCount() const {
        return counter_.RefCount();
    };

private:
    Counter counter_;
};

template <typename Derived, typename D = DefaultDelete>
using SimpleRefCounted = RefCounted<Derived, SimpleCounter, D>;

template <typename T>
class IntrusivePtr {
    template <typename Y>
    friend class IntrusivePtr;
    T* counted_;

public:
    // Constructors
    IntrusivePtr() : counted_(nullptr){};
    IntrusivePtr(std::nullptr_t) : counted_(nullptr){};
    IntrusivePtr(T* ptr) {
        counted_ = ptr;
        counted_->IncRef();
    }

    template <typename Y>
    IntrusivePtr(const IntrusivePtr<Y>& other) {
        counted_ = other.counted_;
        if (counted_) {
            counted_->IncRef();
        }
    };

    template <typename Y>
    IntrusivePtr(IntrusivePtr<Y>&& other) {
        counted_ = other.counted_;
        other.counted_ = nullptr;
    };

    IntrusivePtr(const IntrusivePtr& other) {
        counted_ = other.counted_;
        if (counted_) {
            counted_->IncRef();
        }
    };
    IntrusivePtr(IntrusivePtr&& other) {
        counted_ = other.counted_;
        other.counted_ = nullptr;
    };

    // `operator=`-s
    IntrusivePtr& operator=(const IntrusivePtr& other) {
        if (counted_ != other.counted_) {
            Reset();
            counted_ = other.counted_;
            if (counted_) {
                counted_->IncRef();
            }
        }
        return *this;
    };
    IntrusivePtr& operator=(IntrusivePtr&& other) {
        if (counted_ != other.counted_) {
            Reset();
            counted_ = other.counted_;
            other.counted_ = nullptr;
        }
        return *this;
    };

    // Destructor
    ~IntrusivePtr() {
        Reset();
    };

    // Modifiers
    void Reset() {
        if (counted_) {
            counted_->DecRef();
            counted_ = nullptr;
        }
    };
    void Reset(T* ptr) {
        if (counted_) {
            counted_->DecRef();
            counted_ = ptr;
            counted_->IncRef();
        }
    };
    void Swap(IntrusivePtr& other) {
        std::swap(counted_, other.counted_);
    };

    // Observers
    T* Get() const {
        return counted_;
    };
    T& operator*() const {
        return *counted_;
    };
    T* operator->() const {
        return counted_;
    };
    size_t UseCount() const {
        if (!counted_) {
            return 0;
        }
        return counted_->RefCount();
    };
    explicit operator bool() const {
        if (counted_) {
            return true;
        }
        return false;
    };
};

template <typename T, typename... Args>
IntrusivePtr<T> MakeIntrusive(Args&&... args) {
    auto counted = new T(std::forward<Args>(args)...);
    return IntrusivePtr<T>(counted);
};
