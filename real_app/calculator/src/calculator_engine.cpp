#include "calculator_engine.hpp"
#include <stack>
#include <cctype>
#include <algorithm>
#include <cmath>

namespace enki::calc {

static constexpr double PI_VAL = 3.14159265358979323846;
static constexpr double E_VAL  = 2.71828182845904523536;

CalculatorEngine::CalculatorEngine() {
    clearAll();
}

std::string CalculatorEngine::formatDouble(double value) {
    if (std::isnan(value)) return "Error: NaN";
    if (std::isinf(value)) return (value > 0) ? "Error: Infinity" : "Error: -Infinity";

    // Format with high precision and trim redundant trailing zeros
    std::ostringstream ss;
    if (std::abs(value) > 1e14 || (std::abs(value) < 1e-6 && value != 0.0)) {
        ss << std::scientific << std::setprecision(8) << value;
        return ss.str();
    }

    ss << std::fixed << std::setprecision(10) << value;
    std::string s = ss.str();
    // Trim trailing zeros
    if (s.find('.') != std::string::npos) {
        s.erase(s.find_last_not_of('0') + 1, std::string::npos);
        if (s.back() == '.') s.pop_back();
    }
    return s;
}

void CalculatorEngine::clearAll() {
    display_main_ = "0";
    display_expr_ = "";
    current_entry_ = "";
    is_result_ = false;
    has_error_ = false;
    just_opened_paren_ = false;
}

void CalculatorEngine::clearEntry() {
    if (has_error_) {
        clearAll();
        return;
    }
    current_entry_ = "";
    display_main_ = "0";
}

void CalculatorEngine::backspace() {
    if (has_error_ || is_result_) {
        clearAll();
        return;
    }

    if (!current_entry_.empty()) {
        current_entry_.pop_back();
        if (current_entry_.empty() || current_entry_ == "-") {
            current_entry_ = "";
            display_main_ = "0";
        } else {
            display_main_ = current_entry_;
        }
    } else if (!display_expr_.empty()) {
        while (!display_expr_.empty() && display_expr_.back() == ' ') {
            display_expr_.pop_back();
        }
        if (!display_expr_.empty()) display_expr_.pop_back();
    }
}

void CalculatorEngine::toggleSign() {
    if (has_error_) return;

    if (current_entry_.empty()) {
        if (display_main_ != "0") {
            if (display_main_[0] == '-') {
                display_main_.erase(0, 1);
            } else {
                display_main_ = "-" + display_main_;
            }
            current_entry_ = display_main_;
        }
    } else {
        if (current_entry_[0] == '-') {
            current_entry_.erase(0, 1);
        } else {
            current_entry_ = "-" + current_entry_;
        }
        display_main_ = current_entry_;
    }
}

void CalculatorEngine::inputDigit(char digit) {
    if (has_error_ || is_result_) {
        clearAll();
    }

    if (current_entry_ == "0") {
        current_entry_ = std::string(1, digit);
    } else {
        current_entry_ += digit;
    }
    display_main_ = current_entry_;
}

void CalculatorEngine::inputDecimal() {
    if (has_error_ || is_result_) {
        clearAll();
    }

    if (current_entry_.empty()) {
        current_entry_ = "0.";
    } else if (current_entry_.find('.') == std::string::npos) {
        current_entry_ += ".";
    }
    display_main_ = current_entry_;
}

void CalculatorEngine::commitCurrentEntryToExpr() {
    if (!current_entry_.empty()) {
        if (!display_expr_.empty() && display_expr_.back() != '(' && display_expr_.back() != ' ') {
            display_expr_ += " ";
        }
        display_expr_ += current_entry_;
        current_entry_ = "";
    } else if (display_expr_.empty()) {
        display_expr_ = display_main_;
    }
}

void CalculatorEngine::inputOperator(char op) {
    if (has_error_) clearAll();

    commitCurrentEntryToExpr();

    // If last token in expr is an operator, replace it
    if (!display_expr_.empty()) {
        char last = display_expr_.back();
        if (last == '+' || last == '-' || last == '*' || last == '/' || last == '%' || last == '^') {
            display_expr_.pop_back();
            while (!display_expr_.empty() && display_expr_.back() == ' ') display_expr_.pop_back();
        }
    }

    display_expr_ += " ";
    display_expr_ += op;
    display_expr_ += " ";

    is_result_ = false;
}

void CalculatorEngine::inputParenthesis(char paren) {
    if (has_error_) clearAll();

    if (paren == '(') {
        if (!current_entry_.empty()) {
            commitCurrentEntryToExpr();
            display_expr_ += " * ";
        }
        display_expr_ += "(";
        just_opened_paren_ = true;
    } else if (paren == ')') {
        commitCurrentEntryToExpr();
        display_expr_ += ")";
    }
    is_result_ = false;
}

void CalculatorEngine::inputConstant(const std::string& name) {
    if (has_error_ || is_result_) clearAll();

    double val = (name == "pi" || name == "π") ? PI_VAL : E_VAL;
    current_entry_ = formatDouble(val);
    display_main_ = current_entry_;
}

void CalculatorEngine::inputFunction(const std::string& func) {
    if (has_error_) clearAll();

    // Immediate evaluation on current entry or display
    double val = 0.0;
    try {
        val = std::stod(display_main_);
    } catch (...) {
        val = 0.0;
    }

    double res = 0.0;
    bool valid = true;

    if (func == "sqrt" || func == "√") {
        if (val < 0.0) { has_error_ = true; display_main_ = "Error: Invalid Input"; return; }
        res = std::sqrt(val);
        display_expr_ = "√(" + display_main_ + ")";
    } else if (func == "sqr" || func == "x²") {
        res = val * val;
        display_expr_ = "sqr(" + display_main_ + ")";
    } else if (func == "cbrt") {
        res = std::cbrt(val);
        display_expr_ = "cbrt(" + display_main_ + ")";
    } else if (func == "inv" || func == "1/x") {
        if (val == 0.0) { has_error_ = true; display_main_ = "Error: Div by 0"; return; }
        res = 1.0 / val;
        display_expr_ = "1/(" + display_main_ + ")";
    } else if (func == "sin") {
        double rad = (angle_mode_ == AngleMode::Degree) ? (val * PI_VAL / 180.0) : val;
        res = std::sin(rad);
        display_expr_ = "sin(" + display_main_ + ")";
    } else if (func == "cos") {
        double rad = (angle_mode_ == AngleMode::Degree) ? (val * PI_VAL / 180.0) : val;
        res = std::cos(rad);
        display_expr_ = "cos(" + display_main_ + ")";
    } else if (func == "tan") {
        double rad = (angle_mode_ == AngleMode::Degree) ? (val * PI_VAL / 180.0) : val;
        if (std::abs(std::cos(rad)) < 1e-12) { has_error_ = true; display_main_ = "Error: Undefined"; return; }
        res = std::tan(rad);
        display_expr_ = "tan(" + display_main_ + ")";
    } else if (func == "ln") {
        if (val <= 0.0) { has_error_ = true; display_main_ = "Error: Invalid Input"; return; }
        res = std::log(val);
        display_expr_ = "ln(" + display_main_ + ")";
    } else if (func == "log" || func == "log10") {
        if (val <= 0.0) { has_error_ = true; display_main_ = "Error: Invalid Input"; return; }
        res = std::log10(val);
        display_expr_ = "log(" + display_main_ + ")";
    } else if (func == "fact" || func == "n!") {
        if (val < 0.0 || val > 170.0 || std::floor(val) != val) {
            has_error_ = true; display_main_ = "Error: Overflow/Invalid"; return;
        }
        res = 1.0;
        int n = static_cast<int>(val);
        for (int i = 2; i <= n; ++i) res *= i;
        display_expr_ = "fact(" + display_main_ + ")";
    } else if (func == "abs") {
        res = std::abs(val);
        display_expr_ = "abs(" + display_main_ + ")";
    } else {
        valid = false;
    }

    if (valid) {
        current_entry_ = formatDouble(res);
        display_main_ = current_entry_;
        is_result_ = true;
    }
}

void CalculatorEngine::toggleAngleMode() {
    angle_mode_ = (angle_mode_ == AngleMode::Degree) ? AngleMode::Radian : AngleMode::Degree;
}

void CalculatorEngine::memoryClear() {
    memory_val_ = 0.0;
    memory_has_value_ = false;
}

void CalculatorEngine::memoryRecall() {
    if (!memory_has_value_) return;
    current_entry_ = formatDouble(memory_val_);
    display_main_ = current_entry_;
    is_result_ = false;
}

void CalculatorEngine::memoryAdd() {
    try {
        memory_val_ += std::stod(display_main_);
        memory_has_value_ = true;
    } catch (...) {}
}

void CalculatorEngine::memorySubtract() {
    try {
        memory_val_ -= std::stod(display_main_);
        memory_has_value_ = true;
    } catch (...) {}
}

void CalculatorEngine::recallHistory(size_t index) {
    if (index < history_.size()) {
        clearAll();
        current_entry_ = history_[index].result;
        display_main_ = current_entry_;
        display_expr_ = history_[index].expression + " =";
        is_result_ = true;
    }
}

// ════════════════════════════════════════════════════════════════
// Expression Evaluator (Shunting-Yard & Postfix Evaluation)
// ════════════════════════════════════════════════════════════════

static int getPrecedence(char op) {
    switch (op) {
        case '+': case '-': return 1;
        case '*': case '/': case '%': return 2;
        case '^': return 3;
        default: return 0;
    }
}

static bool isRightAssociative(char op) {
    return op == '^';
}

double CalculatorEngine::evaluateExpression(const std::string& expr, AngleMode, bool& out_error, std::string& out_err_msg) {
    out_error = false;
    out_err_msg = "";

    std::vector<std::string> output_queue;
    std::stack<char> op_stack;

    size_t i = 0;
    size_t len = expr.length();
    bool expect_unary = true;

    while (i < len) {
        char c = expr[i];
        if (std::isspace(c)) {
            i++;
            continue;
        }

        if (std::isdigit(c) || c == '.') {
            std::string num_str;
            while (i < len && (std::isdigit(expr[i]) || expr[i] == '.' || expr[i] == 'e' || expr[i] == 'E')) {
                num_str += expr[i++];
            }
            output_queue.push_back(num_str);
            expect_unary = false;
            continue;
        }

        if (c == '-' && expect_unary) {
            // Unary negation: push a zero first
            output_queue.push_back("0");
            op_stack.push('-');
            i++;
            expect_unary = true;
            continue;
        }

        if (c == '+' && expect_unary) {
            i++; // skip unary plus
            continue;
        }

        if (c == '(') {
            op_stack.push(c);
            expect_unary = true;
            i++;
            continue;
        }

        if (c == ')') {
            while (!op_stack.empty() && op_stack.top() != '(') {
                output_queue.push_back(std::string(1, op_stack.top()));
                op_stack.pop();
            }
            if (op_stack.empty()) {
                out_error = true;
                out_err_msg = "Error: Mismatched Parens";
                return 0.0;
            }
            op_stack.pop(); // pop '('
            expect_unary = false;
            i++;
            continue;
        }

        if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '^') {
            while (!op_stack.empty() && op_stack.top() != '(') {
                char top_op = op_stack.top();
                int p1 = getPrecedence(c);
                int p2 = getPrecedence(top_op);
                if ((!isRightAssociative(c) && p1 <= p2) || (isRightAssociative(c) && p1 < p2)) {
                    output_queue.push_back(std::string(1, top_op));
                    op_stack.pop();
                } else {
                    break;
                }
            }
            op_stack.push(c);
            expect_unary = true;
            i++;
            continue;
        }

        i++;
    }

