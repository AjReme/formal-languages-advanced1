#include <stack>
#include <queue>
#include <vector>
#include <string>
#include <unordered_map>


extern "C" struct symbol_t {
    const char* name;
    void* pointer;
};


struct operand
{

    int32_t operands_number;
    const std::string str;

    operand(const int32_t operands_number, const std::string& str)
        : operands_number(operands_number), str(str) {}

};


class __infix_to_prefix
{

private:

    std::string _expr;
    std::vector<operand> _prefix;
    std::vector<operand> _func;
    std::vector<size_t> _comma_counter;

    bool is_digit_(const char symb) {
        return '0' <= symb && symb <= '9';
    }

    bool is_operand_(const char symb) {
        return symb == '+' || symb == '-' || symb == '*';
    }

    bool is_alpha_(const char symb) {
        return ('a' <= symb && symb <= 'z') || ('A' <= symb && symb <= 'Z') || symb == '_';
    }

    std::string get_num_(size_t& ind) {
        std::string str;
        while (ind < _expr.size() && is_digit_(_expr[ind])) {
            str += _expr[ind++];
        }
        return str;
    }

    std::string get_name_(size_t& ind) {
        std::string str;
        while (ind < _expr.size() && (is_digit_(_expr[ind]) || is_alpha_(_expr[ind]))) {
            str += _expr[ind++];
        }
        return str;
    }

    void pull_to_prefix_() {
        _prefix.push_back(_func.back());
        _func.pop_back();
    }

    int32_t get_priority_(const operand& op) {
        if (op.operands_number == 1) {
            if (op.str.front() == '+' || op.str.front() == '-') {
                return 30;
            }
        }
        else {
            if (op.str.front() == '+' || op.str.front() == '-') {
                return 10;
            }
            if (op.str.front() == '*') {
                return 20;
            }
        }
        return 0;
    }

    bool is_open_bracket_() {
        return _func.back().str.front() == '(';
    }

    bool cmp_priority_(
        const operand& lhs,
        const operand& rhs
    ) {
        return get_priority_(lhs) <= get_priority_(rhs);
    }

public:

    __infix_to_prefix(const std::string& expression) noexcept {
        for (char i : expression) {
            if (i != ' ') {
                _expr += i;
            }
        }
        _comma_counter.push_back(0);
    };

    std::vector<operand> convert() {
        for (size_t ind = 0; ind < _expr.size();) {
            if (is_digit_(_expr[ind])) {
                _prefix.emplace_back(-1, get_num_(ind));
            }
            else if (is_alpha_(_expr[ind])) {
                std::string str = get_name_(ind);
                if (ind >= _expr.size() || _expr[ind] != '(') {
                    _prefix.emplace_back(-1, str);
                }
                else {
                    _func.emplace_back(0, str);
                    _comma_counter.push_back(0);
                }
            }
            else if (_expr[ind] == ',') {
                while (!is_open_bracket_()) {
                    pull_to_prefix_();
                }
                ++ind, ++_comma_counter.back();
            }
            else if (is_operand_(_expr[ind])) {
                operand current(
                    ind == 0 || is_operand_(_expr[ind - 1]) ||
                    _expr[ind - 1] == ',' || _expr[ind - 1] == '(' ? 1 : 2,
                    std::string(1, _expr[ind])
                );
                while (!_func.empty() && !is_open_bracket_() && cmp_priority_(current, _func.back())) {
                    pull_to_prefix_();
                }
                _func.push_back(current);
                ++ind;
            }
            else if (_expr[ind] == '(') {
                _func.emplace_back(0, "(");
                ++ind;
            }
            else if (_expr[ind] == ')') {
                while (!_func.empty() && !is_open_bracket_()) {
                    pull_to_prefix_();
                }
                _func.pop_back();
                if (!_func.empty() && !is_open_bracket_() && _func.back().operands_number == 0) {
                    pull_to_prefix_();
                    if (_expr[ind - 1] != '(') {
                        _prefix.back().operands_number = _comma_counter.back() + 1;
                    }
                    _comma_counter.pop_back();
                }
                ++ind;
            }
        }
        while (!_func.empty()) {
            pull_to_prefix_();
        }
        return _prefix;
    }

};


auto infix_to_prefix(const std::string& str) {
    return __infix_to_prefix(str).convert();
}


class __jit_compiler
{

private:

    enum ARM_COMMAND : uint32_t
    {
        PROLOGUE    = 0xE92D4010, // push   {r4,lr}
        END         = 0xE8BD8010, // pop    {r4,pc}
        PUSH_R0     = 0xE52D0004, // push   {r0}
        POP_R0      = 0xE49D0004, // pop    {r0}
        POP_R1      = 0xE49D1004, // pop    {r1}
        POP_R2      = 0xE49D2004, // pop    {r2}
        POP_R3      = 0xE49D3004, // pop    {r3}
        MOV_R4_R0   = 0xE1A04000, // mov    r4, r0
        ADD_R0_R1   = 0xE0800001, // add    r0, r1
        SUB_R0_R1   = 0xE0400001, // sub    r0, r1
        MUL_R0_R1   = 0xE0000190, // mul    r0, r1
        LDR_R0_R0   = 0xE5900000, // ldr    r0, [r0]
        BLX_R4      = 0xE12FFF34, // blx    r4
        MOVW_R0     = 0xE3000000, // movw   r0, const
        MOWT_R0     = 0xE3400000, // mowt   r0, const
        MVN_R0_R0   = 0xE1E00000, // mvn    r0, r0
        ADD_R0_1    = 0xE2800001, // add    r0, #1
    };

