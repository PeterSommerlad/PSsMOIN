#ifndef PS_SMOIN_H
#define PS_SMOIN_H

#include <cstdint>
#include <type_traits>
#include <iosfwd>
#include <limits>
#include <climits>
#ifdef __cpp_concepts
#include <concepts>
#endif

// configurable error notifications
// define to non-zero for self-kill with core dump, testing requires 0 for throw
#ifndef PSSMOIN_SHOULD_RAISE
#define PSSMOIN_SHOULD_RAISE 0
#endif

#if PSSMOIN_SHOULD_RAISE
#include <csignal>
// SIGFPE (arithmetic error) dumps core (unless prohibited by OS, i.e., ulimit -c 0, macos cores are more tricky to get
#define PSSMOIN_RAISE_SIGFPE() ::raise(SIGFPE)
#else
#define PSSMOIN_RAISE_SIGFPE()
#endif

#define ps_assert( cond, msg) \
   if (not (cond)) { PSSMOIN_RAISE_SIGFPE() ; throw(msg); } ;

#if __cplusplus == 201703L
#define CONSTEVAL constexpr
#elif __cplusplus >= 202002L
#define CONSTEVAL consteval
#else
#error "requires at least C++17, best C++20"
#endif


namespace pssmoin { // Peter Sommerlad's simple modulo arithmetic (wrapping) integers

// unsigned 
enum class [[nodiscard]] ui8 : std::uint8_t { tag_to_prevent_mixing_other_enums };
enum class [[nodiscard]] ui16: std::uint16_t{ tag_to_prevent_mixing_other_enums };
enum class [[nodiscard]] ui32: std::uint32_t{ tag_to_prevent_mixing_other_enums };
enum class [[nodiscard]] ui64: std::uint64_t{ tag_to_prevent_mixing_other_enums };

inline namespace literals {
CONSTEVAL
ui8 operator""_ui8(unsigned long long val) {
    if (val <= std::numeric_limits<std::underlying_type_t<ui8>>::max()) {
        return ui8(val);
    } else {
        throw "integral constant too large"; // trigger compile-time error
    }
}


CONSTEVAL
ui16 operator""_ui16(unsigned long long val) {
    if (val <= std::numeric_limits<std::underlying_type_t<ui16>>::max()) {
        return ui16(val);
    } else {
        throw "integral constant too large"; // trigger compile-time error
    }
}


CONSTEVAL
ui32 operator""_ui32(unsigned long long val) {
    if (val <= std::numeric_limits<std::underlying_type_t<ui32>>::max()) {
        return ui32(val);
    } else {
        throw "integral constant too large"; // trigger compile-time error
    }
}


CONSTEVAL
ui64 operator""_ui64(unsigned long long val) {
    if constexpr (sizeof(ui64) < sizeof(val)){
        if (val > 0xffff'ffff'fffffffful) {
            throw "integral constant too large"; // trigger compile-time error
        }
    }
    return ui64(val);
}

}
// signed
enum class [[nodiscard]] si8 : std::int8_t { tag_to_prevent_mixing_other_enums };
enum class [[nodiscard]] si16: std::int16_t{ tag_to_prevent_mixing_other_enums };
enum class [[nodiscard]] si32: std::int32_t{ tag_to_prevent_mixing_other_enums };
enum class [[nodiscard]] si64: std::int64_t{ tag_to_prevent_mixing_other_enums };

inline namespace literals {
CONSTEVAL
si8 operator""_si8(unsigned long long val) {
    if (val <= std::numeric_limits<std::underlying_type_t<si8>>::max()) {
        return si8(val);
    } else {
        throw "integral constant too large"; // trigger compile-time error
    }
}


CONSTEVAL
si16 operator""_si16(unsigned long long val) {
    if (val <= std::numeric_limits<std::underlying_type_t<si16>>::max()) {
        return si16(val);
    } else {
        throw "integral constant too large"; // trigger compile-time error
    }
}


CONSTEVAL
si32 operator""_si32(unsigned long long val) {
    if (val <= std::numeric_limits<std::underlying_type_t<si32>>::max()) {
        return si32(val);
    } else {
        throw "integral constant too large"; // trigger compile-time error
    }
}


CONSTEVAL
si64 operator""_si64(unsigned long long val) {
    if (val <= std::numeric_limits<std::underlying_type_t<si64>>::max()) {
        return si64(val);
    } else {
        throw "integral constant too large"; // trigger compile-time error
    }
}
}
#undef CONSTEVAL

namespace detail_ {

#if __cplusplus >= 202002L
template<typename T>
using plain = std::remove_cvref_t<T>;

template<typename T>
concept an_enum = std::is_enum_v<plain<T>>;

template<an_enum T>
constexpr bool
is_scoped_enum_v = !std::is_convertible_v<T, std::underlying_type_t<T>>;

// from C++23

template<typename T>
concept a_scoped_enum = is_scoped_enum_v<T>;


#else
template<typename T>
using plain = std::remove_cv_t<std::remove_reference_t<T>>;

template<typename T>
constexpr bool an_enum = std::is_enum_v<plain<T>>;

template<typename T, typename = std::enable_if_t<an_enum<T>>>
constexpr bool
is_scoped_enum_v = !std::is_convertible_v<T, std::underlying_type_t<T>>;

#endif

// detection concept

#if __cplusplus >= 202002L
template<typename T>
constexpr bool
is_moduloint_v = false;

template<a_scoped_enum E>
constexpr bool
is_moduloint_v<E> = requires { E{} == E::tag_to_prevent_mixing_other_enums; } ;

#else
template<typename T, typename=void>
constexpr bool
is_moduloint_v = false;
template<typename E>
constexpr bool
is_moduloint_v<E,std::void_t<decltype( E{} == E::tag_to_prevent_mixing_other_enums )>> = is_scoped_enum_v<E> ;

#endif


template<typename E>
using ULT=std::conditional_t<std::is_enum_v<plain<E>>,std::underlying_type_t<plain<E>>,plain<E>>;

template<typename E>
using promoted_t = // will promote keeping signedness
        std::conditional_t<(sizeof(ULT<E>) < sizeof(int))
            , std::conditional_t<std::is_unsigned_v<ULT<E>>
                , unsigned
                , int >
            , ULT<E>>;



}

using detail_::ULT;
#ifdef __cpp_concepts
template<typename E>
concept a_moduloint = detail_::is_moduloint_v<E>;
#endif



namespace detail_{



#ifdef __cpp_concepts
template<a_moduloint LEFT, a_moduloint RIGHT>
#else
template<typename LEFT, typename RIGHT, typename=std::enable_if_t<is_moduloint_v<LEFT> && is_moduloint_v<RIGHT>>>
#endif
constexpr bool
same_signedness_v = is_moduloint_v<LEFT> && is_moduloint_v<RIGHT> && std::numeric_limits<LEFT>::is_signed == std::numeric_limits<RIGHT>::is_signed;

template<typename CHAR>
constexpr bool
is_chartype_v =    std::is_same_v<char,CHAR>
                || std::is_same_v<wchar_t,CHAR>
#ifdef __cpp_char8_t
                || std::is_same_v<char8_t,CHAR>
#endif
                || std::is_same_v<char16_t,CHAR>
                || std::is_same_v<char32_t,CHAR> ;



template<typename INT, typename TESTED>
constexpr bool
is_compatible_integer_v = std::is_same_v<plain<TESTED>,INT> ||
   (   std::is_integral_v<plain<TESTED>>
   && !std::is_same_v<bool,plain<TESTED>>
   && !is_chartype_v<plain<TESTED>>
   && (std::is_unsigned_v<INT> == std::is_unsigned_v<plain<TESTED>>)
   && std::numeric_limits<plain<TESTED>>::max() == std::numeric_limits<INT>::max() );

template<typename TESTED,typename=void>
constexpr bool
is_known_integer_v=false;

// only support the following sizes:
template<typename TESTED>
constexpr bool
is_known_integer_v<TESTED,std::enable_if_t<std::is_integral_v<TESTED>>> =
                        is_compatible_integer_v<std::uint8_t,  TESTED>
                     || is_compatible_integer_v<std::uint16_t, TESTED>
                     || is_compatible_integer_v<std::uint32_t, TESTED>
                     || is_compatible_integer_v<std::uint64_t, TESTED>
                     || is_compatible_integer_v<std::int8_t,  TESTED>
                     || is_compatible_integer_v<std::int16_t, TESTED>
                     || is_compatible_integer_v<std::int32_t, TESTED>
                     || is_compatible_integer_v<std::int64_t, TESTED>;

}

template<typename LEFT, typename RIGHT>
#ifdef __cpp_concepts
concept same_signedness = detail_::same_signedness_v<LEFT,RIGHT>;
#else
constexpr bool
same_signedness_v = detail_::is_moduloint_v<LEFT> && detail_::is_moduloint_v<RIGHT> && std::numeric_limits<LEFT>::is_signed == std::numeric_limits<RIGHT>::is_signed;
#endif


#ifdef __cpp_concepts
template<a_moduloint E>
#else
template<typename E,typename=std::enable_if_t<detail_::is_moduloint_v<E>>>
#endif
[[nodiscard]]
constexpr auto
promote_keep_signedness(E val) noexcept
{ // promote keeping signedness
    return static_cast<detail_::promoted_t<E>>(val);// promote with sign extension
}

// not used in framework but in tests:
#ifdef __cpp_concepts
template<a_moduloint E>
#else
template<typename E,typename=std::enable_if_t<detail_::is_moduloint_v<E>>>
#endif
[[nodiscard]]
constexpr auto
to_underlying(E val) noexcept 
{ // plain value with all bad properties
    return static_cast<ULT<E>>(val);
}

#ifdef __cpp_concepts
template<a_moduloint E>
#else
template<typename E,typename=std::enable_if_t<detail_::is_moduloint_v<E>>>
#endif
[[nodiscard]]
constexpr auto
promote_to_unsigned(E val) noexcept
{ // promote to unsigned for wrap around arithmetic
    using u_result_t = std::make_unsigned_t<detail_::promoted_t<E>>;
    return static_cast<u_result_t>(promote_keep_signedness(val));
}

#ifdef __cpp_concepts
// deliberately not std::integral, because of bool and characters!
template<typename T>
concept sized_integer = detail_::is_known_integer_v<T>;
#endif

namespace detail_{
#ifdef __cpp_concepts
template<sized_integer TARGET, a_moduloint E>
#else
template<typename TARGET, typename E, typename=std::enable_if_t<is_known_integer_v<TARGET> && is_moduloint_v<E>>>
#endif
[[nodiscard]]
constexpr auto
promote_and_extend_to_unsigned(E val) noexcept
{ // promote to unsigned for wrap around arithmetic, with sign extension if needed
       using u_result_t = std::conditional_t< (sizeof(TARGET) > sizeof(detail_::promoted_t<E>)),
                std::make_unsigned_t<TARGET>, std::make_unsigned_t<detail_::promoted_t<E> > >;
       using s_result_t = std::make_signed_t<u_result_t>;
       return static_cast<u_result_t>(static_cast<s_result_t>(promote_keep_signedness(val)));// promote with sign extension
}
#ifdef __cpp_concepts
template<sized_integer TARGET, a_moduloint E>
#else
template<typename TARGET, typename E, typename=std::enable_if_t<
is_known_integer_v<TARGET> &&
std::numeric_limits<TARGET>::is_signed &&
is_moduloint_v<E>>>
#endif
[[nodiscard]]
constexpr auto
abs_promoted_and_extended_as_unsigned(E val) noexcept
#ifdef __cpp_concepts
 requires (std::numeric_limits<TARGET>::is_signed)
#endif
{ // promote to unsigned for wrap around arithmetic removing sign if negative
  // return just the bits for std::numeric_limits<TARGET>::min()
       using promoted_t = detail_::promoted_t<E>;
       using u_result_t = std::conditional_t< (sizeof(TARGET) > sizeof(promoted_t)),
                std::make_unsigned_t<TARGET>, std::make_unsigned_t<promoted_t > >;
       static_assert(std::is_unsigned_v<u_result_t>);
       using s_result_t = std::make_signed_t<u_result_t>;
       s_result_t value = promote_keep_signedness(val);
       if (val < E{} && value > std::numeric_limits<s_result_t>::min()){
           return static_cast<u_result_t>(-static_cast<s_result_t>(value)); // cannot overflow
       } else {
           return static_cast<u_result_t>(value);
       }
}

}



#ifdef __cpp_concepts
template<sized_integer T>
#else
template<typename T, typename=std::enable_if_t<detail_::is_known_integer_v<T>>>
#endif
[[nodiscard]]
constexpr auto
from_int(T val) noexcept {
    using detail_::is_compatible_integer_v;
    using std::conditional_t;
    struct cannot_convert_integer{};
    using result_t =
            conditional_t<is_compatible_integer_v<std::uint8_t,T>, ui8,
             conditional_t<is_compatible_integer_v<std::uint16_t,T>, ui16,
              conditional_t<is_compatible_integer_v<std::uint32_t,T>, ui32,
               conditional_t<is_compatible_integer_v<std::uint64_t,T>, ui64,
                conditional_t<is_compatible_integer_v<std::int8_t,T>, si8,
                 conditional_t<is_compatible_integer_v<std::int16_t,T>, si16,
                  conditional_t<is_compatible_integer_v<std::int32_t,T>, si32,
                   conditional_t<is_compatible_integer_v<std::int64_t,T>, si64, cannot_convert_integer>>>>>>>>;
    return static_cast<result_t>(val);
}
#ifdef __cpp_concepts
template<a_moduloint TO, sized_integer FROM>
#else
template<typename TO, typename FROM, typename=std::enable_if_t<detail_::is_known_integer_v<FROM> && detail_::is_moduloint_v<TO>>>
#endif
[[nodiscard]]
constexpr auto
from_int_to(FROM val)
{
    using result_t = TO;
    using ultr = ULT<result_t>;
    if constexpr(std::is_unsigned_v<ultr>){
        ps_assert(  (val >= FROM{} && // in case FROM is signed
                     static_cast<std::make_unsigned_t<FROM>>(val) <= std::numeric_limits<ultr>::max()), "from_int_to: integer value out of range") ;
    } else {
        if constexpr (std::is_unsigned_v<FROM>){
            ps_assert(  val <= static_cast<std::make_unsigned_t<ultr>>(std::numeric_limits<ultr>::max()), "from_int_to: integer value out of range");

        } else { // both are signed
            ps_assert(  (val <= std::numeric_limits<ultr>::max() &&
                                val >= std::numeric_limits<ultr>::min()),  "from_int_to:  integer value out of range");
        }
    }
    return static_cast<result_t>(val); // cast is checked above
}


// comparison
// not needed, we won't mix types in comparison.


// negation for signed types only, two's complement
#ifdef __cpp_concepts
template<a_moduloint E>
#else
template<typename E,typename=std::enable_if_t<detail_::is_moduloint_v<E> && std::numeric_limits<E>::is_signed >>
#endif
constexpr E
operator-(E l) noexcept
#ifdef __cpp_concepts
requires std::numeric_limits<E>::is_signed
#endif
{
    return static_cast<E>(1u + ~promote_to_unsigned(l));
}

// increment/decrement

#ifdef __cpp_concepts
template<a_moduloint E>
#else
template<typename E,typename=std::enable_if_t<detail_::is_moduloint_v<E>>>
#endif
constexpr E&
operator++(E& l) noexcept
{
    return l = static_cast<E>(1u + promote_to_unsigned(l));
}

#ifdef __cpp_concepts
template<a_moduloint E>
#else
template<typename E,typename=std::enable_if_t<detail_::is_moduloint_v<E>>>
#endif
constexpr E
operator++(E& l, int) noexcept
{
    auto result=l;
    ++l;
    return result;
}
#ifdef __cpp_concepts
template<a_moduloint E>
#else
template<typename E,typename=std::enable_if_t<detail_::is_moduloint_v<E>>>
#endif
constexpr E&
operator--(E& l) noexcept {
    return l = static_cast<E>(promote_to_unsigned(l) - 1u);
}

#ifdef __cpp_concepts
template<a_moduloint E>
#else
template<typename E,typename=std::enable_if_t<detail_::is_moduloint_v<E>>>
#endif
constexpr E
operator--(E& l, int) noexcept {
    auto result=l;
    --l;
    return result;
}



// arithmetic


#ifdef __cpp_concepts
template<a_moduloint LEFT, a_moduloint RIGHT>
#else
template<typename LEFT, typename RIGHT, typename=std::enable_if_t<detail_::is_moduloint_v<LEFT>&&detail_::is_moduloint_v<RIGHT> && detail_::same_signedness_v<LEFT,RIGHT>>>
#endif
constexpr auto
operator+(LEFT l, RIGHT r) noexcept
#ifdef __cpp_concepts
requires same_signedness<LEFT,RIGHT>
#endif
{
    // need to handle sign extension
    using result_t=std::conditional_t<sizeof(LEFT)>=sizeof(RIGHT),LEFT,RIGHT>;
    using ult = ULT<result_t>;
    return static_cast<result_t>(
            static_cast<ult>(
                    detail_::promote_and_extend_to_unsigned<ult>(l)
                    + // use unsigned op to prevent signed overflow, but wrap.
                    detail_::promote_and_extend_to_unsigned<ult>(r)
            )
    );
}


#ifdef __cpp_concepts
template<a_moduloint LEFT, a_moduloint RIGHT>
#else
template<typename LEFT, typename RIGHT, typename=std::enable_if_t<detail_::is_moduloint_v<LEFT>&&detail_::is_moduloint_v<RIGHT> && detail_::same_signedness_v<LEFT,RIGHT>>>
#endif
constexpr auto&
operator+=(LEFT &l, RIGHT r) noexcept
#ifdef __cpp_concepts
requires same_signedness<LEFT,RIGHT>
#endif
{
    static_assert(sizeof(LEFT) >= sizeof(RIGHT),"adding too large integer type");
    l = static_cast<LEFT>(l+r);
    return l;
}

#ifdef __cpp_concepts
template<a_moduloint LEFT, a_moduloint RIGHT>
#else
template<typename LEFT, typename RIGHT, typename=std::enable_if_t<detail_::is_moduloint_v<LEFT>&&detail_::is_moduloint_v<RIGHT> && detail_::same_signedness_v<LEFT,RIGHT>>>
#endif
constexpr auto
operator-(LEFT l, RIGHT r) noexcept
#ifdef __cpp_concepts
requires same_signedness<LEFT,RIGHT>
#endif
{
    using result_t=std::conditional_t<sizeof(LEFT)>=sizeof(RIGHT),LEFT,RIGHT>;
    using ult = ULT<result_t>;

    return static_cast<result_t>(
            static_cast<ult>(
                    detail_::promote_and_extend_to_unsigned<ult>(l)
                    - // use unsigned op to prevent signed overflow, but wrap.
                    detail_::promote_and_extend_to_unsigned<ult>(r)
            )
    );
}
#ifdef __cpp_concepts
template<a_moduloint LEFT, a_moduloint RIGHT>
#else
template<typename LEFT, typename RIGHT, typename=std::enable_if_t<detail_::is_moduloint_v<LEFT>&&detail_::is_moduloint_v<RIGHT> && detail_::same_signedness_v<LEFT,RIGHT>>>
#endif
constexpr auto&
operator-=(LEFT &l, RIGHT r) noexcept
#ifdef __cpp_concepts
requires same_signedness<LEFT,RIGHT>
#endif
{
    static_assert(sizeof(LEFT) >= sizeof(RIGHT),"subtracting too large integer type");
    l = static_cast<LEFT>(l-r);
    return l;
}


#ifdef __cpp_concepts
template<a_moduloint LEFT, a_moduloint RIGHT>
#else
template<typename LEFT, typename RIGHT>
#endif
constexpr auto
operator*(LEFT l, RIGHT r) noexcept
#ifdef __cpp_concepts
requires same_signedness<LEFT,RIGHT>
#else
-> std::enable_if_t<detail_::is_moduloint_v<LEFT>&&detail_::is_moduloint_v<RIGHT> && detail_::same_signedness_v<LEFT,RIGHT>,
std::conditional_t<sizeof(LEFT)>=sizeof(RIGHT),LEFT,RIGHT> >
#endif
{
    using result_t=std::conditional_t<sizeof(LEFT)>=sizeof(RIGHT),LEFT,RIGHT>;
    using ult = ULT<result_t>;
    return static_cast<result_t>(
            static_cast<ult>(
                    detail_::promote_and_extend_to_unsigned<ult>(l)
                    * // use unsigned op to prevent signed overflow, but wrap.
                    detail_::promote_and_extend_to_unsigned<ult>(r)
            )
    );
}

#ifdef __cpp_concepts
template<a_moduloint LEFT, sized_integer RIGHT>
constexpr auto
operator*(LEFT l, RIGHT r) noexcept
{
        return l * from_int_to<LEFT>(r);
}
template<sized_integer LEFT, a_moduloint RIGHT>
constexpr auto
operator*(LEFT l, RIGHT r) noexcept
{
        return from_int_to<RIGHT>(l) * r;
}

#else
template<typename LEFT, typename RIGHT, typename=std::enable_if_t<
(detail_::is_moduloint_v<LEFT>&&detail_::is_known_integer_v<RIGHT>) ||
(detail_::is_moduloint_v<RIGHT>&&detail_::is_known_integer_v<LEFT>)> >
constexpr auto
operator*(LEFT l, RIGHT r) noexcept
{
    if constexpr (detail_::is_moduloint_v<LEFT>)
        return l * from_int_to<LEFT>(r);
    else
        return from_int_to<RIGHT>(l) * r;

}
#endif


#ifdef __cpp_concepts
template<a_moduloint LEFT, a_moduloint RIGHT>
#else
template<typename LEFT, typename RIGHT, typename=std::enable_if_t<detail_::is_moduloint_v<LEFT>&&detail_::is_moduloint_v<RIGHT> && detail_::same_signedness_v<LEFT,RIGHT>>>
#endif
constexpr auto&
operator*=(LEFT &l, RIGHT r) noexcept
#ifdef __cpp_concepts
requires same_signedness<LEFT,RIGHT>
#endif
{
    static_assert(sizeof(LEFT) >= sizeof(RIGHT),"multiplying too large integer type");
    l = static_cast<LEFT>(l*r);
    return l;
}
#ifdef __cpp_concepts
template<a_moduloint LEFT, sized_integer RIGHT>
constexpr auto&
#else
template<typename LEFT, typename RIGHT>
constexpr auto
#endif
operator*=(LEFT &l, RIGHT r) noexcept
#ifndef __cpp_concepts
->std::enable_if_t<detail_::is_moduloint_v<LEFT> && detail_::is_known_integer_v<RIGHT>, LEFT&>
#endif
{
    return l *= from_int_to<LEFT>(r);
}



#ifdef __cpp_concepts
template<a_moduloint LEFT, a_moduloint RIGHT>
#else
template<typename LEFT, typename RIGHT, typename=std::enable_if_t<detail_::is_moduloint_v<LEFT>&&detail_::is_moduloint_v<RIGHT> && detail_::same_signedness_v<LEFT,RIGHT>>>
#endif
constexpr auto
operator/(LEFT l, RIGHT r)
#ifdef __cpp_concepts
requires same_signedness<LEFT,RIGHT>
#endif
{
    using result_t=std::conditional_t<sizeof(LEFT)>=sizeof(RIGHT),LEFT,RIGHT>;
    using ult = ULT<result_t>;

    ps_assert(r != RIGHT{}, "pssmoin: division by zero");
    if constexpr (std::numeric_limits<result_t>::is_signed){
        bool result_is_negative = (l < LEFT{}) != (r < RIGHT{});
        auto absresult =  static_cast<result_t>(
                             static_cast<ult>(
                                detail_::abs_promoted_and_extended_as_unsigned<ult>(l)
                                / // use unsigned op to prevent signed overflow, but wrap.
                                detail_::abs_promoted_and_extended_as_unsigned<ult>(r)));
        if (result_is_negative) {
            return -absresult; // compute two's complement, not built-in
        } else {
            return absresult;
        }
    } else {
    return static_cast<result_t>(
            static_cast<ult>(
                    detail_::promote_and_extend_to_unsigned<ult>(l)
                    / // use unsigned op to prevent signed overflow, but wrap.
                    detail_::promote_and_extend_to_unsigned<ult>(r)
            )
    );
    }

}
#ifdef __cpp_concepts
template<a_moduloint LEFT, sized_integer RIGHT>
#else
template<typename LEFT, typename RIGHT>
#endif
constexpr auto
operator/(LEFT l, RIGHT r)
#ifndef __cpp_concepts
-> std::enable_if_t<detail_::is_moduloint_v<LEFT> && detail_::is_known_integer_v<RIGHT>, LEFT>
#endif
{
    return l / from_int_to<LEFT>(r);
}

#ifdef __cpp_concepts
template<a_moduloint LEFT, a_moduloint RIGHT>
#else
template<typename LEFT, typename RIGHT, typename=std::enable_if_t<detail_::is_moduloint_v<LEFT>&&detail_::is_moduloint_v<RIGHT> && detail_::same_signedness_v<LEFT,RIGHT>>>
#endif
constexpr auto&
operator/=(LEFT &l, RIGHT r)
#ifdef __cpp_concepts
requires same_signedness<LEFT,RIGHT>
#endif
{
    static_assert(sizeof(LEFT) >= sizeof(RIGHT),"dividing by too large integer type");
    l = static_cast<LEFT>(l/r);
    return l;
}
#ifdef __cpp_concepts
template<a_moduloint LEFT, sized_integer RIGHT>
constexpr auto&
#else
template<typename LEFT, typename RIGHT>
constexpr auto
#endif
operator/=(LEFT &l, RIGHT r) noexcept
#ifndef __cpp_concepts
->std::enable_if_t<detail_::is_moduloint_v<LEFT> && detail_::is_known_integer_v<RIGHT>, LEFT&>
#endif
{
    return l /= from_int_to<LEFT>(r);
}

#ifdef __cpp_concepts
template<a_moduloint LEFT, a_moduloint RIGHT>
#else
template<typename LEFT, typename RIGHT, typename=std::enable_if_t<
detail_::is_moduloint_v<LEFT> &&
detail_::is_moduloint_v<RIGHT> &&
detail_::same_signedness_v<LEFT,RIGHT> &&
std::is_unsigned_v<ULT<LEFT>>>>
#endif
constexpr auto
operator%(LEFT l, RIGHT r)
#ifdef __cpp_concepts
requires same_signedness<LEFT,RIGHT> && std::is_unsigned_v<ULT<LEFT>>
#endif
{
    using result_t=std::conditional_t<sizeof(LEFT)>=sizeof(RIGHT),LEFT,RIGHT>;
    using ult = ULT<result_t>;
    ps_assert(r != RIGHT{}, "pssmoin: division by zero");
    return static_cast<result_t>(
            static_cast<ult>(
                    detail_::promote_and_extend_to_unsigned<ult>(l)
                    % // use unsigned op to prevent signed overflow, but wrap.
                    detail_::promote_and_extend_to_unsigned<ult>(r)
            )
    );
}
#ifdef __cpp_concepts
template<a_moduloint LEFT, a_moduloint RIGHT>
#else
template<typename LEFT, typename RIGHT, typename=std::enable_if_t<
detail_::is_moduloint_v<LEFT> &&
detail_::is_moduloint_v<RIGHT> &&
detail_::same_signedness_v<LEFT,RIGHT> &&
std::is_unsigned_v<ULT<LEFT>>>>
#endif
constexpr auto&
operator%=(LEFT &l, RIGHT r)
#ifdef __cpp_concepts
requires same_signedness<LEFT,RIGHT> && std::is_unsigned_v<ULT<LEFT>>
#endif
{
    static_assert(sizeof(LEFT) >= sizeof(RIGHT),"dividing by too large integer type");
    l = static_cast<LEFT>(l%r);
    return l;
}

// bitwise operators

#ifdef __cpp_concepts
template<a_moduloint LEFT, a_moduloint RIGHT>
#else
template<typename LEFT, typename RIGHT, typename=std::enable_if_t<
detail_::is_moduloint_v<LEFT> &&
detail_::is_moduloint_v<RIGHT> &&
std::is_unsigned_v<ULT<LEFT>> &&
std::is_unsigned_v<ULT<RIGHT>>>>
#endif
constexpr auto
operator&(LEFT l, RIGHT r) noexcept
#ifdef __cpp_concepts
requires std::is_unsigned_v<ULT<LEFT>> && std::is_unsigned_v<ULT<RIGHT>>
#endif
{
    using result_t=std::conditional_t<sizeof(LEFT)>=sizeof(RIGHT),LEFT,RIGHT>;
    return static_cast<result_t>(promote_keep_signedness(l)&promote_keep_signedness(r));
}
#ifdef __cpp_concepts
template<a_moduloint LEFT, a_moduloint RIGHT>
#else
template<typename LEFT, typename RIGHT, typename=std::enable_if_t<
detail_::is_moduloint_v<LEFT> &&
detail_::is_moduloint_v<RIGHT> &&
std::is_unsigned_v<ULT<LEFT>> &&
std::is_unsigned_v<ULT<RIGHT>>>>
#endif
constexpr auto&
operator&=(LEFT &l, RIGHT r) noexcept
#ifdef __cpp_concepts
requires std::is_unsigned_v<ULT<LEFT>> && std::is_unsigned_v<ULT<RIGHT>>
#endif
{
    static_assert(sizeof(LEFT) == sizeof(RIGHT),"bitand by different sized integer type");
    l = static_cast<LEFT>(l&r);
    return l;
}

#ifdef __cpp_concepts
template<a_moduloint LEFT, a_moduloint RIGHT>
#else
template<typename LEFT, typename RIGHT, typename=std::enable_if_t<
detail_::is_moduloint_v<LEFT> &&
detail_::is_moduloint_v<RIGHT> &&
std::is_unsigned_v<ULT<LEFT>> &&
std::is_unsigned_v<ULT<RIGHT>>>>
#endif
constexpr auto
operator|(LEFT l, RIGHT r) noexcept
#ifdef __cpp_concepts
requires std::is_unsigned_v<ULT<LEFT>> && std::is_unsigned_v<ULT<RIGHT>>
#endif
{
    using result_t=std::conditional_t<sizeof(LEFT)>=sizeof(RIGHT),LEFT,RIGHT>;
    return static_cast<result_t>(promote_keep_signedness(l)|promote_keep_signedness(r));
}
#ifdef __cpp_concepts
template<a_moduloint LEFT, a_moduloint RIGHT>
#else
template<typename LEFT, typename RIGHT, typename=std::enable_if_t<
detail_::is_moduloint_v<LEFT> &&
detail_::is_moduloint_v<RIGHT> &&
std::is_unsigned_v<ULT<LEFT>> &&
std::is_unsigned_v<ULT<RIGHT>>>>
#endif
constexpr auto&
operator|=(LEFT &l, RIGHT r) noexcept
#ifdef __cpp_concepts
requires std::is_unsigned_v<ULT<LEFT>> && std::is_unsigned_v<ULT<RIGHT>>
#endif
{
    static_assert(sizeof(LEFT) == sizeof(RIGHT),"bitor by different sized integer type");
    l = static_cast<LEFT>(l|r);
    return l;
}

#ifdef __cpp_concepts
template<a_moduloint LEFT, a_moduloint RIGHT>
#else
template<typename LEFT, typename RIGHT, typename=std::enable_if_t<
detail_::is_moduloint_v<LEFT> &&
detail_::is_moduloint_v<RIGHT> &&
std::is_unsigned_v<ULT<LEFT>> &&
std::is_unsigned_v<ULT<RIGHT>>>>
#endif
constexpr auto
operator^(LEFT l, RIGHT r) noexcept
#ifdef __cpp_concepts
requires std::is_unsigned_v<ULT<LEFT>> && std::is_unsigned_v<ULT<RIGHT>>
#endif
{
    using result_t=std::conditional_t<sizeof(LEFT)>=sizeof(RIGHT),LEFT,RIGHT>;
    return static_cast<result_t>(promote_keep_signedness(l)^promote_keep_signedness(r));
}
#ifdef __cpp_concepts
template<a_moduloint LEFT, a_moduloint RIGHT>
#else
template<typename LEFT, typename RIGHT, typename=std::enable_if_t<
detail_::is_moduloint_v<LEFT> &&
detail_::is_moduloint_v<RIGHT> &&
std::is_unsigned_v<ULT<LEFT>> &&
std::is_unsigned_v<ULT<RIGHT>>>>
#endif
constexpr auto&
operator^=(LEFT &l, RIGHT r) noexcept
#ifdef __cpp_concepts
requires std::is_unsigned_v<ULT<LEFT>> && std::is_unsigned_v<ULT<RIGHT>>
#endif
{
    static_assert(sizeof(LEFT) == sizeof(RIGHT),"xor by different sized integer type");
    l = static_cast<LEFT>(l^r);
    return l;
}
#ifdef __cpp_concepts
template<a_moduloint LEFT>
#else
template<typename LEFT, typename=std::enable_if_t<
detail_::is_moduloint_v<LEFT> &&
std::is_unsigned_v<ULT<LEFT>> >>
#endif
constexpr LEFT
operator~(LEFT l) noexcept
#ifdef __cpp_concepts
requires std::is_unsigned_v<ULT<LEFT>>
#endif
{
    return static_cast<LEFT>(~promote_keep_signedness(l));
}


#ifdef __cpp_concepts
template<a_moduloint LEFT, a_moduloint RIGHT>
#else
template<typename LEFT, typename RIGHT, typename=std::enable_if_t<
detail_::is_moduloint_v<LEFT> &&
detail_::is_moduloint_v<RIGHT> &&
std::is_unsigned_v<ULT<LEFT>> &&
std::is_unsigned_v<ULT<RIGHT>>>>
#endif
constexpr LEFT
operator<<(LEFT l, RIGHT r)
#ifdef __cpp_concepts
requires std::is_unsigned_v<ULT<LEFT>> && std::is_unsigned_v<ULT<RIGHT>>
#endif
{
    ps_assert(static_cast<size_t>(promote_keep_signedness(r)) < sizeof(LEFT)*CHAR_BIT,
            "pssmoin: trying to shift left by too many bits");
    return static_cast<LEFT>(promote_keep_signedness(l)<<promote_keep_signedness(r));
}
#ifdef __cpp_concepts
template<a_moduloint LEFT, a_moduloint RIGHT>
#else
template<typename LEFT, typename RIGHT, typename=std::enable_if_t<
detail_::is_moduloint_v<LEFT> &&
detail_::is_moduloint_v<RIGHT> &&
std::is_unsigned_v<ULT<LEFT>> &&
std::is_unsigned_v<ULT<RIGHT>>>>
#endif
constexpr auto&
operator<<=(LEFT &l, RIGHT r)
#ifdef __cpp_concepts
requires std::is_unsigned_v<ULT<LEFT>> && std::is_unsigned_v<ULT<RIGHT>>
#endif
{
    l = (l<<r);
    return l;
}
#ifdef __cpp_concepts
template<a_moduloint LEFT, a_moduloint RIGHT>
#else
template<typename LEFT, typename RIGHT, typename=std::enable_if_t<
detail_::is_moduloint_v<LEFT> &&
detail_::is_moduloint_v<RIGHT> &&
std::is_unsigned_v<ULT<LEFT>> &&
std::is_unsigned_v<ULT<RIGHT>>>>
#endif
constexpr LEFT
operator>>(LEFT l, RIGHT r)
#ifdef __cpp_concepts
requires std::is_unsigned_v<ULT<LEFT>> && std::is_unsigned_v<ULT<RIGHT>>
#endif
{
    ps_assert(static_cast<size_t>(promote_keep_signedness(r)) < sizeof(LEFT)*CHAR_BIT,
            "pssmoin: trying to shift right by too many bits");
    return static_cast<LEFT>(promote_keep_signedness(l)>>promote_keep_signedness(r));
}
#ifdef __cpp_concepts
template<a_moduloint LEFT, a_moduloint RIGHT>
#else
template<typename LEFT, typename RIGHT, typename=std::enable_if_t<
detail_::is_moduloint_v<LEFT> &&
detail_::is_moduloint_v<RIGHT> &&
std::is_unsigned_v<ULT<LEFT>> &&
std::is_unsigned_v<ULT<RIGHT>>>>
#endif
constexpr auto&
operator>>=(LEFT &l, RIGHT r)
#ifdef __cpp_concepts
requires std::is_unsigned_v<ULT<LEFT>> && std::is_unsigned_v<ULT<RIGHT>>
#endif
{
    l = (l>>r);
    return l;
}

#ifdef __cpp_concepts
template<a_moduloint LEFT>
#else
template<typename LEFT, typename=std::enable_if_t<
detail_::is_moduloint_v<LEFT>  >>
#endif
std::ostream& operator<<(std::ostream &out, LEFT  value){
    out << promote_keep_signedness(value);
    return out;
}

#ifndef __cpp_concepts
namespace detail_{
template<typename type, typename=std::enable_if_t<pssmoin::detail_::is_moduloint_v<type>> >
  struct numeric_limits
  {
    using ult = pssmoin::detail_::ULT<type>;
    static constexpr bool is_specialized = true;

