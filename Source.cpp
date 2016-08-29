#include <iostream>
#include <iomanip>
#include <array>
#include <cassert>

using namespace std;

enum instruction_t : uint16_t
{
    OP_AND = 0x1000,
    OP_OR = 0x2000,
    OP_XOR = 0x3000,
    OP_LS1 = 0x4000,
    OP_LS0 = 0x5000,

    OP_MASK = 0xF000,

    RA_IMM = 0x0010,
    RA_CONSUME = 0x0008,
    RA_MASK = 0x001F,
#define RA_IMMVAL(X) (RA_IMM | (X))
#define RA_IDX(X) (X)

    RB_IMM = 0x0010 << 5,
    RB_CONSUME = 0x0008 << 5,
    RB_MASK = 0x001F << 5,
#define RB_IMMVAL(X) (RB_IMM | ((X) << 5))
#define RB_IDX(X) ((X) << 5)

    RO_DROP = 0x0000,
    RO_PC = 0x0800,
    RO_STK = 0x0400,
    RO_MASK = 0x0C00,
};

struct Instruction
{
    uint16_t instr;

    constexpr uint16_t regA_consume() const { return instr & RA_CONSUME; }
    constexpr uint16_t regA_imm() const { return instr & RA_IMM; }
    constexpr uint16_t regB_consume() const { return instr & RB_CONSUME; }
    constexpr uint16_t regB_imm() const { return instr & RB_IMM; }
    constexpr uint16_t regA() const { return instr & RA_MASK; }
    constexpr uint16_t regB() const { return (instr & RB_MASK) >> 5; }
    constexpr uint16_t op() const { return instr & OP_MASK; }
    constexpr uint16_t out() const { return instr & RO_MASK; }
};

struct ProcState
{
    uint16_t ip = 0;
    std::array<uint16_t, 16> stack = { 0 };

    uint16_t opval1 = 0;
    uint16_t opval2 = 0;

    uint16_t opresult = 0;

    uint16_t regval(uint16_t r) const
    {
        if (r & 0x10)
            return r & 0xF;
        else
            return stack[r & 0x7];
    }

    void simulateStep(const Instruction& in, ProcState& out) const
    {

        out.opval1 = regval(in.regA());
        out.opval2 = regval(in.regB());

        switch (in.op())
        {
        case OP_AND: out.opresult = opval1 & opval2; break;
        case OP_OR: out.opresult = opval1 | opval2; break;
        case OP_XOR: out.opresult = opval1 ^ opval2; break;
        case OP_LS1: out.opresult = ((opval1 + 1) << opval2) - 1; break; // fill with 1s
        case OP_LS0: out.opresult = opval1 << opval2; break; // fill with 0s
        default:
            abort();
        }

        if (in.out() == RO_PC)
            out.ip = opresult;
        else
            out.ip = ip + 1;

        // Overall, this should copy the stack over while removing all "consumed" registers and inserting if output to stack
        auto out_b = out.stack.begin();
        auto in_b = stack.begin();
        if (in.out() == RO_STK)
        {
            *out_b++ = opresult;
        }
        for (int x = 0;x < 7;++x)
        {
            if (!in.regA_imm() && in.regA_consume() && in.regA() == x)
            {
                in_b++;
                continue;
            }
            if (!in.regB_imm() && in.regB_consume() && in.regB() == x)
            {
                in_b++;
                continue;
            }
            *out_b++ = *in_b++;
        }
        out_b = std::copy(in_b, in_b + min(stack.end() - in_b, out.stack.end() - out_b), out_b);
        std::fill(out_b, out.stack.end(), 0);
    }

    void print() const
    {
        cout << hex;
        cout << '[' << setw(4) << opval1 << ' ' << setw(4) << opval2 << ' ' << setw(4) << opresult << "] ";
        for (auto&& s : stack)
        {
            cout << setw(4) << s << ' ';
        }
        cout << "\n";
    }
};


