/*
 * e6809 for Raspberry Pi Pico
 *
 * @version     1.0.0
 * @author      smittytone
 * @copyright   2022
 * @licence     MIT
 *
 */
#ifndef _CPU_HEADER_
#define _CPU_HEADER_


/*
 * CONSTANTS
 */
#define KB64                    65536
#define BRANCH_MARKER           0x20

#define MODE_UNKNOWN            0
#define MODE_IMMEDIATE          1
#define MODE_DIRECT             2
#define MODE_INDEXED            3
#define MODE_EXTENDED           4
#define MODE_INHERENT           5

#define OPCODE_EXTENDED_1       0x10
#define OPCODE_EXTENDED_2       0x11

#define C_BIT                   0
#define V_BIT                   1
#define Z_BIT                   2
#define N_BIT                   3
#define I_BIT                   4
#define H_BIT                   5
#define F_BIT                   6
#define E_BIT                   7

#define REG_X                   0
#define REG_Y                   1
#define REG_U                   2
#define REG_S                   3

#define NMI_BIT                 0
#define IRQ_BIT                 1
#define FIRQ_BIT                2

#define SIGN_BIT_8              7
#define SIGN_BIT_16             15

#define MASK_ZC                 0xFA
#define MASK_NZ                 0xF3
#define MASK_NZC                0xF2
#define MASK_NZV                0xF1
#define MASK_NZVC               0xF0

#define PUSH_PULL_CC_REG        0x01
#define PUSH_PULL_ALL_REGS      0xFE
#define PUSH_PULL_EVERY_REG     0xFF
#define PUSH_PULL_PC_REG        0x80

#define PUSH_TO_HARD_STACK      true

#define START_VECTORS           0xFFF0
#define SWI3_VECTOR             0xFFF2
#define SWI2_VECTOR             0xFFF4
#define FIRQ_VECTOR             0xFFF6
#define IRQ_VECTOR              0xFFF8
#define SWI1_VECTOR             0xFFFA
#define NMI_VECTOR              0xFFFC
#define RESET_VECTOR            0xFFFE

#define BUS_STATE_RUN_BA        0
#define BUS_STATE_INT_BA        1
#define BUS_STATE_SYN_BA        2
#define BUS_STATE_HLT_BA        3
#define BUS_STATE_RUN_BS        4
#define BUS_STATE_INT_BS        5
#define BUS_STATE_SYN_BS        6
#define BUS_STATE_HLT_BS        7

#define BREAK_TO_MONITOR        0xFF

#define IRQ_STATE_ASSERTED      1
#define IRQ_STATE_HANDLED       2


/*
 * STRUCTURES
 */
typedef struct {
    uint8_t  a;
    uint8_t  b;
    uint16_t d; // Only used for TFR/EXG ops which require a pointer to a unit16_7
    uint16_t x;
    uint16_t y;
    uint16_t s;
    uint16_t u;
    uint16_t pc;
    uint8_t  cc;
    uint8_t  dp;
} REG_6809;

typedef struct {
    bool        wait_for_interrupt;
    bool        is_sync;
    uint8_t     interrupts;
    // May drop these below
    uint8_t     bus_state_pins;
    uint8_t     interrupt_state;

} STATE_6809;


/*
 * PROTOTYPES
 */
uint32_t    process_next_instruction();
void        do_branch(uint8_t bop, bool is_long);

// Memory access
uint8_t     get_next_byte();
uint8_t     get_byte(uint16_t address);
void        set_byte(uint16_t address, uint8_t value);
void        move_pc(int16_t amount);
bool        is_bit_set(uint16_t value, uint8_t bit);

// Condition code register bit-level getters and setters
bool        is_cc_bit_set(uint8_t bit);
void        set_cc_bit(uint8_t bit);
void        clr_cc_bit(uint8_t bit);
void        flp_cc_bit(uint8_t bit);
void        clr_cc_nzv();
void        set_cc_nz(uint16_t value, bool is_16_bit);
void        set_cc_after_clr();
void        set_cc_after_load(uint16_t value, bool is_16_bit);
void        set_cc_after_store(uint16_t value, bool is_16_bit);