    static constexpr type
    min() noexcept { return type{std::numeric_limits<ult>::min()}; }

    static constexpr type
    max() noexcept { return type{std::numeric_limits<ult>::max()}; }

    static constexpr type
    lowest() noexcept { return type{std::numeric_limits<ult>::lowest()}; }

    static constexpr int digits = std::numeric_limits<ult>::digits;
    static constexpr int digits10 = std::numeric_limits<ult>::digits10;
    static constexpr int max_digits10 = std::numeric_limits<ult>::max_digits10;
    static constexpr bool is_signed = std::numeric_limits<ult>::is_signed;
    static constexpr bool is_integer = std::numeric_limits<ult>::is_integer;
    static constexpr bool is_exact = std::numeric_limits<ult>::is_exact;
    static constexpr int radix = std::numeric_limits<ult>::radix;

    static constexpr type
    epsilon() noexcept {  return type{std::numeric_limits<ult>::epsilon()}; }

    static constexpr type
    round_error() noexcept {  return type{std::numeric_limits<ult>::round_error()}; }

    static constexpr int min_exponent = std::numeric_limits<ult>::min_exponent;
    static constexpr int min_exponent10 = std::numeric_limits<ult>::min_exponent10;
    static constexpr int max_exponent = std::numeric_limits<ult>::max_exponent;
    static constexpr int max_exponent10 = std::numeric_limits<ult>::max_exponent10;

