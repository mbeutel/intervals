
#include <array>

#include <gsl-lite/gsl-lite.hpp>  // for fail_fast, type_identity<>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <intervals/set.hpp>


namespace {

namespace gsl = ::gsl_lite;


enum Color { red = 2, green = 1, blue = 4 };
consteval auto
reflect(gsl::type_identity<Color>)
{
    return std::array{ red, green, blue };
}


TEST_CASE("set<>", "algebra with bounds")
{
    using intervals::set;

    SECTION("fail on uninitialized read")
    {
        auto value = GENERATE(false, true);
        CAPTURE(value);
        auto s0 = set<bool>{ };
        auto svalue = set<bool>{ value };

            // `contains()` and `matches()` are supported for uninitialized values.
        CHECK_NOTHROW(s0.contains(value));
        CHECK_NOTHROW(s0.contains(svalue));
        CHECK_NOTHROW(s0.matches(value));
        CHECK_NOTHROW(s0.matches(svalue));

        CHECK_THROWS_AS(s0 == value, gsl::fail_fast);
        CHECK_THROWS_AS(s0 != value, gsl::fail_fast);
        CHECK_THROWS_AS(!s0, gsl::fail_fast);
    }
    SECTION("single-valued sets (bool)")
    {
        auto value = GENERATE(false, true);
        CAPTURE(value);
        auto svalue = set{ value };
        auto nsvalue = set{ !value };
        CHECK(svalue.contains(value));
        CHECK(svalue.contains(svalue));
        CHECK_FALSE(svalue.contains(!value));
        CHECK_FALSE(svalue.contains(nsvalue));
        CHECK(svalue.matches(value));
        CHECK(svalue.matches(svalue));
        CHECK_FALSE(svalue.matches(!value));
        CHECK_FALSE(svalue.matches(nsvalue));
        CHECK((svalue == svalue).matches(set{ true }));
        CHECK((svalue == nsvalue).matches(set{ false }));
        CHECK((svalue != svalue).matches(set{ false }));
        CHECK((svalue != nsvalue).matches(set{ true }));
        CHECK((!svalue).matches(nsvalue));
        CHECK((!nsvalue).matches(svalue));
        CHECK(maybe(svalue) == value);
        CHECK(definitely(svalue) == value);
    }
    SECTION("multi-valued sets (bool)")
    {
        auto value = GENERATE(false, true);
        CAPTURE(value);
        auto svalue = set{ value };
        auto s1 = set{ false, true };
        CHECK(s1.contains(s1));
        CHECK(s1.contains(value));
        CHECK(s1.contains(svalue));
        CHECK(s1.matches(s1));
        CHECK_FALSE(s1.matches(value));
        CHECK_FALSE(s1.matches(svalue));
        CHECK((s1 == s1).matches(s1));
        CHECK((s1 == svalue).matches(s1));
        CHECK((s1 != svalue).matches(s1));
        CHECK((!s1).matches(s1));
        CHECK(maybe(s1));
        CHECK_FALSE(definitely(s1));
    }
    SECTION("assignment (bool)")
    {
        using intervals::assign_partial;
        using intervals::reset;

        auto value = GENERATE(false, true);
        auto svalue = set{ value };
        auto nsvalue = set{ !value };
        auto s = intervals::set<bool>{ };
        s.assign(value);
        CHECK(maybe(s == set{ value }));
        CHECK(definitely(s == set{ value }));
        CHECK((s == svalue).matches(set{ true }));
        s.assign(!value);
        CHECK(maybe(s == set{ true, false }));
        CHECK_FALSE(definitely(s == set{ true, false }));
        s.reset();
        assign_partial(s, value);
        CHECK(s.matches(value));
        assign_partial(s, !value);
        CHECK(s.matches(set{ false, true }));
        reset(s, value);
        CHECK(s.matches(value));
    }
    SECTION("multi-valued sets (enum)")
    {
        auto s0 = set<Color>{ };
        auto sp1 = set{ red, green };
        auto sp2 = set{ red, blue };
        auto sp3 = set{ green, blue };
        auto s1 = set{ red, green, blue };
        SECTION("multiple values")
        {
            CHECK_THROWS_AS(s0.value(), gsl::fail_fast);
            CHECK_THROWS_AS(sp1.value(), gsl::fail_fast);
            CHECK_THROWS_AS(sp2.value(), gsl::fail_fast);
            CHECK_THROWS_AS(sp3.value(), gsl::fail_fast);
            CHECK_THROWS_AS(s1.value(), gsl::fail_fast);
            CHECK(s1.contains(s1));
            CHECK(s1.contains(sp1));
            CHECK(s1.contains(sp2));
            CHECK(s1.contains(sp3));
            CHECK(s1.matches(s1));
            CHECK_FALSE(s1.matches(sp1));
            CHECK_FALSE(s1.matches(sp2));
            CHECK_FALSE(s1.matches(sp3));
            CHECK_FALSE(sp1.matches(sp2));
            CHECK_FALSE(sp1.matches(sp3));
            CHECK_FALSE(sp2.matches(sp3));
            CHECK((s1 == s1).matches(set{ false, true }));
            CHECK((sp1 == set{ red }).matches(set{ false, true }));
            CHECK((sp1 == set{ blue }).matches(set{ false }));
            CHECK((sp1 == sp2).matches(set{ false, true }));
            CHECK((sp1 == sp3).matches(set{ false, true }));
            CHECK((sp2 == sp3).matches(set{ false, true }));
            CHECK((s1 == sp1).matches(set{ false, true }));
            CHECK((s1 == sp2).matches(set{ false, true }));
            CHECK((s1 == sp3).matches(set{ false, true }));
        }
        SECTION("single value")
        {
            auto value = GENERATE(red, green, blue);
            CAPTURE(value);
            auto svalue = set{ value };
            CHECK(s1.contains(value));
            CHECK(s1.contains(svalue));
            CHECK_FALSE(s1.matches(value));
            CHECK_FALSE(s1.matches(svalue));
            CHECK(svalue.value() == value);
        }
    }
    SECTION("Boolean operators")
    {
            // !
        CHECK((!set{ true        }).matches(set{ false       }));
        CHECK((!set{ false       }).matches(set{ true        }));
        CHECK((!set{ false, true }).matches(set{ false, true }));

            // &
        CHECK((set{ true        } & set{ true        }).matches(set{ true        }));
        CHECK((set{ true        } & set{ false, true }).matches(set{ false, true }));
        CHECK((set{ true        } & set{ false       }).matches(set{ false       }));
        CHECK((set{ false, true } & set{ true        }).matches(set{ false, true }));
        CHECK((set{ false, true } & set{ false, true }).matches(set{ false, true }));
        CHECK((set{ false, true } & set{ false       }).matches(set{ false       }));
        CHECK((set{ false       } & set{ true        }).matches(set{ false       }));
        CHECK((set{ false       } & set{ false, true }).matches(set{ false       }));
        CHECK((set{ false       } & set{ false       }).matches(set{ false       }));

            // |
        CHECK((set{ true        } | set{ true        }).matches(set{ true        }));
        CHECK((set{ true        } | set{ false, true }).matches(set{ true        }));
        CHECK((set{ true        } | set{ false       }).matches(set{ true        }));
        CHECK((set{ false, true } | set{ true        }).matches(set{ true        }));
        CHECK((set{ false, true } | set{ false, true }).matches(set{ false, true }));
        CHECK((set{ false, true } | set{ false       }).matches(set{ false, true }));
        CHECK((set{ false       } | set{ true        }).matches(set{ true        }));
        CHECK((set{ false       } | set{ false, true }).matches(set{ false, true }));
        CHECK((set{ false       } | set{ false       }).matches(set{ false       }));

            // ^
        CHECK((set{ true        } ^ set{ true        }).matches(set{ false       }));
        CHECK((set{ true        } ^ set{ false, true }).matches(set{ false, true }));
        CHECK((set{ true        } ^ set{ false       }).matches(set{ true        }));
        CHECK((set{ false, true } ^ set{ true        }).matches(set{ false, true }));
        CHECK((set{ false, true } ^ set{ false, true }).matches(set{ false, true }));
        CHECK((set{ false, true } ^ set{ false       }).matches(set{ false, true }));
        CHECK((set{ false       } ^ set{ true        }).matches(set{ true        }));
        CHECK((set{ false       } ^ set{ false, true }).matches(set{ false, true }));
        CHECK((set{ false       } ^ set{ false       }).matches(set{ false       }));

            // ==
        CHECK((set{ true        } == set{ true        }).matches(set{ true        }));
        CHECK((set{ true        } == set{ false, true }).matches(set{ false, true }));
        CHECK((set{ true        } == set{ false       }).matches(set{ false       }));
        CHECK((set{ false, true } == set{ true        }).matches(set{ false, true }));
        CHECK((set{ false, true } == set{ false, true }).matches(set{ false, true }));
        CHECK((set{ false, true } == set{ false       }).matches(set{ false, true }));
        CHECK((set{ false       } == set{ true        }).matches(set{ false       }));
        CHECK((set{ false       } == set{ false, true }).matches(set{ false, true }));
        CHECK((set{ false       } == set{ false       }).matches(set{ true        }));

            // !=
        CHECK((set{ true        } != set{ true        }).matches(set{ false       }));
        CHECK((set{ true        } != set{ false, true }).matches(set{ false, true }));
        CHECK((set{ true        } != set{ false       }).matches(set{ true        }));
        CHECK((set{ false, true } != set{ true        }).matches(set{ false, true }));
        CHECK((set{ false, true } != set{ false, true }).matches(set{ false, true }));
        CHECK((set{ false, true } != set{ false       }).matches(set{ false, true }));
        CHECK((set{ false       } != set{ true        }).matches(set{ true        }));
        CHECK((set{ false       } != set{ false, true }).matches(set{ false, true }));
        CHECK((set{ false       } != set{ false       }).matches(set{ false       }));
    }
    SECTION("Boolean predicates")
    {
            // maybe()
        CHECK(intervals::maybe(true));
        CHECK_FALSE(intervals::maybe(false));
        CHECK(maybe(set{ true }));
        CHECK(maybe(set{ false, true }));
        CHECK_FALSE(maybe(set{ false }));

            // maybe_not()
        CHECK(intervals::maybe_not(false));
        CHECK_FALSE(intervals::maybe_not(true));
        CHECK(maybe_not(set{ false }));
        CHECK(maybe_not(set{ false, true }));
        CHECK_FALSE(maybe_not(set{ true }));

            // definitely()
        CHECK(intervals::definitely(true));
        CHECK_FALSE(intervals::definitely(false));
        CHECK(definitely(set{ true }));
        CHECK_FALSE(definitely(set{ false, true }));
        CHECK_FALSE(definitely(set{ false }));

            // definitely_not()
        CHECK(intervals::definitely_not(false));
        CHECK_FALSE(intervals::definitely_not(true));
        CHECK(definitely_not(set{ false }));
        CHECK_FALSE(definitely_not(set{ false, true }));
        CHECK_FALSE(definitely_not(set{ true }));

            // contingent()
        CHECK_FALSE(intervals::contingent(true));
        CHECK_FALSE(intervals::contingent(false));
        CHECK_FALSE(contingent(set{ true }));
        CHECK(contingent(set{ false, true }));
        CHECK_FALSE(contingent(set{ false }));
    }
}


}  // anonymous namespace
