#pragma once

#include "compressed_pair.h"

#include <cstddef>  // std::nullptr_t

template <typename T>
struct DefaultDeleter {
    DefaultDeleter() = default;
    template <typename U>
    DefaultDeleter(const DefaultDeleter<U>&) noexcept {
    }

    void operator()(T* ptr) const noexcept {
        static_assert(sizeof(T) > 0);
        static_assert(!std::is_void<T>::value);
        delete ptr;
    }
};

template <typename T>
struct DefaultDeleter<T[]> {
    DefaultDeleter() = default;
    template <typename U>
    DefaultDeleter(const DefaultDeleter<U[]>&) noexcept {
    }

    void operator()(T* ptr) const noexcept {
        static_assert(sizeof(T) > 0);
        static_assert(!std::is_void<T>::value);
        delete[] ptr;
    }
};
template <typename T, typename My_Deleter = DefaultDeleter<T>>
class UniquePtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors
    explicit UniquePtr(T* ptr = nullptr) : ptr_and_del_(ptr, My_Deleter{}) {
    }
    template <typename other_Deleter>
    UniquePtr(T* ptr, const other_Deleter& deleter) : ptr_and_del_(ptr, deleter) {
    }
    template <typename other_Deleter>
    UniquePtr(T* ptr, other_Deleter&& deleter) : ptr_and_del_(ptr, std::move(deleter)) {
    }

    explicit UniquePtr(UniquePtr&& other) noexcept
        : ptr_and_del_(other.Release(), std::move(other.GetDeleter())) {
    }

    template <typename U, typename other_Deleter>
    UniquePtr(UniquePtr<U, other_Deleter>&& other) noexcept
        : ptr_and_del_(other.Release(), std::move(other.GetDeleter())) {
    }
    UniquePtr(const UniquePtr&) = delete;
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s
    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (ptr_and_del_.GetFirst() == other.ptr_and_del_.GetFirst()) {
            return *this;
        }
        GetDeleter() = std::move(other.GetDeleter());
        Reset(other.Release());
        return *this;
    };
    UniquePtr& operator=(std::nullptr_t) {
        Reset();
        return *this;
    };
    UniquePtr& operator=(const UniquePtr&) = delete;
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        Reset();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        auto ptr = ptr_and_del_.GetFirst();
        ptr_and_del_.GetFirst() = nullptr;
        return ptr;
    }
    void Reset(T* ptr = nullptr) {
        auto ptr_to_delete = ptr_and_del_.GetFirst();
        ptr_and_del_.GetFirst() = ptr;
        if (ptr_to_delete) {
            GetDeleter()(ptr_to_delete);
        }
    }
    void Swap(UniquePtr& other) {
        std::swap(ptr_and_del_, other.ptr_and_del_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers
    T* Get() const {
        return GetData();
    }
    My_Deleter& GetDeleter() {
        return ptr_and_del_.GetSecond();
    }
    const My_Deleter& GetDeleter() const {
        return ptr_and_del_.GetSecond();
    }
    explicit operator bool() const {
        if (Get()) {
            return true;
        }
        return false;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    std::add_lvalue_reference_t<T> operator*() const {
        return *(ptr_and_del_.GetFirst());
    }
    T* operator->() const {
        return ptr_and_del_.GetFirst();
    }

private:
    CompressedPair<T*, My_Deleter> ptr_and_del_;

    auto& GetData() {
        return ptr_and_del_.GetFirst();
    }
    T* GetData() const {
        return ptr_and_del_.GetFirst();
    }
};

// Specialization for arrays

template <typename T, typename My_Deleter>
class UniquePtr<T[], My_Deleter> {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) : ptr_and_del_(ptr, My_Deleter{}) {
    }

    template <typename other_Deleter>
    UniquePtr(T* ptr, other_Deleter&& deleter) : ptr_and_del_(ptr, std::move(deleter)) {
    }

    template <typename other_Deleter>
    UniquePtr(T* ptr, const other_Deleter& deleter) : ptr_and_del_(ptr, deleter) {
    }

    explicit UniquePtr(UniquePtr&& other) noexcept
        : ptr_and_del_(other.Release(), std::move(other.GetDeleter())) {
    }
    UniquePtr(const UniquePtr&) = delete;

    template <typename U, typename other_Deleter>
    UniquePtr(UniquePtr<U, other_Deleter>&& other) noexcept
        : ptr_and_del_(other.Release(), std::move(other.GetDeleter())) {
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s
    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (ptr_and_del_.GetFirst() == other.ptr_and_del_.GetFirst()) {
            return *this;
        }
        GetDeleter() = std::move(other.GetDeleter());
        Reset(other.Release());
        return *this;
    };
    UniquePtr& operator=(std::nullptr_t) {
        Reset();
        return *this;
    };
    UniquePtr& operator=(const UniquePtr&) = delete;
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        Reset();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        auto ptr = ptr_and_del_.GetFirst();
        ptr_and_del_.GetFirst() = nullptr;
        return ptr;
    }
    void Reset(T* ptr = nullptr) {
        auto ptr_to_delete = ptr_and_del_.GetFirst();
        ptr_and_del_.GetFirst() = ptr;
        if (ptr_to_delete) {
            GetDeleter()(ptr_to_delete);
        }
    }
    void Swap(UniquePtr& other) {
        std::swap(ptr_and_del_, other.ptr_and_del_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers
    T& operator[](size_t index) const {
        return ptr_and_del_.GetFirst()[index];
    }
    T* Get() const {
        return ptr_and_del_.GetFirst();
    }
    My_Deleter& GetDeleter() {
        return ptr_and_del_.GetSecond();
    }
    const My_Deleter& GetDeleter() const {
        return ptr_and_del_.GetSecond();
    }
    explicit operator bool() const {
        if (ptr_and_del_.GetFirst()) {
            return true;
        }
        return false;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    std::add_lvalue_reference_t<T> operator*() const {
        return *(ptr_and_del_.GetFirst());
    }
    T* operator->() const {
        return ptr_and_del_.GetFirst();
    }

private:
    CompressedPair<T*, My_Deleter> ptr_and_del_;
};