    static constexpr bool has_infinity = std::numeric_limits<ult>::has_infinity;
    static constexpr bool has_quiet_NaN = std::numeric_limits<ult>::has_quiet_NaN;
    static constexpr bool has_signaling_NaN = std::numeric_limits<ult>::has_signaling_NaN;
    static constexpr std::float_denorm_style has_denorm
     = std::numeric_limits<ult>::has_denorm;
    static constexpr bool has_denorm_loss = std::numeric_limits<ult>::has_denorm_loss;

    static constexpr type
    infinity() noexcept { return type{std::numeric_limits<ult>::infinity()}; }

    static constexpr type
    quiet_NaN() noexcept { return type{std::numeric_limits<ult>::quiet_NaN()}; }

    static constexpr type
    signaling_NaN() noexcept
    { return type{std::numeric_limits<ult>::signaling_NaN()}; }

    static constexpr type
    denorm_min() noexcept
    { return type{std::numeric_limits<ult>::denorm_min()}; }


    static constexpr bool is_iec559 =  std::numeric_limits<ult>::is_iec559;
    static constexpr bool is_bounded =  std::numeric_limits<ult>::is_bounded;
    static constexpr bool is_modulo =  true;

    static constexpr bool traps = false;
    static constexpr bool tinyness_before =  std::numeric_limits<ult>::tinyness_before;
    static constexpr std::float_round_style round_style =  std::numeric_limits<ult>::round_style;
  };


}
#endif

}
// provide std::numeric_limits
namespace std {

#ifdef __cpp_concepts
template<pssmoin::a_moduloint type>
  struct numeric_limits<type>
  {
    using ult = pssmoin::ULT<type>;
    static constexpr bool is_specialized = true;