// Op Primary Functions
void        abx();
void        adc(uint8_t op, uint8_t mode);
void        add(uint8_t op, uint8_t mode);
void        add_16(uint8_t op, uint8_t mode);
void        and(uint8_t op, uint8_t mode);
void        andcc(uint8_t value);
void        asl(uint8_t op, uint8_t mode);
void        asr(uint8_t op, uint8_t mode);
void        bit(uint8_t op, uint8_t mode);
void        clr(uint8_t op, uint8_t mode);
void        cmp(uint8_t op, uint8_t mode);
void        cmp_16(uint8_t op, uint8_t mode, uint8_t ex_op);
void        com(uint8_t op, uint8_t mode);
void        cwai();
void        daa();
void        dec(uint8_t op, uint8_t mode);
void        eor(uint8_t op, uint8_t mode);
void        inc(uint8_t op, uint8_t mode);
void        jmp(uint8_t mode);
void        jsr(uint8_t mode);
void        ld(uint8_t op, uint8_t mode);
void        ld_16(uint8_t op, uint8_t mode, uint8_t ex_op);
void        lea(uint8_t op);
void        lsr(uint8_t op, uint8_t mode);
void        mul();
void        neg(uint8_t op, uint8_t mode);
void        orr(uint8_t op, uint8_t mode);
void        orcc(uint8_t value);
void        rol(uint8_t op, uint8_t mode);
void        ror(uint8_t op, uint8_t mode);
void        rti();
void        rts();
void        sbc(uint8_t op, uint8_t mode);
void        sex();
void        st(uint8_t op, uint8_t mode);
void        st_16(uint8_t op, uint8_t mode, uint8_t ex_op);
void        sub(uint8_t op, uint8_t mode);
void        sub_16(uint8_t op, uint8_t mode, uint8_t ex_op);
void        swi(uint8_t number);
void        sync();
void        tst(uint8_t op, uint8_t mode);

// Op Helper Functions
uint8_t     alu(uint8_t value_1, uint8_t value_2, bool use_carry);
uint16_t    alu_16(uint16_t value_1, uint16_t value_2, bool use_carry);
uint8_t     add_no_carry(uint8_t value, uint8_t amount);
uint8_t     add_with_carry(uint8_t value, uint8_t amount);
uint8_t     subtract(uint8_t value, uint8_t amount);
uint16_t    subtract_16(uint16_t value_1, uint16_t value_2);
uint8_t     sub_with_carry(uint8_t value, uint8_t amount);
uint8_t     base_sub(uint8_t value, uint8_t amount, bool use_carry);
uint8_t     do_and(uint8_t value, uint8_t amount);
uint8_t     do_or(uint8_t value, uint8_t amount);
uint8_t     do_xor(uint8_t value, uint8_t amount);
uint8_t     arith_shift_right(uint8_t value);
uint8_t     logic_shift_left(uint8_t value);
uint8_t     logic_shift_right(uint8_t value);
uint8_t     partial_shift_right(uint8_t value);
uint8_t     rotate_left(uint8_t value);
uint8_t     rotate_right(uint8_t value);
void        compare(uint8_t value, uint8_t amount);
uint8_t     negate(uint8_t value, bool ignore);
uint8_t     ones_complement(uint8_t value);
uint8_t     twos_complement(uint8_t value);
uint8_t     complement(uint8_t value);
uint8_t     decrement(uint8_t value);
uint8_t     increment(uint8_t value);
uint8_t     *set_reg_ptr(uint8_t reg_code);
uint16_t    *set_reg_16_ptr(uint8_t reg_code);
void        transfer_decode2(uint8_t reg_code, bool is_swap);
void        transfer_decode(uint8_t reg_code, bool is_swap);
uint8_t     exchange(uint8_t value, uint8_t reg_code);
uint16_t    exchange_16(uint16_t value, uint8_t reg_code);
void        load_effective(uint16_t amount, uint8_t reg_code);
void        push(bool to_hardware, uint8_t post_byte);
void        pull(bool from_hardware, uint8_t post_byte);
void        test(uint8_t value);

// Addressing Functions
uint16_t    address_from_mode(uint8_t mode);
uint16_t    address_from_next_two_bytes();
uint16_t    address_from_dpr(int16_t offset);
uint16_t    indexed_address(uint8_t post_byte);
uint16_t    register_value(uint8_t source_reg);
void        increment_register(uint8_t source_reg, int16_t amount);

// IO
void        process_interrupt(uint8_t irq);

// Misc
void init_cpu();
void init_vectors(uint16_t* vectors);
void reset_registers();


#endif // _CPU_HEADER_
