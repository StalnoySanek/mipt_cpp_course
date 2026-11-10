#ifndef NANO_EDR_RING_BUFFER_H
#define NANO_EDR_RING_BUFFER_H

#include <array>
#include <cstddef>

#include "event.h"

namespace nano_edr
{

template <typename T, size_t N>
class RingBuffer
{
    static_assert(N > 0);

 public:
    class Iterator
    {
     public:
        Iterator(const RingBuffer* ring, std::size_t index)
            : ring_(ring), index_(index) {}

        const T& operator*() const { return (*ring_)[index_]; }
        Iterator& operator++()
        {
            ++index_;
            return *this;
        }
        bool operator!=(const Iterator& other) const
        {
            return ring_ != other.ring_ || index_ != other.index_;
        }

     private:
        const RingBuffer* ring_ = nullptr;
        size_t index_ = 0;
    };

    void PushBack(const T& value)
    {
        array_[(head_ + size_) % N] = value;
        if (full())
            head_ = (head_ + 1) % N;
        else
            ++size_;
    }

    void Clear()
    {
        for (size_t i = 0; i < size_; ++i)
        {
            T empty{};
            std::swap(array_[i], empty);
        }
        head_ = 0;
        size_ = 0;
    }

    std::size_t size() const { return size_; }
    static constexpr std::size_t capacity() { return N; }
    bool empty() const { return size_ == 0; }
    bool full() const { return size_ == N; }

    const T& operator[](std::size_t index) const { return array_[(head_ + index) % N]; }

    const T& back() const { return (*this)[size_ - 1]; }

    Iterator begin() const { return Iterator(this, 0); }
    Iterator end() const { return Iterator(this, size_); }

 private:
    std::array<T, N> array_;
    size_t head_ = 0;
    size_t size_ = 0;
};

}  // namespace nano_edr

#endif  // NANO_EDR_RING_BUFFER_H