    static constexpr type
    min() noexcept { return type{numeric_limits<ult>::min()}; }

    static constexpr type
    max() noexcept { return type{numeric_limits<ult>::max()}; }

    static constexpr type
    lowest() noexcept { return type{numeric_limits<ult>::lowest()}; }

    static constexpr int digits = numeric_limits<ult>::digits;
    static constexpr int digits10 = numeric_limits<ult>::digits10;
    static constexpr int max_digits10 = numeric_limits<ult>::max_digits10;
    static constexpr bool is_signed = numeric_limits<ult>::is_signed;
    static constexpr bool is_integer = numeric_limits<ult>::is_integer;
    static constexpr bool is_exact = numeric_limits<ult>::is_exact;
    static constexpr int radix = numeric_limits<ult>::radix;

    static constexpr type
    epsilon() noexcept {  return type{numeric_limits<ult>::epsilon()}; }

    static constexpr type
    round_error() noexcept {  return type{numeric_limits<ult>::round_error()}; }

    static constexpr int min_exponent = numeric_limits<ult>::min_exponent;
    static constexpr int min_exponent10 = numeric_limits<ult>::min_exponent10;
    static constexpr int max_exponent = numeric_limits<ult>::max_exponent;
    static constexpr int max_exponent10 = numeric_limits<ult>::max_exponent10;

