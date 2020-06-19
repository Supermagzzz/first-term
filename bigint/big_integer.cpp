#include "big_integer.h"

#include <cstring>
#include <stdexcept>
#include <algorithm>

big_integer::big_integer() : num({0}), sign(false) {}
big_integer::big_integer(big_integer const& other) : num(other.num), sign(other.sign) {}
big_integer& big_integer::operator=(big_integer const& other) = default;
big_integer::~big_integer() = default;
big_integer::big_integer(int a) : num({static_cast<unsigned int>(std::abs(a))}), sign(a < 0) {}
big_integer::big_integer(unsigned int a) : num({a}), sign(false) {}

int addInt(unsigned int &a, unsigned int b) {
    unsigned int c = a;
    a += b;
    return b != 0 && a <= c;
}

int subInt(unsigned int &a, unsigned int b) {
    unsigned int c = a;
    a -= b;
    return b != 0 && a >= c;
}

void big_integer::normalize() {
    while ((int)num.size() > 1 && num.back() == 0) {
        num.pop_back();
    }
    if (num.back() == 0) {
        sign = false;
    }
}

std::pair<unsigned int, unsigned int> mul(unsigned int n, unsigned int m) {
    unsigned int a = n >> 16U, b = n & ((1U << 16U) - 1U),
            c = m >> 16U, d = m & ((1U << 16U) - 1U);
    unsigned int top = a * c, bot = b * d, bc = b * c, ad = a * d;
    a = (bc >> 16U);
    b = (bc & ((1U << 16U) - 1U)) << 16U;
    c = (ad >> 16U);
    d = (ad & ((1U << 16U) - 1U)) << 16U;
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
    big_integer res;
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
    big_integer res;
    res.num.resize(num.size() + rhs.num.size() + 1);
    res.sign = sign != rhs.sign;
    for (size_t i = 0; i < num.size(); i++) {
        unsigned int carry = 0;
        for (size_t j = 0; j < rhs.num.size() || carry; j++) {
            if (j == rhs.num.size()) {
                res.num[i + j] = carry;
                break;
            }
            std::pair<unsigned int, unsigned int> p = mul(num[i], rhs.num[j]);
            p.first += addInt(p.second, carry);
            carry = p.first;
            carry += addInt(res.num[i + j], p.second);
        }
    }
    *this = res;
    normalize();
    return *this;
}

big_integer& big_integer::operator/=(big_integer const& rhs) {
    if (rhs == 0) {
        throw std::invalid_argument("division by zero");
    }
    big_integer a = *this;
    big_integer b = rhs;
    a.sign = b.sign = false;
    big_integer result, current;
    result.num.resize(num.size());
    for (int i = (int)num.size() - 1; i >= 0; i--) {
        current <<= 32;
        current += a.num[i];
        unsigned int x = 0, l = 0, r = UINT32_MAX;
        while (l <= r) {
            unsigned int m = l / 2 + r / 2 + (r % 2 + l % 2) / 2;
            big_integer tmp = b * m;
            if (tmp <= current) {
                x = m;
                l = m + 1;
            } else {
                r = m - 1;
            }
        }
        result <<= 32;
        result += x;
        current = current - b * x;
    }
    result.sign = sign != rhs.sign;
    result.normalize();
    return *this = result;
}

big_integer& big_integer::operator%=(big_integer const& rhs) {
    big_integer res = *this - (*this / rhs) * rhs;
    res.normalize();
    return *this = res;
}

big_integer& big_integer::operator&=(big_integer const& rhs) {
    for (size_t i = 0; i < num.size(); i++) {
        num[i] = (sign ? -1 : 1) * num[i] & (i < rhs.num.size() ? (rhs.sign ? -1 : 1) * rhs.num[i] : 0);
    }
    normalize();
    return *this;
}

big_integer& big_integer::operator|=(big_integer const& rhs) {
    for (size_t i = 0; i < std::max(rhs.num.size(), num.size()); i++) {
        unsigned int a = (i < num.size() ? (sign ? ~num[i] + 1 : num[i]): 0);
        unsigned int b = (i < rhs.num.size() ? (rhs.sign ? ~rhs.num[i] + 1 : rhs.num[i]): 0);
        unsigned int c = a | b;
        if (sign != rhs.sign) {
            c = ~c + 1;
        }
        if (i == num.size()) {
            num.push_back(c);
        } else {
            num[i] = c;
        }
    }
    sign = sign != rhs.sign;
    normalize();
    return *this;
}

big_integer& big_integer::operator^=(big_integer const& rhs) {
    for (size_t i = 0; i < std::max(rhs.num.size(), num.size()); i++) {
        unsigned int a = (i < num.size() ? (sign ? ~num[i] + 1 : num[i]): 0);
        unsigned int b = (i < rhs.num.size() ? (rhs.sign ? ~rhs.num[i] + 1 : rhs.num[i]): 0);
        unsigned int c = a ^ b;
        if (sign != rhs.sign) {
            c = ~c + 1;
        }
        if (i == num.size()) {
            num.push_back(c);
        } else {
            num[i] = c;
        }
    }
    sign = sign != rhs.sign;
    normalize();
    return *this;
}

big_integer& big_integer::operator<<=(int rhs) {
    if (rhs < 0) {
        throw std::invalid_argument("negative shift");
    }
    big_integer res = *this;
    res.sign = sign;
    sign = false;
    for (int i = 0; i <= rhs / 31 + 1; i++) {
        res.num.push_back(0);
    }
    for (int i = 0; i < (int)res.num.size(); i++) {
        int left = i * 32 - rhs;
        int right = (i + 1) * 32 - 1 - rhs;
        int numLeft = left / 32;
        int numRight = right / 32;
        unsigned int cnt = (32 - rhs % 32) % 32;
        unsigned int a, b;
        if (left < 0 || numLeft >= (int)num.size()) {
            a = 0;
        } else {
            a = (sign ? ~num[numLeft] + 1 : num[numLeft]);
        }
        if (right < 0 || numRight >= (int)num.size()) {
            b = 0;
        } else {
            b = (sign ? ~num[numRight] + 1 : num[numRight]);
        }
        a >>= cnt;
        b &= (1U << cnt) - 1U;
        res.num[i] = (b << (32 - cnt)) + a;
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
    for (int i = 0; i < (int)res.num.size(); i++) {
        int left = i * 32 + rhs;
        int right = (i + 1) * 32 - 1 + rhs;
        int numLeft = left / 32;
        int numRight = right / 32;
        unsigned int cnt = rhs % 32;
        unsigned int a, b;
        if (numLeft >= (int)num.size()) {
            a = sign * UINT32_MAX;
        } else {
            a = (sign ? ~num[numLeft] + 1 : num[numLeft]);
        }
        if (numRight >= (int)num.size()) {
            b = sign * UINT32_MAX;
        } else {
            b = (sign ? ~num[numRight] + 1 : num[numRight]);
        }
        a >>= cnt;
        b &= (1U << cnt) - 1U;
        res.num[i] = (b << (32 - cnt)) + a;
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
    return *(this + 1);
}

big_integer big_integer::operator++(int) {
    big_integer r = *this;
    ++*this;
    return r;
}

big_integer& big_integer::operator--() {
    return *(this - 1);
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
