#pragma once
/// @file calculator_engine.hpp
/// @brief High-precision mathematical evaluation engine for the ENKI Quantum Calculator.
///
/// Features:
///   - Robust Shunting-Yard expression evaluation supporting operator precedence and associativity.
///   - Standard arithmetic: +, -, *, /, % (modulo), ^ (power).
///   - Unary operators: negation (-x), percentage (x%).
///   - Full parenthesis nesting: (...).
///   - Scientific functions: sin, cos, tan, asin, acos, atan, sqrt, cbrt, log10, ln, abs, fact (n!).
///   - Constants: pi (π), e.
///   - Angle modes: Degree and Radian.
///   - Memory registers: MC, MR, M+, M-.
///   - Comprehensive calculation history tape.
///   - Graceful error detection (e.g. division by zero, domain errors).
///
/// @copyright ENKI Framework — MIT License

#include <string>
#include <vector>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <functional>
#include <optional>

namespace enki::calc {

enum class AngleMode {
    Degree,
    Radian
};

struct HistoryItem {
    std::string expression;
    std::string result;
};

class CalculatorEngine {
public:
    CalculatorEngine();

    // ── Input Actions ──────────────────────────────────────────
    void inputDigit(char digit);
    void inputDecimal();
    void inputOperator(char op);
    void inputFunction(const std::string& func);
    void inputParenthesis(char paren);
    void inputConstant(const std::string& name);
    void toggleSign();
    void backspace();
    void clearAll();
    void clearEntry();

    // ── Execution ──────────────────────────────────────────────
    bool evaluate();

    // ── Memory Operations ──────────────────────────────────────
    void memoryClear();
    void memoryRecall();
    void memoryAdd();
    void memorySubtract();
    [[nodiscard]] bool hasMemory() const { return memory_has_value_; }

    // ── Angle Mode ─────────────────────────────────────────────
    void toggleAngleMode();
    [[nodiscard]] AngleMode angleMode() const { return angle_mode_; }
    [[nodiscard]] std::string angleModeStr() const { return angle_mode_ == AngleMode::Degree ? "DEG" : "RAD"; }

    // ── Display Queries ────────────────────────────────────────
    [[nodiscard]] const std::string& displayMain() const { return display_main_; }
    [[nodiscard]] const std::string& displayExpression() const { return display_expr_; }
    [[nodiscard]] bool hasError() const { return has_error_; }
    [[nodiscard]] bool isResult() const { return is_result_; }

    // ── History Tape ───────────────────────────────────────────
    [[nodiscard]] const std::vector<HistoryItem>& history() const { return history_; }
    void clearHistory() { history_.clear(); }
    void recallHistory(size_t index);

    // ── Direct Formula Evaluation Helper ───────────────────────
    static double evaluateExpression(const std::string& expr, AngleMode mode, bool& out_error, std::string& out_err_msg);

    // ── Number Formatting ──────────────────────────────────────
    static std::string formatDouble(double value);

private:
    std::string display_main_ = "0";
    std::string display_expr_ = "";
    std::string current_entry_ = "";
    bool is_result_ = false;
    bool has_error_ = false;
    bool just_opened_paren_ = false;

    double memory_val_ = 0.0;
    bool memory_has_value_ = false;

    AngleMode angle_mode_ = AngleMode::Degree;
    std::vector<HistoryItem> history_;

    void commitCurrentEntryToExpr();
};

} // namespace enki::calc
