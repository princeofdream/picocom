#ifndef _APF_CUSTOM_CATCH_MATCHER__
#define _APF_CUSTOM_CATCH_MATCHER__

#include "catch2/catch.hpp"
#include <sstream>

namespace apf {
namespace unit_test {
namespace catch_matcher {


// The matcher class
template <typename T>
class EqualTo : public Catch::MatcherBase<T> {
    T m_expected;
public:
    EqualTo(T const & _expected) : m_expected(_expected) {}

    // Performs the test for this matcher
    virtual bool match(T const& _test_target) const override {
        if (m_expected == _test_target) {return true;}

        if (m_expected.size() != _test_target.size()) {
            UNSCOPED_INFO("Size is not equal, test target's size is "<< _test_target.size() <<", and expected size is " << m_expected.size());
            return false;
        }

        size_t idx = 0;
        auto iter_exptected = std::begin(m_expected);
        auto iter_test_target = std::begin(_test_target);
        for (; iter_exptected != std::end(m_expected); ++iter_exptected, ++iter_test_target, ++idx) {
            if (*iter_exptected != *iter_test_target) {
                UNSCOPED_INFO("Items at index = [" << idx 
                            << "] are not equal, item in test target is {" << int(*iter_test_target) 
                            << "} and item in expected is {" << int(*iter_exptected) << "}.");
                return false; 
            }
        }

        return false;
    }

    // Produces a string describing what this matcher does. It should
    // include any provided data (the begin/ end in this case) and
    // be written as if it were stating a fact (in the output it will be
    // preceded by the value under test).
    virtual std::string describe() const override {
        std::ostringstream ss;
        ss << " is equal to { ";

        size_t idx = 0;
        auto iter_exptected = std::begin(m_expected);
        ss << int(*iter_exptected);
        for (++idx, ++iter_exptected; iter_exptected != std::end(m_expected) && idx < 10; ++idx, ++iter_exptected) {
            ss << ", " << int(*iter_exptected) ;
        }

        if (iter_exptected != std::end(m_expected)) {
            ss << ", ...";
        }

        ss << " }";
        
        return ss.str();
    }
};

// The builder function
template <typename T>
inline EqualTo<T> EqualsTo(T const & _expected) {
    return EqualTo<T>(_expected);
}

// New Style matcher.
// @todo need #include <catch2/catch_test_macros.hpp>
//            #include <catch2/matchers/catch_matchers_templated.hpp>
//
// template<typename Range>
// struct EqualsRangeMatcher : public Catch::MatcherGenericBase {
//     EqualsRangeMatcher(Range const& _expected):
//         m_expected{ _expected }
//     {}

//     template<typename OtherRange>
//     bool match(OtherRange const& _test_target) const {
//         using std::begin; using std::end;

//         if (std::equal(begin(m_expected), end(m_expected), begin(_test_target), end(_test_target))) {
//             return true;
//         }

//         size_t idx = 0;
//         auto iter_exptected   = begin(m_expected);
//         auto iter_test_target = begin(_test_target);
//         for (  ; 
//                iter_exptected != end(m_expected) && iter_test_target != end(_test_target); 
//                ++iter_exptected, ++iter_test_target, ++idx) 
//         {
//             if (*iter_exptected != *iter_test_target) {
//                 UNSCOPED_INFO("Items at index = [" << idx 
//                             << "] are not equal, item in test target is {" << int(*iter_test_target) 
//                             << "} and item in expected is {" << int(*iter_exptected) << "}.");
//                 return false; 
//             }
//         }

//         if (iter_exptected != end(m_expected) || iter_test_target != end(_test_target)) {
//             UNSCOPED_INFO("Size is not equal");
//             return false;
//         }

//         return false;

//     }

//     std::string describe() const override {
//         return "Equals: " + Catch::rangeToString(m_expected);
//     }

// private:
//     Range const& m_expected;
// };

// template<typename Range>
// auto EqualsRange(const Range& range) -> EqualsRangeMatcher<Range> {
//     return EqualsRangeMatcher<Range>{range};
// }


} // end namespace CatchMatcher
} // end namespace UnitTest
} // end namespace apf


#endif // _APF_CUSTOM_CATCH_MATCHER__

