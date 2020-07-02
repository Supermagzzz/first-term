#include "shared_vector.h"

#include <utility>

shared_vector::shared_vector()
    : counter(1)
    , data({0}) {}

shared_vector::shared_vector(std::vector<uint32_t> x)
    : counter(1)
    , data(std::move(x)) {}

size_t shared_vector::size() const {
    return data.size();
}

uint32_t & shared_vector::back() {
    return data.back();
}

void shared_vector::pop_back() {
    data.pop_back();
}

void shared_vector::push_back(uint32_t x) {
    data.push_back(x);
}

void shared_vector::resize(size_t x) {
    data.resize(x);
}

uint32_t const& shared_vector::operator[](size_t x) const {
    return data[x];
}

uint32_t& shared_vector::operator[](size_t x) {
    return data[x];
}
