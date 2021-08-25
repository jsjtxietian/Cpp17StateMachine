#pragma once

#include <array>
#include <string>
#include <cstdlib>
#include <iostream>
#include <cstdio>

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
constexpr std::array<T, NewSize> resize(const std::array<T, OldSize> &arr, std::index_sequence<Indexes...>)
{
    return {arr[Indexes]...};
}

template <std::size_t NewSize, typename T, std::size_t OldSize>
constexpr std::array<T, NewSize> resize(const std::array<T, OldSize> &arr)
{
    constexpr std::size_t minSize = std::min(OldSize, NewSize);
    return resize<NewSize>(arr, std::make_index_sequence<minSize>());
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
        return join(resize<N - 1>(chars), rhs.chars);
    }

    constexpr bool operator==(const StaticString<N> &rhs) const
    {
        return areEqual(chars, rhs.chars);
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

        [[maybe_unused]] constexpr void testAdding()
        {
            constexpr StaticString lhs{"abc"};
            constexpr StaticString rhs{"de"};
            constexpr StaticString expected{"abcde"};
            static_assert(expected == lhs + rhs);
        }

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
            constexpr std::array expectedLonger = {1, 2, 3, 0};
            static_assert(areEqual(expectedShorter, resize<2>(input)));
            static_assert(areEqual(expectedLonger, resize<4>(input)));
        }

    }
}

__declspec(noinline) void test()
{
    constexpr StaticString first{"<"};
    constexpr StaticString second{"hello"};
    constexpr StaticString third{">"};
    constexpr StaticString result = first + second + third;
    puts(result.data());
}

int main()
{
    test();
    return 0;
}