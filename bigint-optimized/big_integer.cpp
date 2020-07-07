#include "big_integer.h"

#include <cstring>
#include <stdexcept>
#include <algorithm>
#include <cstdint>
#include <utility>
#include <functional>

static const uint32_t SHIFT = 32;
static const uint32_t HALF_SHIFT = SHIFT / 2;
static const uint32_t HALF_BITS = (1ULL << 16ULL) - 1;

big_integer::big_integer()
    : num({0})
    , sign(false) {}

big_integer::big_integer(big_integer const& other)
    : num(other.num)
    , sign(other.sign) {}

big_integer& big_integer::operator=(big_integer const& other) = default;

big_integer::~big_integer() = default;

big_integer::big_integer(int a)
    : num({static_cast<uint32_t>(std::abs(1ll * a))})
    , sign(a < 0) {
    normalize();
}

big_integer::big_integer(uint32_t a)
    : num({a})
    , sign(false) {
    normalize();
}

int addInt(uint32_t &a, uint32_t b) {
    uint32_t c = a;
    a += b;
    return b != 0 && a <= c;
}

int subInt(uint32_t &a, uint32_t b) {
    uint32_t c = a;
    a -= b;
    return b != 0 && a >= c;
}

void big_integer::normalize() {
    while (num.size() > 1 && num.back() == 0) {
        num.pop_back();
    }
    if (num.back() == 0) {
        sign = false;
    }
}

std::pair<uint32_t, uint32_t> mul(uint32_t n, uint32_t m) {
    uint32_t a = n >> HALF_SHIFT, b = n & HALF_BITS,
            c = m >> HALF_SHIFT, d = m & HALF_BITS;
    uint32_t top = a * c, bot = b * d, bc = b * c, ad = a * d;
    a = (bc >> HALF_SHIFT);
    b = (bc & HALF_BITS) << HALF_SHIFT;
    c = (ad >> HALF_SHIFT);
    d = (ad & HALF_BITS) << HALF_SHIFT;
    top += a + c;
    if (bot + b < bot) {
        top++;
    }
    bot += b;
    if (bot + d < bot) {
        top++;
    }
    bot += d;
    return std::make_pair(top, bot);
}

big_integer::big_integer(std::string const& str) {
    big_integer res = 0;
    for (size_t i = str[0] == '-'; i < str.size(); i++) {
        res *= 10;
        res += (int)(str[i] - '0');
    }
    res.sign = str[0] == '-';
    *this = res;
}

big_integer& big_integer::operator+=(big_integer const& rhs) {
    if (sign == rhs.sign) {
        int carry = 0;
        for (size_t i = 0; i < rhs.num.size() || carry; i++) {
            if (i == num.size()) {
                num.push_back(0);
            }
            int newCarry = addInt(num[i], (i < rhs.num.size() ? rhs.num[i] : 0));
            newCarry += addInt(num[i], carry);
            carry = newCarry;
        }
    } else {
        *this -= -rhs;
    }
    normalize();
    return *this;
}

big_integer& big_integer::operator-=(big_integer const& rhs) {
    if (rhs == 0) {
        return *this;
    }
    if (sign == rhs.sign) {
        if (*this == rhs) {
            return *this = big_integer(0);
        }
        if ((*this < rhs) != sign) {
            big_integer tmp = rhs;
            tmp -= *this;
            return *this = -tmp;
        }
        int carry = 0;
        for (size_t i = 0; i < rhs.num.size() || carry; i++) {
            int newCarry = subInt(num[i], (i < rhs.num.size() ? rhs.num[i] : 0));
            newCarry += subInt(num[i], carry);
            carry = newCarry;
        }
    } else {
        *this += -rhs;
    }
    normalize();
    return *this;
}

big_integer& big_integer::operator*=(big_integer const& rhs) {
    big_integer res = 0;
    res.num.resize(num.size() + rhs.num.size() + 1);
    res.sign = sign != rhs.sign;
    for (size_t i = 0; i < num.size(); i++) {
        uint32_t carry = 0;
        for (size_t j = 0; j < rhs.num.size() || carry; j++) {
            if (j == rhs.num.size()) {
                res.num[i + j] = carry;
                break;
            }
            std::pair<uint32_t, uint32_t> p = mul(num[i], rhs.num[j]);
            p.first += addInt(p.second, carry);
            carry = p.first;
            carry += addInt(res.num[i + j], p.second);
        }
    }
    *this = res;
    normalize();
    return *this;
}

uint32_t trial(uint64_t a, uint64_t b, uint64_t d) {
    uint64_t x = (a << SHIFT) + b;
    uint64_t y = d;
    return x / y;
}

bool smaller(big_integer &a, big_integer &b, size_t k, size_t m) {
    for (std::ptrdiff_t i = k - 1; i >= 0; i--) {
        if (a.num[i] != (m - k + i < b.num.size() ? b.num[m - k + i] : 0)) {
            return a.num[i] < (m - k + i < b.num.size() ? b.num[m - k + i] : 0);
        }
    }
    return false;
}

