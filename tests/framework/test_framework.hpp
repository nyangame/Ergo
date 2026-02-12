#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <functional>
#include <iostream>
#include <cmath>
#include <cstdint>
#include <sstream>

// ============================================================
// Ergo Test Framework
// Lightweight, GUI-independent test runner for the Ergo engine.
// Runs as a separate assembly — no engine runtime or window required.
// ============================================================

namespace ergo::test {

// ------------------------------------------------------------
// Test result
// ------------------------------------------------------------
enum class TestResult : uint8_t {
    Passed,
    Failed
};

struct TestFailure {
    std::string file;
    int line = 0;
    std::string expression;
    std::string message;
};

// ------------------------------------------------------------
// TestContext: accumulates failures during a single test run
// ------------------------------------------------------------
struct TestContext {
    std::vector<TestFailure> failures;
    bool failed() const { return !failures.empty(); }

    void add_failure(const char* file, int line,
                     std::string_view expr, std::string_view msg = "") {
        failures.push_back({std::string(file), line,
                            std::string(expr), std::string(msg)});
    }
};

// ------------------------------------------------------------
// TestCase: a named test function
// ------------------------------------------------------------
struct TestCase {
    std::string name;
    std::function<void(TestContext&)> func;
};

// ------------------------------------------------------------
// TestSuite: collects and runs TestCases, reports results
// ------------------------------------------------------------
class TestSuite {
    std::string name_;
    std::vector<TestCase> cases_;

public:
    explicit TestSuite(std::string_view name) : name_(name) {}

    void add(std::string_view case_name, std::function<void(TestContext&)> fn) {
        cases_.push_back({std::string(case_name), std::move(fn)});
    }

    struct SuiteResult {
        uint32_t total = 0;
        uint32_t passed = 0;
        uint32_t failed = 0;
    };

    SuiteResult run() const {
        SuiteResult result;
        result.total = static_cast<uint32_t>(cases_.size());

        std::cout << "=== Suite: " << name_
                  << " (" << result.total << " tests) ===\n";

        for (const auto& tc : cases_) {
            TestContext ctx;
            tc.func(ctx);

            if (!ctx.failed()) {
                ++result.passed;
                std::cout << "  [PASS] " << tc.name << "\n";
            } else {
                ++result.failed;
                std::cout << "  [FAIL] " << tc.name << "\n";
                for (const auto& f : ctx.failures) {
                    std::cout << "         " << f.file << ":" << f.line
                              << "  " << f.expression;
                    if (!f.message.empty())
                        std::cout << "  -- " << f.message;
                    std::cout << "\n";
                }
            }
        }
        std::cout << "--- " << name_ << ": "
                  << result.passed << "/" << result.total << " passed";
        if (result.failed > 0)
            std::cout << " (" << result.failed << " FAILED)";
        std::cout << " ---\n\n";
        return result;
    }

    std::string_view name() const { return name_; }
    size_t size() const { return cases_.size(); }
};

// ------------------------------------------------------------
// TestRunner: aggregates suites, runs all, returns exit code
// ------------------------------------------------------------
class TestRunner {
    std::vector<TestSuite*> suites_;

public:
    void add_suite(TestSuite& suite) {
        suites_.push_back(&suite);
    }

    int run() const {
        uint32_t total = 0, passed = 0, failed = 0;

        std::cout << "==============================\n"
                  << "  Ergo Test Runner\n"
                  << "==============================\n\n";

        for (auto* suite : suites_) {
            auto r = suite->run();
            total  += r.total;
            passed += r.passed;
            failed += r.failed;
        }

        std::cout << "==============================\n"
                  << "  Total: " << total
                  << "  Passed: " << passed
                  << "  Failed: " << failed << "\n"
                  << "==============================\n";

        return (failed == 0) ? 0 : 1;
    }
};

} // namespace ergo::test

// ============================================================
// Assertion macros
// ============================================================

#define ERGO_TEST_ASSERT(ctx, expr)                                       \
    do {                                                                   \
        if (!(expr)) {                                                     \
            (ctx).add_failure(__FILE__, __LINE__, #expr);                   \
        }                                                                  \
    } while (false)

#define ERGO_TEST_ASSERT_MSG(ctx, expr, msg)                              \
    do {                                                                   \
        if (!(expr)) {                                                     \
            (ctx).add_failure(__FILE__, __LINE__, #expr, (msg));            \
        }                                                                  \
    } while (false)

#define ERGO_TEST_ASSERT_EQ(ctx, actual, expected)                        \
    do {                                                                   \
        auto _a = (actual);                                                \
        auto _e = (expected);                                              \
        if (_a != _e) {                                                    \
            std::ostringstream _oss;                                        \
            _oss << "expected " << _e << ", got " << _a;                   \
            (ctx).add_failure(__FILE__, __LINE__,                           \
                              #actual " == " #expected, _oss.str());        \
        }                                                                  \
    } while (false)

#define ERGO_TEST_ASSERT_NEAR(ctx, actual, expected, eps)                 \
    do {                                                                   \
        auto _a = (actual);                                                \
        auto _e = (expected);                                              \
        if (std::abs(_a - _e) > (eps)) {                                   \
            std::ostringstream _oss;                                        \
            _oss << "expected ~" << _e << ", got " << _a                   \
                 << " (eps=" << (eps) << ")";                              \
            (ctx).add_failure(__FILE__, __LINE__,                           \
                              #actual " ~= " #expected, _oss.str());       \
        }                                                                  \
    } while (false)

#define ERGO_TEST_ASSERT_TRUE(ctx, expr)  ERGO_TEST_ASSERT(ctx, expr)
#define ERGO_TEST_ASSERT_FALSE(ctx, expr) ERGO_TEST_ASSERT(ctx, !(expr))
