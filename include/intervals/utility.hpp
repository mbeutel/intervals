
#ifndef INCLUDED_INTERVALS_UTILITY_HPP_
#define INCLUDED_INTERVALS_UTILITY_HPP_


#include <utility>  // for move()


namespace intervals {


template <typename T>
class as_regular
{
private:
    T value_;

    constexpr void
    _construct_inplace(T& rhs) noexcept  // use `noexcept` to enforce termination if anything goes wrong
    {
        value_.~T();
        new (&value_) T(std::move(rhs));
    }

public:
    as_regular() = default;
    as_regular(as_regular const&) = default;
    as_regular(as_regular&&) = default;

    constexpr as_regular(T _value)
        : value_(std::move(_value))
    {
    }

    constexpr as_regular&
    operator =(as_regular const& rhs)
    {
        auto copy = T(rhs.value_);  // make sure any exception is thrown before attempting to reconstruct in-place
        _construct_inplace(copy);
        return *this;
    }
    constexpr as_regular&
    operator =(as_regular&& rhs) noexcept
    {
        _construct_inplace(rhs.value_);
        return *this;
    }

    [[nodiscard]] constexpr T&
    operator ()() & noexcept
    {
        return value_;
    }
    [[nodiscard]] constexpr T&&
    operator ()() && noexcept
    {
        return std::move(value_);
    }
    [[nodiscard]] constexpr T const&
    operator ()() const & noexcept
    {
        return value_;
    }
};


} // namespace intervals


#endif // INCLUDED_INTERVALS_UTILITY_HPP_
