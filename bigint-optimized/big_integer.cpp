#include "big_integer.h"

#include <cstring>
#include <stdexcept>
#include <algorithm>

big_integer::big_integer() {
    small_int = true;
    num = 0;
    sign = false;
}
big_integer::big_integer(big_integer const& other) {
    if (!other.small_int) {
        data = other.data;
        data->ref_counter++;
    } else {
        num = other.num;
    }
    sign = other.sign;
    small_int = other.small_int;
}
big_integer& big_integer::operator=(big_integer const& other) {
    if (this != &other) {
        if (!other.small_int) {
            data = other.data;
            data->ref_counter++;
        } else {
            num = other.num;
        }
        sign = other.sign;
        small_int = other.small_int;
    }
    return *this;
}
big_integer::~big_integer() {
    if (!small_int) {
        data->ref_counter--;
        if (data->ref_counter == 0) {
            delete (data);
        }
    }
}
big_integer::big_integer(int a) {
    small_int = true;
    num = abs(a);
    sign = a < 0;
}
big_integer::big_integer(unsigned int a) {
    small_int = true;
    num = a;
    sign = false;
}

void big_integer::check_ref_count() {
    if (!small_int && data->ref_counter > 1) {
        data->ref_counter--;
        data = new buffer(data->num);
    }
}

big_integer big_integer::check_sizes(big_integer rhs) {
    if (small_int && !rhs.small_int) {
        small_int = false;
        data = new buffer({num});
    } else if (!small_int && rhs.small_int) {
        rhs.small_int = false;
        rhs.data = new buffer({rhs.num});
        return rhs;
    }
    if (!small_int) {
        check_ref_count();
    }
    return rhs;
}

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
    if (!small_int) {
        check_ref_count();
        while ((int) data->num.size() > 1 && data->num.back() == 0) {
            data->num.pop_back();
        }
        if (data->num.back() == 0) {
            sign = false;
        }
        if (data->num.size() == 1) {
            int x = data->num[0];
            data->ref_counter--;
            if (data->ref_counter == 0) {
                delete (data);
            }
            num = x;
            small_int = true;
        }
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
    normalize();
}

big_integer& big_integer::operator+=(big_integer const& rhs) {
    if (sign != rhs.sign) {
        *this -= -rhs;
    } else {
        big_integer right = check_sizes(rhs);
        if (small_int) {
            unsigned int carry = addInt(num, right.num);
            if (carry) {
                data = new buffer({num, carry});
                small_int = false;
            }
        } else {
            int carry = 0;
            for (size_t i = 0; i < right.data->num.size() || carry; i++) {
                if (i == data->num.size()) {
                    data->num.push_back(0);
                }
                int newCarry = addInt(data->num[i], (i < right.data->num.size() ? right.data->num[i] : 0));
                newCarry += addInt(data->num[i], carry);
                carry = newCarry;
            }
        }
        normalize();
    }
    return *this;
}

big_integer& big_integer::operator-=(big_integer const& rhs) {
    if (sign != rhs.sign) {
        *this += -rhs;
    } else {
        big_integer right = check_sizes(rhs);
        if (small_int) {
            unsigned int carry;
            if (num < right.num) {
                unsigned int x = num;
                num = right.num;
                carry = subInt(num, x);
                sign = !sign;
            } else {
                carry = subInt(num, right.num);
            }
            if (carry) {
                data = new buffer({num, carry});
                small_int = false;
            }
        } else {
            if (right == 0) {
                return *this;
            }
            if (*this == right) {
                return *this = big_integer(0);
            }
            if ((*this < right) != sign) {
                big_integer tmp = right;
                tmp -= *this;
                return *this = -tmp;
            }
            int carry = 0;
            for (size_t i = 0; i < right.data->num.size() || carry; i++) {
                int newCarry = subInt(data->num[i], (i < right.data->num.size() ? right.data->num[i] : 0));
                newCarry += subInt(data->num[i], carry);
                carry = newCarry;
            }
        }
        normalize();
    }
    return *this;
}