int main()
{
    // Add reg1 to reg2
    // reg3 is P, reg4 is G
    //constexpr Instruction code[] = {
    //    InputLines(OP_XOR, REG_1, REG_2, REG_3),
    //    InputLines(OP_AND, REG_1, REG_2, REG_4),
    //    InputLines(OP_AND, REG_3, REG_3, REG_2),
    //    // G = ...
    //    InputLines(OP_LS0, REG_4, REG_4, REG_1, 1),

    //    InputLines(OP_AND, REG_1, REG_3, REG_1),
    //    InputLines(OP_OR, REG_1, REG_4, REG_4),
    //    // P = ...
    //    InputLines(OP_LS1, REG_3, REG_3, REG_1, 1),
    //    InputLines(OP_AND, REG_3, REG_1, REG_3),
    //    // G = ...

    //    InputLines(OP_LS0, REG_4, REG_4, REG_1, 2),
    //    InputLines(OP_AND, REG_1, REG_3, REG_1),
    //    InputLines(OP_OR, REG_1, REG_4, REG_4),
    //    // P = ...
    //    InputLines(OP_LS1, REG_3, REG_3, REG_1, 2),

    //    InputLines(OP_AND, REG_3, REG_1, REG_3),
    //    // G = ...
    //    InputLines(OP_LS0, REG_4, REG_4, REG_1, 4),
    //    InputLines(OP_AND, REG_1, REG_3, REG_1),
    //    InputLines(OP_OR, REG_1, REG_4, REG_4),

    //    // P = ...
    //    InputLines(OP_LS1, REG_3, REG_3, REG_1, 4),
    //    InputLines(OP_AND, REG_3, REG_1, REG_3),
    //    // G = ...
    //    InputLines(OP_LS0, REG_4, REG_4, REG_1, 8),
    //    InputLines(OP_AND, REG_1, REG_3, REG_1),
    //    InputLines(OP_OR, REG_1, REG_4, REG_4),

    //    // P = ...
    //    InputLines(OP_LS1, REG_3, REG_3, REG_1, 8),
    //    InputLines(OP_AND, REG_3, REG_1, REG_3),
    //    // C = G (nop)
    //    // S = P ^ Cprev
    //    InputLines(OP_LS0, REG_4, REG_4, REG_1, 1),
    //    InputLines(OP_XOR, REG_2, REG_1, REG_1),
    //};

#define RA_POP(X) (RA_IDX(X) | RA_CONSUME)
    Instruction program[] = {
        // 5 => push $A
        RA_IMMVAL(5)  | RB_IMMVAL(5)  | OP_OR  | RO_DROP,
        // 3 => push $B
        RA_IMMVAL(3)  | RB_IMMVAL(3)  | OP_OR  | RO_DROP,
        // bubble
        RA_IMMVAL(0)  | RB_IMMVAL(0)  | OP_OR  | RO_STK,
        // bubble
        RA_IMMVAL(0)  | RB_IMMVAL(0)  | OP_OR  | RO_STK,

        // $B xor $A => push $Pi
        RA_IDX(0)     | RB_IDX(1)     | OP_OR  | RO_DROP,
        // $B and $A => push $Gi
        RA_IDX(0)     | RB_IDX(1)     | OP_XOR | RO_DROP,
        // bubble
        RA_IDX(0)     | RB_IDX(1)     | OP_AND | RO_STK,

        // $Pi <<o 1 => push $Pp
        RA_IDX(0)     | RB_IMMVAL(1)  | OP_OR  | RO_STK,
        // $Gi <<z 1 => push $Gp
        RA_IDX(0)     | RB_IMMVAL(1)  | OP_LS1 | RO_DROP,
        // bubble
        RA_IDX(0)     | RB_IDX(1)     | OP_LS0 | RO_STK,
        // pop $Pp and $Pi => push $P
        RA_POP(0)     | RB_IDX(2)     | OP_OR  | RO_STK,
        // pop $Gp and $Pi => push $Gtmp
        // bubble
        // bubble
        // pop $Gtmp or pop $Gi => push $G
        // bubble

        // $P <<o 2 => push $Pp
        // $G <<z 2 => push $Gp
        // bubble
        // pop $Pp and $Pi => push $P
        RA_POP(0) | RB_IDX(2) | OP_OR | RO_STK,
        // pop $Gp and $Pi => push $Gtmp
        // bubble
        // bubble
        // pop $Gtmp or pop $Gi => push $G
        // bubble
        // bubble

        RA_IMMVAL(3)  | RB_IMMVAL(3)  | OP_AND | RO_DROP,
        RA_IMMVAL(3)  | RB_IMMVAL(3)  | OP_OR  | RO_STK,
        RA_IMMVAL(3)  | RB_IMMVAL(3)  | OP_OR  | RO_DROP,
        RA_IMMVAL(3)  | RB_IMMVAL(3)  | OP_OR  | RO_DROP,
        RA_IMMVAL(3)  | RB_IMMVAL(3)  | OP_OR  | RO_DROP,
        RA_IMMVAL(3)  | RB_IMMVAL(3)  | OP_OR  | RO_DROP,
    };
    constexpr size_t STAGES = sizeof(program) / sizeof(program[0]);
    ProcState proc[STAGES + 1];

    cout << setw(4) << 0 << ") ";
    proc[0].print();

    for (int x = 0;x < STAGES;++x)
    {
        proc[x].simulateStep(program[x], proc[x + 1]);
        cout << setw(4) << x+1 << ") ";
        proc[x + 1].print();
    }

    cout << "program finish!\n";
    cin.get();
    return 0;
}