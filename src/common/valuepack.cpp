#include "common/valuepack.h"
#include <cstdint>

/*
enum class A
{
    A1,
    A2,
    Count
};
enum class B
{
    A1,
    A2,
    A3,
    A4,
    A5,
    A6,
    A7,
    A8,
    Count
};
enum class C
{
    A1,
    A2,
    A3,
    A4,
    A5,
    A6,
    A7,
    A8,
    A9
};
//template<> struct value_pack_max < C > { static const size_t value = static_cast<size_t>(C::A9); };
//VALUE_PACK_ENUM_MAX( C, A9 );

void test_value_pack()
{
    grt::value_pack<std::uint8_t, B, B, grt::packed_int<2>> pack1;
    grt::value_pack<std::uint32_t, grt::packed_enum<C, C::A9>, bool> pack2;
    pack1.set<1>( B::A2 );
    pack2.set<1>( true );
    pack2.set<0>( C::A5 );
    auto a = pack2.get<1>();
}

*/