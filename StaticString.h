#pragma once

#include <string>
#include <cstdlib>
#include <iostream>
#include <array>
#include <type_traits>

template <typename T, std::size_t N, std::size_t... Idx>
constexpr std::array<T, N> toStdArray(T (&arr)[N], std::index_sequence<Idx...>)
{
    return {arr[Idx]...};
}

template <typename T, std::size_t N>
constexpr std::array<T, N> toStdArray(T (&arr)[N])
{
    return toStdArray(arr, std::make_index_sequence<N>());
}

template <typename T, std::size_t LeftSize, std::size_t RightSize, std::size_t... LeftIdx, std::size_t... RightIdx>
constexpr std::array<T, LeftSize + RightSize> join(const std::array<T, LeftSize> &lhs, const std::array<T, RightSize> &rhs, std::index_sequence<LeftIdx...>, std::index_sequence<RightIdx...>)
{
    return {lhs[LeftIdx]..., rhs[RightIdx]...};
}

template <typename T, std::size_t LeftSize, std::size_t RightSize>
constexpr std::array<T, LeftSize + RightSize> join(const std::array<T, LeftSize> &lhs, const std::array<T, RightSize> &rhs)
{
    return join(lhs, rhs, std::make_index_sequence<LeftSize>(), std::make_index_sequence<RightSize>());
}

template <std::size_t NewSize, typename T, std::size_t OldSize, std::size_t... Indexes>
constexpr std::array<T, NewSize> resize(const std::array<T, OldSize> &arr, std::remove_const_t<T> defaultValue, std::index_sequence<Indexes...>)
{
    return {((Indexes < OldSize) ? arr[Indexes] : defaultValue)...};
}

template <std::size_t NewSize, typename T, std::size_t OldSize>
constexpr std::array<T, NewSize> resize(const std::array<T, OldSize> &arr, std::remove_const_t<T> defaultValue)
{
    return resize<NewSize>(arr, defaultValue, std::make_index_sequence<NewSize>());
}

template <typename T, std::size_t N, std::size_t... Idx>
constexpr bool areEqual(const std::array<T, N> &lhs, const std::array<T, N> &rhs, std::index_sequence<Idx...>)
{
    return ((lhs[Idx] == rhs[Idx]) && ...);
}

template <typename T, std::size_t N>
constexpr bool areEqual(const std::array<T, N> &lhs, const std::array<T, N> &rhs)
{
    return areEqual(lhs, rhs, std::make_index_sequence<N>());
}

template <std::size_t N>
class StaticString
{
public:
    constexpr StaticString(const char (&chars)[N])
        : chars(toStdArray(chars))
    {
    }

    constexpr StaticString(const std::array<const char, N> &chars)
        : chars(chars)
    {
    }

    template <std::size_t M>
    constexpr StaticString<N + M - 1> operator+(const StaticString<M> &rhs) const
    {
        return join(resize<N - 1>(chars, '\0'), rhs.chars);
    }

    constexpr bool operator==(const StaticString<N> &rhs) const
    {
        return areEqual(chars, rhs.chars);
    }

    constexpr std::size_t length() const
    {
        return N - 1;
    }

    template <std::size_t TargetLen>
    constexpr StaticString<TargetLen + 1> changeLength(char fill) const
    {
        constexpr std::array<const char, 1> stringEnd{'\0'};
        return join(resize<TargetLen>(resize<N - 1>(chars, fill), fill), stringEnd);
    }

    template <std::size_t M>
    friend class StaticString;

    constexpr const char *data() const
    {
        return chars.data();
    }

private:
    std::array<const char, N> chars;
};

namespace tests
{
    namespace
    {
        [[maybe_unused]] constexpr void testToStdArray()
        {
            constexpr int input[] = {1, 2, 3};
            constexpr auto output = toStdArray(input);
            constexpr std::array<const int, 3> expected = {1, 2, 3};
            static_assert(areEqual(expected, output));
        }

        [[maybe_unused]] constexpr void testJoin()
        {
            constexpr std::array inputA = {1, 2, 3};
            constexpr std::array inputB = {4, 5};
            constexpr std::array expected = {1, 2, 3, 4, 5};
            static_assert(areEqual(expected, join(inputA, inputB)));
        }

        [[maybe_unused]] constexpr void testResize()
        {
            constexpr std::array input = {1, 2, 3};
            constexpr std::array expectedShorter = {1, 2};
            constexpr std::array expectedLonger = {1, 2, 3, 9};
            static_assert(areEqual(expectedShorter, resize<2>(input, 9)));
            static_assert(areEqual(expectedLonger, resize<4>(input, 9)));
        }

        [[maybe_unused]] constexpr void testAdding()
        {
            constexpr StaticString lhs{"abc"};
            constexpr StaticString rhs{"de"};
            constexpr StaticString expected{"abcde"};
            static_assert(expected == lhs + rhs);
        }

        [[maybe_unused]] constexpr void testLength()
        {
            constexpr StaticString lhs{"abc"};
            constexpr size_t expected{3};
            static_assert(lhs.length() == expected);
        }

        [[maybe_unused]] constexpr void test0Length()
        {
            constexpr StaticString lhs{""};
            constexpr size_t expected{0};
            static_assert(lhs.length() == expected);
        }

        [[maybe_unused]] constexpr void testChangeLength()
        {
            constexpr StaticString shorter{"abc"};
            constexpr StaticString longer{"abcdef"};
            constexpr StaticString empty{""};

            constexpr size_t minLength{5};
            constexpr StaticString expectedShorter{"abcxx"};
            constexpr StaticString expectedLonger{"abcde"};
            constexpr StaticString expectedEmpty{"zzzzz"};

            constexpr auto res = shorter.changeLength<minLength>('x');

            static_assert(res.data()[3] == expectedShorter.data()[3]);

            static_assert(shorter.changeLength<minLength>('x') == expectedShorter);
            static_assert(longer.changeLength<minLength>('y') == expectedLonger);
            static_assert(empty.changeLength<minLength>('z') == expectedEmpty);
        }

    }
}