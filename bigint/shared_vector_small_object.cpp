#include "shared_vector_small_object.h"

#include <utility>
#include <algorithm>
#include <vector>

shared_vector_small_object::shared_vector_small_object()
    : is_small(true)
    , small_size(1) {
    std::fill(small, small + SIZE, 0);
}

shared_vector_small_object::shared_vector_small_object(std::vector<uint32_t> x){
    if (x.size() <= 2) {
        is_small = true;
        small_size = x.size();
        std::fill(small, small + SIZE, 0);
        copy_n(x.begin(), x.size(), small);
    } else {
        is_small = false;
        small_size = 0;
        num = new shared_vector(x);
    }
}

shared_vector_small_object::shared_vector_small_object(shared_vector_small_object const& other) {
    is_small = other.is_small;
    if (other.is_small) {
        std::fill(small, small + SIZE, 0);
        std::copy_n(other.small, SIZE, small);
        small_size = other.small_size;
    } else {
        num = other.num;
        num->counter++;
        small_size = 0;
    }
}

shared_vector_small_object::~shared_vector_small_object() {
    if (!is_small) {
        num->counter--;
        if (num->counter == 0) {
            delete num;
        }
    }
}

size_t shared_vector_small_object::size() const {
    if (is_small) {
        return small_size;
    } else {
        return num->size();
    }
}

uint32_t & shared_vector_small_object::back() {
    if (is_small) {
        return small[small_size - 1];
    } else {
        check_counter();
        return num->back();
    }
}

void shared_vector_small_object::pop_back() {
    if (is_small) {
        small_size--;
    } else {
        check_counter();
        num->pop_back();
    }
}

void shared_vector_small_object::push_back(uint32_t x) {
    if (is_small) {
        if (small_size != SIZE) {
            small[small_size++] = x;
        } else {
            to_big();
            num->push_back(x);
        }
    } else {
        check_counter();
        num->push_back(x);
    }
}

void shared_vector_small_object::resize(size_t x) {
    if (x <= SIZE) {
        to_small();
        small_size = x;
    } else {
        to_big();
        check_counter();
        num->resize(x);
    }
}

uint32_t const& shared_vector_small_object::operator[](size_t x) const {
    if (is_small) {
        return small[x];
    } else {
        return (*num)[x];
    }
}

uint32_t& shared_vector_small_object::operator[](size_t x) {
    if (is_small) {
        return small[x];
    } else {
        check_counter();
        return (*num)[x];
    }
}

std::vector<uint32_t> shared_vector_small_object::get_vector() const {
    if (is_small) {
        std::vector<uint32_t> data(small_size);
        copy_n(small, small_size, data.begin());
        return data;
    } else {
        return num->data;
    }
}

bool shared_vector_small_object::operator == (shared_vector_small_object const &b) const {
    return get_vector() == b.get_vector();
}

bool operator == (shared_vector_small_object const &a, shared_vector_small_object const &b) {
    return a.operator==(b);
}

void shared_vector_small_object::check_counter() {
    if (num->counter > 1) {
        num->counter--;
        num = new shared_vector(num->data);
    }
}

shared_vector_small_object& shared_vector_small_object::operator=
        (shared_vector_small_object const& other) {
    if (this != &other) {
        if (!is_small) {
            delete_num();
        }
        is_small = other.is_small;
        small_size = other.small_size;
        if (is_small) {
            std::copy_n(other.small, SIZE, small);
        } else {
            num = new shared_vector(other.num->data);
        }
    }
    return *this;
}

void shared_vector_small_object::to_big() {
    if (is_small) {
        std::vector<uint32_t> new_num(small_size);
        copy_n(small, small_size, new_num.begin());
        num = new shared_vector(new_num);
        is_small = false;
        small_size = 0;
    }
}

void shared_vector_small_object::to_small() {
    if (!is_small && size() <= SIZE) {
        std::vector<uint32_t> data = num->data;
        size_t size = num->size();
        delete_num();
        std::fill(small, small + SIZE, 0);
        std::copy_n(data.begin(), size, small);
        small_size = size;
        is_small = true;
    }
}

void shared_vector_small_object::delete_num() {
    if (!is_small) {
        check_counter();
        delete num;
    }
}