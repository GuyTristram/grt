#ifndef VALUEPACK_H
#define VALUEPACK_H

#include <tuple>
#include <limits>
#include <cassert>

namespace grt
{
template<class T>
struct value_pack_max { static const size_t value = static_cast<size_t>(T::Count) - 1; };

namespace vp_helpers
{
    template<size_t I> struct bits    { static const size_t value = bits<I / 2>::value + 1; };
    template<>         struct bits<0> { static const size_t value = 0; };

    template<class T, size_t I> struct mask       { static const T value = ( mask<T, I - 1>::value << 1 ) + 1; };
    template<class T>           struct mask<T, 0> { static const T value = 0; };

    template<size_t I, class... Args>             struct shift{};
    template<size_t I, class Head, class... Tail> struct shift <I, Head, Tail...>
    {
        static const size_t value = shift<I - 1, Tail...>::value +
            bits<value_pack_max<Head>::value>::value;
    };
    template<class... Args> struct shift <0, Args...> { static const size_t value = 0; };
}

/** Pack enum values, bools and integers into a single unsigned int.
 *
 * The first template argument specifies the type into which the values
 * will be packed (usually one of uint8_t, uint16_t, uint32_t, uint64_t).
 *
 * To use an enum in a value_pack you must either specialize value_pack_max
 * for the enum type (there is a macro VALUE_PACK_ENUM_MAX to make this
 * easier) or provide "Count" as the last enumerator.
 *
 * To use an unsigned integer in a value_pack, specify the maximum value
 * using packed_int<max>. Signed integers are not supported.
 *
 * Example use:
 *
 *  enum class A
 *  {
 *    A1,
 *    A2,
 *    Count
 *  };
 *
 *  enum class B
 *  {
 *    B1,
 *    B2,
 *    B3
 *  };
 *  VALUE_PACK_ENUM_MAX( B, B3 );
 *
 *  value_pack<std::uint32_t, A, B, bool, packed_int<5>> pack;
 *
 *  pack.set<0>(A::A2);
 *  pack.set<1>(B::B1);
 *  pack.set<2>(true);
 *  pack.set<3>(5); // Ok
 *  pack.set<3>(6); // Will assert
 *  auto a = pack.get<0>();
 *
 */
template<class T, class... Args> class value_pack
{
public:
    template <size_t I>
    using type_at = typename std::tuple_element<I, std::tuple<Args...>>::type;

    template<size_t I>
    type_at<I> get() const
    {
        return static_cast<type_at<I>>((m_packed >> shift<I>()) & mask<I>());
    }

    template<size_t I> 
    void set(type_at<I> value)
    {
        assert(static_cast<T>(value) <= value_pack_max<type_at<I>>::value);
        m_packed = (m_packed & ~(mask<I>() << shift<I>())) | (static_cast<T>(value) << shift<I>());
    }

    bool operator==(value_pack other) const { return m_packed == other.m_packed; }
    bool operator<(value_pack other) const { return m_packed < other.m_packed; }

private:
    T m_packed = 0;

    template<size_t I> static size_t shift() { return vp_helpers::shift<I, Args...>::value; }
    template<size_t I> static T mask()
    {
        return vp_helpers::mask<T, vp_helpers::bits<value_pack_max<type_at<I>>::value>::value>::value;
    }

    static const size_t total_size = vp_helpers::shift<sizeof...(Args), Args...>::value;
    static_assert(total_size <= std::numeric_limits<T>::digits, "Pack type is too small");
};


template<class E, E Max>
struct packed_enum {
    packed_enum( E e ) : e( e ) {}
    operator E() const { return e; }
    operator size_t() const { return static_cast< size_t >( e ); }
private:
    E e;
};
template<class E, E Max> struct value_pack_max<packed_enum<E, Max>> { static const size_t value = static_cast<size_t>(Max); };

template<size_t Max>
struct packed_int {
    packed_int( unsigned int i ) : i( i ) {}
    operator unsigned int() const { return i; }
private:
    unsigned int i;
};
template<size_t Max> struct value_pack_max<packed_int<Max>> { static const size_t value = Max; };
template<> struct value_pack_max<bool> { static const size_t value = 1; };
}

#define VALUE_PACK_ENUM_MAX(Enum, Max) template<> struct grt::value_pack_max<Enum> { static const size_t value = static_cast<size_t>(Enum::Max); };

void test_value_pack();

#endif // VALUEPACK_H
