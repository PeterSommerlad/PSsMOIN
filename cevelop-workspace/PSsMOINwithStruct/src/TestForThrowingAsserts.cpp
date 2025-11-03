#include "TestForThrowingAsserts.h"
#include "cute.h"

#include "moins.h"
using namespace moins;

void DivisionByZeroThrows() {
    constexpr auto divisor = 0_si8;
    auto divident = 1_si8;

    ASSERT_THROWS(std::ignore=(divident/divisor), char const *);
    //constexpr auto should_not_compile = 1_si8 / 0_si8; in CompilationTests.cpp
}

void ModuloByZeroThrows() {
    constexpr auto divisor = 0_ui8;
    auto divident = 1_ui8;

    ASSERT_THROWS(std::ignore=(divident % divisor), char const *);

    //constexpr auto should_not_compile = 1_ui8 % 0_ui8; in CompilationTests.cpp
}

void ShiftLeftByTooManyBitsThrows() {
    constexpr auto shiftby = 8_ui8;
    auto toshift = 0xff_ui8;

    ASSERT_THROWS(std::ignore=(toshift << shiftby), char const *);
     //constexpr auto should_not_compile = 1_ui8 << shiftby; in CompilationTests.cpp
}
void ShiftRightByTooManyBitsThrows() {
    constexpr auto shiftby = 8_ui8;
    auto toshift = 0xff_ui8;

    ASSERT_THROWS(std::ignore=(toshift >> shiftby), char const *);
     //constexpr auto should_not_compile = 1_ui8 >> shiftby; in CompilationTests.cpp
}


cute::suite make_suite_TestForThrowingAsserts() {
	cute::suite s { };
	s.push_back(CUTE(DivisionByZeroThrows));
	s.push_back(CUTE(ModuloByZeroThrows));
	s.push_back(CUTE(ShiftLeftByTooManyBitsThrows));
	s.push_back(CUTE(ShiftRightByTooManyBitsThrows));
	return s;
}
