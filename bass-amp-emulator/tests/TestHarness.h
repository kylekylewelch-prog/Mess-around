#pragma once
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

namespace tst {

inline int& failures() { static int f = 0; return f; }
inline int& checks()   { static int c = 0; return c; }

inline void check (bool condition, const std::string& what)
{
    ++checks();
    if (! condition) { ++failures(); std::printf ("  FAIL  %s\n", what.c_str()); }
}

inline void checkNear (double actual, double expected, double tolerance, const std::string& what)
{
    ++checks();
    if (! (std::fabs (actual - expected) <= tolerance))
    {
        ++failures();
        std::printf ("  FAIL  %s  (got %.6g, expected %.6g +/- %.3g)\n",
                     what.c_str(), actual, expected, tolerance);
    }
}

inline void section (const std::string& name) { std::printf ("\n[%s]\n", name.c_str()); }

inline int report()
{
    std::printf ("\n%d checks, %d failures\n", checks(), failures());
    return failures() == 0 ? 0 : 1;
}

} // namespace tst