    static constexpr bool has_infinity = numeric_limits<ult>::has_infinity;
    static constexpr bool has_quiet_NaN = numeric_limits<ult>::has_quiet_NaN;
    static constexpr bool has_signaling_NaN = numeric_limits<ult>::has_signaling_NaN;
    static constexpr float_denorm_style has_denorm
     = numeric_limits<ult>::has_denorm;
    static constexpr bool has_denorm_loss = numeric_limits<ult>::has_denorm_loss;

    static constexpr type
    infinity() noexcept { return type{numeric_limits<ult>::infinity()}; }

    static constexpr type
    quiet_NaN() noexcept { return type{numeric_limits<ult>::quiet_NaN()}; }

    static constexpr type
    signaling_NaN() noexcept
    { return type{numeric_limits<ult>::signaling_NaN()}; }

    static constexpr type
    denorm_min() noexcept
    { return type{numeric_limits<ult>::denorm_min()}; }

    static constexpr bool is_iec559 =  numeric_limits<ult>::is_iec559;
    static constexpr bool is_bounded =  numeric_limits<ult>::is_bounded;
    static constexpr bool is_modulo =  true;

    static constexpr bool traps = false;
    static constexpr bool tinyness_before =  numeric_limits<ult>::tinyness_before;
    static constexpr float_round_style round_style =  numeric_limits<ult>::round_style;
  };
#else
template<>
struct numeric_limits<pssmoin::si8>: pssmoin::detail_::numeric_limits<pssmoin::si8>{};
template<>
struct numeric_limits<pssmoin::si16>: pssmoin::detail_::numeric_limits<pssmoin::si16>{};
template<>
struct numeric_limits<pssmoin::si32>: pssmoin::detail_::numeric_limits<pssmoin::si32>{};
template<>
struct numeric_limits<pssmoin::si64>: pssmoin::detail_::numeric_limits<pssmoin::si64>{};
template<>
struct numeric_limits<pssmoin::ui8>: pssmoin::detail_::numeric_limits<pssmoin::ui8>{};
template<>
struct numeric_limits<pssmoin::ui16>: pssmoin::detail_::numeric_limits<pssmoin::ui16>{};
template<>
struct numeric_limits<pssmoin::ui32>: pssmoin::detail_::numeric_limits<pssmoin::ui32>{};
template<>
struct numeric_limits<pssmoin::ui64>: pssmoin::detail_::numeric_limits<pssmoin::ui64>{};

#endif
}
#undef ps_assert

#endif /* SRC_PSSSAFEINT_ */