    std::string _expr;
    uint32_t* _ptr;
    std::unordered_map<std::string, uint32_t> _globals;

    bool is_digit_(const char symb) {
        return '0' <= symb && symb <= '9';
    }

    void get_globals_(const symbol_t* externs) {
        for (size_t i = 0; externs[i].name != 0 || externs[i].pointer != 0; ++i) {
            std::string name;
            for (size_t j = 0; externs[i].name[j] != 0; ++j) {
                name += externs[i].name[j];
            }
            _globals[name] = (uint32_t)externs[i].pointer;
        }
    }

    void write_command_(const uint32_t cmd) {
        *_ptr = cmd;
        ++_ptr;
    }

    uint32_t get_w16bit_mask_(const uint32_t val) {
        return (val & 0xFFF) | (((val >> 12) & 0xF) << 16);
    }

    void set_constant_(const uint32_t val) {
        const uint32_t W = ((val << 16) >> 16);
        const uint32_t T = (val >> 16);
        write_command_(ARM_COMMAND::MOVW_R0 | get_w16bit_mask_(W));
        write_command_(ARM_COMMAND::MOWT_R0 | get_w16bit_mask_(T));
    }

    void push_constant_(const uint32_t val) {
        set_constant_(val);
        write_command_(ARM_COMMAND::PUSH_R0);
    }

    void push_global_(const uint32_t ptr) {
        set_constant_(ptr);
        write_command_(ARM_COMMAND::LDR_R0_R0);
        write_command_(ARM_COMMAND::PUSH_R0);
    }

    void unary_minus_() {
        write_command_(ARM_COMMAND::POP_R0);
        write_command_(ARM_COMMAND::MVN_R0_R0);
        write_command_(ARM_COMMAND::ADD_R0_1);
        write_command_(ARM_COMMAND::PUSH_R0);
    }

    void binary_plus_() {
        write_command_(ARM_COMMAND::POP_R1);
        write_command_(ARM_COMMAND::POP_R0);
        write_command_(ARM_COMMAND::ADD_R0_R1);
        write_command_(ARM_COMMAND::PUSH_R0);
    }

    void binary_minus_() {
        write_command_(ARM_COMMAND::POP_R1);
        write_command_(ARM_COMMAND::POP_R0);
        write_command_(ARM_COMMAND::SUB_R0_R1);
        write_command_(ARM_COMMAND::PUSH_R0);
    }

    void binary_mul_() {
        write_command_(ARM_COMMAND::POP_R1);
        write_command_(ARM_COMMAND::POP_R0);
        write_command_(ARM_COMMAND::MUL_R0_R1);
        write_command_(ARM_COMMAND::PUSH_R0);
    }

    void call_func_(const uint32_t ptr, const uint32_t operands_number) {
        set_constant_(ptr);
        write_command_(ARM_COMMAND::MOV_R4_R0);
        if (operands_number >= 4) {
            write_command_(ARM_COMMAND::POP_R3);
        }
        if (operands_number >= 3) {
            write_command_(ARM_COMMAND::POP_R2);
        }
        if (operands_number >= 2) {
            write_command_(ARM_COMMAND::POP_R1);
        }
        if (operands_number >= 1) {
            write_command_(ARM_COMMAND::POP_R0);
        }
        write_command_(ARM_COMMAND::BLX_R4);
        write_command_(ARM_COMMAND::PUSH_R0);
    }

    void begin_() {
        write_command_(ARM_COMMAND::PROLOGUE);
    }

    void end_() {
        write_command_(ARM_COMMAND::POP_R0);
        write_command_(ARM_COMMAND::END);
    }

public:

    __jit_compiler(
        const char* expression,
        const symbol_t* externs,
        void* out_buffer
    ) : _ptr((uint32_t*)out_buffer) {
        for (size_t i = 0; expression[i] != 0; ++i) {
            _expr += expression[i];
        }
        get_globals_(externs);
    }

    void compile() {
        std::vector<operand> vec = infix_to_prefix(_expr);
        std::stack<operand> operands;
        begin_();
        for (const auto& i : vec) {
            if (i.operands_number == -1) {
                if (is_digit_(i.str.front())) {
                    push_constant_(std::stoi(i.str));
                }
                else {
                    push_global_(_globals[i.str]);
                }
            }
            else if (i.operands_number == 1 && i.str.front() == '-') {
                unary_minus_();
            }
            else if (i.operands_number == 1 && i.str.front() == '+') {
                continue;
            }
            else if (i.operands_number == 2 && i.str.front() == '-') {
                binary_minus_();
            }
            else if (i.operands_number == 2 && i.str.front() == '+') {
                binary_plus_();
            }
            else if (i.operands_number == 2 && i.str.front() == '*') {
                binary_mul_();
            }
            else {
                call_func_(_globals[i.str], i.operands_number);
            }
        }
        end_();
    }

};


extern "C" void jit_compile_expression_to_arm(
    const char* expression,
    const symbol_t* externs,
    void* out_buffer
) {
    return __jit_compiler(expression, externs, out_buffer).compile();
}
