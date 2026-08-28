#ifndef SRC_MOINS_H_
#define SRC_MOINS_H_


#include <cstdint>
#include <type_traits>
#include <iosfwd>
#include <limits>
#include <climits>
#include <compare>
#ifdef __cpp_concepts
#include <concepts>
#endif

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
   if (not (cond)) { PSSMOIN_RAISE_SIGFPE() ; throw(msg); } ;


// no NDEBUG-like support, because we want safety!


namespace moins { // SATuration Integral Numbers with struct
namespace detail_ {
#if __cplusplus >= 202002L
template<typename T>
using plain = std::remove_cvref_t<T>;
#else
template<typename T>
using plain = std::remove_cv_t<std::remove_reference_t<T>>;
#endif

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

template<typename TESTED,typename=void>
constexpr bool
is_known_integer_v =false;
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
// deliberately not sized_integer, because of bool and characters!
#ifdef __cpp_concepts
template<typename TESTED>
concept sized_integer = detail_::is_known_integer_v<TESTED>;
#endif


#ifdef __cpp_concepts
template<sized_integer INT>
#else
template<typename INT, typename=std::enable_if_t<detail_::is_known_integer_v<INT>>>
#endif
struct [[nodiscard]] Moin;

namespace detail_{
template<typename T, typename=void>
constexpr bool is_moduloint_v = false;
#ifdef __cpp_concepts
template<sized_integer INT>
constexpr bool is_moduloint_v<Moin<INT>> = true;
#else
template<typename INT, typename VOID=std::enable_if_t<detail_::is_known_integer_v<INT>>>
constexpr bool is_moduloint_v<Moin<INT,VOID>> = true;
#endif

template<typename C>
struct ULT_impl;
template<typename I>
struct ULT_impl<Moin<I>>{
    using type=I;
};
} // NS detail_
#ifdef __cpp_concepts
template<typename E>
concept a_moduloint = detail_::is_moduloint_v<E>;
#endif

template<typename C>
using ULT=typename detail_::ULT_impl<detail_::plain<C>>::type;


template<typename E>
using promoted_t = // will promote the underlying type keeping signedness
        std::conditional_t<(sizeof(ULT<E>) < sizeof(int))
            , std::conditional_t<std::is_unsigned_v<ULT<E>>
                , unsigned
                , int >
            , ULT<E>>;
#ifdef __cpp_concept
template<a_moduloint E>
#else
template<typename E, typename=std::enable_if_t<detail_::is_moduloint_v<E>>>
#endif
[[nodiscard]]
constexpr auto
promote_keep_signedness(E value) noexcept
{ // promote keeping signedness
    return static_cast<promoted_t<E>>((value.value_which_should_not_be_referred_to_from_user_code));// promote with sign extension
}
namespace detail_ {


#ifdef __cpp_concept
template<a_moduloint E>
#else
template<typename E, typename=std::enable_if_t<detail_::is_moduloint_v<E>>>
#endif
[[nodiscard]]
constexpr auto
promote_to_unsigned(E value) noexcept
{ // promote to unsigned for wrap around arithmetic
    using u_result_t = std::make_unsigned_t<promoted_t<E>>;
    return static_cast<u_result_t>(promote_keep_signedness(value));
}
#ifdef __cpp_concept
template<sized_integer TARGET, a_moduloint E>
#else
template<typename TARGET, typename E, typename=std::enable_if_t<detail_::is_known_integer_v<TARGET>&&detail_::is_moduloint_v<E> >>
#endif
[[nodiscard]]
constexpr auto
promote_and_extend_to_unsigned(E value) noexcept
{ // promote to unsigned for wrap around arithmetic, with sign extension if needed
       using u_result_t = std::conditional_t< (sizeof(TARGET) > sizeof(promoted_t<E>)),
                std::make_unsigned_t<TARGET>, std::make_unsigned_t<promoted_t<E> > >;
       using s_result_t = std::make_signed_t<u_result_t>;
       return static_cast<u_result_t>(static_cast<s_result_t>(promote_keep_signedness(value)));// promote with sign extension
}

#ifdef __cpp_concept
template<sized_integer TARGET, a_moduloint E>
#else
template<typename TARGET, typename E, typename=std::enable_if_t<detail_::is_known_integer_v<TARGET>&&std::numeric_limits<TARGET>::is_signed&&detail_::is_moduloint_v<E> >>
#endif
[[nodiscard]]
constexpr auto
abs_promoted_and_extended_as_unsigned(E val) noexcept
#ifdef __cpp_concept
requires (std::numeric_limits<TARGET>::is_signed)
#endif
{ // promote to unsigned for wrap around arithmetic removing sign if negative
  // return just the bits for std::numeric_limits<TARGET>::min()
       using promoted_t = promoted_t<E>;
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

#ifdef __cpp_concepts
template<typename LEFT, typename RIGHT>
concept same_signedness = same_signedness_v<LEFT,RIGHT>;
#endif

// the following are not really needed for class types,
// because of the availability of constructors, kept for symmetry

// path tests are compile-time checked:
#ifdef __cpp_concepts
template<a_moduloint TO, sized_integer FROM>
#else
template<typename TO, typename FROM, typename=std::enable_if_t<detail_::is_moduloint_v<TO> && detail_::is_known_integer_v<FROM>>>
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
    return static_cast<result_t>(static_cast<ultr>(val)); // cast is checked above
}

#ifdef __cpp_concepts
[[nodiscard]]
constexpr auto to_underlying(moins::a_moduloint auto v){
#else
template<typename T, typename=std::enable_if_t<moins::detail_::is_moduloint_v<T>>>
[[nodiscard]]
constexpr auto to_underlying(T v){
#endif
    return v.value_which_should_not_be_referred_to_from_user_code;
}

#ifdef __cpp_concepts
template<sized_integer INT>
#else
template<typename INT, typename>
#endif
struct [[nodiscard]] Moin{
    constexpr Moin() noexcept:value_which_should_not_be_referred_to_from_user_code{}{}
#if __cplusplus >= 202002L
    explicit constexpr Moin(std::same_as<INT> auto v) noexcept:value_which_should_not_be_referred_to_from_user_code(v){
    }
    friend constexpr auto operator<=>(Moin, Moin) noexcept = default;
    template<sized_integer FROM>
    explicit constexpr Moin(FROM v)
    requires (not std::same_as<INT,detail_::plain<FROM>>)
    :value_which_should_not_be_referred_to_from_user_code{from_int_to<Moin>(v)}{
    }
#else
    template<typename T, typename=std::enable_if_t<detail_::is_known_integer_v<T> && not std::is_same_v<T,INT>>>
    explicit constexpr Moin(T v) noexcept
    :value_which_should_not_be_referred_to_from_user_code{from_int_to<Moin>(v)}{}
    explicit constexpr Moin(INT v) noexcept
    :value_which_should_not_be_referred_to_from_user_code{v}{}
   friend constexpr bool operator<(Moin left, Moin right) {
        return left.value_which_should_not_be_referred_to_from_user_code < right.value_which_should_not_be_referred_to_from_user_code;
    }
    friend constexpr bool operator>=(Moin left, Moin right) {
        return not (left < right);
    }
    friend constexpr bool operator>(Moin left, Moin right) {
        return right < left;
    }
    friend constexpr bool operator<=(Moin left, Moin right) {
        return not (left > right);
    }
    friend constexpr bool operator==(Moin left, Moin right) {
        return left.value_which_should_not_be_referred_to_from_user_code == right.value_which_should_not_be_referred_to_from_user_code;
    }
    friend constexpr bool operator!=(Moin left, Moin right) {
        return not (left==right);
    }
#endif

    // member/friend operators
    explicit constexpr operator INT() const noexcept { return value_which_should_not_be_referred_to_from_user_code;}

    // negation for signed types only, two's complement
    template <typename T=INT>
    constexpr
    auto
    operator-() const
#ifdef __cpp_concepts
    requires std::numeric_limits<INT>::is_signed
#else
    -> std::enable_if_t<std::numeric_limits<T>::is_signed,Moin>
#endif
    {
        //static_assert(std::numeric_limits<INT>::is_signed, "negation only available for signed types");

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

#ifdef __cpp_concepts
    template<a_moduloint RIGHT>
#else
    template<typename RIGHT, typename=std::enable_if_t<detail_::is_moduloint_v<RIGHT> && same_signedness_v<Moin,RIGHT>>>
#endif
    friend constexpr auto
    operator+(Moin l, RIGHT r)
#ifdef __cpp_concepts
    requires same_signedness<Moin,RIGHT>
#endif
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


#ifdef __cpp_concepts
    template<a_moduloint RIGHT>
#else
    template<typename RIGHT, typename=std::enable_if_t<detail_::is_moduloint_v<RIGHT> && same_signedness_v<Moin,RIGHT>>>
#endif
    constexpr auto&
    operator+=(RIGHT r)  &
#ifdef __cpp_concepts
    requires same_signedness<Moin,RIGHT>
#endif
    {
        static_assert(sizeof(Moin) >= sizeof(RIGHT),"moins: adding too large integer type");
        *this = static_cast<Moin>(*this+r);
        return *this;
    }

#ifdef __cpp_concepts
    template<a_moduloint RIGHT>
#else
    template<typename RIGHT, typename=std::enable_if_t<detail_::is_moduloint_v<RIGHT> && same_signedness_v<Moin,RIGHT>>>
#endif
    friend constexpr auto
    operator-(Moin l, RIGHT r)
#ifdef __cpp_concepts
    requires same_signedness<Moin,RIGHT>
#endif
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
#ifdef __cpp_concepts
    template<a_moduloint RIGHT>
#else
    template<typename RIGHT, typename=std::enable_if_t<detail_::is_moduloint_v<RIGHT> && same_signedness_v<Moin,RIGHT>>>
#endif
    constexpr auto&
    operator-=(RIGHT r) &
#ifdef __cpp_concepts
    requires same_signedness<Moin,RIGHT>
#endif
    {
        static_assert(sizeof(Moin) >= sizeof(RIGHT),"subtracting too large integer type");
        *this = static_cast<Moin>(*this-r);
        return *this;
    }


#ifdef __cpp_concepts
    template<a_moduloint RIGHT>
#else
    template<typename RIGHT, typename=std::enable_if_t<detail_::is_moduloint_v<RIGHT> && same_signedness_v<Moin,RIGHT>>>
#endif
    friend constexpr auto
    operator*(Moin l, RIGHT r)
#ifdef __cpp_concepts
    requires same_signedness<Moin,RIGHT>
#endif
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
#ifdef __cpp_concepts
    template<sized_integer RIGHT>
#else
    template<typename RIGHT>
#endif
    friend constexpr auto
    operator*(Moin l, RIGHT r)
#ifndef __cpp_concepts
    -> std::enable_if_t<detail_::is_known_integer_v<RIGHT>,Moin>
#endif
    {
        return l * from_int_to<Moin>(r);
    }
#ifdef __cpp_concepts
    template<sized_integer LEFT>
#else
    template<typename LEFT>
#endif
    friend constexpr auto
    operator*(LEFT l, Moin r)
#ifndef __cpp_concepts
    -> std::enable_if_t<detail_::is_known_integer_v<LEFT>,Moin>
#endif
    {
        return from_int_to<Moin>(l) * r;
    }
    
#ifdef __cpp_concepts
    template<a_moduloint RIGHT>
#else
    template<typename RIGHT, typename=std::enable_if_t<detail_::is_moduloint_v<RIGHT> && same_signedness_v<Moin,RIGHT>>>
#endif
    constexpr auto&
    operator*=(RIGHT r) &
#ifdef __cpp_concepts
    requires same_signedness<Moin,RIGHT>
#endif
    {
        static_assert(sizeof(Moin) >= sizeof(RIGHT),"multiplying too large integer type");
        *this = static_cast<Moin>(*this*r);
        return *this;
    }
#ifdef __cpp_concepts
    template<sized_integer RIGHT>
    constexpr auto&
#else
    template<typename RIGHT>
    constexpr auto
#endif
    operator*=(RIGHT r) &
#ifndef __cpp_concepts
    -> std::enable_if_t<detail_::is_known_integer_v<RIGHT>,Moin&>
#endif
    {
        return *this *= from_int_to<Moin>(r);
    }
    
#ifdef __cpp_concepts
    template<a_moduloint RIGHT>
#else
    template<typename RIGHT, typename=std::enable_if_t<detail_::is_moduloint_v<RIGHT> && same_signedness_v<Moin,RIGHT>>>
#endif
    friend constexpr auto
    operator/(Moin const l, RIGHT const r)
#ifdef __cpp_concepts
    requires same_signedness<Moin,RIGHT>
#endif
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
#ifdef __cpp_concepts
    template<sized_integer RIGHT>
#else
    template<typename RIGHT>
#endif
    friend constexpr auto
    operator/(Moin const l, RIGHT const r)
#ifndef __cpp_concepts
    -> std::enable_if_t<detail_::is_known_integer_v<RIGHT>,Moin>
#endif
    {
        return l / from_int_to<Moin>(r);
    }
    // don't provide commuted operation for scalar multiplication!

    
#ifdef __cpp_concepts
    template<a_moduloint RIGHT>
#else
    template<typename RIGHT, typename=std::enable_if_t<detail_::is_moduloint_v<RIGHT> && same_signedness_v<Moin,RIGHT>>>
#endif
    constexpr auto&
    operator/=(RIGHT r) &
#ifdef __cpp_concepts
    requires same_signedness<Moin,RIGHT>
#endif
    {
        static_assert(sizeof(Moin) >= sizeof(RIGHT),"dividing by too large integer type");
        *this = static_cast<Moin>(*this/r);
        return *this;
    }
#ifdef __cpp_concepts
    template<sized_integer RIGHT>
    constexpr auto&
#else
    template<typename RIGHT>
    constexpr auto
#endif
    operator/=(RIGHT r) &
#ifndef __cpp_concepts
    -> std::enable_if_t<detail_::is_known_integer_v<RIGHT>,Moin&>
#endif
    {
        return *this /= from_int_to<Moin>(r);
    }
    
#ifdef __cpp_concepts
    template<a_moduloint RIGHT>
#else
    template<typename RIGHT, typename=std::enable_if_t<detail_::is_moduloint_v<RIGHT> && std::is_unsigned_v<ULT<Moin>> && std::is_unsigned_v<ULT<RIGHT>>>>
#endif
    friend constexpr auto
    operator%(Moin l, RIGHT r)
#ifdef __cpp_concepts
    requires std::is_unsigned_v<ULT<Moin>> && std::is_unsigned_v<ULT<RIGHT>>
#endif
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
#ifdef __cpp_concepts
    template<a_moduloint RIGHT>
#else
    template<typename RIGHT, typename=std::enable_if_t<detail_::is_moduloint_v<RIGHT> && std::is_unsigned_v<ULT<Moin>> && std::is_unsigned_v<ULT<RIGHT>>>>
#endif
    constexpr auto&
    operator%=(RIGHT r) &
#ifdef __cpp_concepts
    requires std::is_unsigned_v<ULT<Moin>> && std::is_unsigned_v<ULT<RIGHT>>
#endif
    {
        static_assert(sizeof(Moin) >= sizeof(RIGHT),"dividing by too large integer type");
        *this = static_cast<Moin>(*this % r);
        return *this;
    }
    // bitwise operators

#ifdef __cpp_concepts
    template<a_moduloint RIGHT>
#else
    template<typename RIGHT, typename=std::enable_if_t<detail_::is_moduloint_v<RIGHT> && std::is_unsigned_v<ULT<Moin>> && std::is_unsigned_v<ULT<RIGHT>>>>
#endif
    friend constexpr auto
    operator&(Moin l, RIGHT r) noexcept
#ifdef __cpp_concepts
    requires std::is_unsigned_v<ULT<Moin>> && std::is_unsigned_v<ULT<RIGHT>>
#endif
    {
        using result_t=std::conditional_t<sizeof(Moin)>=sizeof(RIGHT),Moin,RIGHT>;
        return static_cast<result_t>(promote_keep_signedness(l)&promote_keep_signedness(r));
    }
#ifdef __cpp_concepts
    template<a_moduloint RIGHT>
#else
    template<typename RIGHT, typename=std::enable_if_t<detail_::is_moduloint_v<RIGHT> && std::is_unsigned_v<ULT<Moin>> && std::is_unsigned_v<ULT<RIGHT>>>>
#endif
    constexpr auto&
    operator&=(RIGHT r) & noexcept
#ifdef __cpp_concepts
    requires std::is_unsigned_v<ULT<Moin>> && std::is_unsigned_v<ULT<RIGHT>>
#endif
    {
        static_assert(sizeof(Moin) == sizeof(RIGHT),"bitand by different sized integer type");
        *this = static_cast<Moin>(*this&r);
        return *this;
    }

#ifdef __cpp_concepts
    template<a_moduloint RIGHT>
#else
    template<typename RIGHT, typename=std::enable_if_t<detail_::is_moduloint_v<RIGHT> && std::is_unsigned_v<ULT<Moin>> && std::is_unsigned_v<ULT<RIGHT>>>>
#endif
    friend constexpr auto
    operator|(Moin l, RIGHT r) noexcept
#ifdef __cpp_concepts
    requires std::is_unsigned_v<ULT<Moin>> && std::is_unsigned_v<ULT<RIGHT>>
#endif
    {
        using result_t=std::conditional_t<sizeof(Moin)>=sizeof(RIGHT),Moin,RIGHT>;
        return static_cast<result_t>(promote_keep_signedness(l)|promote_keep_signedness(r));
    }
#ifdef __cpp_concepts
    template<a_moduloint RIGHT>
#else
    template<typename RIGHT, typename=std::enable_if_t<detail_::is_moduloint_v<RIGHT> && std::is_unsigned_v<ULT<Moin>> && std::is_unsigned_v<ULT<RIGHT>>>>
#endif
    constexpr auto&
    operator|=(RIGHT r) & noexcept
#ifdef __cpp_concepts
    requires std::is_unsigned_v<ULT<Moin>> && std::is_unsigned_v<ULT<RIGHT>>
#endif
    {
        static_assert(sizeof(Moin) == sizeof(RIGHT),"bitor by different sized integer type");
        *this = static_cast<Moin>(*this|r);
        return *this;
    }

#ifdef __cpp_concepts
    template<a_moduloint RIGHT>
#else
    template<typename RIGHT, typename=std::enable_if_t<detail_::is_moduloint_v<RIGHT> && std::is_unsigned_v<ULT<Moin>> && std::is_unsigned_v<ULT<RIGHT>>>>
#endif
    friend constexpr auto
    operator^(Moin l, RIGHT r) noexcept
#ifdef __cpp_concepts
    requires std::is_unsigned_v<ULT<Moin>> && std::is_unsigned_v<ULT<RIGHT>>
#endif
    {
        using result_t=std::conditional_t<sizeof(Moin)>=sizeof(RIGHT),Moin,RIGHT>;
        return static_cast<result_t>(promote_keep_signedness(l)^promote_keep_signedness(r));
    }
#ifdef __cpp_concepts
    template<a_moduloint RIGHT>
#else
    template<typename RIGHT, typename=std::enable_if_t<detail_::is_moduloint_v<RIGHT> && std::is_unsigned_v<ULT<Moin>> && std::is_unsigned_v<ULT<RIGHT>>>>
#endif
    constexpr auto&
    operator^=(RIGHT r) & noexcept
#ifdef __cpp_concepts
    requires std::is_unsigned_v<ULT<Moin>> && std::is_unsigned_v<ULT<RIGHT>>
#endif
    {
        static_assert(sizeof(Moin) == sizeof(RIGHT),"xor by different sized integer type");
        *this = static_cast<Moin>(*this^r);
        return *this;
    }

    template<typename ult=ULT<Moin>>
    constexpr
    auto
    operator~() noexcept
#ifdef __cpp_concepts
    requires std::is_unsigned_v<ULT<Moin>>
#else
    ->std::enable_if_t<std::is_unsigned_v<ult>,Moin>
#endif
    {
        return Moin(static_cast<INT>(~ promote_keep_signedness(*this)));
    }


#ifdef __cpp_concepts
    template<a_moduloint RIGHT>
#else
    template<typename RIGHT, typename=std::enable_if_t<detail_::is_moduloint_v<RIGHT> && std::is_unsigned_v<ULT<Moin>> && std::is_unsigned_v<ULT<RIGHT>>>>
#endif
    friend constexpr Moin
    operator<<(Moin l, RIGHT r)
#ifdef __cpp_concepts
    requires std::is_unsigned_v<ULT<Moin>> && std::is_unsigned_v<ULT<RIGHT>>
#endif
    {
        ps_assert( static_cast<size_t>(promote_keep_signedness(r)) < sizeof(Moin)*CHAR_BIT , "moins: trying to left-shift by too many bits");
        return static_cast<Moin>(promote_keep_signedness(l)<<promote_keep_signedness(r));
    }
#ifdef __cpp_concepts
    template<a_moduloint RIGHT>
#else
    template<typename RIGHT, typename=std::enable_if_t<detail_::is_moduloint_v<RIGHT> && std::is_unsigned_v<ULT<Moin>> && std::is_unsigned_v<ULT<RIGHT>>>>
#endif
    constexpr auto&
    operator<<=(RIGHT r) &
#ifdef __cpp_concepts
    requires std::is_unsigned_v<ULT<Moin>> && std::is_unsigned_v<ULT<RIGHT>>
#endif
    {
        *this = (*this<<r);
        return *this;
    }
#ifdef __cpp_concepts
    template<a_moduloint RIGHT>
#else
    template<typename RIGHT, typename=std::enable_if_t<detail_::is_moduloint_v<RIGHT> && std::is_unsigned_v<ULT<Moin>> && std::is_unsigned_v<ULT<RIGHT>>>>
#endif
    friend constexpr Moin
    operator>>(Moin l, RIGHT r)
#ifdef __cpp_concepts
    requires std::is_unsigned_v<ULT<Moin>> && std::is_unsigned_v<ULT<RIGHT>>
#endif
    {
        ps_assert( static_cast<size_t>(promote_keep_signedness(r)) < sizeof(Moin)*CHAR_BIT , "moins: trying to right-shift by too many bits");
        return static_cast<Moin>(promote_keep_signedness(l)>>promote_keep_signedness(r));
    }
#ifdef __cpp_concepts
    template<a_moduloint RIGHT>
#else
    template<typename RIGHT, typename=std::enable_if_t<detail_::is_moduloint_v<RIGHT> && std::is_unsigned_v<ULT<Moin>> && std::is_unsigned_v<ULT<RIGHT>>>>
#endif
    constexpr auto&
    operator>>=( RIGHT r) &
#ifdef __cpp_concepts
    requires std::is_unsigned_v<ULT<Moin>> && std::is_unsigned_v<ULT<RIGHT>>
#endif
    {
        *this = (*this >>r);
        return *this;
    }


    friend std::ostream& operator<<(std::ostream &out, Moin value){
        out << promote_keep_signedness(value);
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

#if __cplusplus == 201703L
#define CONSTEVAL constexpr
#elif __cplusplus >= 202002L
#define CONSTEVAL consteval
#else
#error "requires at least C++17, best C++20"
#endif


inline namespace literals {
CONSTEVAL
auto operator""_ui8(unsigned long long value) {
    if (value <= std::numeric_limits<std::uint8_t>::max()) {
        return ui8(static_cast<std::uint8_t>(value));
    } else {
        throw "integral constant too large"; // trigger compile-time error
    }
}


CONSTEVAL
auto operator""_ui16(unsigned long long value) {
    if (value <= std::numeric_limits<std::uint16_t>::max()) {
        return ui16(static_cast<std::uint16_t>(value));
    } else {
        throw "integral constant too large"; // trigger compile-time error
    }
}


CONSTEVAL
auto operator""_ui32(unsigned long long value) {
    if (value <= std::numeric_limits<uint32_t>::max()) {
        return ui32(static_cast<std::uint32_t>(value));
    } else {
        throw "integral constant too large"; // trigger compile-time error
    }
}


CONSTEVAL
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
CONSTEVAL
auto operator""_si8(unsigned long long value) {
    if (value <= std::numeric_limits<std::int8_t>::max()) {
        return si8(static_cast<int8_t>(value));
    } else {
        throw "integral constant too large"; // trigger compile-time error
    }
}


CONSTEVAL
auto operator""_si16(unsigned long long value) {
    if (value <= std::numeric_limits<int16_t>::max()) {
        return si16(static_cast<int16_t>(value));
    } else {
        throw "integral constant too large"; // trigger compile-time error
    }
}


CONSTEVAL
auto operator""_si32(unsigned long long value) {
    if (value <= std::numeric_limits<int32_t>::max()) {
        return si32(static_cast<int32_t>(value));
    } else {
        throw "integral constant too large"; // trigger compile-time error
    }
}


CONSTEVAL
auto operator""_si64(unsigned long long value) {
    if (value <= std::numeric_limits<int64_t>::max()) {
        return si64(static_cast<int64_t>(value));
    } else {
        throw "integral constant too large"; // trigger compile-time error
    }
}
#undef CONSTEVAL
} // NS literals
#ifdef __cpp_concepts
template<sized_integer T>
#else
template<typename T, typename=std::enable_if_t<detail_::is_known_integer_v<T>>>
#endif
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
#ifndef __cpp_concepts
namespace detail_{
template<typename type, typename=std::enable_if_t<moins::detail_::is_moduloint_v<type>> >
  struct numeric_limits
  {
    using ult = moins::ULT<type>;
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


} // NS moins


// provide std::numeric_limits
namespace std {

#ifdef __cpp_concepts
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
#else
template<>
struct numeric_limits<moins::si8>: moins::detail_::numeric_limits<moins::si8>{};
template<>
struct numeric_limits<moins::si16>: moins::detail_::numeric_limits<moins::si16>{};
template<>
struct numeric_limits<moins::si32>: moins::detail_::numeric_limits<moins::si32>{};
template<>
struct numeric_limits<moins::si64>: moins::detail_::numeric_limits<moins::si64>{};
template<>
struct numeric_limits<moins::ui8>: moins::detail_::numeric_limits<moins::ui8>{};
template<>
struct numeric_limits<moins::ui16>: moins::detail_::numeric_limits<moins::ui16>{};
template<>
struct numeric_limits<moins::ui32>: moins::detail_::numeric_limits<moins::ui32>{};
template<>
struct numeric_limits<moins::ui64>: moins::detail_::numeric_limits<moins::ui64>{};

#endif
}




#endif /* SRC_MOINS_H_ */
