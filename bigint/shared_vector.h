#ifndef BIGINT_SHARED_VECTOR_H
#define BIGINT_SHARED_VECTOR_H

#include <algorithm>
#include <cstdint>
#include <vector>

struct shared_vector {
public:
    shared_vector();
    explicit shared_vector(std::vector<uint32_t>);

    size_t size() const;
    uint32_t& back();
    void pop_back();
    void push_back(uint32_t);
    void resize(size_t);
    uint32_t const& operator[](size_t x) const;
    uint32_t& operator[](size_t x);
    friend bool operator==(shared_vector const& a, shared_vector const& b) {
        return a.data == b.data;
    }

    size_t counter;
    std::vector<uint32_t> data;
};

#endif //BIGINT_SHARED_VECTOR_H
