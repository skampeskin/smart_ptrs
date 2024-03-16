#pragma once
#include <utility>
#include <type_traits>
#include <cstdint>

template <typename T, int first_second, bool is_empty = std::is_empty_v<T> && !std::is_final_v<T>>
struct BaseOptimisationItem {
    BaseOptimisationItem() = default;

    BaseOptimisationItem(T&& data) : data_(std::move(data)) {
    }
    BaseOptimisationItem(const T& data) : data_(data) {
    }
    T& Get() {
        return data_;
    }
    const T& Get() const {
        return data_;
    }

    T data_ = {};
};

template <typename T, int first_second>
struct BaseOptimisationItem<T, first_second, true> : private T {
    BaseOptimisationItem() = default;

    BaseOptimisationItem(T&&) {
    }
    BaseOptimisationItem(const T&) {
    }
    T& Get() {
        return *this;
    }
    const T& Get() const {
        return *this;
    }
};

template <typename F, typename S>
class CompressedPair : private BaseOptimisationItem<F, 1>, private BaseOptimisationItem<S, 2> {
public:
    CompressedPair() = default;

    template <typename First, typename Second>
    CompressedPair(First&& first, Second&& second)
        : BaseOptimisationItem<F, 1>(std::forward<First>(first)),
          BaseOptimisationItem<S, 2>(std::forward<Second>(second)) {
    }

    F& GetFirst() {
        return BaseOptimisationItem<F, 1>::Get();
    }
    S& GetSecond() {
        return BaseOptimisationItem<S, 2>::Get();
    }
    const F& GetFirst() const {
        return BaseOptimisationItem<F, 1>::Get();
    }
    const S& GetSecond() const {
        return BaseOptimisationItem<S, 2>::Get();
    }
};
// Paste here your implementation of compressed_pair from seminar 2 to use in UniquePtr
