#ifndef BIGINT_SHARED_VECTOR_SMALL_OBJECT_H
#define BIGINT_SHARED_VECTOR_SMALL_OBJECT_H

#include "shared_vector.h"

class shared_vector_small_object {
private:
    static constexpr size_t SIZE = sizeof(shared_vector*) / sizeof(uint32_t);
    bool is_small;
    size_t small_size;
    union {
        uint32_t small[SIZE]{};
        shared_vector *num;
    };
    void delete_num();
    void check_counter();
    void to_big();

public:
    shared_vector_small_object();
    explicit shared_vector_small_object(std::vector<uint32_t>);
    shared_vector_small_object(shared_vector_small_object const&);
    ~shared_vector_small_object();

    size_t size() const;
    uint32_t& back();
    void pop_back();
    void push_back(uint32_t);
    void resize(size_t);
    uint32_t const& operator[](size_t x) const;
    uint32_t& operator[](size_t x);
    friend bool operator==(shared_vector_small_object const &a,
            shared_vector_small_object const &b);
    shared_vector_small_object& operator=(shared_vector_small_object const& other);
};


#endif //BIGINT_SHARED_VECTOR_SMALL_OBJECT_H
