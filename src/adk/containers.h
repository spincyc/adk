#pragma once

#include <stddef.h>

// Containers with the standard library's shape. The AVR has no C++ standard
// library, so there is no std::array or std::vector; these stand in for them,
// with the same names for the same operations. None of them uses the heap:
// every one reserves all of its room when it is declared.

namespace adk {

    // A fixed number of things that knows its own size, like std::array.
    //
    //     adk::Array pins {22, 23, 24, 25};
    //     for (auto pin : pins) { ... }
    template <typename T, size_t N>
    struct Array
    {
        T items [N];

        constexpr size_t size  () const { return N; }
        constexpr bool   empty () const { return N == 0; }

        constexpr T&       operator[] (size_t index)       { return items[index]; }
        constexpr const T& operator[] (size_t index) const { return items[index]; }

        constexpr T&       front ()       { return items[0]; }
        constexpr const T& front () const { return items[0]; }
        constexpr T&       back  ()       { return items[N - 1]; }
        constexpr const T& back  () const { return items[N - 1]; }

        constexpr T*       begin ()       { return items; }
        constexpr const T* begin () const { return items; }
        constexpr T*       end   ()       { return items + N; }
        constexpr const T* end   () const { return items + N; }
        constexpr T*       data  ()       { return items; }
        constexpr const T* data  () const { return items; }

        constexpr void fill (const T& value)
        {
            for (auto& item : items)
            {
                item = value;
            }
        }
    };

    template <typename T, typename... More>
    Array (T, More...) -> Array<T, 1 + sizeof... (More)>;

    // A list that grows and shrinks up to a fixed capacity, like std::vector
    // but with all its room reserved up front. push_back () says whether
    // there was room.
    template <typename T, size_t Capacity>
    struct Vector
    {
        constexpr size_t size     () const { return size_; }
        constexpr size_t capacity () const { return Capacity; }
        constexpr bool   empty    () const { return size_ == 0; }
        constexpr bool   full     () const { return size_ == Capacity; }

        constexpr bool push_back (const T& item)
        {
            if (full ())
            {
                return false;
            }

            items_[size_++] = item;
            return true;
        }

        constexpr void pop_back ()
        {
            if (!empty ())
            {
                --size_;
            }
        }

        // Remove one item, keeping the rest in order.
        constexpr void erase (size_t index)
        {
            if (index >= size_)
            {
                return;
            }

            for (auto at = index; at + 1 < size_; ++at)
            {
                items_[at] = items_[at + 1];
            }

            --size_;
        }

        constexpr void clear () { size_ = 0; }

        constexpr T&       operator[] (size_t index)       { return items_[index]; }
        constexpr const T& operator[] (size_t index) const { return items_[index]; }

        constexpr T&       front ()       { return items_[0]; }
        constexpr const T& front () const { return items_[0]; }
        constexpr T&       back  ()       { return items_[size_ - 1]; }
        constexpr const T& back  () const { return items_[size_ - 1]; }

        constexpr T*       begin ()       { return items_; }
        constexpr const T* begin () const { return items_; }
        constexpr T*       end   ()       { return items_ + size_; }
        constexpr const T* end   () const { return items_ + size_; }
        constexpr T*       data  ()       { return items_; }
        constexpr const T* data  () const { return items_; }

      private:
        T      items_ [Capacity] {};
        size_t size_ = 0;
    };

    // A line of things you can add to and take from at either end, like
    // std::deque, in a fixed ring of room: a snake's body, a queue of notes.
    // push_back () and push_front () say whether there was room.
    template <typename T, size_t Capacity>
    struct Deque
    {
        constexpr size_t size     () const { return size_; }
        constexpr size_t capacity () const { return Capacity; }
        constexpr bool   empty    () const { return size_ == 0; }
        constexpr bool   full     () const { return size_ == Capacity; }

        constexpr bool push_back (const T& item)
        {
            if (full ())
            {
                return false;
            }

            items_[slot (size_++)] = item;
            return true;
        }

        constexpr bool push_front (const T& item)
        {
            if (full ())
            {
                return false;
            }

            first_ = first_ == 0 ? Capacity - 1 : first_ - 1;
            items_[first_] = item;
            ++size_;
            return true;
        }

