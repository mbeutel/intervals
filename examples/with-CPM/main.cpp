
#include <iostream>

#include <intervals/interval.hpp>

template <typename T>
T max3(T a, T b)
{
    using namespace intervals::math;   // for constrain(), assign_partial()
    using namespace intervals::logic;  // for possibly()
    auto x = T{ };
    auto c = (a < b);
    if (possibly(c))
    {
        auto bc = constrain(b, c);
        assign_partial(x, bc);
    }
    if (possibly(!c))
    {
        auto ac = constrain(a, !c);
        assign_partial(x, ac);
    }
    return x;
}
int main()
{
    auto a = 2.;
    auto b = 3.;
    std::cout << "A = " << a << "\n"
              << "B = " << b << "\n"
              << "max3(a,b) = " << max3(a,b) << "\n\n";  // prints "max3(a,b) = 3"

    auto A = intervals::interval{ 0., 3. };
    auto B = intervals::interval{ 1., 2. };
    std::cout << "A = " << a << "\n"
              << "B = " << b << "\n"
              << "max3(A,B) = " << max3(A,B) << "\n";  // prints "max3(A,B) = [1, 3]"
}
