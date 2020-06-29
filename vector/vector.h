#ifndef H_VECTOR
#define H_VECTOR
#include <cstring>
#include <cassert>
#include <utility>

template <typename T>
struct vector
{
    typedef T* iterator;
    typedef T const* const_iterator;

    vector();                               // O(1) nothrow
    vector(vector const&);                  // O(N) strong
    vector& operator=(vector const& other); // O(N) strong

    ~vector();                              // O(N) nothrow

    T& operator[](size_t i);                // O(1) nothrow
    T const& operator[](size_t i) const;    // O(1) nothrow

    T* data();                              // O(1) nothrow
    T const* data() const;                  // O(1) nothrow
    size_t size() const;                    // O(1) nothrow

    T& front();                             // O(1) nothrow
    T const& front() const;                 // O(1) nothrow

    T& back();                              // O(1) nothrow
    T const& back() const;                  // O(1) nothrow
    void push_back(T const&);               // O(1)* strong
    void pop_back();                        // O(1) nothrow

    bool empty() const;                     // O(1) nothrow

    size_t capacity() const;                // O(1) nothrow
    void reserve(size_t);                   // O(N) strong
    void shrink_to_fit();                   // O(N) strong

    void clear();                           // O(N) nothrow

    void swap(vector&);                     // O(1) nothrow

    iterator begin();                       // O(1) nothrow
    iterator end();                         // O(1) nothrow

    const_iterator begin() const;           // O(1) nothrow
    const_iterator end() const;             // O(1) nothrow

    iterator insert(iterator pos, T const&); // O(N) weak
    iterator insert(const_iterator pos, T const&); // O(N) weak

    iterator erase(iterator pos);           // O(N) weak
    iterator erase(const_iterator pos);     // O(N) weak

    iterator erase(iterator first, iterator last); // O(N) weak
    iterator erase(const_iterator first, const_iterator last); // O(N) weak

private:
    static void clear_buffer(T* &, size_t &);
    void safe_copy(size_t);
    T* buffer;
    size_t cntBuffer;

private:
    T* data_;
    size_t size_;
    size_t capacity_;
};

template <typename T>
vector<T>::vector() {
    size_ = 0;
    capacity_ = 0;
    data_ = nullptr;
    buffer = nullptr;
    cntBuffer = 0;
}

template <typename T>
vector<T>::vector(vector const& other) {
    cntBuffer = 0;
    data_ = nullptr;
    if (other.capacity_ != 0) {
        buffer = static_cast<T *>(operator new(other.size_ * sizeof(T)));
        try {
            for (size_t i = 0; i < other.size_; i++) {
                new(buffer + i) T(other.data_[i]);
                cntBuffer++;
            }
        } catch (...) {
            clear_buffer(buffer, cntBuffer);
            throw ;
        }
        std::swap(data_, buffer);
        cntBuffer = 0;
    }
    size_ = other.size_;
    capacity_ = other.size_;
    buffer = nullptr;
}

template <typename T>
vector<T> & vector<T>::operator=(const vector<T> &other) {
    vector<T> tmp(other);
    swap(tmp);
    return *this;
}

template <typename T>
vector<T>::~vector() {
    clear_buffer(buffer, cntBuffer);
    clear();
    if (data_ != nullptr) {
        operator delete(data_);
    }
}

template <typename T>
T& vector<T>::operator[](size_t i) {
#ifdef _GLIBCXX_DEBUG
    assert(i < size_);
#endif
    return data_[i];
}

template <typename T>
const T & vector<T>::operator[](size_t i) const {
#ifdef _GLIBCXX_DEBUG
    assert(i < size_);
#endif
    return data_[i];
}

template <typename T>
T* vector<T>::data() {
    return data_;
}

template <typename T>
const T * vector<T>::data() const {
    return data_;
}

template <typename T>
size_t vector<T>::size() const {
    return size_;
}

template <typename T>
T & vector<T>::front() {
#ifdef _GLIBCXX_DEBUG
    assert(size_ != 0);
#endif
    return *data_;
}

template <typename T>
const T & vector<T>::front() const {
#ifdef _GLIBCXX_DEBUG
    assert(size_ != 0);
#endif
    return *data_;
}

template <typename T>
T & vector<T>::back() {
#ifdef _GLIBCXX_DEBUG
    assert(size_ != 0);
#endif
    return data_[size_ - 1];
}