    while (!op_stack.empty()) {
        if (op_stack.top() == '(' || op_stack.top() == ')') {
            out_error = true;
            out_err_msg = "Error: Mismatched Parens";
            return 0.0;
        }
        output_queue.push_back(std::string(1, op_stack.top()));
        op_stack.pop();
    }

    // Evaluate Postfix
    std::stack<double> val_stack;
    for (const auto& token : output_queue) {
        if (token.size() == 1 && (token[0] == '+' || token[0] == '-' || token[0] == '*' || token[0] == '/' || token[0] == '%' || token[0] == '^')) {
            if (val_stack.size() < 2) {
                out_error = true;
                out_err_msg = "Error: Syntax";
                return 0.0;
            }
            double b = val_stack.top(); val_stack.pop();
            double a = val_stack.top(); val_stack.pop();
            char op = token[0];
            double r = 0.0;

            if (op == '+') r = a + b;
            else if (op == '-') r = a - b;
            else if (op == '*') r = a * b;
            else if (op == '/') {
                if (b == 0.0) { out_error = true; out_err_msg = "Error: Div by 0"; return 0.0; }
                r = a / b;
            } else if (op == '%') {
                if (b == 0.0) { out_error = true; out_err_msg = "Error: Div by 0"; return 0.0; }
                r = std::fmod(a, b);
            } else if (op == '^') {
                r = std::pow(a, b);
            }
            val_stack.push(r);
        } else {
            try {
                val_stack.push(std::stod(token));
            } catch (...) {
                out_error = true;
                out_err_msg = "Error: Parse";
                return 0.0;
            }
        }
    }

    if (val_stack.empty()) return 0.0;
    return val_stack.top();
}

bool CalculatorEngine::evaluate() {
    if (has_error_) return false;

    commitCurrentEntryToExpr();

    if (display_expr_.empty()) {
        display_expr_ = display_main_;
    }

    bool err = false;
    std::string err_msg;
    double res = evaluateExpression(display_expr_, angle_mode_, err, err_msg);

    if (err) {
        has_error_ = true;
        display_main_ = err_msg;
        return false;
    }

    std::string prev_expr = display_expr_;
    std::string result_str = formatDouble(res);

    history_.push_back({prev_expr, result_str});
    display_expr_ = prev_expr + " =";
    display_main_ = result_str;
    current_entry_ = "";
    is_result_ = true;
    return true;
}

} // namespace enki::calc
