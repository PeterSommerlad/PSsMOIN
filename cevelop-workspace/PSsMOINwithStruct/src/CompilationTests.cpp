#include "moins.h"
#include <cstddef>
using namespace moins::literals;
static_assert(0x8000_ui16 == 32768_ui16);
template<auto ...value>
using consume_value = void;
using namespace moins;

namespace _testing {

namespace compile_checks {



#define concat_line_impl(A, B) A##_##B
#define concat_line(A, B) concat_line_impl(A,B)

#define check_expr_does_compile(NOT, FROM, expr) \
namespace concat_line(NOT##_test, __LINE__) { \
        template<typename T, typename=void>\
        constexpr bool\
        expression_compiles{false};\
template<typename T> \
constexpr bool \
expression_compiles<T, consume_value< (expr) > > {true};\
static_assert(NOT expression_compiles<FROM>, "should " #NOT " compile: " #expr );\
} // namespace tag

#define check_does_compile(NOT, FROM, OPER) check_expr_does_compile(NOT, FROM, (T{} OPER T{}))


// need to be on separate lines for disambiguation
check_does_compile(not,  si8 , <<  )
check_does_compile(   ,  ui8 , << )
check_does_compile(not,  si8 , >>  )
check_does_compile(   ,  ui8 , >> )
check_does_compile(not,  ui8 , + (1_ui8 << 010_ui8) + ) // too wide shift
check_does_compile(   ,  ui8 , + (1_ui8 << 7_ui8) + ) // not too wide shift
check_does_compile(not,  ui8 , + (0x80_ui8 >> 010_ui8) + ) // too wide shift
check_does_compile(   ,  ui8 , + (0x80_ui8 >> 7_ui8) + ) // not too wide shift
check_does_compile(not,  ui8 ,  % ) // modulo 0
check_does_compile(not,  si8 ,  / ) // div 0
check_does_compile(not,  si8 ,  % ) // modulo not working
check_does_compile(not,  ui8 ,  / ) // div 0
check_expr_does_compile(   ,  ui8 , ( 1_ui8  / 1_ui8) ) // div
check_expr_does_compile(   ,  ui8 , ( 11_ui8  % 3_ui8) ) // mod
check_does_compile(not,  ui8 , + 1_si8 + ) // mixed
check_does_compile(   ,  ui8 , + 255_ui8 + 1_ui8 + ) // wrap

check_does_compile(not, si8, + from_int_to<si8>(128) + ) // cannot cast too large value
check_does_compile(   , si8, + from_int_to<si8>(127) + ) // cannot cast too large value

}

template<typename FROM, typename=void>
constexpr bool
from_int_compiles=false;

template<typename FROM>
constexpr bool
from_int_compiles<FROM,std::void_t<decltype(from_int(FROM{}))>> = true;

static_assert(from_int_compiles<unsigned char>);
static_assert(from_int_compiles<signed char>);
static_assert(from_int_compiles<short>);
static_assert(from_int_compiles<unsigned short>);
static_assert(from_int_compiles<int>);
static_assert(from_int_compiles<unsigned>);
static_assert(from_int_compiles<long>);
static_assert(from_int_compiles<unsigned long>);
static_assert(from_int_compiles<long long>);
static_assert(from_int_compiles<unsigned long long>);
static_assert(from_int_compiles<std::uint8_t>);
static_assert(from_int_compiles<std::uint16_t>);
static_assert(from_int_compiles<std::uint32_t>);
static_assert(from_int_compiles<std::uint64_t>);
static_assert(from_int_compiles<std::int8_t>);
static_assert(from_int_compiles<std::int16_t>);
static_assert(from_int_compiles<std::int32_t>);
static_assert(from_int_compiles<std::int64_t>);


static_assert(! from_int_compiles<bool>);
static_assert(! from_int_compiles<char>);
#ifdef __cpp_char8_t
static_assert(! from_int_compiles<char8_t>);
#endif
static_assert(! from_int_compiles<wchar_t>);
static_assert(! from_int_compiles<char16_t>);
static_assert(! from_int_compiles<char32_t>);


static_assert(sizeof(long) == sizeof(long long)); // on my mac...
static_assert(42_si64 == from_int(42L));
static_assert(42_si64 == from_int(42LL));
static_assert(42_si32 == from_int(42));
static_assert(42_ui64 == from_int(42uL));
static_assert(42_ui64 == from_int(42uLL));
static_assert(42_ui32 == from_int(42u));



static_assert(detail_::is_moduloint_v<ui8>);
static_assert(detail_::is_moduloint_v<ui16>);
static_assert(detail_::is_moduloint_v<ui32>);
static_assert(detail_::is_moduloint_v<ui64>);
static_assert(detail_::is_moduloint_v<si8>);
static_assert(detail_::is_moduloint_v<si16>);
static_assert(detail_::is_moduloint_v<si32>);
static_assert(detail_::is_moduloint_v<si64>);
enum class enum4test{};
static_assert(!detail_::is_moduloint_v<enum4test>);
static_assert(!detail_::is_moduloint_v<std::byte>);
static_assert(!detail_::is_moduloint_v<int>);
static_assert(std::is_same_v<unsigned,decltype(promote_keep_signedness(1_ui8)+1)>);
static_assert(std::is_same_v<unsigned,decltype(promote_keep_signedness(2_ui16)+1)>);
static_assert(std::is_same_v<int,decltype(promote_keep_signedness(1_si8))>);
static_assert(std::is_same_v<int,decltype(promote_keep_signedness(2_si16))>);
static_assert(std::is_same_v<uint8_t,ULT<ui8>>);
static_assert(std::is_same_v<uint16_t,ULT<ui16>>);


static_assert(promote_keep_signedness(0xffff_ui16 * 0xffff_ui16) == 0x1u); // wraps
static_assert(0xff_ui8 * 0xff_ui8 == 1_ui8);

// the following does not compile due to signed integer overflow on 32bit int
//static_assert(static_cast<uint16_t>(0xffffu)* static_cast<uint16_t>(0xffffu));

static_assert(0x7fff'ffff_si32+2_si32 == -0x7fff'ffff_si32);
//static_assert(0x7fff'ffff + 2); // doesn't compile, integer overflow
static_assert(-0x7fff'ffff_si32 - 2_si32 == 0x7fff'ffff_si32);
//static_assert(-0x7fff'ffff - 2); // doesn't compile, integer overflow
static_assert(std::is_same_v<int,decltype(+to_underlying(42_ui8))>);
static_assert(std::is_same_v<uint8_t,decltype(to_underlying(42_ui8))>);

// doesn't work directly, because concept check in constrained template from_int causes hard failure
namespace _testing {
namespace compile_checks {
constexpr auto from_int(auto b){return b;} // provide overload to make concept check not be a hard error
check_expr_does_compile(not , ui8,(T(32) == from_int(' ')) ) // expression must depend on template parameter T
check_expr_does_compile(not , ui8,(T(32) == from_int(u' ')) ) // expression must depend on template parameter T
check_expr_does_compile(not , ui8,(T(32) == from_int(U' ')) ) // expression must depend on template parameter T
check_expr_does_compile(not , ui8,(T(32) == from_int(L' ')) ) // expression must depend on template parameter T
}}
static_assert(1_ui8 == from_int(uint8_t(1)));
static_assert(42_si8 == from_int_to<si8>(42));
//static_assert(32_ui8 == from_int(' ')); // does not compile
//static_assert(32_ui8 == from_int(u' ')); // does not compile
//static_assert(32_ui8 == from_int(U' ')); // does not compile
//static_assert(32_ui8 == from_int(L' ')); // does not compile
//static_assert(1_ui8 == from_int_to<ui8>(true)); // does not compile

// Test case from Frank Buergel (extended):
// missing sign extension from 32 to 64bit
// discovered further bugs wrt signed division
namespace {
constexpr auto v1_64 {  1_si64 };
constexpr auto v1_32 {  1_si32 };
constexpr auto v1_16 {  1_si16 };
constexpr auto v1_8  {  1_si8 };
constexpr auto v2_64 {  2_si64 };
constexpr auto v2_32 {  2_si32 };
constexpr auto v2_16 {  2_si16 };
constexpr auto v2_8  {  2_si8 };
constexpr auto vminus1_64 { -1_si64 };
constexpr auto vminus1_32 { -1_si32 };
constexpr auto vminus1_16 { -1_si16 };
constexpr auto vminus1_8  { -1_si8 };

constexpr auto min_8  { std::numeric_limits<si8 >::min() };
constexpr auto min_16 { std::numeric_limits<si16>::min() };
constexpr auto min_32 { std::numeric_limits<si32>::min() };
constexpr auto min_64 { std::numeric_limits<si64>::min() };
constexpr auto max_8  { std::numeric_limits<si8 >::max() };
constexpr auto max_16 { std::numeric_limits<si16>::max() };
constexpr auto max_32 { std::numeric_limits<si32>::max() };
constexpr auto max_64 { std::numeric_limits<si64>::max() };

}

static_assert(v1_64 + vminus1_64 == 0_si64 );
static_assert(v1_64 + vminus1_32 == 0_si64 );
static_assert(v1_64 + vminus1_16 == 0_si64 );
static_assert(v1_64 + vminus1_8  == 0_si64 );
static_assert(v1_32 + vminus1_64 == 0_si64 );
static_assert(v1_32 + vminus1_32 == 0_si32 );
static_assert(v1_32 + vminus1_16 == 0_si32 );
static_assert(v1_32 + vminus1_8  == 0_si32 );
static_assert(v1_16 + vminus1_64 == 0_si64 );
static_assert(v1_16 + vminus1_32 == 0_si32 );
static_assert(v1_16 + vminus1_16 == 0_si16 );
static_assert(v1_16 + vminus1_8  == 0_si16 );
static_assert(v1_8  + vminus1_64 == 0_si64 );
static_assert(v1_8  + vminus1_32 == 0_si32 );
static_assert(v1_8  + vminus1_16 == 0_si16 );
static_assert(v1_8  + vminus1_8  == 0_si8 );

static_assert(vminus1_64 +  v1_64== 0_si64 );
static_assert(vminus1_32 +  v1_64== 0_si64 );
static_assert(vminus1_16 +  v1_64== 0_si64 );
static_assert(vminus1_8  +  v1_64== 0_si64 );
static_assert(vminus1_64 +  v1_32== 0_si64 );
static_assert(vminus1_32 +  v1_32== 0_si32 );
static_assert(vminus1_16 +  v1_32== 0_si32 );
static_assert(vminus1_8  +  v1_32== 0_si32 );
static_assert(vminus1_64 +  v1_16== 0_si64 );
static_assert(vminus1_32 +  v1_16== 0_si32 );
static_assert(vminus1_16 +  v1_16== 0_si16 );
static_assert(vminus1_8  +  v1_16== 0_si16 );
static_assert(vminus1_64 +  v1_8 == 0_si64 );
static_assert(vminus1_32 +  v1_8 == 0_si32 );
static_assert(vminus1_16 +  v1_8 == 0_si16 );
static_assert(vminus1_8  +  v1_8 == 0_si8 );


// demonstrate wrapping:
static_assert(max_64 + v1_64 == min_64 );
static_assert(max_64 + v1_32 == min_64 );
static_assert(max_64 + v1_16 == min_64 );
static_assert(max_64 + v1_8  == min_64 );
static_assert(max_32 + v1_64 == 0x8000'0000_si64 );
static_assert(max_32 + v1_32 == min_32 );
static_assert(max_32 + v1_16 == min_32 );
static_assert(max_32 + v1_8  == min_32 );
static_assert(max_16 + v1_64 == 0x8000_si64 );
static_assert(max_16 + v1_32 == 0x8000_si32 );
static_assert(max_16 + v1_16 == min_16 );
static_assert(max_16 + v1_8  == min_16 );
static_assert(max_8  + v1_64 == 128_si64 );
static_assert(max_8  + v1_32 == 128_si32 );
static_assert(max_8  + v1_16 == 128_si16 );
static_assert(max_8  + v1_8  == min_8 );


static_assert(v1_64 - vminus1_64 == 2_si64 );
static_assert(v1_64 - vminus1_32 == 2_si64 );
static_assert(v1_64 - vminus1_16 == 2_si64 );
static_assert(v1_64 - vminus1_8  == 2_si64 );
static_assert(v1_32 - vminus1_64 == 2_si64 );
static_assert(v1_32 - vminus1_32 == 2_si32 );
static_assert(v1_32 - vminus1_16 == 2_si32 );
static_assert(v1_32 - vminus1_8  == 2_si32 );
static_assert(v1_16 - vminus1_64 == 2_si64 );
static_assert(v1_16 - vminus1_32 == 2_si32 );
static_assert(v1_16 - vminus1_16 == 2_si16 );
static_assert(v1_16 - vminus1_8  == 2_si16 );
static_assert(v1_8  - vminus1_64 == 2_si64 );
static_assert(v1_8  - vminus1_32 == 2_si32 );
static_assert(v1_8  - vminus1_16 == 2_si16 );
static_assert(v1_8  - vminus1_8  == 2_si8 );


static_assert(vminus1_64 - v1_64  == -2_si64 );
static_assert(vminus1_32 - v1_64  == -2_si64 );
static_assert(vminus1_16 - v1_64  == -2_si64 );
static_assert(vminus1_8  - v1_64  == -2_si64 );
static_assert(vminus1_64 - v1_32  == -2_si64 );
static_assert(vminus1_32 - v1_32  == -2_si32 );
static_assert(vminus1_16 - v1_32  == -2_si32 );
static_assert(vminus1_8  - v1_32  == -2_si32 );
static_assert(vminus1_64 - v1_16  == -2_si64 );
static_assert(vminus1_32 - v1_16  == -2_si32 );
static_assert(vminus1_16 - v1_16  == -2_si16 );
static_assert(vminus1_8  - v1_16  == -2_si16 );
static_assert(vminus1_64 - v1_8   == -2_si64 );
static_assert(vminus1_32 - v1_8   == -2_si32 );
static_assert(vminus1_16 - v1_8   == -2_si16 );
static_assert(vminus1_8  - v1_8   == -2_si8 );

static_assert(v1_64 * vminus1_64 == -1_si64 );
static_assert(v1_64 * vminus1_32 == -1_si64 );
static_assert(v1_64 * vminus1_16 == -1_si64 );
static_assert(v1_64 * vminus1_8  == -1_si64 );
static_assert(v1_32 * vminus1_64 == -1_si64 );
static_assert(v1_32 * vminus1_32 == -1_si32 );
static_assert(v1_32 * vminus1_16 == -1_si32 );
static_assert(v1_32 * vminus1_8  == -1_si32 );
static_assert(v1_16 * vminus1_64 == -1_si64 );
static_assert(v1_16 * vminus1_32 == -1_si32 );
static_assert(v1_16 * vminus1_16 == -1_si16 );
static_assert(v1_16 * vminus1_8  == -1_si16 );
static_assert(v1_8  * vminus1_64 == -1_si64 );
static_assert(v1_8  * vminus1_32 == -1_si32 );
static_assert(v1_8  * vminus1_16 == -1_si16 );
static_assert(v1_8  * vminus1_8  == -1_si8 );

static_assert(vminus1_64 *  v1_64== -1_si64 );
static_assert(vminus1_32 *  v1_64== -1_si64 );
static_assert(vminus1_16 *  v1_64== -1_si64 );
static_assert(vminus1_8  *  v1_64== -1_si64 );
static_assert(vminus1_64 *  v1_32== -1_si64 );
static_assert(vminus1_32 *  v1_32== -1_si32 );
static_assert(vminus1_16 *  v1_32== -1_si32 );
static_assert(vminus1_8  *  v1_32== -1_si32 );
static_assert(vminus1_64 *  v1_16== -1_si64 );
static_assert(vminus1_32 *  v1_16== -1_si32 );
static_assert(vminus1_16 *  v1_16== -1_si16 );
static_assert(vminus1_8  *  v1_16== -1_si16 );
static_assert(vminus1_64 *  v1_8 == -1_si64 );
static_assert(vminus1_32 *  v1_8 == -1_si32 );
static_assert(vminus1_16 *  v1_8 == -1_si16 );
static_assert(vminus1_8  *  v1_8 == -1_si8 );

static_assert(vminus1_64 *  v2_64== -2_si64 );
static_assert(vminus1_32 *  v2_64== -2_si64 );
static_assert(vminus1_16 *  v2_64== -2_si64 );
static_assert(vminus1_8  *  v2_64== -2_si64 );
static_assert(vminus1_64 *  v2_32== -2_si64 );
static_assert(vminus1_32 *  v2_32== -2_si32 );
static_assert(vminus1_16 *  v2_32== -2_si32 );
static_assert(vminus1_8  *  v2_32== -2_si32 );
static_assert(vminus1_64 *  v2_16== -2_si64 );
static_assert(vminus1_32 *  v2_16== -2_si32 );
static_assert(vminus1_16 *  v2_16== -2_si16 );
static_assert(vminus1_8  *  v2_16== -2_si16 );
static_assert(vminus1_64 *  v2_8 == -2_si64 );
static_assert(vminus1_32 *  v2_8 == -2_si32 );
static_assert(vminus1_16 *  v2_8 == -2_si16 );
static_assert(vminus1_8  *  v2_8 == -2_si8 );

static_assert(vminus1_64 * -v2_64==  2_si64 );
static_assert(vminus1_32 * -v2_64==  2_si64 );
static_assert(vminus1_16 * -v2_64==  2_si64 );
static_assert(vminus1_8  * -v2_64==  2_si64 );
static_assert(vminus1_64 * -v2_32==  2_si64 );
static_assert(vminus1_32 * -v2_32==  2_si32 );
static_assert(vminus1_16 * -v2_32==  2_si32 );
static_assert(vminus1_8  * -v2_32==  2_si32 );
static_assert(vminus1_64 * -v2_16==  2_si64 );
static_assert(vminus1_32 * -v2_16==  2_si32 );
static_assert(vminus1_16 * -v2_16==  2_si16 );
static_assert(vminus1_8  * -v2_16==  2_si16 );
static_assert(vminus1_64 * -v2_8 ==  2_si64 );
static_assert(vminus1_32 * -v2_8 ==  2_si32 );
static_assert(vminus1_16 * -v2_8 ==  2_si16 );
static_assert(vminus1_8  * -v2_8 ==  2_si8 );

static_assert(v2_64 * -v2_64== -4_si64 );
static_assert(v2_32 * -v2_64== -4_si64 );
static_assert(v2_16 * -v2_64== -4_si64 );
static_assert(v2_8  * -v2_64== -4_si64 );
static_assert(v2_64 * -v2_32== -4_si64 );
static_assert(v2_32 * -v2_32== -4_si32 );
static_assert(v2_16 * -v2_32== -4_si32 );
static_assert(v2_8  * -v2_32== -4_si32 );
static_assert(v2_64 * -v2_16== -4_si64 );
static_assert(v2_32 * -v2_16== -4_si32 );
static_assert(v2_16 * -v2_16== -4_si16 );
static_assert(v2_8  * -v2_16== -4_si16 );
static_assert(v2_64 * -v2_8 == -4_si64 );
static_assert(v2_32 * -v2_8 == -4_si32 );
static_assert(v2_16 * -v2_8 == -4_si16 );
static_assert(v2_8  * -v2_8 == -4_si8 );

static_assert(-v2_64 * -v2_64== 4_si64 );
static_assert(-v2_32 * -v2_64== 4_si64 );
static_assert(-v2_16 * -v2_64== 4_si64 );
static_assert(-v2_8  * -v2_64== 4_si64 );
static_assert(-v2_64 * -v2_32== 4_si64 );
static_assert(-v2_32 * -v2_32== 4_si32 );
static_assert(-v2_16 * -v2_32== 4_si32 );
static_assert(-v2_8  * -v2_32== 4_si32 );
static_assert(-v2_64 * -v2_16== 4_si64 );
static_assert(-v2_32 * -v2_16== 4_si32 );
static_assert(-v2_16 * -v2_16== 4_si16 );
static_assert(-v2_8  * -v2_16== 4_si16 );
static_assert(-v2_64 * -v2_8 == 4_si64 );
static_assert(-v2_32 * -v2_8 == 4_si32 );
static_assert(-v2_16 * -v2_8 == 4_si16 );
static_assert(-v2_8  * -v2_8 == 4_si8 );

// demonstrate wrapping:
static_assert(min_64 * v2_64 == 0_si64 );
static_assert(min_64 * v2_32 == 0_si64  );
static_assert(min_64 * v2_16 == 0_si64  );
static_assert(min_64 * v2_8  == 0_si64 );
static_assert(min_32 * v2_64 == -0x1'0000'0000_si64 );
static_assert(min_32 * v2_32 == 0_si32 );
static_assert(min_32 * v2_16 == 0_si32 );
static_assert(min_32 * v2_8  == 0_si32 );
static_assert(min_16 * v2_64 == -0x1'0000_si64  );
static_assert(min_16 * v2_32 == -0x1'0000_si32 );
static_assert(min_16 * v2_16 == 0_si16 );
static_assert(min_16 * v2_8  == 0_si16 );
static_assert(min_8  * v2_64 == -256_si64 );
static_assert(min_8  * v2_32 == -256_si32 );
static_assert(min_8  * v2_16 == -256_si16 );
static_assert(min_8  * v2_8  == 0_si8 );


static_assert(v1_64 / vminus1_64 == -1_si64 );
static_assert(v1_64 / vminus1_32 == -1_si64 );
static_assert(v1_64 / vminus1_16 == -1_si64 );
static_assert(v1_64 / vminus1_8  == -1_si64 );
static_assert(v1_32 / vminus1_64 == -1_si64 );
static_assert(v1_32 / vminus1_32 == -1_si32 );
static_assert(v1_32 / vminus1_16 == -1_si32 );
static_assert(v1_32 / vminus1_8  == -1_si32 );
static_assert(v1_16 / vminus1_64 == -1_si64 );
static_assert(v1_16 / vminus1_32 == -1_si32 );
static_assert(v1_16 / vminus1_16 == -1_si16 );
static_assert(v1_16 / vminus1_8  == -1_si16 );
static_assert(v1_8  / vminus1_64 == -1_si64 );
static_assert(v1_8  / vminus1_32 == -1_si32 );
static_assert(v1_8  / vminus1_16 == -1_si16 );
static_assert(v1_8  / vminus1_8  == -1_si8 );

static_assert(vminus1_64 /  v1_64== -1_si64 );
static_assert(vminus1_32 /  v1_64== -1_si64 );
static_assert(vminus1_16 /  v1_64== -1_si64 );
static_assert(vminus1_8  /  v1_64== -1_si64 );
static_assert(vminus1_64 /  v1_32== -1_si64 );
static_assert(vminus1_32 /  v1_32== -1_si32 );
static_assert(vminus1_16 /  v1_32== -1_si32 );
static_assert(vminus1_8  /  v1_32== -1_si32 );
static_assert(vminus1_64 /  v1_16== -1_si64 );
static_assert(vminus1_32 /  v1_16== -1_si32 );
static_assert(vminus1_16 /  v1_16== -1_si16 );
static_assert(vminus1_8  /  v1_16== -1_si16 );
static_assert(vminus1_64 /  v1_8 == -1_si64 );
static_assert(vminus1_32 /  v1_8 == -1_si32 );
static_assert(vminus1_16 /  v1_8 == -1_si16 );
static_assert(vminus1_8  /  v1_8 == -1_si8 );


static_assert(v2_64 / vminus1_64 == -2_si64 );
static_assert(v2_64 / vminus1_32 == -2_si64 );
static_assert(v2_64 / vminus1_16 == -2_si64 );
static_assert(v2_64 / vminus1_8  == -2_si64 );
static_assert(v2_32 / vminus1_64 == -2_si64 );
static_assert(v2_32 / vminus1_32 == -2_si32 );
static_assert(v2_32 / vminus1_16 == -2_si32 );
static_assert(v2_32 / vminus1_8  == -2_si32 );
static_assert(v2_16 / vminus1_64 == -2_si64 );
static_assert(v2_16 / vminus1_32 == -2_si32 );
static_assert(v2_16 / vminus1_16 == -2_si16 );
static_assert(v2_16 / vminus1_8  == -2_si16 );
static_assert(v2_8  / vminus1_64 == -2_si64 );
static_assert(v2_8  / vminus1_32 == -2_si32 );
static_assert(v2_8  / vminus1_16 == -2_si16 );
static_assert(v2_8  / vminus1_8  == -2_si8 );

static_assert(vminus1_64 /  v2_64== 0_si64 );
static_assert(vminus1_32 /  v2_64== 0_si64 );
static_assert(vminus1_16 /  v2_64== 0_si64 );
static_assert(vminus1_8  /  v2_64== 0_si64 );
static_assert(vminus1_64 /  v2_32== 0_si64 );
static_assert(vminus1_32 /  v2_32== 0_si32 );
static_assert(vminus1_16 /  v2_32== 0_si32 );
static_assert(vminus1_8  /  v2_32== 0_si32 );
static_assert(vminus1_64 /  v2_16== 0_si64 );
static_assert(vminus1_32 /  v2_16== 0_si32 );
static_assert(vminus1_16 /  v2_16== 0_si16 );
static_assert(vminus1_8  /  v2_16== 0_si16 );
static_assert(vminus1_64 /  v2_8 == 0_si64 );
static_assert(vminus1_32 /  v2_8 == 0_si32 );
static_assert(vminus1_16 /  v2_8 == 0_si16 );
static_assert(vminus1_8  /  v2_8 == 0_si8 );

static_assert(min_64 /  min_64 ==  1_si64 );
static_assert(min_32 /  min_64 ==  0_si64 );
static_assert(min_16 /  min_64 ==  0_si64 );
static_assert(min_8  /  min_64 ==  0_si64 );
static_assert(min_64 /  min_32 ==  0x1'0000'0000_si64 );
static_assert(min_32 /  min_32 ==  1_si32 );
static_assert(min_16 /  min_32 ==  0_si32 );
static_assert(min_8  /  min_32 ==  0_si32 );
static_assert(min_64 /  min_16 ==  0x1'0000'0000'0000_si64 );
static_assert(min_32 /  min_16 ==  0x1'0000_si32 );
static_assert(min_16 /  min_16 ==  1_si16 );
static_assert(min_8  /  min_16 ==  0_si16 );
static_assert(min_64 /  min_8  ==  0x100'0000'0000'0000_si64 );
static_assert(min_32 /  min_8  ==  0x100'0000_si32 );
static_assert(min_16 /  min_8  ==  256_si16 );
static_assert(min_8  /  min_8  ==  1_si8 );

// demonstrate wrapping:
static_assert(min_64 / vminus1_64 == min_64 );
static_assert(min_64 / vminus1_32 == min_64 );
static_assert(min_64 / vminus1_16 == min_64 );
static_assert(min_64 / vminus1_8  == min_64 );
static_assert(min_32 / vminus1_64 == 0x8000'0000_si64 );
static_assert(min_32 / vminus1_32 == min_32 );
static_assert(min_32 / vminus1_16 == min_32 );
static_assert(min_32 / vminus1_8  == min_32 );static_assert(min_16 / vminus1_64 == -static_cast<si64>(to_underlying(min_16))  );
static_assert(min_16 / vminus1_32 == 0x8000_si32 );
static_assert(min_16 / vminus1_16 == min_16 );
static_assert(min_16 / vminus1_8  == min_16 );
static_assert(min_8  / vminus1_64 == 128_si64 );
static_assert(min_8  / vminus1_32 == 128_si32 );
static_assert(min_8  / vminus1_16 == 128_si16 );
static_assert(min_8  / vminus1_8  == min_8 );

static_assert(min_64 / v1_64 == min_64 );
static_assert(min_64 / v1_32 == min_64 );
static_assert(min_64 / v1_16 == min_64 );
static_assert(min_64 / v1_8  == min_64 );
static_assert(min_32 / v1_64 == -0x8000'0000_si64 );
static_assert(min_32 / v1_32 == min_32 );
static_assert(min_32 / v1_16 == min_32 );
static_assert(min_32 / v1_8  == min_32 );
static_assert(min_16 / v1_64 == -0x8000_si64  );
static_assert(min_16 / v1_32 == -0x8000_si32 );
static_assert(min_16 / v1_16 == min_16 );
static_assert(min_16 / v1_8  == min_16 );
static_assert(min_8  / v1_64 == -128_si64 );
static_assert(min_8  / v1_32 == -128_si32 );
static_assert(min_8  / v1_16 == -128_si16 );
static_assert(min_8  / v1_8  == min_8 );

static_assert(min_64 / v2_64 == -0x4000'0000'0000'0000_si64 );
static_assert(min_64 / v2_32 == -0x4000'0000'0000'0000_si64  );
static_assert(min_64 / v2_16 == -0x4000'0000'0000'0000_si64  );
static_assert(min_64 / v2_8  == -0x4000'0000'0000'0000_si64 );
static_assert(min_32 / v2_64 == -0x4000'0000_si64 );
static_assert(min_32 / v2_32 == -0x4000'0000_si32 );
static_assert(min_32 / v2_16 == -0x4000'0000_si32 );
static_assert(min_32 / v2_8  == -0x4000'0000_si32 );
static_assert(min_16 / v2_64 == -0x4000_si64  );
static_assert(min_16 / v2_32 == -0x4000_si32 );
static_assert(min_16 / v2_16 == -0x4000_si16 );
static_assert(min_16 / v2_8  == -0x4000_si16 );
static_assert(min_8  / v2_64 == -64_si64 );
static_assert(min_8  / v2_32 == -64_si32 );
static_assert(min_8  / v2_16 == -64_si16 );
static_assert(min_8  / v2_8  == -64_si8 );


static_assert(-100_si32 / -9_si64 == 11_si64);
static_assert(100_si32 / 9_si64 == 11_si64);
static_assert(-100_si32 / 9_si64 == -11_si64);
static_assert(100_si32 / -9_si64 == -11_si64);

static_assert(std::numeric_limits<si32>::min() / 1_si32 == std::numeric_limits<si32>::min()); // wraps
static_assert(std::numeric_limits<si32>::min() / -1_si32 == std::numeric_limits<si32>::min()); // wraps

template<typename T, typename WHAT>
constexpr bool
isa = std::is_same_v<std::remove_cvref_t<T>,WHAT>;


template<typename T>
constexpr bool
is_unsigned = isa<T, unsigned char>
|| isa<T, unsigned short>
|| isa<T, unsigned >
|| isa<T, unsigned long>
|| isa<T, unsigned long long>;

template<typename T>
constexpr bool
is_signed = isa<T, signed char>
|| isa<T, short>
|| isa<T, int >
|| isa<T, long>
|| isa<T, long long>;


template<typename T>
constexpr bool
is_integer = is_unsigned<T>||is_signed<T>;

static_assert(is_integer<ULT<ui16>>);
static_assert(!is_integer<ui16>);
static_assert(is_integer<ULT<si16>>);
static_assert(!is_integer<si16>);

#undef check_does_compile
#undef check_expr_does_compile
#undef concat_line_impl
#undef concat_line
}