template <typename T>
const T & vector<T>::back() const {
#ifdef _GLIBCXX_DEBUG
    assert(size_ != 0);
#endif
    return data_[size_ - 1];
}

template <typename T>
void vector<T>::push_back(const T & x) {
    if (size_ != capacity_) {
        new(data_ + size_) T(x);
    } else {
        T copy(x);
        reserve(capacity_ * 2 + 1);
        new(data_ + size_) T(copy);
    }
    size_++;
}

template <typename T>
void vector<T>::pop_back() {
#ifdef _GLIBCXX_DEBUG
    assert(size_ != 0);
#endif
    data_[--size_].~T();
}

template <typename T>
bool vector<T>::empty() const {
    return size_ == 0;
}

template <typename T>
size_t vector<T>::capacity() const {
    return capacity_;
}

template <typename T>
void vector<T>::safe_copy(size_t cnt) {
    buffer = static_cast<T*>(operator new(cnt * sizeof(T)));
    for (size_t i = 0; i < size_; i++) {
        new(buffer + i) T(data_[i]);
        cntBuffer++;
    }
    std::swap(data_, buffer);
    clear_buffer(buffer, cntBuffer);
}

template <typename T>
void vector<T>::reserve(size_t n) {
    if (capacity_ < n) {
        safe_copy(n);
        capacity_ = n;
    }
}

template <typename T>
void vector<T>::shrink_to_fit() {
    if (capacity_ != size_) {
        if (size_ == 0) {
            operator delete(data_);
            data_ = nullptr;
        } else {
            safe_copy(size_);
        }
        capacity_ = size_;
    }
}

template <typename T>
void vector<T>::clear() {
    for (std::ptrdiff_t i = size_ - 1; i >= 0; i--) {
        data_[i].~T();
    }
    size_ = 0;
}

template <typename T>
void vector<T>::swap(vector<T> & other) {
    std::swap(data_, other.data_);
    std::swap(size_, other.size_);
    std::swap(capacity_, other.capacity_);
}

template <typename T>
typename vector<T>::iterator vector<T>::begin() {
    return data_;
}

template <typename T>
typename vector<T>::iterator vector<T>::end() {
    return data_ + size_;
}

template <typename T>
typename vector<T>::const_iterator vector<T>::begin() const {
    return data_;
}

template <typename T>
typename vector<T>::const_iterator vector<T>::end() const {
    return data_ + size_;
}

template <typename T>
typename vector<T>::iterator vector<T>::insert(iterator pos, const T & x) {
    return insert(static_cast<vector<T>::const_iterator>(pos), x);
}

template <typename T>
typename vector<T>::iterator vector<T>::insert(const_iterator pos, const T & x) {
    size_t j = pos - data_;
    push_back(x);
    for (size_t i = size() - 1; i != j; i--) {
        std::swap(data_[i], data_[i - 1]);
    }
    return data_ + j;
}

template <typename T>
typename vector<T>::iterator vector<T>::erase(iterator pos) {
    return erase(static_cast<vector<T>::const_iterator>(pos));
}

template <typename T>
typename vector<T>::iterator vector<T>::erase(const_iterator pos) {
#ifdef _GLIBCXX_DEBUG
    assert(pos >= data_ && pos < data_ + size);
#endif
    std::ptrdiff_t delta = pos - data_;
    for (int i = delta; i + 1 < size_; i++) {
        std::swap(data_[i], data_[i + 1]);
    }
    pop_back();
    return data_ + delta;
}

template <typename T>
typename vector<T>::iterator vector<T>::erase(iterator first, iterator last) {
    return erase(static_cast<vector<T>::const_iterator>(first),
            static_cast<vector<T>::const_iterator>(last));
}

template <typename T>
typename vector<T>::iterator vector<T>::erase(const_iterator first, const_iterator last) {
#ifdef _GLIBCXX_DEBUG
    assert(data_ <= first && last < data_ + size_);
#endif
    if (last <= first) {
        return data_;
    }
    std::ptrdiff_t cnt = last - first;
    std::ptrdiff_t j = first - data_;
    for (size_t i = j; i + cnt < size_; i++) {
        std::swap(data_[i], data_[i + cnt]);
    }
    while (cnt--) {
        pop_back();
    }
    return data_ + j;
}

template <typename T>
void vector<T>::clear_buffer(T* & buffer_, size_t & cnt) {
    while (cnt) {
        cnt--;
        buffer_[cnt].~T();
    }
    operator delete(buffer_);
    buffer_ = nullptr;
}
#endif
