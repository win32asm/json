//     __ _____ _____ _____
//  __|  |   __|     |   | |  JSON for Modern C++ (supporting code)
// |  |  |__   |  |  | | | |  version 3.11.0
// |_____|_____|_____|_|___|  https://github.com/nlohmann/json
//
// SPDX-FileCopyrightText: 2013-2022 Niels Lohmann <https://nlohmann.me>
// SPDX-FileCopyrightText: 2022 Vladimir Bespalov <vlad.bespalov@jetstreamsoft.com>
// SPDX-License-Identifier: MIT

#include "doctest_compatibility.h"

#include <nlohmann/json.hpp>
#include <stdexcept>
#include <sstream>

using nlohmann::json;
using std::overflow_error;
using std::stringstream;

class bound_uint64_t {
    uint64_t    value;

public:
    bound_uint64_t() = default;
    bound_uint64_t(const bound_uint64_t &copy) = default;
    bound_uint64_t(bound_uint64_t &&move) noexcept = default;   // not required - optimization
    // implicit conversions from uint64_t required
    bound_uint64_t(uint64_t v)
    {
        if (v >= INT64_MAX) {
            std::stringstream errmsg;
            errmsg << "Value " << v << " out of bound.";
            throw overflow_error(errmsg.str());
        }
        value = v;
    }

    explicit operator uint64_t() const { return value; }
    explicit operator unsigned int() const { return value; }                // required for conversion to decimal string
    explicit operator std::intmax_t() const { return value; }               // required for generic sign-conversion

    explicit operator short unsigned int() const { return value; }          // required for return value conversion

    bound_uint64_t &operator=(const bound_uint64_t &copy) = default;
    bound_uint64_t &operator=(bound_uint64_t &&move) noexcept = default;    // not required - optimization
    bound_uint64_t &operator=(uint64_t v)
    {
        if (v >= INT64_MAX) {
            std::stringstream errmsg;
            errmsg << "Value " << v << " out of bound.";
            throw overflow_error(errmsg.str());
        }
        value = v;
        return *this;
    }

    // operators for conversion to decimal string
    bool operator>=(uint64_t x) const {
        return value >= x;
    }
    bool operator<(uint64_t x) const {
        return value < x;
    }
    bool operator==(uint64_t x) const {
        return value == x;
    }
    bound_uint64_t &operator/=(uint64_t x){
        value /= x;
        return *this;
    }
    long operator%(uint64_t x) const{
        return long(value % x);
    }
    bound_uint64_t operator/(uint64_t x) const{
        return {value / x};
    }
    friend char operator+(char x, const bound_uint64_t& b) {
        return char(b.value + x);
    }
    friend int64_t operator+(int64_t x, const bound_uint64_t& b) {
        if (x > 0 && INT64_MAX - int64_t(b.value) < x) {
            std::stringstream errmsg;
            errmsg << __PRETTY_FUNCTION__ << ": value " << x << " + " << b.value <<
                " is out of bound";
            throw overflow_error(errmsg.str());
        }
        return int64_t(b.value + x);
    }
};

// Common json type declaration.
// Full basic_json<> template declaration is:
//   basic_json<ObjectType, ArrayType, StringType, BooleanType,
//     NumberIntegerType, NumberUnsignedType, NumberFloatType,
//     AllocatorType, JSONSerializer>
// with all types having defaults to produce nlohmann::json alias type.
// To achieve required conversion of values potentially overflowing
// receiving type in Java we have to override NumberUnsignedType with
// a custom specializer type:
//   basic_json< NumberUnsignedType = bound_uint64_t >
// C++ requires that we supply all arguments prior to the changed one,
// so we use default values here.
using Json = nlohmann::basic_json<std::map, std::vector, std::string, bool, int64_t, bound_uint64_t>;

// required overloads of conversions to and from json
template<typename BasicJsonType> inline static void
to_json(BasicJsonType& j, bound_uint64_t&& e) noexcept
{
    nlohmann::detail::external_constructor<nlohmann::detail::value_t::number_unsigned>::construct(j, e);
}

template<typename BasicJsonType> inline static void
to_json(BasicJsonType& j, const bound_uint64_t& e) noexcept
{
    nlohmann::detail::external_constructor<nlohmann::detail::value_t::number_unsigned>::construct(j, e);
}

template<typename BasicJsonType> inline static void
from_json(const BasicJsonType& j, bound_uint64_t& e) noexcept
{
    e = j.template get<bound_uint64_t>();
}


TEST_CASE("test for using a bound integer type")
{
    SECTION("Assign various values close to bound border")
    {

        Json bound;
        uint64_t value;

        value = INT64_MAX - 1;
        CHECK_NOTHROW(bound["in_bound"] = value);

        value = INT64_MAX + 1;
        CHECK_THROWS_WITH_AS(bound["out_of_bound"] = value, "Value 9223372036854775808 out of bound.", overflow_error);
    }
}