void difference(big_integer &a, big_integer &b, size_t k, size_t m) {
    bool carry = false;
    for (size_t i = 0; i < m; ++i) {
        int64_t c = (int64_t) a.num[k - m + i] -
                    (i < b.num.size() ? b.num[i] : 0) -
                    carry;
        carry = c < 0;
        a.num[k - m + i] = c;
    }
    if (!a.num.back()) {
        a.num.pop_back();
    }
}


big_integer div_short(big_integer a, uint32_t b) {
    big_integer res;
    res.num.resize(a.num.size());
    uint64_t ost = 0;
    for (size_t i = a.num.size(); i > 0; i--) {
        ost <<= SHIFT;
        ost += a.num[i - 1];
        uint32_t cur = ost / b;
        ost -= cur * b;
        res.num[i - 1] = cur;
    }
    res.normalize();
    return res;
}

big_integer& big_integer::operator/=(big_integer const& rhs) {
    if (rhs == 0) {
        throw std::invalid_argument("division by zero");
    }
    big_integer a = *this;
    big_integer b = rhs;
    bool new_sign = a.sign ^ b.sign;
    a.sign = b.sign = false;
    if (a < b) {
        return *this = 0;
    }
    if (b.num.size() == 1) {
        a = div_short(a, b.num.back());
        a.sign = new_sign;
        return *this = a;
    }
    uint32_t f = (1ULL << SHIFT) / (b.num.back() + 1);
    a *= f;
    b *= f;
    big_integer res;
    size_t n = a.num.size() + 1, m = b.num.size() + 1;
    a.num.push_back(0);
    res.num.resize(n - m + 1);
    size_t j = res.num.size() - 1;
    for (size_t i = m; i <= n; i++) {
        uint32_t cur = trial(
                a.num.back(),
                a.num.size() >= 2 ? a.num[a.num.size() - 2] : 0U,
                b.num.back());
        big_integer t = b * cur;
        while (smaller(a, t, a.num.size(), m)) {
            cur--;
            t -= b;
        }
        res.num[j--] = cur;
        difference(a, t, a.num.size(), m);
    }
    res.normalize();
    res.sign = new_sign;
    return *this = res;
}

big_integer& big_integer::operator%=(big_integer const& rhs) {
    big_integer res = *this - (*this / rhs) * rhs;
    res.normalize();
    return *this = res;
}

big_integer bitwise_operations(big_integer a, big_integer b, std::function<uint32_t(uint32_t, uint32_t)> f) {
    size_t len = std::max(a.num.size(), b.num.size()) + 1;
    if (a < 0) {
        a = a + 1;
        a.num.resize(len);
        for (size_t i = 0; i < a.num.size(); i++) {
            a.num[i] = ~a.num[i];
        }
    } else {
        a.num.resize(len);
    }
    if (b < 0) {
        b = b + 1;
        b.num.resize(len);
        for (size_t i = 0; i < b.num.size(); i++) {
            b.num[i] = ~b.num[i];
        }
    } else {
        b.num.resize(len);
    }
    for (size_t i = 0; i < len; i++) {
        a.num[i] = f(a.num[i], b.num[i]);
    }
    a.sign = f(a.sign, b.sign);
    if (a.sign) {
        for (size_t i = 0; i < a.num.size(); i++) {
            a.num[i] = ~a.num[i];
        }
        a = a - 1;
    }
    a.normalize();
    return a;
}

big_integer& big_integer::operator&=(const big_integer& rhs) {
    return *this = bitwise_operations(*this, rhs, [](uint32_t a, uint32_t b){
        return a & b;
    });
}

big_integer& big_integer::operator|=(const big_integer& rhs) {
    return *this = bitwise_operations(*this, rhs, [](uint32_t a, uint32_t b){
        return a | b;
    });
}

big_integer& big_integer::operator^=(const big_integer& rhs) {
    return *this = bitwise_operations(*this, rhs, [](uint32_t a, uint32_t b){
        return a ^ b;
    });
}

big_integer& big_integer::operator<<=(int rhs) {
    if (rhs < 0) {
        throw std::invalid_argument("negative shift");
    }
    big_integer res = *this;
    res.sign = sign;
    sign = false;
    for (size_t i = 0; i <= rhs / (SHIFT - 1) + 1; i++) {
        res.num.push_back(0);
    }
    for (size_t i = 0; i < res.num.size(); i++) {
        ptrdiff_t left = i * SHIFT - rhs;
        ptrdiff_t right = (i + 1) * SHIFT - 1 - rhs;
        ptrdiff_t numLeft = left / SHIFT;
        ptrdiff_t numRight = right / SHIFT;
        uint32_t cnt = (SHIFT - rhs % SHIFT) % SHIFT;
        uint32_t a, b;
        if (left < 0 || numLeft >= (ptrdiff_t)num.size()) {
            a = 0;
        } else {
            a = (sign ? ~num[numLeft] + 1 : num[numLeft]);
        }
        if (right < 0 || numRight >= (ptrdiff_t)num.size()) {
            b = 0;
        } else {
            b = (sign ? ~num[numRight] + 1 : num[numRight]);
        }
        a >>= cnt;
        b &= (1U << cnt) - 1U;
        res.num[i] = (cnt ? (b << (SHIFT - cnt)) : 0) + a;
        if (sign) {
            res.num[i] = ~res.num[i] + 1;
        }
    }
    res.normalize();
    return *this = res;
}

