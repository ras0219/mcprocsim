#include <iostream>
#include <iomanip>

using namespace std;

enum reg_idx : uint32_t
{
    REG_1 = 0,
    REG_2 = 1,
    REG_3 = 2,
    REG_4 = 3,
};
enum op_idx : uint32_t
{
    OP_AND,
    OP_OR,
    OP_XOR,
    OP_LSHIFTIMMONE,
    OP_LSHIFTIMMZERO,
};

struct InputLines
{
    constexpr InputLines() : InputLines(OP_AND, REG_1, REG_1, REG_1, 0) {}
    constexpr InputLines(op_idx o, reg_idx r1, reg_idx r2, reg_idx ro, uint32_t i = 0)
        : op(o)
        , reg1(r1)
        , reg2(r2)
        , regOut(ro)
        , imm(i)
    {}

    op_idx op : 3;
    reg_idx reg1 : 2;
    reg_idx reg2 : 2;
    reg_idx regOut : 2;
    uint32_t imm : 11;
};

struct ProcState
{
    uint32_t reg1 = 0;
    uint32_t reg2 = 0;
    uint32_t reg3 = 0;
    uint32_t reg4 = 0;

    uint32_t decode_reg(reg_idx x) const
    {
        switch (x)
        {
        case REG_1: return reg1;
        case REG_2: return reg2;
        case REG_3: return reg3;
        case REG_4: return reg4;
        default: abort();
        }
    }

    uint32_t& decode_reg(reg_idx x)
    {
        switch (x)
        {
        case REG_1: return reg1;
        case REG_2: return reg2;
        case REG_3: return reg3;
        case REG_4: return reg4;
        default: abort();
        }
    }

    void simulateStep(const InputLines& in, ProcState& out) const
    {
        auto v1 = decode_reg(in.reg1);
        auto v2 = decode_reg(in.reg2);
        uint32_t result;
        switch (in.op)
        {
        case OP_AND: result = v1 & v2; break;
        case OP_OR: result = v1 | v2; break;
        case OP_XOR: result = v1 ^ v2; break;
        case OP_LSHIFTIMMONE: result = ((v1+1) << in.imm) - 1; break; // fill with 1s
        case OP_LSHIFTIMMZERO: result = v1 << in.imm; break; // fill with 0s
        default:
            abort();
        }

        out = *this;
        out.decode_reg(in.regOut) = result;
    }

    void print() const
    {
        cout << hex << "regs: {" << reg1 << ", " << reg2 << ", " << reg3 << ", " << reg4 << "}\n";
    }
};


int main()
{
    constexpr size_t STAGES = 25;
    ProcState proc[STAGES + 1];
    proc[0].reg1 = 0xF0F1;
    proc[0].reg2 = 0x0F0F;

    constexpr auto x = 0xF0F1 + 0x0F0F - 0x10000;

    // Add reg1 to reg2
    // reg3 is P, reg4 is G
    constexpr InputLines code[STAGES] = {
        InputLines(OP_XOR, REG_1, REG_2, REG_3),
        InputLines(OP_AND, REG_1, REG_2, REG_4),
        InputLines(OP_AND, REG_3, REG_3, REG_2),
        // G = ...
        InputLines(OP_LSHIFTIMMZERO, REG_4, REG_4, REG_1, 1),

        InputLines(OP_AND, REG_1, REG_3, REG_1),
        InputLines(OP_OR, REG_1, REG_4, REG_4),
        // P = ...
        InputLines(OP_LSHIFTIMMONE, REG_3, REG_3, REG_1, 1),
        InputLines(OP_AND, REG_3, REG_1, REG_3),
        // G = ...

        InputLines(OP_LSHIFTIMMZERO, REG_4, REG_4, REG_1, 2),
        InputLines(OP_AND, REG_1, REG_3, REG_1),
        InputLines(OP_OR, REG_1, REG_4, REG_4),
        // P = ...
        InputLines(OP_LSHIFTIMMONE, REG_3, REG_3, REG_1, 2),

        InputLines(OP_AND, REG_3, REG_1, REG_3),
        // G = ...
        InputLines(OP_LSHIFTIMMZERO, REG_4, REG_4, REG_1, 4),
        InputLines(OP_AND, REG_1, REG_3, REG_1),
        InputLines(OP_OR, REG_1, REG_4, REG_4),

        // P = ...
        InputLines(OP_LSHIFTIMMONE, REG_3, REG_3, REG_1, 4),
        InputLines(OP_AND, REG_3, REG_1, REG_3),
        // G = ...
        InputLines(OP_LSHIFTIMMZERO, REG_4, REG_4, REG_1, 8),
        InputLines(OP_AND, REG_1, REG_3, REG_1),
        InputLines(OP_OR, REG_1, REG_4, REG_4),

        // P = ...
        InputLines(OP_LSHIFTIMMONE, REG_3, REG_3, REG_1, 8),
        InputLines(OP_AND, REG_3, REG_1, REG_3),
        // C = G (nop)
        // S = P ^ Cprev
        InputLines(OP_LSHIFTIMMZERO, REG_4, REG_4, REG_1, 1),
        InputLines(OP_XOR, REG_2, REG_1, REG_1),
    };
    for (int x = 0;x < STAGES;++x)
    {
        proc[x].simulateStep(code[x], proc[x + 1]);
        cout << x << ") ";
        proc[x + 1].print();
    }

    cout << "program finish!\n";
    cin.get();
    return 0;
}