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

    iterator insert(const_iterator pos, T const&); // O(N) weak

    iterator erase(const_iterator pos);     // O(N) weak

    iterator erase(const_iterator first, const_iterator last); // O(N) weak

private:
    static T* safe_copy(const T*, size_t, size_t);
    static void clear_buffer(const T*, size_t, size_t);
    void new_capacity(size_t);
    T* data_;
    size_t size_;
    size_t capacity_;
};

template <typename T>
vector<T>::vector() :
    data_(nullptr),
    size_(0),
    capacity_(0) {}

template <typename T>
vector<T>::vector(vector const& other) {
    data_ = nullptr;
    if (other.capacity_ != 0) {
        data_ = safe_copy(other.data_, other.size_, other.size_);
    }
    size_ = other.size_;
    capacity_ = other.size_;
}

template <typename T>
vector<T> & vector<T>::operator=(const vector<T> &other) {
    vector<T> tmp(other);
    swap(tmp);
    return *this;
}

template <typename T>
vector<T>::~vector() {
    clear();
    operator delete(data_);
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
void vector<T>::clear_buffer(const T* data, size_t count, size_t index) {
    while (count--) {
        data[--index].~T();
    }
}

template <typename T>
T* vector<T>::safe_copy(const T* data, size_t last_size, size_t new_size) {
    size_t i;
    T* buffer = static_cast<T*>(operator new(new_size * sizeof(T)));
    try {
        for (i = 0; i < last_size; ++i) {
            new(buffer + i) T(data[i]);
        }
    } catch (...) {
        clear_buffer(buffer, i, i);
        operator delete(buffer);
        throw;
    }
    return buffer;
}

template <typename T>
void vector<T>::new_capacity(size_t n) {
    T* new_data = safe_copy(data_, size_, n);
    clear_buffer(data_, size_, size_);
    std::swap(data_, new_data);
    operator delete(new_data);
    capacity_ = n;
}

template <typename T>
void vector<T>::reserve(size_t n) {
    if (capacity_ < n) {
        new_capacity(n);
    }
}

template <typename T>
void vector<T>::shrink_to_fit() {
    if (capacity_ != size_) {
        if (size_ == 0) {
            operator delete(data_);
            data_ = nullptr;
        } else {
            new_capacity(size_);
        }
    }
    capacity_ = size_;
}

template <typename T>
void vector<T>::clear() {
    clear_buffer(data_, size_, size_);
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
typename vector<T>::iterator vector<T>::insert(const_iterator pos, const T & x) {
    size_t j = pos - data_;
    push_back(x);
    for (size_t i = size_ - 1; i != j; i--) {
        std::swap(data_[i], data_[i - 1]);
    }
    return data_ + j;
}

template <typename T>
typename vector<T>::iterator vector<T>::erase(const_iterator pos) {
    return erase(pos, pos + 1);
}

template <typename T>
typename vector<T>::iterator vector<T>::erase(const_iterator first, const_iterator last) {
#ifdef _GLIBCXX_DEBUG
        assert(data_ <= first && last <= data_ + size_);
#endif
    if (last <= first) {
        return data_;
    }
    std::ptrdiff_t cnt = last - first;
    std::ptrdiff_t j = first - data_;
    for (size_t i = j; i + cnt < size_; i++) {
        std::swap(data_[i], data_[i + cnt]);
    }
    clear_buffer(data_, cnt, size_);
    size_ -= cnt;
    return data_ + j;
}
#endif