big_integer& big_integer::operator>>=(int rhs) {
    if (rhs < 0) {
        throw std::invalid_argument("negative shift");
    }
    big_integer res = *this;
    res.sign = sign;
    sign = false;
    for (size_t i = 0; i < res.num.size(); i++) {
        ptrdiff_t left = i * SHIFT + rhs;
        ptrdiff_t right = (i + 1) * SHIFT - 1 + rhs;
        ptrdiff_t numLeft = left / SHIFT;
        ptrdiff_t numRight = right / SHIFT;
        uint32_t cnt = rhs % SHIFT;
        uint32_t a, b;
        if (numLeft >= (ptrdiff_t) num.size()) {
            a = sign * UINT32_MAX;
        } else {
            a = (sign ? ~num[numLeft] + 1 : num[numLeft]);
        }
        if (numRight >= (ptrdiff_t)num.size()) {
            b = sign * UINT32_MAX;
        } else {
            b = (sign ? ~num[numRight] + 1 : num[numRight]);
        }
        a >>= cnt;
        b &= (1U << cnt) - 1U;
        res.num[i] = (b << (SHIFT - cnt)) + a;
        if (sign) {
            res.num[i] = ~res.num[i] + 1;
        }
    }
    res.normalize();
    if (res.sign) {
        res -= 1;
    }
    return *this = res;
}

big_integer big_integer::operator+() const {
    return *this;
}

big_integer big_integer::operator-() const {
    big_integer res = *this;
    res.sign = !res.sign;
    res.normalize();
    return res;
}

big_integer big_integer::operator~() const {
    return -*this - 1;
}

big_integer& big_integer::operator++() {
    return *this += 1;
}

big_integer big_integer::operator++(int) {
    big_integer r = *this;
    ++*this;
    return r;
}

big_integer& big_integer::operator--() {
    return *this -= 1;
}

big_integer big_integer::operator--(int) {
    big_integer r = *this;
    --*this;
    return r;
}

big_integer operator+(big_integer a, big_integer const& b) {
    return a += b;
}

big_integer operator-(big_integer a, big_integer const& b) {
    return a -= b;
}

big_integer operator*(big_integer a, big_integer const& b) {
    return a *= b;
}

big_integer operator/(big_integer a, big_integer const& b) {
    return a /= b;
}

big_integer operator%(big_integer a, big_integer const& b) {
    return a %= b;
}

big_integer operator&(big_integer a, big_integer const& b) {
    return a &= b;
}

big_integer operator|(big_integer a, big_integer const& b) {
    return a |= b;
}

big_integer operator^(big_integer a, big_integer const& b) {
    return a ^= b;
}

big_integer operator<<(big_integer a, int b) {
    return a <<= b;
}

big_integer operator>>(big_integer a, int b) {
    return a >>= b;
}

bool operator==(big_integer const& a, big_integer const& b) {
    return a.sign == b.sign && a.num == b.num;
}

bool operator!=(big_integer const& a, big_integer const& b) {
    return !(a == b);
}

bool operator<(big_integer const& a, big_integer const& b){
    if (a == b) {
        return false;
    }
    if (a.sign != b.sign) {
        return a.sign > b.sign;
    } else {
        bool result = false;
        if (a.num.size() != b.num.size()) {
            result = a.num.size() < b.num.size();
        } else {
            for (int i = (int)a.num.size() - 1; i >= 0; i--) {
                if (a.num[i] != b.num[i]) {
                    result = a.num[i] < b.num[i];
                    break;
                }
            }
        }
        return result ^ a.sign;
    }
}

bool operator>(big_integer const& a, big_integer const& b) {
    return !(a <= b);
}

bool operator<=(big_integer const& a, big_integer const& b) {
    return a < b || a == b;
}

bool operator>=(big_integer const& a, big_integer const& b) {
    return !(a < b);
}

std::string to_string(big_integer const& a) {
    std::string s;
    big_integer x = a;
    x.sign = false;
    while (x != 0) {
        s += (char)((x % 10).num.back() + '0');
        x /= 10;
    }
    std::reverse(s.begin(), s.end());
    if (s.length() == 0) {
        s = "0";
    } else {
        s = (a.sign ? "-" : "") + s;
    }
    return s;
}

std::ostream& operator<<(std::ostream& s, big_integer const& a) {
    return s << to_string(a);
}