        constexpr void pop_front ()
        {
            if (!empty ())
            {
                first_ = slot (1);
                --size_;
            }
        }

        constexpr void pop_back ()
        {
            if (!empty ())
            {
                --size_;
            }
        }

        constexpr void clear ()
        {
            first_ = 0;
            size_  = 0;
        }

        constexpr T&       operator[] (size_t index)       { return items_[slot (index)]; }
        constexpr const T& operator[] (size_t index) const { return items_[slot (index)]; }

        constexpr T&       front ()       { return (*this)[0]; }
        constexpr const T& front () const { return (*this)[0]; }
        constexpr T&       back  ()       { return (*this)[size_ - 1]; }
        constexpr const T& back  () const { return (*this)[size_ - 1]; }

        // Walks the line from front to back.
        template <typename Owner, typename Item>
        struct Cursor
        {
            Owner* line;
            size_t index;

            constexpr Item& operator* () const { return (*line)[index]; }
            constexpr bool  operator== (const Cursor&) const = default;

            constexpr Cursor& operator++ ()
            {
                ++index;
                return *this;
            }
        };

        constexpr auto begin ()       { return Cursor<Deque, T> {this, 0}; }
        constexpr auto end   ()       { return Cursor<Deque, T> {this, size_}; }
        constexpr auto begin () const { return Cursor<const Deque, const T> {this, 0}; }
        constexpr auto end   () const { return Cursor<const Deque, const T> {this, size_}; }

      private:
        constexpr size_t slot (size_t index) const { return (first_ + index) % Capacity; }

        T      items_ [Capacity] {};
        size_t first_ = 0;
        size_t size_  = 0;
    };

    // A view of things stored somewhere else, like std::span: a way to hand
    // a whole array, Array or Vector to a function without copying it.
    template <typename T>
    struct Span
    {
        constexpr Span (T* items, size_t size) : items_ (items), size_ (size) {}

        template <size_t N>
        constexpr Span (T (&items) [N]) : items_ (items), size_ (N) {}

        template <typename U, size_t N>
        constexpr Span (Array<U, N>& items) : items_ (items.data ()), size_ (N) {}

        template <typename U, size_t N>
        constexpr Span (const Array<U, N>& items) : items_ (items.data ()), size_ (N) {}

        template <typename U, size_t Capacity>
        constexpr Span (Vector<U, Capacity>& items)
            : items_ (items.data ())
            , size_  (items.size ())
        {
        }

        template <typename U, size_t Capacity>
        constexpr Span (const Vector<U, Capacity>& items)
            : items_ (items.data ())
            , size_  (items.size ())
        {
        }

        constexpr size_t size  () const { return size_; }
        constexpr bool   empty () const { return size_ == 0; }

        constexpr T& operator[] (size_t index) const { return items_[index]; }
        constexpr T& front      () const { return items_[0]; }
        constexpr T& back       () const { return items_[size_ - 1]; }
        constexpr T* begin      () const { return items_; }
        constexpr T* end        () const { return items_ + size_; }

      private:
        T*     items_;
        size_t size_;
    };

    template <typename T, size_t N>
    Span (T (&) [N]) -> Span<T>;

    template <typename T, size_t N>
    Span (Array<T, N>&) -> Span<T>;

    template <typename T, size_t N>
    Span (const Array<T, N>&) -> Span<const T>;

    template <typename T, size_t Capacity>
    Span (Vector<T, Capacity>&) -> Span<T>;

    template <typename T, size_t Capacity>
    Span (const Vector<T, Capacity>&) -> Span<const T>;

    // Whether two lists hold equal items in the same order, like
    // std::ranges::equal: any two of Array, Vector, Deque and Span.
    constexpr bool equal (const auto& list, const auto& other)
    {
        if (list.size () != other.size ())
        {
            return false;
        }

        auto item = other.begin ();

        for (const auto& mine : list)
        {
            if (!(mine == *item))
            {
                return false;
            }

            ++item;
        }

        return true;
    }
}
