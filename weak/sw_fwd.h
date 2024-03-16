#pragma once

#include <exception>

class BadWeakPtr : public std::exception {};

template <typename T>
class SharedPtr;

template <typename T>
class WeakPtr;

struct BaseControlBlock {
    virtual size_t GetSharedCnt() = 0;
    virtual void IncSharedCnt() = 0;
    virtual void DecSharedCnt() = 0;
    virtual size_t GetWeakCnt() = 0;
    virtual void IncWeakCnt() = 0;
    virtual void DecWeakCnt() = 0;
    virtual ~BaseControlBlock() = default;
};
template <typename U>
struct ControlBlockObj : public BaseControlBlock {
    size_t shared_counter;
    size_t weak_counter = 0;
    std::aligned_storage_t<sizeof(U), alignof(U)> object;
    template <typename... Args>
    ControlBlockObj(Args&&... args) : shared_counter(1) {
        new (static_cast<void*>(&object)) U(std::forward<Args>(args)...);
    }
    size_t GetSharedCnt() override {
        return shared_counter;
    }
    void IncSharedCnt() override {
        ++shared_counter;
    }
    void DecSharedCnt() override {
        --shared_counter;
        if (!shared_counter) {
            reinterpret_cast<U*>(&object)->~U();
            if (!weak_counter) {
                delete this;
            }
        }
    }
    U* GetPtr() {
        return reinterpret_cast<U*>(&object);
    }
    size_t GetWeakCnt() override {
        return weak_counter;
    }
    void IncWeakCnt() override {
        ++weak_counter;
    }
    void DecWeakCnt() override {
        --weak_counter;
    }
};
template <typename U>
struct ControlBlockPtr : public BaseControlBlock {
    size_t shared_counter;
    size_t weak_counter = 0;
    U* ptr;
    bool is_deleted = false;
    ControlBlockPtr(U* inptr) : ptr(inptr), shared_counter(1) {
    }
    size_t GetSharedCnt() override {
        return shared_counter;
    }
    void IncSharedCnt() override {
        ++shared_counter;
    }
    void DecSharedCnt() override {
        --shared_counter;
        if (!shared_counter) {
            if (!weak_counter) {
                delete this;
            } else {
                delete ptr;
                ptr = nullptr;
            }
        }
    }
    size_t GetWeakCnt() override {
        return weak_counter;
    }
    void IncWeakCnt() override {
        ++weak_counter;
    }
    void DecWeakCnt() override {
        --weak_counter;
    }
    ~ControlBlockPtr() override {
        if (ptr) {
            delete ptr;
            ptr = nullptr;
        }
    }
};