big_integer& big_integer::operator*=(big_integer const& rhs) {
    big_integer right = check_sizes(rhs);
    if (small_int) {
        sign = sign ^ right.sign;
        std::pair<unsigned int, unsigned int> p = mul(right.num, num);
        if (p.first) {
            small_int = false;
            data = new buffer({p.second, p.first});
        } else {
            num = p.second;
        }
    } else {
        big_integer res;
        res.data = new buffer(std::vector<unsigned int>(data->num.size() + right.data->num.size() + 1));
        res.sign = sign != right.sign;
        res.small_int = false;
        for (size_t i = 0; i < data->num.size(); i++) {
            unsigned int carry = 0;
            for (size_t j = 0; j < right.data->num.size() || carry; j++) {
                if (j == right.data->num.size()) {
                    res.data->num[i + j] = carry;
                    break;
                }
                std::pair<unsigned int, unsigned int> p = mul(data->num[i], right.data->num[j]);
                p.first += addInt(p.second, carry);
                carry = p.first;
                carry += addInt(res.data->num[i + j], p.second);
            }
        }
        *this = res;
        normalize();
    }
    return *this;
}

big_integer& big_integer::operator/=(big_integer const& rhs) {
    big_integer right = check_sizes(rhs);
    if (rhs == 0) {
        throw std::invalid_argument("division by zero");
    }
    big_integer result;
    if (small_int) {
        sign = sign ^ right.sign;
        result = num / right.num;
    } else {
        big_integer a = *this;
        big_integer b = right;
        big_integer current;
        result.small_int = false;
        a.sign = b.sign = false;
        result.data = new buffer(std::vector<unsigned int>(data->num.size()));
        for (int i = (int) data->num.size() - 1; i >= 0; i--) {
            current <<= 32;
            current += a.data->num[i];
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
        result.sign = sign != right.sign;
        result.normalize();
    }
    return *this = result;
}

big_integer& big_integer::operator%=(big_integer const& rhs) {
    big_integer right = check_sizes(rhs);
    if (small_int) {
        if (right.num == 0) {
            throw std::invalid_argument("division by zero");
        }
        return *this = num % right.num;
    } else {
        big_integer res = *this - (*this / right) * right;
        res.normalize();
        return *this = res;
    }
}

big_integer& big_integer::operator&=(big_integer const& rhs) {
    big_integer right = check_sizes(rhs);
    if (small_int) {
        return *this = (unsigned int) ((sign ? -1 : 1) * num & (right.sign ? -1 : 1) * right.num);
    } else {
        for (size_t i = 0; i < data->num.size(); i++) {
            data->num[i] = (sign ? -1 : 1) * data->num[i] &
                           (i < right.data->num.size() ? (right.sign ? -1 : 1) * right.data->num[i] : 0);
        }
        normalize();
    }
    return *this;
}

big_integer& big_integer::operator|=(big_integer const& rhs) {
    big_integer right = check_sizes(rhs);
    if (small_int) {
        unsigned int c = (sign ? ~num + 1 : num) | (right.sign ? ~right.num + 1 : right.num);
        if (sign != right.sign) {
            c = ~c + 1;
        }
        return *this = c;
    } else {
        for (size_t i = 0; i < std::max(right.data->num.size(), data->num.size()); i++) {
            unsigned int a = (i < data->num.size() ? (sign ? ~data->num[i] + 1 : data->num[i]) : 0);
            unsigned int b = (i < right.data->num.size() ? (right.sign ? ~right.data->num[i] + 1 : right.data->num[i]) : 0);
            unsigned int c = a | b;
            if (sign != right.sign) {
                c = ~c + 1;
            }
            if (i == data->num.size()) {
                data->num.push_back(c);
            } else {
                data->num[i] = c;
            }
        }
        sign = sign != right.sign;
        normalize();
        return *this;
    }
}

big_integer& big_integer::operator^=(big_integer const& rhs) {
    big_integer right = check_sizes(rhs);
    if (small_int) {
        unsigned int c = (sign ? ~num + 1 : num) ^ (right.sign ? ~right.num + 1 : right.num);
        if (sign != right.sign) {
            c = ~c + 1;
        }
        return *this = c;
    } else {
        for (size_t i = 0; i < std::max(right.data->num.size(), data->num.size()); i++) {
            unsigned int a = (i < data->num.size() ? (sign ? ~data->num[i] + 1 : data->num[i]): 0);
            unsigned int b = (i < right.data->num.size() ? (right.sign ? ~right.data->num[i] + 1 : right.data->num[i]): 0);
            unsigned int c = a ^ b;
            if (sign != right.sign) {
                c = ~c + 1;
            }
            if (i == data->num.size()) {
                data->num.push_back(c);
            } else {
                data->num[i] = c;
            }
        }
        sign = sign != right.sign;
        normalize();
        return *this;
    }
}

big_integer& big_integer::operator<<=(int rhs) {
    if (rhs < 0) {
        throw std::invalid_argument("negative shift");
    }
    check_ref_count();
    if (small_int) {
        small_int = false;
        data = new buffer({num});
        return *this <<= rhs;
    } else {
        big_integer res;
        res.data = new buffer(data->num);
        res.sign = sign;
        res.small_int = false;
        res.check_ref_count();
        sign = false;
        for (int i = 0; i <= rhs / 31 + 1; i++) {
            res.data->num.push_back(0);
        }
        for (int i = 0; i < (int) res.data->num.size(); i++) {
            int left = i * 32 - rhs;
            int right = (i + 1) * 32 - 1 - rhs;
            int numLeft = left / 32;
            int numRight = right / 32;
            unsigned int cnt = (32 - rhs % 32) % 32;
            unsigned int a, b;
            if (left < 0 || numLeft >= (int) data->num.size()) {
                a = 0;
            } else {
                a = (sign ? ~data->num[numLeft] + 1 : data->num[numLeft]);
            }
            if (right < 0 || numRight >= (int) data->num.size()) {
                b = 0;
            } else {
                b = (sign ? ~data->num[numRight] + 1 : data->num[numRight]);
            }
            a >>= cnt;
            b &= (1U << cnt) - 1U;
            res.data->num[i] = (b << (32 - cnt)) + a;
            if (sign) {
                res.data->num[i] = ~res.data->num[i] + 1;
            }
        }
        res.normalize();
        return *this = res;
    }
}

big_integer& big_integer::operator>>=(int rhs) {
    check_ref_count();
    if (rhs < 0) {
        throw std::invalid_argument("negative shift");
    }
    if (small_int) {
        num >>= rhs;
        if (sign) {
            num++;
        }
        return *this;
    } else {
        big_integer res = *this;
        res.check_ref_count();
        res.sign = sign;
        sign = false;
        for (int i = 0; i < (int) res.data->num.size(); i++) {
            int left = i * 32 + rhs;
            int right = (i + 1) * 32 - 1 + rhs;
            int numLeft = left / 32;
            int numRight = right / 32;
            unsigned int cnt = rhs % 32;
            unsigned int a, b;
            if (numLeft >= (int) data->num.size()) {
                a = sign * UINT32_MAX;
            } else {
                a = (sign ? ~data->num[numLeft] + 1 : data->num[numLeft]);
            }
            if (numRight >= (int) data->num.size()) {
                b = sign * UINT32_MAX;
            } else {
                b = (sign ? ~data->num[numRight] + 1 : data->num[numRight]);
            }
            a >>= cnt;
            b &= (1U << cnt) - 1U;
            res.data->num[i] = (b << (32 - cnt)) + a;
            if (sign) {
                res.data->num[i] = ~res.data->num[i] + 1;
            }
        }
        res.normalize();
        if (res.sign) {
            res -= 1;
        }
        return *this = res;
    }
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
    if (a.small_int != b.small_int) {
        return false;
    } else if (!a.small_int) {
        return a.sign == b.sign && a.data->num == b.data->num;
    } else {
        return a.num == b.num;
    }
}

bool operator!=(big_integer const& a, big_integer const& b) {
    return !(a == b);
}

bool operator<(big_integer const& left, big_integer const& right){
    big_integer a = left;
    big_integer b = right;
    if (a == b) {
        return false;
    }
    b = a.check_sizes(b);
    if (a.sign != b.sign) {
        return a.sign > b.sign;
    } else {
        if (a.small_int) {
            return (a.num < b.num) ^ a.sign;
        }
        bool result = false;
        if (a.data->num.size() != b.data->num.size()) {
            result = a.data->num.size() < b.data->num.size();
        } else {
            for (int i = (int)a.data->num.size() - 1; i >= 0; i--) {
                if (a.data->num[i] != b.data->num[i]) {
                    result = a.data->num[i] < b.data->num[i];
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
    if (a.small_int) {
        return std::to_string(a.num);
    } else {
        std::string s;
        big_integer x = a;
        x.sign = false;
        while (x != 0) {
            x % 10;
            s += (char) ((x % 10).num + '0');
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
}

std::ostream& operator<<(std::ostream& s, big_integer const& a) {
    return s << to_string(a);
}
