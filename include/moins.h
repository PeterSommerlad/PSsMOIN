#ifndef SRC_MOINS_H_
#define SRC_MOINS_H_


#include <cstdint>
#include <type_traits>
#include <iosfwd>
#include <limits>
#include <climits>
#include <compare>
#include <concepts>

// define to non-zero for self-kill with core dump, testing requires 0 for throw
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
   if (not (cond)) { PSSMOIN_RAISE_SIGFPE() ; throw(#msg); } ;


// no NDEBUG-like support, because we want safety!


namespace moins { // SATuration Integral Numbers with struct
namespace detail_ {
template<typename T>
using plain = std::remove_cvref_t<T>;

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
is_compatible_integer_v = std::is_same_v<TESTED,INT> ||
   (   std::is_integral_v<TESTED>
   && not std::is_same_v<bool,TESTED>
   && not is_chartype_v<TESTED>
   && (std::is_unsigned_v<INT> == std::is_unsigned_v<TESTED>)
   && std::numeric_limits<TESTED>::max() == std::numeric_limits<INT>::max() );

// only support the following sizes:
template<typename TESTED>
constexpr bool
is_known_integer_v =    is_compatible_integer_v<std::uint8_t,  TESTED>
                     || is_compatible_integer_v<std::uint16_t, TESTED>
                     || is_compatible_integer_v<std::uint32_t, TESTED>
                     || is_compatible_integer_v<std::uint64_t, TESTED>
                     || is_compatible_integer_v<std::int8_t,  TESTED>
                     || is_compatible_integer_v<std::int16_t, TESTED>
                     || is_compatible_integer_v<std::int32_t, TESTED>
                     || is_compatible_integer_v<std::int64_t, TESTED>;
}
template<typename TESTED>
concept sized_integer = detail_::is_known_integer_v<TESTED>;
// deliberately not std::integral, because of bool and characters!
namespace detail_ {





} // NS detail_


template<sized_integer INT>
struct Moin;

namespace detail_{
template<typename T>
constexpr bool is_moduloint_v = false;
template<sized_integer I>
constexpr bool is_moduloint_v<Moin<I>> = true;

template<typename C>
struct ULT_impl;
template<typename I>
struct ULT_impl<Moin<I>>{
    using type=I;
};
} // NS detail_
template<typename E>
concept a_moduloint = detail_::is_moduloint_v<E>;

template<typename C>
using ULT=detail_::ULT_impl<detail_::plain<C>>::type;

namespace detail_ {

template<typename E>
using promoted_t = // will promote the underlying type keeping signedness
        std::conditional_t<(sizeof(ULT<E>) < sizeof(int))
            , std::conditional_t<std::is_unsigned_v<ULT<E>>
                , unsigned
                , int >
            , ULT<E>>;

template<a_moduloint E>
[[nodiscard]]
constexpr auto
promote_keep_signedness(E value) noexcept
{ // promote keeping signedness
    return static_cast<promoted_t<E>>((value.value_which_should_not_be_referred_to_from_user_code));// promote with sign extension
}


template<a_moduloint E>
[[nodiscard]]
constexpr auto
promote_to_unsigned(E value) noexcept
{ // promote to unsigned for wrap around arithmetic
    using u_result_t = std::make_unsigned_t<promoted_t<E>>;
    return static_cast<u_result_t>(promote_keep_signedness(value));
}
template<sized_integer TARGET, a_moduloint E>
[[nodiscard]]
constexpr auto
promote_and_extend_to_unsigned(E value) noexcept
{ // promote to unsigned for wrap around arithmetic, with sign extension if needed
       using u_result_t = std::conditional_t< (sizeof(TARGET) > sizeof(promoted_t<E>)),
                std::make_unsigned_t<TARGET>, std::make_unsigned_t<promoted_t<E> > >;
       using s_result_t = std::make_signed_t<u_result_t>;
       return static_cast<u_result_t>(static_cast<s_result_t>(promote_keep_signedness(value)));// promote with sign extension
}

template<sized_integer TARGET, a_moduloint E>
[[nodiscard]]
constexpr auto
abs_promoted_and_extended_as_unsigned(E val) noexcept
 requires (std::numeric_limits<TARGET>::is_signed)
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

} // NS detail_

template<typename LEFT, typename RIGHT>
constexpr bool
same_signedness_v = std::numeric_limits<LEFT>::is_signed == std::numeric_limits<RIGHT>::is_signed;

template<typename LEFT, typename RIGHT>
concept same_signedness = same_signedness_v<LEFT,RIGHT>;



template<sized_integer INT>
struct Moin{
    constexpr Moin() noexcept:value_which_should_not_be_referred_to_from_user_code{}{}
    explicit constexpr Moin(std::same_as<INT> auto v) noexcept:value_which_should_not_be_referred_to_from_user_code(v){
    }
    friend constexpr auto operator<=>(Moin, Moin) noexcept = default;
    explicit constexpr operator INT() const noexcept { return value_which_should_not_be_referred_to_from_user_code;}
    template<std::integral FROM>
    explicit constexpr Moin(FROM v)
    requires (not std::same_as<INT,detail_::plain<FROM>>)
    :value_which_should_not_be_referred_to_from_user_code(v){
        if constexpr(std::is_unsigned_v<INT>){
            ps_assert(  (v >= FROM{} && // in case FROM is signed
                         static_cast<std::make_unsigned_t<FROM>>(v) <= std::numeric_limits<INT>::max()), "moins: integer value out of range") ;
        } else {
            if constexpr (std::is_unsigned_v<FROM>){
                ps_assert(  v <= static_cast<std::make_unsigned_t<INT>>(std::numeric_limits<INT>::max()), "moins: integer value out of range");

            } else { // both are signed
                ps_assert(  (v <= std::numeric_limits<INT>::max() &&
                                    v >= std::numeric_limits<INT>::min()), "moins: integer value out of range");
            }
        }
    }

    // member/friend operators

    // negation for signed types only, two's complement
    constexpr auto
    operator-() const
    requires std::numeric_limits<INT>::is_signed
    {
        return static_cast<Moin>(static_cast<INT>(1u + ~detail_::promote_to_unsigned(*this)));
    }

    // increment/decrement

    constexpr Moin& operator++() &
    {
        return *this =  *this + static_cast<Moin>(1) ;
    }

    constexpr Moin operator++(int) &
    {
        auto result=*this;
        ++*this;
        return result;
    }
    constexpr Moin& operator--() &
    {
        return *this = *this - static_cast<Moin>(1);
    }

    constexpr Moin operator--(int) &
    {
        auto result=*this;
        --*this;
        return result;
    }

    // arithmetic

    template<a_moduloint RIGHT>
    friend constexpr auto
    operator+(Moin l, RIGHT r)
    requires same_signedness<Moin,RIGHT>
    {
        // handle sign extension
        using result_t=std::conditional_t<sizeof(Moin)>=sizeof(RIGHT),Moin,RIGHT>;
        using ult = ULT<result_t>;
        return static_cast<result_t>(
                static_cast<ult>(
                        detail_::promote_and_extend_to_unsigned<ult>(l)
                        + // use unsigned op to prevent signed overflow, but wrap.
                        detail_::promote_and_extend_to_unsigned<ult>(r)
                )
        );
    }


    template<a_moduloint RIGHT>
    constexpr auto&
    operator+=(RIGHT r)  &
    requires same_signedness<Moin,RIGHT>
    {
        static_assert(sizeof(Moin) >= sizeof(RIGHT),"moins: adding too large integer type");
        *this = static_cast<Moin>(*this+r);
        return *this;
    }

    template<a_moduloint RIGHT>
    friend constexpr auto
    operator-(Moin l, RIGHT r)
    requires same_signedness<Moin,RIGHT>
    {
        using result_t=std::conditional_t<sizeof(Moin)>=sizeof(RIGHT),Moin,RIGHT>;
        using ult = ULT<result_t>;

        return static_cast<result_t>(
                static_cast<ult>(
                        detail_::promote_and_extend_to_unsigned<ult>(l)
                        - // use unsigned op to prevent signed overflow, but wrap.
                        detail_::promote_and_extend_to_unsigned<ult>(r)
                )
        );
    }
    template<a_moduloint RIGHT>
    constexpr auto&
    operator-=(RIGHT r) &
    requires same_signedness<Moin,RIGHT>
    {
        static_assert(sizeof(Moin) >= sizeof(RIGHT),"subtracting too large integer type");
        *this = static_cast<Moin>(*this-r);
        return *this;
    }


    template<a_moduloint RIGHT>
    friend constexpr auto
    operator*(Moin l, RIGHT r)
    requires same_signedness<Moin,RIGHT>
    {
        using result_t=std::conditional_t<sizeof(Moin)>=sizeof(RIGHT),Moin,RIGHT>;
        using ult = ULT<result_t>;

        return static_cast<result_t>(
                static_cast<ult>(
                        detail_::promote_and_extend_to_unsigned<ult>(l)
                        * // use unsigned op to prevent signed overflow, but wrap.
                        detail_::promote_and_extend_to_unsigned<ult>(r)
                )
        );
    }
    template<a_moduloint RIGHT>
    constexpr auto&
    operator*=(RIGHT r) &
    requires same_signedness<Moin,RIGHT>
    {
        static_assert(sizeof(Moin) >= sizeof(RIGHT),"multiplying too large integer type");
        *this = static_cast<Moin>(*this*r);
        return *this;
    }
    template<a_moduloint RIGHT>
    friend constexpr auto
    operator/(Moin const l, RIGHT const r)
    requires same_signedness<Moin,RIGHT>
    {
        using result_t=std::conditional_t<sizeof(Moin)>=sizeof(RIGHT),Moin,RIGHT>;
        using ult = ULT<result_t>;
        ps_assert(  r != RIGHT{} , "moins: division by zero");
        if constexpr (std::numeric_limits<result_t>::is_signed){
            bool result_is_negative = (l < Moin{}) != (r < RIGHT{});
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
    template<a_moduloint RIGHT>
    constexpr auto&
    operator/=(RIGHT r) &
    requires same_signedness<Moin,RIGHT>
    {
        static_assert(sizeof(Moin) >= sizeof(RIGHT),"dividing by too large integer type");
        *this = static_cast<Moin>(*this/r);
        return *this;
    }
    template<a_moduloint RIGHT>
    friend constexpr auto
    operator%(Moin l, RIGHT r)
    requires std::is_unsigned_v<ULT<Moin>> && std::is_unsigned_v<ULT<RIGHT>>
    {
        using result_t=std::conditional_t<sizeof(Moin)>=sizeof(RIGHT),Moin,RIGHT>;
        using ult = ULT<result_t>;
        ps_assert(  r != RIGHT{}, "moins: modulo division by zero");
        return static_cast<result_t>(
                static_cast<ult>(
                        detail_::promote_and_extend_to_unsigned<ult>(l)
                        % // unsigned modulo cannot overflow
                        detail_::promote_and_extend_to_unsigned<ult>(r)
                )
        );
    }
    template<a_moduloint RIGHT>
    constexpr auto&
    operator%=(RIGHT r) &
    requires std::is_unsigned_v<ULT<Moin>> && std::is_unsigned_v<ULT<RIGHT>>
    {
        static_assert(sizeof(Moin) >= sizeof(RIGHT),"dividing by too large integer type");
        *this = static_cast<Moin>(*this % r);
        return *this;
    }
    // bitwise operators

    template<a_moduloint RIGHT>
    friend constexpr auto
    operator&(Moin l, RIGHT r) noexcept
    requires std::is_unsigned_v<ULT<Moin>> && std::is_unsigned_v<ULT<RIGHT>>
    {
        using detail_::promote_keep_signedness;

        using result_t=std::conditional_t<sizeof(Moin)>=sizeof(RIGHT),Moin,RIGHT>;
        return static_cast<result_t>(promote_keep_signedness(l)&promote_keep_signedness(r));
    }
    template<a_moduloint RIGHT>
    constexpr auto&
    operator&=(RIGHT r) & noexcept
    requires std::is_unsigned_v<ULT<Moin>> && std::is_unsigned_v<ULT<RIGHT>>
    {
        static_assert(sizeof(Moin) == sizeof(RIGHT),"bitand by different sized integer type");
        *this = static_cast<Moin>(*this&r);
        return *this;
    }

    template<a_moduloint RIGHT>
    friend constexpr auto
    operator|(Moin l, RIGHT r) noexcept
    requires std::is_unsigned_v<ULT<Moin>> && std::is_unsigned_v<ULT<RIGHT>>
    {
        using detail_::promote_keep_signedness;
        using result_t=std::conditional_t<sizeof(Moin)>=sizeof(RIGHT),Moin,RIGHT>;
        return static_cast<result_t>(promote_keep_signedness(l)|promote_keep_signedness(r));
    }
    template<a_moduloint RIGHT>
    constexpr auto&
    operator|=(RIGHT r) & noexcept
    requires std::is_unsigned_v<ULT<Moin>> && std::is_unsigned_v<ULT<RIGHT>>
    {
        static_assert(sizeof(Moin) == sizeof(RIGHT),"bitor by different sized integer type");
        *this = static_cast<Moin>(*this|r);
        return *this;
    }

    template<a_moduloint RIGHT>
    friend constexpr auto
    operator^(Moin l, RIGHT r) noexcept
    requires std::is_unsigned_v<ULT<Moin>> && std::is_unsigned_v<ULT<RIGHT>>
    {
        using detail_::promote_keep_signedness;

        using result_t=std::conditional_t<sizeof(Moin)>=sizeof(RIGHT),Moin,RIGHT>;
        return static_cast<result_t>(promote_keep_signedness(l)^promote_keep_signedness(r));
    }
    template<a_moduloint RIGHT>
    constexpr auto&
    operator^=(RIGHT r) & noexcept
    requires std::is_unsigned_v<ULT<Moin>> && std::is_unsigned_v<ULT<RIGHT>>
    {
        static_assert(sizeof(Moin) == sizeof(RIGHT),"xor by different sized integer type");
        *this = static_cast<Moin>(*this^r);
        return *this;
    }


    constexpr Moin
    operator~() noexcept
    requires std::is_unsigned_v<ULT<Moin>>
    {
        return Moin(static_cast<INT>(~detail_::promote_keep_signedness(*this)));
    }


    template<a_moduloint RIGHT>
    friend constexpr Moin
    operator<<(Moin l, RIGHT r)
    requires std::is_unsigned_v<ULT<Moin>> && std::is_unsigned_v<ULT<RIGHT>>
    {
        using detail_::promote_keep_signedness;
        ps_assert( static_cast<size_t>(promote_keep_signedness(r)) < sizeof(Moin)*CHAR_BIT , "moins: trying to left-shift by too many bits");
        return static_cast<Moin>(promote_keep_signedness(l)<<promote_keep_signedness(r));
    }
    template<a_moduloint RIGHT>
    constexpr auto&
    operator<<=(RIGHT r) &
    requires std::is_unsigned_v<ULT<Moin>> && std::is_unsigned_v<ULT<RIGHT>>
    {
        *this = (*this<<r);
        return *this;
    }
    template<a_moduloint RIGHT>
    friend constexpr Moin
    operator>>(Moin l, RIGHT r)
    requires std::is_unsigned_v<ULT<Moin>> && std::is_unsigned_v<ULT<RIGHT>>
    {   using detail_::promote_keep_signedness;
        ps_assert( static_cast<size_t>(promote_keep_signedness(r)) < sizeof(Moin)*CHAR_BIT , "moins: trying to right-shift by too many bits");
        return static_cast<Moin>(promote_keep_signedness(l)>>promote_keep_signedness(r));
    }
    template<a_moduloint RIGHT>
    constexpr auto&
    operator>>=( RIGHT r) &
    requires std::is_unsigned_v<ULT<Moin>> && std::is_unsigned_v<ULT<RIGHT>>
    {
        *this = (*this >>r);
        return *this;
    }


    friend std::ostream& operator<<(std::ostream &out, Moin value){
        out << detail_::promote_keep_signedness(value);
        return out;
    }
    // no need for private, makes compilability checks possible
    // all possible values of INT are valid
    INT value_which_should_not_be_referred_to_from_user_code;
};
// unsigned

using ui8  = Moin<std::uint8_t >;
using ui16 = Moin<std::uint16_t>;
using ui32 = Moin<std::uint32_t>;
using ui64 = Moin<std::uint64_t>;

inline namespace literals {
consteval
auto operator""_ui8(unsigned long long value) {
    if (value <= std::numeric_limits<std::uint8_t>::max()) {
        return ui8(static_cast<std::uint8_t>(value));
    } else {
        throw "integral constant too large"; // trigger compile-time error
    }
}


consteval
auto operator""_ui16(unsigned long long value) {
    if (value <= std::numeric_limits<std::uint16_t>::max()) {
        return ui16(static_cast<std::uint16_t>(value));
    } else {
        throw "integral constant too large"; // trigger compile-time error
    }
}


consteval
auto operator""_ui32(unsigned long long value) {
    if (value <= std::numeric_limits<uint32_t>::max()) {
        return ui32(static_cast<std::uint32_t>(value));
    } else {
        throw "integral constant too large"; // trigger compile-time error
    }
}


consteval
auto operator""_ui64(unsigned long long value) {
    if constexpr (sizeof(ui64) < sizeof(value)){
        if (value > 0xffff'ffff'fffffffful) {
            throw "integral constant too large"; // trigger compile-time error
        }
    }
    return ui64(static_cast<std::uint64_t>(value));
}

}

// signed
using si8  = Moin<std::int8_t >;
using si16 = Moin<std::int16_t>;
using si32 = Moin<std::int32_t>;
using si64 = Moin<std::int64_t>;


inline namespace literals {
consteval
auto operator""_si8(unsigned long long value) {
    if (value <= std::numeric_limits<std::int8_t>::max()) {
        return si8(static_cast<int8_t>(value));
    } else {
        throw "integral constant too large"; // trigger compile-time error
    }
}


consteval
auto operator""_si16(unsigned long long value) {
    if (value <= std::numeric_limits<int16_t>::max()) {
        return si16(static_cast<int16_t>(value));
    } else {
        throw "integral constant too large"; // trigger compile-time error
    }
}


consteval
auto operator""_si32(unsigned long long value) {
    if (value <= std::numeric_limits<int32_t>::max()) {
        return si32(static_cast<int32_t>(value));
    } else {
        throw "integral constant too large"; // trigger compile-time error
    }
}


consteval
auto operator""_si64(unsigned long long value) {
    if (value <= std::numeric_limits<int64_t>::max()) {
        return si64(static_cast<int64_t>(value));
    } else {
        throw "integral constant too large"; // trigger compile-time error
    }
}
} // NS literals

// the following are not really needed for class types,
// because of the availability of constructors, kept for symmetry

template<sized_integer T>
[[nodiscard]]
constexpr auto
from_int(T value) noexcept {
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
    return static_cast<result_t>(value); // no need to check, result_t corresponds to input T's range
}
// path tests are compile-time checked:
template<a_moduloint TO, sized_integer FROM>
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

} // NS moins


// provide std::numeric_limits
namespace std {

template<moins::a_moduloint type>
  struct numeric_limits<type>
  {
    using ult = moins::ULT<type>;
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
    static constexpr bool is_modulo =  true; // modulo arithmetic
    static constexpr bool traps = false; // modulo arithmetic never traps
    static constexpr bool tinyness_before =  numeric_limits<ult>::tinyness_before;
    static constexpr float_round_style round_style =  numeric_limits<ult>::round_style;
  };

}




#endif /* SRC_MOINS_H_ */
