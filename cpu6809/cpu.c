/*
 * e6809 for Raspberry Pi Pico
 *
 * @version     1.0.0
 * @author      smittytone
 * @copyright   2022
 * @licence     MIT
 *
 */
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "ops.h"
#include "cpu.h"


/*
 * GLOBALS
 */
REG_6809    reg;
uint8_t     mem[KB64];
STATE_6809  state;


/*
 * SETUP FUNCTIONS
 */

/**
 * @brief Initalise the virtual 6809.
 */
void init_cpu() {
    // Set simulator state
    state.bus_state_pins = 0;
    state.interrupt_state = 0;
    state.interrupts = 0;
    state.is_sync = false;
    state.wait_for_interrupt = false;

    // Set initial register values
    reset_registers();
}


/**
 * @brief Initalise the virtual 6809's interrupt vectors.
 *
 * @param vectors: Pointer to the vector address table.
 */
void init_vectors(uint16_t* vectors) {
    uint16_t start = 0xFFFD;

    for (uint8_t i = 0 ; i < 8 ; ++i) {
        uint16_t vector = vectors[i];
        mem[start++] = (vector >> 8) & 0xFF;
        mem[start++] = (uint8_t)(vector & 0xFF);
        start -= 4;
    }
}


/**
 * @brief Load, decode and process a single instruction.
 *
 * @retval The number of CPU cycles consumed, or a break signal on RTI/RTS.
 */
uint32_t process_next_instruction() {
    // See Zaks p.250

    uint32_t cycles_used = 0;

    // IF HALT
    //      bus_state_pins = 3

    state.bus_state_pins = 0;

    if (state.wait_for_interrupt || state.interrupts > 0) {
        // Process interrupts
        if (state.interrupts > 0) {
            state.interrupt_state = IRQ_STATE_ASSERTED;

            if (state.interrupts & NMI_BIT) {
                process_interrupt(NMI_BIT);
                state.interrupt_state = IRQ_STATE_HANDLED;
            }

            if ((state.interrupts & FIRQ_BIT) && !is_cc_bit_set(F_BIT)) {
                process_interrupt(FIRQ_BIT);
                state.interrupt_state = IRQ_STATE_HANDLED;
            }

            if (state.interrupts & FIRQ_BIT) state.interrupts &= !FIRQ_BIT;

            if ((state.interrupts & IRQ_BIT) && !is_cc_bit_set(I_BIT)) {
                process_interrupt(IRQ_BIT);
                state.interrupt_state = IRQ_STATE_HANDLED;
            }

            if (state.interrupts & IRQ_BIT) state.interrupts &= !IRQ_BIT;

            // CWAI and SYNC end if the IRQ was handled
            if (state.interrupt_state == IRQ_STATE_HANDLED) {
                state.wait_for_interrupt = false;
                state.is_sync = false;
            } else {
                if (state.is_sync) {
                    // SYNC continues processing on unhandled IRQ
                    state.wait_for_interrupt = false;
                    state.is_sync = false;
                }
            }
        }

        return cycles_used;
    }

    // 1 -> LIC -- signals on last cycle of instruction

    uint8_t opcode = get_next_byte();
    uint8_t extended_opcode = 0;
    uint8_t address_mode = MODE_UNKNOWN;

    // Keep reading extended opcodes until we hit a real one
    // See MC6809 data sheet fig.17
    while (opcode == OPCODE_EXTENDED_1 || opcode == OPCODE_EXTENDED_2) {
        extended_opcode = opcode;
        opcode = get_next_byte();
    }

    // Process all ops but NOP
    if (opcode != NOP) {
        uint8_t msn = (opcode & 0xF0) >> 4;
        uint8_t lsn = opcode & 0x0F;

        // NOTE Run this first to favour SWIx, CWAI, RTI over others
        //      See Zaks p.250
        if (msn == 0x03) {
            // These ops have only one, specific address mode each
            if (lsn == 0x0F) swi(extended_opcode);
            if (lsn == 0x0C) cwai();
            if (lsn == 0x0B) {
                // Use this to break to monitor, unless we're
                // actually processing an interrupt
                if (state.interrupts == 0) {
                    printf("Breaking to monitor on RTI\n");
                    return BREAK_TO_MONITOR;
                } else {
                    printf("Returning on 1/%i interrupts\n", state.interrupts);
                }

                rti();
            }
            if (lsn  < 0x04) lea(opcode);
            if (lsn == 0x04) push(true,  get_next_byte());
            if (lsn == 0x06) push(false, get_next_byte());
            if (lsn == 0x05) pull(true,  get_next_byte());
            if (lsn == 0x07) pull(false, get_next_byte());
            if (lsn == 0x09) rts();
            if (lsn == 0x0A) abx();
            if (lsn == 0x0D) mul();
            return cycles_used;
        }

        if (msn == 0x01) {
            // These ops all use inherent addressing
            if (lsn == 0x03) sync();
            if (lsn == 0x06) do_branch(BRA, true);  // Set correct op for LBRA handling
            if (lsn == 0x07) do_branch(BSR, true);  // Set correct op for LBSR handling
            if (lsn == 0x09) daa();
            if (lsn == 0x0A) orcc(get_next_byte());
            if (lsn == 0x0C) andcc(get_next_byte());
            if (lsn == 0x0D) sex();
            if (lsn == 0x0E) transfer_decode2(get_next_byte(), true);
            if (lsn == 0x0F) transfer_decode2(get_next_byte(), false);
            return cycles_used;
        }

        if (msn == 0x02 || opcode == BSR) {
            // All 0x02 ops are branch ops, but BSR is 0x8D
            do_branch(opcode, (extended_opcode != 0));
            return cycles_used;
        }

        // Set the addressing mode as far as we can
        if (msn == 0x08 || msn == 0x0C) address_mode = MODE_IMMEDIATE;
        if (msn == 0x00 || msn == 0x09 || msn == 0x0D) address_mode = MODE_DIRECT;
        if (msn == 0x06 || msn == 0x0A || msn == 0x0E) address_mode = MODE_INDEXED;
        if (msn == 0x07 || msn == 0x0B || msn == 0x0F) address_mode = MODE_EXTENDED;
        if (msn == 0x04 || msn == 0x05) address_mode = MODE_INHERENT;

        // Jump to specific ops or groups of ops
        if (lsn == 0x00) {
            if (msn == 0x07) {
                sub(opcode, address_mode);
            } else {
                neg(opcode, address_mode);
            }

            return cycles_used;
        }

        if (lsn == 0x01) {
            cmp(opcode, address_mode);
            return cycles_used;
        }

        if (lsn == 0x02) {
            sbc(opcode, address_mode);
            return cycles_used;
        }

        if (lsn == 0x03) {
            if (msn > 0x0B) {
                add_16(opcode, address_mode);
            } else if (msn > 0x07) {
                sub_16(opcode, address_mode, extended_opcode);
            } else {
                com(opcode, address_mode);
            }

            return cycles_used;
        }

        if (lsn == 0x04) {
            if (msn < 0x08) {
                lsr(opcode, address_mode);
            } else {
                and(opcode, address_mode);
            }

            return cycles_used;
        }

        if (lsn == 0x05) {
            bit(opcode, address_mode);
            return cycles_used;
        }

        if (lsn == 0x06) {
            if (msn < 0x08) {
                ror(opcode, address_mode);
            } else {
                ld(opcode, address_mode);
            }

            return cycles_used;
        }

        if (lsn == 0x07) {
            if (msn < 0x08) {
                asr(opcode, address_mode);
            } else {
                st(opcode, address_mode);
            }

            return cycles_used;
        }

        if (lsn == 0x08) {
            if (msn < 0x08) {
                asl(opcode, address_mode);
            } else {
                eor(opcode, address_mode);
            }

            return cycles_used;
        }

        if (lsn == 0x09) {
            if (msn < 0x08) {
                rol(opcode, address_mode);
            } else {
                adc(opcode, address_mode);
            }

            return cycles_used;
        }

        if (lsn == 0x0A) {
            if (msn < 0x08) {
                dec(opcode, address_mode);
            } else {
                orr(opcode, address_mode);
            }

            return cycles_used;
        }

        if (lsn == 0x0B) {
            add(opcode, address_mode);
            return cycles_used;
        }

        if (lsn == 0x0C) {
            if (msn < 0x08) {
                inc(opcode, address_mode);
            } else if (msn > 0x0C) {
                // LDD
                ld_16(opcode, address_mode, extended_opcode);
            } else {
                cmp_16(opcode, address_mode, extended_opcode);
            }

            return cycles_used;
        }

        if (lsn == 0x0D) {
            if (msn < 0x08) {
                tst(opcode, address_mode);
            } else if (msn > 0x0C) {
                // STD
                st_16(opcode, address_mode, extended_opcode);
            } else {
                jsr(address_mode);
            }

            return cycles_used;
        }

        if (lsn == 0x0E) {
            if (msn < 0x08) {
                jmp(address_mode);
            } else {
                ld_16(opcode, address_mode, extended_opcode);
            }

            return cycles_used;
        }

        if (lsn == 0x0F) {
            if (msn < 0x08) {
                clr(opcode, address_mode);
            } else {
                st_16(opcode, address_mode, extended_opcode);
            }
        }
    }
}

/**
 * @brief Perfom a branch (long or short) operation.
 *
 * @param bop:     The branch opcode.
 * @param is_long: `true` if its a long branch op, otherwise `false`.
 */
void do_branch(uint8_t bop, bool is_long) {

    int16_t offset = 0;

    if (is_long) {
        offset = address_from_next_two_bytes();
        if (is_bit_set(offset, SIGN_BIT_16)) offset -= 65536;
    } else {
        offset = get_next_byte();
        if (is_bit_set(offset, SIGN_BIT_8)) offset -= 256;
    }

    bool branch = false;

    if (bop == BRA) branch = true;

    if (bop == BEQ &&  is_cc_bit_set(Z_BIT)) branch = true;
    if (bop == BNE && !is_cc_bit_set(Z_BIT)) branch = true;

    if (bop == BMI &&  is_cc_bit_set(N_BIT)) branch = true;
    if (bop == BPL && !is_cc_bit_set(N_BIT)) branch = true;

    if (bop == BVS &&  is_cc_bit_set(V_BIT)) branch = true;
    if (bop == BVC && !is_cc_bit_set(V_BIT)) branch = true;

    if (bop == BLO &&  is_cc_bit_set(C_BIT)) branch = true; // Also BCS
    if (bop == BHS && !is_cc_bit_set(C_BIT)) branch = true; // Also BCC

    if (bop == BLE &&  (is_cc_bit_set(Z_BIT) || (is_cc_bit_set(N_BIT) != is_cc_bit_set(V_BIT)))) branch = true;
    if (bop == BGT && !(is_cc_bit_set(Z_BIT) || (is_cc_bit_set(N_BIT) != is_cc_bit_set(V_BIT)))) branch = true;
    if (bop == BLT &&  (is_cc_bit_set(N_BIT) != is_cc_bit_set(V_BIT))) branch = true;
    if (bop == BGE && !(is_cc_bit_set(N_BIT) != is_cc_bit_set(V_BIT))) branch = true;

    if (bop == BLS &&  (is_cc_bit_set(C_BIT) || is_cc_bit_set(Z_BIT))) branch = true;
    if (bop == BHI && !(is_cc_bit_set(C_BIT) || is_cc_bit_set(Z_BIT))) branch = true;

    if (bop == BSR) {
        // Branch to Subroutine: push PC to hardware stack (S) first
        branch = true;
        reg.s--;
        set_byte(reg.s, (reg.pc & 0xFF));
        reg.s--;
        set_byte(reg.s, ((reg.pc >> 8) & 0xFF));
    }

    if (branch) reg.pc += offset;
}


/*
 * MEMORY ACCESS FUNCTIONS
 */

/**
 * @brief Increment the PC and get the byte at the referenced address.
 *
 * @retval The byte value.
 */
uint8_t get_next_byte() {
    return mem[reg.pc++];
}


/**
 * @brief Get the byte at the specified address.
 *
 * @param address: The 16-bit memory address.
 *
 * @retval The byte value.
 */
uint8_t get_byte(uint16_t address) {
    return mem[address];
}


/**
 * @brief Write the byte at the specified address.
 *
 * @param address: The 16-bit memory address.
 * @param value:   The byte value.
 */
void set_byte(uint16_t address, uint8_t value) {
    mem[address] = value;
}


/**
 * @brief Increment the PC.
 *
 * @param amount: The increment or decrement.
 */
void move_pc(int16_t amount) {
    reg.pc += amount;
}


/**
 * @brief Check if a bit is set.
 *
 * @param value: The byte value to use.
 * @param bit:   The bit to check (0-7).
 *
 * @retval `true` if the bit is 1, otherwise `false`.
 */
bool is_bit_set(uint16_t value, uint8_t bit) {
    return ((value & (1 << bit)) != 0);
}


/*
 * CONDITION CODE REGISTER BIT-LEVEL GETTERS AND SETTERS
 */

bool is_cc_bit_set(uint8_t bit) {
    return (((reg.cc >> bit) & 1) == 1);
}

void set_cc_bit(uint8_t bit) {
    reg.cc |= (1 << bit);
}

void clr_cc_bit(uint8_t bit) {
    reg.cc &= ~(1 << bit);
}

void flp_cc_bit(uint8_t bit) {
    reg.cc ^= (1 << bit);
}

/**
 * @brief The N, V and Z bits are frequently cleared, so clear bits 1-3.
 */
void clr_cc_nzv() {
    reg.cc &= MASK_NZV;
}


/**
 * @brief Set Z and N based on a single 8- or 16-bit input value.
 *
 * @param value: The value to test.
 * @param is_16_bit: `true` if the value is 16 bits long.
 */
void set_cc_nz(uint16_t value, bool is_16_bit) {
    reg.cc &= MASK_NZ;
    if (value == 0) set_cc_bit(Z_BIT);
    if (is_bit_set(value, (is_16_bit ? SIGN_BIT_16 : SIGN_BIT_8))) set_cc_bit(N_BIT);
}


/**
 * @brief Clear N, V, C. Set Z.
 *        Affects N, Z, V, C
 */
void set_cc_after_clr() {
    reg.cc &= MASK_NZVC;
    set_cc_bit(Z_BIT);
}


/**
 * @brief Set the CC after an 8- or 16-bit load.
 *        Affects N, Z, V -- V is always cleared.
 *
 * @param value: The value to test.
 * @param is_16_bit: `true` if the value is 16 bits long.
 */
void set_cc_after_load(uint16_t value, bool is_16_bit) {
    clr_cc_nzv();
    set_cc_nz(value, is_16_bit);
}


/**
 * @brief Set the CC after an 8- or 16-bit store.
 *        Affects N, Z, V -- V is always cleared.
 *
 * @param value: The value to test.
 * @param is_16_bit: `true` if the value is 16 bits long.
 */
void set_cc_after_store(uint16_t value, bool is_16_bit) {
    //
    set_cc_after_load(value, is_16_bit);
}


/*
 * OP PRIMARY FUNCTIONS
 */

/**
 * @brief ABX: B + X -> X (unsigned)
 *        Affects NONE
 */
void abx() {
    reg.x += (uint16_t)reg.b;
}


/**
 * @brief ADC: A + M + C -> A,
 *        B + M + C -> A.
 *
 * @param op:   The opcode.
 * @param mode: The addressing mode.
 */
void adc(uint8_t op, uint8_t mode) {
    uint16_t address = address_from_mode(mode);
    if (op < ADCB_immed) {
        reg.a = add_with_carry(reg.a, get_byte(address));
    } else {
        reg.b = add_with_carry(reg.b, get_byte(address));
    }
}


/**
 * @brief ADD: A + M -> A,
 *        B + M -> N.
 *
 * @param op:   The opcode.
 * @param mode: The addressing mode.
 */
void add(uint8_t op, uint8_t mode) {
    uint16_t address = address_from_mode(mode);
    if (op < ADDB_immed) {
        reg.a = add_no_carry(reg.a, get_byte(address));
    } else {
        reg.b = add_no_carry(reg.b, get_byte(address));
    }
}


/**
 * @brief ADDD: D + M:M+1 -> D.
 *        Affects N, Z, V, C
 *        `alu()` affects H, V, C
 *
 * @param op:   The opcode.
 * @param mode: The addressing mode.
 */
void add_16(uint8_t op, uint8_t mode) {
    // Should not affect H, so preserve it
    bool h_is_set = is_bit_set(reg.cc, H_BIT);

    // Clear N and Z
    clr_cc_bit(N_BIT);
    clr_cc_bit(Z_BIT);

    // Get bytes
    uint16_t address = address_from_mode(mode);
    uint8_t msb = get_byte(address);
    uint8_t lsb = get_byte(address + 1);

    // Add the two LSBs (M+1, B) to set the carry,
    // then add the two MSBs (M, A) with the carry
    lsb = alu(reg.b, lsb, false);
    msb = alu(reg.a, msb, true);

    // Convert the bytes back to a 16-bit value and set N, Z
    uint16_t answer = (msb << 8) | lsb;
    set_cc_nz(answer, true);

    // Set D's component registers
    reg.a = (answer >> 8) & 0xFF;
    reg.b = answer & 0xFF;

    // Restore H -- may have been changed by 'alu()'
    if (h_is_set) {
        set_cc_bit(H_BIT);
    } else {
        clr_cc_bit(H_BIT);
    }
}


/**
 * @brief AND: A & M -> A,
 *        B & M -> N.
 *
 * @param op:   The opcode.
 * @param mode: The addressing mode.
 */
void and(uint8_t op, uint8_t mode) {
    uint16_t address = address_from_mode(mode);
    if (op < ANDB_immed) {
        reg.a = do_and(reg.a, get_byte(address));
    } else {
        reg.b = do_and(reg.b, get_byte(address));
    }
}


/**
 * @brief AND CC: CC & M -> CC.
 *
 * @param value: The value to AND CC with.
 */
void andcc(uint8_t value) {
    reg.cc &= value;
}


/**
 * @brief ASL: arithmetic shift left A -> A,
 *             arithmetic shift left B -> B,
 *             arithmetic shift left M -> M.
 *
 * This is is the same as LSL.
 *
 * @param op:   The opcode.
 * @param mode: The addressing mode.
 */
void asl(uint8_t op, uint8_t mode) {
    if (mode == MODE_INHERENT) {
        if (op == ASLA) {
            reg.a = logic_shift_left(reg.a);
        } else {
            reg.b = logic_shift_left(reg.b);
        }
    } else {
        uint16_t address = address_from_mode(mode);
        set_byte(address, logic_shift_left(get_byte(address)));
    }
}


/**
 * @brief ASR: arithmetic shift right A -> A,
 *             arithmetic shift right B -> B,
 *             arithmetic shift right M -> M.
 *
 * @param op:   The opcode.
 * @param mode: The addressing mode.
 */
void asr(uint8_t op, uint8_t mode) {
    if (mode == MODE_INHERENT) {
        if (op == ASRA) {
            reg.a = arith_shift_right(reg.a);
        } else {
            reg.b = arith_shift_right(reg.b);
        }
    } else {
        uint16_t address = address_from_mode(mode);
        set_byte(address, arith_shift_right(get_byte(address)));
    }
}


/**
 * @brief BIT: Bit test A (A & M),
 *             Bit test B (B & M).
 *
 * Does not affect the operands, only the CC register.
 *
 * @param op:   The opcode.
 * @param mode: The addressing mode.
 */
void bit(uint8_t op, uint8_t mode) {
    uint16_t address = address_from_mode(mode);
    uint8_t ignored = do_and((op < BITB_immed ? reg.a : reg.b), get_byte(address));
}


/**
 * @brief CLR: 0 -> A,
 *             0 -> B,
 *             0 -> M.
 *
 * @param op:   The opcode.
 * @param mode: The addressing mode.
 */
void clr(uint8_t op, uint8_t mode) {
    if (mode == MODE_INHERENT) {
        if (op == CLRA) {
            reg.a = 0;
        } else {
            reg.b = 0;
        }
    } else {
        uint16_t address = address_from_mode(mode);
        set_byte(address, 0);
    }

    set_cc_after_clr();
}


/**
 * @brief CMP: Compare M to A,
 *                     M to B.
 *
 * @param op:   The opcode.
 * @param mode: The addressing mode.
 */
void cmp(uint8_t op, uint8_t mode) {
    uint16_t address = address_from_mode(mode);
    compare((op < CMPB_immed ? reg.a : reg.b), get_byte(address));
}


/**
 * @brief CMP: Compare M:M + 1 to D,
 *                     M:M + 1 to X,
 *                     M:M + 1 to Y,
 *                     M:M + 1 to S,
 *                     M:M + 1 to U.
 *
 * @param op:   The opcode.
 * @param mode: The addressing mode.
 */
void cmp_16(uint8_t op, uint8_t mode, uint8_t ex_op) {
    // Get the effective address of the data
    uint16_t address = (uint16_t)address_from_mode(mode);

    // 'addressFromMode:' assumes an 8-bit read, so we need to increase PC by 1
    reg.pc++;

    // Set a pointer to the target register, D by default
    uint16_t d = (reg.a << 8) + reg.b;
    uint16_t *reg_ptr = &d;
    if (op == 0x83 && ex_op == OPCODE_EXTENDED_2) reg_ptr = &reg.u;
    if (op == CMPX_immed) reg_ptr = ex_op == 0 ? &reg.x : (ex_op == OPCODE_EXTENDED_1 ? &reg.y : &reg.s);

    // Get the data and subtract from the target register
    uint16_t comp_value = (get_byte(address) << 8) | (get_byte(address + 1));

    // Subtract 'compValue' from the target register - this will set the CC register
    // NOTE ignore the return value - we're just setting the CC bits
    subtract_16(*reg_ptr, comp_value);
}


/**
 * @brief COM: !A -> A,
 *             !B -> B,
 *             !M -> A.
 *
 * @param op:   The opcode.
 * @param mode: The addressing mode.
 */
void com(uint8_t op, uint8_t mode) {
    if (mode == MODE_INHERENT) {
        if (op == 0x43) {
            reg.a = complement(reg.a);
        } else {
            reg.b = complement(reg.b);
        }
    } else {
        uint16_t address = address_from_mode(mode);
        set_byte(address, complement(get_byte(address)));
    }
}


/**
 * @brief CWAI: Clear CC bits and Wait for Interrupt.
 *        AND CC with the operand, set e, then push every register,
 *        including CC to the hardware stack.
 */
void cwai() {
    reg.cc &= get_next_byte();
    set_cc_bit(E_BIT);
    push(PUSH_TO_HARD_STACK, PUSH_PULL_EVERY_REG);
    state.wait_for_interrupt = true;
}


/**
 * @brief DAA: Decimal Adjust after Addition.
 */
void daa() {
    bool carry = is_bit_set(reg.cc, C_BIT);
    clr_cc_bit(C_BIT);

    // Set Least Significant Conversion Factor, which will be either 0 or 6
    uint8_t correction = 0;
    uint8_t lsn = reg.a & 0x0F;
    if (is_bit_set(reg.cc, H_BIT) || lsn > 9) correction = 6;
    reg.a += correction;

    // Set Most Significant Conversion Factor, which will be either 0 or 6
    uint8_t msn = (reg.a & 0xF0) >> 4;
    correction = 0;
    if (carry || msn > 8 || lsn > 9) correction = 6;
    msn += correction;
    if (msn > 0x0F) set_cc_bit(C_BIT);

    reg.a = (msn << 4) | (reg.a & 0x0F);
    set_cc_nz(reg.a, false);
}


/**
 * @brief DEC: A - 1 -> A,
 *             B - 1 -> B,
 *             M - 1 -> M.
 *
 * @param op:   The opcode.
 * @param mode: The addressing mode.
 */
void dec(uint8_t op, uint8_t mode) {
    if (mode == MODE_INHERENT) {
        if (op == DECA) {
            reg.a = decrement(reg.a);
        } else {
            reg.b = decrement(reg.b);
        }
    } else {
        uint16_t address = address_from_mode(mode);
        set_byte(address, decrement(get_byte(address)));
    }
}


/**
 * @brief EOR: A ^ M -> A,
 *             B ^ M -> B.
 *
 * @param op:   The opcode.
 * @param mode: The addressing mode.
 */
void eor(uint8_t op, uint8_t mode) {
    uint16_t address = address_from_mode(mode);
    if (op < EORB_immed) {
        reg.a = do_xor(reg.a, get_byte(address));
    } else {
        reg.b = do_xor(reg.b, get_byte(address));
    }
}


/**
 * @brief INC: A + 1 -> A,
 *             B + 1 -> B,
 *             M + 1 -> M.
 *
 * @param op:   The opcode.
 * @param mode: The addressing mode.
 */
void inc(uint8_t op, uint8_t mode) {
    if (mode == MODE_INHERENT) {
        if (op == INCA) {
            reg.a = increment(reg.a);
        } else {
            reg.b = increment(reg.b);
        }
    } else {
        uint16_t address = address_from_mode(mode);
        set_byte(address, increment(get_byte(address)));
    }
}


/**
 * @brief JMP: M -> PC.
 *
 * @param mode: The addressing mode.
 */
void jmp(uint8_t mode) {
    uint16_t address = address_from_mode(mode);
    reg.pc = mode == MODE_INDEXED ? address : ((get_byte(address) << 8) | get_byte(address + 1));
}


/**
 * @brief JSR: S = S - 1;
 *             PC LSB to stack;
 *             S = S- 1;
 *             PC MSB to stack;
 *             M -> PC.
 *
 * @param mode: The addressing mode.
 */
void jsr(uint8_t mode) {
    uint16_t address = address_from_mode(mode);
    reg.s--;
    set_byte(reg.s, (reg.pc & 0xFF));
    reg.s--;
    set_byte(reg.s, ((reg.pc >> 8) & 0xFF));
    reg.pc = address;
}


/**
 * @brief LD: M -> A,
 *            M -> B.
 *
 * @param op:   The opcode.
 * @param mode: The addressing mode.
 */
void ld(uint8_t op, uint8_t mode) {
    uint16_t address = address_from_mode(mode);
    if (op < LDB_immed) {
        reg.a = get_byte(address);
        set_cc_after_load(reg.a, false);
    } else {
        reg.b = get_byte(address);
        set_cc_after_load(reg.b, false);
    }
}


/**
 * @brief LD: M:M + 1 -> D,
 *            M:M + 1 -> X,
 *            M:M + 1 -> Y,
 *            M:M + 1 -> S,
 *            M:M + 1 -> U.
 *
 * @param op:    The opcode.
 * @param mode:  The addressing mode code.
 * @param ex_op: `true` if the opcode is two bytes long.
 */
void ld_16(uint8_t op, uint8_t mode, uint8_t ex_op) {
    uint16_t address = address_from_mode(mode);

    // 'address_from_mode()' assumes an 8-bit read, so we need to increase PC by 1
    if (mode == MODE_IMMEDIATE) reg.pc++;

    bool touched_d = false;
    uint16_t *reg_ptr;
    if (op < 0xCC) {
        reg_ptr = ex_op == OPCODE_EXTENDED_1 ? &reg.y : &reg.x;
    } else if ((op & 0x0F) == 0x0E) {
        reg_ptr = ex_op == OPCODE_EXTENDED_1 ? &reg.s : &reg.u;
    } else {
        reg_ptr = &reg.d;
        touched_d = true;
    }

    // Write the data into the target register
    *reg_ptr = (get_byte(address) << 8) | get_byte(address + 1);

    // Set the A and B registers if we're targeting D
    if (touched_d) {
        reg.a = (reg.d >> 8) & 0xFF;
        reg.b = reg.d & 0xFF;
    }

    // Update the CC register
    set_cc_after_load(*reg_ptr, true);
}


/**
 * @brief LEA: EA -> S,
 *             EA -> U,
 *             EA -> X,
 *             EA -> Y.
 *
 * @param op:    The opcode.
 */
void lea(uint8_t op) {
    load_effective(indexed_address(get_next_byte()), (op & 0x03));
}


/**
 * @brief LSR: logic shift right A -> A,
 *             logic shift right B -> B,
 *             logic shift right M -> M.
 *
 * @param op:   The opcode.
 * @param mode: The addressing mode.
 */
void lsr(uint8_t op, uint8_t mode) {
    if (mode == MODE_INHERENT) {
        if (op == LSRA) {
            reg.a = logic_shift_right(reg.a);
        } else {
            reg.b = logic_shift_right(reg.b);
        }
    } else {
        uint16_t address = address_from_mode(mode);
        set_byte(address, logic_shift_right(get_byte(address)));
    }
}


/**
 * @brief MUL: A x B -> D (unsigned)
 *        Affects Z, C.
 */
void mul() {
    reg.cc &= MASK_ZC;
    uint16_t d = reg.a * reg.b;
    if (is_bit_set(d, 7)) set_cc_bit(C_BIT);
    if (d == 0) set_cc_bit(Z_BIT);

    // Decompose D
    reg.a = (d >> 8) & 0xFF;
    reg.b = d & 0xFF;
}


/**
 * @brief NEG: !R + 1 -> R,
 *             !M + 1 -> M.
 *
 * @param op:   The opcode.
 * @param mode: The addressing mode.
 */
void neg(uint8_t op, uint8_t mode) {
    if (mode == MODE_INHERENT) {
        if (op == NEGA) {
            reg.a = negate(reg.a, false);
        } else {
            reg.b = negate(reg.b, false);
        }
    } else {
        uint16_t address = address_from_mode(mode);
        set_byte(address, negate(get_byte(address), false));
    }
}


/**
 * @brief OR: A | M -> A,
 *            B | M -> B.
 *
 * @param op:   The opcode.
 * @param mode: The addressing mode.
 */
void orr(uint8_t op, uint8_t mode) {
    uint16_t address = address_from_mode(mode);
    if (op < ORB_immed) {
        reg.a = do_or(reg.a, get_byte(address));
    } else {
        reg.b = do_or(reg.b, get_byte(address));
    }
}


/**
 * @brief OR CC: CC | M -> CC.
 *
 * @param value: The value to OR CC with.
 */
void orcc(uint8_t value) {
    reg.cc |= value;
}


/**
 * @brief ROL: rotate left A -> A,
 *             rotate left B -> B,
 *             rotate left M -> M.
 *
 * @param op:   The opcode.
 * @param mode: The addressing mode.
 */
void rol(uint8_t op, uint8_t mode) {
    if (mode == MODE_INHERENT) {
        if (op == ROLA) {
            reg.a = rotate_left(reg.a);
        } else {
            reg.b = rotate_left(reg.b);
        }
    } else {
        uint16_t address = address_from_mode(mode);
        set_byte(address, rotate_left(get_byte(address)));
    }
}


/**
 * @brief ROR: rotate right A -> A,
 *             rotate right B -> B,
 *             rotate right M -> M.
 *
 * @param op:   The opcode.
 * @param mode: The addressing mode.
 */
void ror(uint8_t op, uint8_t mode) {
    if (mode == MODE_INHERENT) {
        if (op == RORA) {
            reg.a = rotate_right(reg.a);
        } else {
            reg.b = rotate_right(reg.b);
        }
    } else {
        uint16_t address = address_from_mode(mode);
        set_byte(address, rotate_right(get_byte(address)));
    }
}


/**
 * @brief RTI: Pull CC from the hardware stack; if E is set, pull all the registers
 *             from the hardware stack, otherwise pull the PC register only.
 */
void rti() {
    pull(true, PUSH_PULL_CC_REG);
    if (is_cc_bit_set(E_BIT)) {
        pull(true, PUSH_PULL_ALL_REGS);
    } else {
        pull(true, PUSH_PULL_PC_REG);
    }
}


/**
 * @brief RTS: Pull the PC from the hardware stack.
 */
void rts() {
    pull(true, PUSH_PULL_PC_REG);
}


/**
 * @brief SBC: A - M - C -> A,
 *             B - M - C -> A.
 *
 * @param op:   The opcode.
 * @param mode: The addressing mode.
 */
void sbc(uint8_t op, uint8_t mode) {
    uint16_t address = address_from_mode(mode);
    if (op < SBCB_immed) {
        reg.a = sub_with_carry(reg.a, get_byte(address));
    } else {
        reg.b = sub_with_carry(reg.b, get_byte(address));
    }
}


/**
 * @brief SEX: sign-extend B into A,
 *             Affects N, Z.
 */
void sex() {
    reg.cc &= MASK_NZ;
    reg.a = 0;
    if (is_bit_set(reg.b, SIGN_BIT_8)) {
        reg.a = 0xFF;
        set_cc_bit(N_BIT);
    }

    if (reg.b == 0) set_cc_bit(Z_BIT);
}


/**
 * @brief ST: A -> M,
 *            B -> M.
 *            Affects N, Z, V -- V is always cleared.
 *
 * @param op:   The opcode.
 * @param mode: The addressing mode.
 */
void st(uint8_t op, uint8_t mode) {
    uint16_t address = address_from_mode(mode);
    if (op < STB_direct) {
        set_byte(address, reg.a);
        set_cc_after_store(reg.a, false);
    } else {
        set_byte(address, reg.b);
        set_cc_after_store(reg.b, false);
    }
}


/**
 * @brief ST: D -> M:M + 1,
 *            X -> M:M + 1,
 *            Y -> M:M + 1,
 *            S -> M:M + 1,
 *            U -> M:M + 1.
 *
 * @param op:   The opcode.
 * @param mode: The addressing mode.
 * @param ex_op: `true` if the opcode is two bytes long.
 */
void st_16(uint8_t op, uint8_t mode, uint8_t ex_op) {
    // Get the effective address of the data
    uint16_t address = address_from_mode(mode);

    // Set a pointer to the target register
    bool touched_d = false;
    uint16_t *reg_ptr;
    if (op < STD_direct) {
        reg_ptr = ex_op == OPCODE_EXTENDED_1 ? &reg.y : &reg.x;
    } else if ((op & 0x0F) == 0x0F) {
        reg_ptr = ex_op == OPCODE_EXTENDED_1 ? &reg.s : &reg.u;
    } else {
        reg.d = (reg.a << 8) | reg.d;
        reg_ptr = &reg.d;
        touched_d = true;
    }

    // Write the target register out
    set_byte(address, ((*reg_ptr >> 8) & 0xFF));
    set_byte(address + 1, (*reg_ptr & 0xFF));

    // Update the CC register
    set_cc_after_store(*reg_ptr, true);

    // If the op touched D re-set the components
    if (touched_d) {
        reg.b = reg.d & 0xFF;
        reg.a = (reg.d >> 8) & 0xFF;
    }
}


/**
 * @brief SUB: A - M -> A,
 *             B - M -> B.
 *
 * @param op:   The opcode.
 * @param mode: The addressing mode.
 */
void sub(uint8_t op, uint8_t mode) {
    uint16_t address = address_from_mode(mode);
    if (op < SUBB_immed) {
        reg.a = subtract(reg.a, get_byte(address));
    } else {
        reg.b = subtract(reg.b, get_byte(address));
    }
}


/**
 * @brief SUBD: D - M:M + 1 -> D.
 *              Affects N, Z, V, C -- V, C (H) set by `alu()`.
 *
 * @param op:   The opcode.
 * @param mode: The addressing mode.
 * @param ex_op: `true` if the opcode is two bytes long.
 */
void sub_16(uint8_t op, uint8_t mode, uint8_t ex_op) {
    reg.cc &= MASK_NZV;
    uint8_t cc = reg.cc;

    // Complement the value at M:M + 1
    // NOTE Don't use 'negate()' because we need to
    //      include the carry into the MSB
    uint16_t address = address_from_mode(mode);
    uint8_t msb = complement(get_byte(address));
    uint8_t lsb = complement(get_byte(address + 1));

    // Add 1 to form the 2's complement
    lsb = alu(lsb, 1, false);
    msb = alu(msb, 0, true);
    reg.cc = cc;

    // Add D - A and B
    lsb = alu(reg.b, lsb, false);
    msb = alu(reg.a, msb, true);

    // Convert the bytes back to a 16-bit value and set the CC
    uint16_t answer = (msb << 8) | lsb;
    set_cc_nz(answer, true);

    // Set D's component registers
    reg.a = (answer >> 8) & 0x0FF;
    reg.b = answer & 0xFF;
}


/**
 * @brief SWI: SoftWare Interrupt.
 *
 * Covers 1, 2 and 3.
 *
 * @param number: The SWI number (1-3).
 */
void swi(uint8_t number) {
    // Set e to 1 then push every register to the hardware stac
    set_cc_bit(E_BIT);
    push(true, PUSH_PULL_EVERY_REG);

    if (number == 1) {
        // Set I and F
        set_cc_bit(I_BIT);
        set_cc_bit(F_BIT);

        // Set the PC to the interrupt vector
        reg.pc = (mem[SWI1_VECTOR] << 8) | mem[SWI1_VECTOR + 1];
    }

    if (number == 2) {
        reg.pc = (mem[SWI2_VECTOR] << 8) | mem[SWI2_VECTOR + 1];
    }

    if (number == 3) {
        reg.pc = (mem[SWI3_VECTOR] << 8) | mem[SWI3_VECTOR + 1];
    }
}


/**
 * @brief SYNC.
 *
 * If interrupt is masked, or is < 3 cycles - continue.
 * If interrupt > 2 cycle, wait.
 */
void sync() {
    state.wait_for_interrupt = true;
    state.is_sync = true;
}


/**
 * @brief TST: test A,
 *             test B,
 *             test M.
 *
 * @param op:   The opcode.
 * @param mode: The addressing mode.
 */
void tst(uint8_t op, uint8_t mode) {
    if (mode == MODE_INHERENT) {
        test(op == TSTA ? reg.a : reg.b);
    } else {
        uint16_t address = address_from_mode(mode);
        test(get_byte(address));
    }
}


/*
 * OP HELPER FUNCTIONS
 */


/**
 * @brief Simulates addition of two unsigned 8-bit values in a binary ALU.
 *        Checks for half-carry, carry and overflow.
 *        Affects H, C, V.
 *
 * @param value_1:   The addee.
 * @param value_2:   The adder.
 * @param use_carry: Include any previous carry.
 *
 * @retval The addition.
 */
uint8_t alu(uint8_t value_1, uint8_t value_2, bool use_carry) {
    uint8_t binary_1[8], binary_2[8], answer[8];
    bool bit_carry = false;
    bool bit_6_carry = false;

    for (uint32_t i = 0 ; i < 8 ; i++) {
        binary_1[i] = ((value_1 >> i) & 0x01);
        binary_2[i] = ((value_2 >> i) & 0x01);
    }

    if (use_carry) bit_carry = is_cc_bit_set(C_BIT);

    clr_cc_bit(C_BIT);
    clr_cc_bit(V_BIT);

    for (uint32_t i = 0 ; i < 8 ; ++i) {
        if (binary_1[i] == binary_2[i]) {
            // Both digits are the same, ie. 1 and 1, or 0 and 0,
            // so added value is 0 + a carry to the next digit
            answer[i] = bit_carry ? 1 : 0;
            bit_carry = binary_1[i] == 1;
        } else {
            // Both digits are different, ie. 1 and 0, or 0 and 1, so the
            // added value is 1 plus any carry from the previous addition
            // 1 + 1 -> 0 + carry 1, or 0 + 1 -> 1
            answer[i] = bit_carry ? 0 : 1;
        }

        // Check for half carry, ie. result of bit 3 add is a carry into bit 4
        if (i == 3 && bit_carry) set_cc_bit(H_BIT);

        // Record bit 6 carry for overflow check
        if (i == 6) bit_6_carry = bit_carry;
    }

    // Preserve the final carry value in the CC register's C bit
    if (bit_carry) set_cc_bit(C_BIT);

    /* Check for an overflow:
       V = 1 if C XOR c6 == 1
       "A processor [sets] the overflow flag when the carry out of the
        most significant bit is different from the carry out of the next most
        significant bit; that is, an overflow is the exclusive-OR of the carries
        into and out of the sign bit." */
    if (bit_carry != bit_6_carry) set_cc_bit(V_BIT);

    // Copy answer into bits[] array for conversion to decimal
    uint8_t final = 0;
    for (uint32_t i = 0 ; i < 8 ; ++i) {
        if (answer[i] != 0) final = (final | (1 << i));
    }

    // Return the answer (condition codes already set elsewhere)
    return final;
}


/**
 * @brief Add two unsigned 16-bit values.
 *        `alu()` affects H, C, V.
 *
 * @param value_1:   The addee.
 * @param value_2:   The adder.
 * @param use_carry: Include any previous carry.
 *
 * @retval The addition.
 */
uint16_t alu_16(uint16_t value_1, uint16_t value_2, bool use_carry) {
    // Add the LSBs
    uint8_t v_1 = value_1 & 0xFF;
    uint8_t v_2 = value_2 & 0xFF;
    uint8_t total = alu(v_1, v_2, use_carry);

    // Now add the MSBs, using the carry (if any) from the LSB calculation
    v_1 = (value_1 >> 8) & 0xFF;
    v_2 = (value_2 >> 8) & 0xFF;
    uint8_t subtotal = alu(v_1, v_2, true);
    return ((subtotal << 8) + total);
}


/**
 * @brief Add two unsigned 8-bit values with no carry.
 *        Affects H, N, Z, V, C -- `alu()` sets H, V, C,.
 *
 * @param value:  The addee.
 * @param amount: The adder.
 *
 * @retval The addition.
 */
uint8_t add_no_carry(uint8_t value, uint8_t amount) {
    uint8_t answer = alu(value, amount, false);
    set_cc_nz(answer, false);
    return answer;
}


/**
 * @brief Add two unsigned 8-bit values plus carry.
 *        Affects H, N, Z, V, C -- `alu()` sets H, V, C,.
 *
 * @param value:  The addee.
 * @param amount: The adder.
 *
 * @retval The addition.
 */
uint8_t add_with_carry(uint8_t value, uint8_t amount) {
    uint8_t answer = alu(value, amount, true);
    set_cc_nz(answer, false);
    return answer;
}


/**
 * @brief Subtract two 8-bit values without carry: REG = REG - M.
 *
 * @param value:  The addee.
 * @param amount: The adder.
 *
 * @retval The subtraction.
 */
uint8_t subtract(uint8_t value, uint8_t amount) {
    return base_sub(value, amount, false);
}


/**
 * @brief Subtract two 8-bit values with carry (borrow): REG = REG - M - C.
 *
 * @param value:  The addee.
 * @param amount: The adder.
 *
 * @retval The subtraction.
 */
uint8_t sub_with_carry(uint8_t value, uint8_t amount) {
    return base_sub(value, amount, true);
}


/**
 * @brief Generic 8-bit subtraction function.
 *        Affects N, Z, V, C -- V, C set by 'negate()'
 *                           -- V, C (H) set by 'alu()'
 *
 * @param value:     The addee.
 * @param amount:    The adder.
 * @param use_carry: `true` to include the carry in the sum.
 *
 * @retval The subtraction.
 */
uint8_t base_sub(uint8_t value, uint8_t amount, bool use_carry) {
    bool carry_set = is_bit_set(reg.cc, C_BIT);
    uint8_t comp = twos_complement(amount);
    uint8_t answer = alu(value, comp, false);

    // Don't run 'use_carry' through alu() as will may ADD 1,
    // so peform the borrow here, if C is set. 0xFF = 2C of 1.
    if (use_carry && carry_set) {
        answer = alu(answer, 0xFF, false);
    }

    // C represents a borrow and is set to the complement of the carry
    // of the internal binary addition
    flp_cc_bit(C_BIT);

    set_cc_nz(answer, false);
    return answer;
}


/**
 * @brief Generic 16-bit subtraction function.
 *        Affects N, Z, V, C -- N, Z, V, C set by 'complement()'
 *                           -- V, C (H) set by 'alu()'
 *
 * @param value:     The addee.
 * @param amount:    The adder.
 *
 * @retval The subtraction.
 */
uint16_t subtract_16(uint16_t value, uint16_t amount) {
    // Complement the value at M:M + 1
    uint8_t msb = ones_complement((amount >> 8) & 0xFF);
    uint8_t lsb = ones_complement(amount & 0xFF);
    uint16_t am2 = (msb << 8) + lsb + 1;
    lsb = (am2 & 0xFF);
    msb = (am2 >> 8) & 0xFF;

    reg.cc &= MASK_NZ;

    // Add 1 to form the 2's complement
    //lsb = alu(lsb, 1, false);
    //msb = alu(msb, 0, true);

    // Add the register value
    lsb = alu(value & 0xFF, lsb, false);
    msb = alu((value >> 8) & 0xFF, msb, true);

    // Convert the bytes back to a 16-bit value and set the CC
    uint16_t answer = (msb << 8) | lsb;
    set_cc_nz(answer, true);

    // c represents a borrow and is set to the complement of the carry
    // of the internal binary addition
    flp_cc_bit(C_BIT);

    return answer;
}


/**
 * @brief Returns 2's complement of 8-bit value.
 *        Affects N, Z, V, C -- C, V (H) set by `alu()`
 *                           -- V set only if `value` is 0x80 (see Zaks p 167)
 *
 * @param value: The value to complement.
 * @param ignore: ????
 *
 * @retval The 2's complement.
 */
uint8_t negate(uint8_t value, bool ignore) {
    // Preserve `value` to set V bit
    uint8_t cc = reg.cc;

    // Flip value's bits to make the 1s complenent
    uint8_t answer = ones_complement(value);

    // Add 1 to the bits to get the 2s complement
    answer = alu(answer, 1, false);

    // C represents a borrow and is set to the complement of
    // the carry of the internal binary addition
    flp_cc_bit(C_BIT);

    if (value == 0x80) set_cc_bit(V_BIT);
    set_cc_nz(answer, false);
    return answer;
}


/**
 * @brief Generic calculation of the 1's complement of an 8-bit value.
 *        Does not affect CC.
 *
 * @param value: The value to complement.
 *
 * @retval The 1's complement.
 */
uint8_t ones_complement(uint8_t value) {
    // Flip value's bits to make the 1s complenent
    for (uint32_t i = 0 ; i < 8 ; i++) {
        value ^= (1 << i);
    }
    return value;
}


/**
 * @brief Is this necessary?
 */
uint8_t twos_complement(uint8_t value) {
    // Flip value's bits to make the 1s complenent
    // NOTE This call does not affect CC
    for (uint32_t i = 0 ; i < 8 ; i++) {
        value ^= (1 << i);
    }

    // Add 1 for the two's complement
    return value + 1;
}


/**
 * @brief Returns 1's complement of 8-bit value.
 *        Affects N, Z, V, C -- V, C take fixed values: 0, 1.
 *
 * @param value: The value to complement.
 *
 * @retval The 1's complement.
 */
uint8_t complement(uint8_t value) {
    value = ones_complement(value);
    set_cc_nz(value, false);
    clr_cc_bit(V_BIT);
    set_cc_bit(C_BIT);
    return value;
}


/**
 * @brief Compare two values by subtracting the second from the first.
 *        Result is discarded: we only care about the effect on CC.
 *        Affects N, Z, V, C -- C, V (H) set by `alu()` via `base_sub()`,
 *                           -- `base_sub()` sets N, Z.
 *
 * @param value:  The first value.
 * @param amount: The second value.
 */
void compare(uint8_t value, uint8_t amount) {
    uint8_t answer = base_sub(value, amount, false);
}


/**
 * @brief ANDs the two supplied values.
 *        Affects N, Z, V -- V is always cleared.
 *
 * @param value:  The first value.
 * @param amount: The second value.
 *
 * @retval The result.
 */
uint8_t do_and(uint8_t value, uint8_t amount) {
    clr_cc_bit(V_BIT);
    uint8_t answer = value & amount;
    set_cc_nz(answer, false);
    return answer;
}


/**
 * @brief ORs the two supplied values.
 *        Affects N, Z, V -- V is always cleared.
 *
 * @param value:  The first value.
 * @param amount: The second value.
 *
 * @retval The result.
 */
uint8_t do_or(uint8_t value, uint8_t amount) {
    clr_cc_bit(V_BIT);
    uint8_t answer = value | amount;
    set_cc_nz(answer, false);
    return answer;
}


/**
 * @brief XORs the two supplied values.
 *        Affects N, Z, V -- V is always cleared.
 *
 * @param value:  The first value.
 * @param amount: The second value.
 *
 * @retval The result.
 */
uint8_t do_xor(uint8_t value, uint8_t amount) {
    clr_cc_bit(V_BIT);
    uint8_t answer = value ^ amount;
    set_cc_nz(answer, false);
    return answer;
}


/**
 * @brief Generic arithmetic shift right.
 *        Affects N, Z, C.
 *
 * @param value: The value.
 *
 * @retval The result.
 */
uint8_t arith_shift_right(uint8_t value) {
    uint8_t answer = partial_shift_right(value);
    set_cc_nz(answer, false);
    return answer;
}


/**
 * @brief Generic logical shift left.
 *        Affects N, Z, V, C.
 *
 * @param value: The value.
 *
 * @retval The result.
 */
uint8_t logic_shift_left(uint8_t value) {
    reg.cc &= MASK_NZVC;
    if (is_bit_set(value, 7)) set_cc_bit(C_BIT);
    if (is_bit_set(value, 7) != is_bit_set(value, 6)) set_cc_bit(V_BIT);

    for (uint32_t i = 7 ; i > 0  ; i--) {
        if (is_bit_set(value, i - 1)) {
            value |= (1 << i);
        } else {
            value &= ~(1 << i);
        }
    }

    // Clear bit 0
    value = value & 0xFE;
    set_cc_nz(value, false);
    return value;
}


/**
 * @brief Generic logical shift right.
 *        Affects N, Z, C -- N is always cleared.
 *
 * @param value: The value.
 *
 * @retval The result.
 */
uint8_t logic_shift_right(uint8_t value) {
    uint8_t answer = partial_shift_right(value);

    // Clear bit 7
    answer &= 0x7F;
    if (answer == 0) set_cc_bit(Z_BIT);
    return answer;
}


/**
 * @brief Code common to LSR and ASR.
 *
 * @param value: The value.
 *
 * @retval The result.
 */
uint8_t partial_shift_right(uint8_t value) {
    reg.cc &= MASK_NZC;
    if (is_bit_set(value, 0)) set_cc_bit(C_BIT);

    for (uint32_t i = 0 ; i < 7 ; i++) {
        if (is_bit_set(value, i + 1)) {
            value |= (1 << i);
        } else {
            value &= ~(1 << i);
        }
    }

    return value;
}


/**
 * @brief Rotate left.
 *        Affects N, Z, V, C -- C becomes bit 7 of original operand.
 *
 * @param value: The value.
 *
 * @retval The result.
 */
uint8_t rotate_left(uint8_t value) {
    // C becomes bit 7 of original operand
    bool carry = is_cc_bit_set(C_BIT);
    reg.cc &= MASK_NZVC;
    if (is_bit_set(value, 7)) set_cc_bit(C_BIT);

    // N is bit 7 XOR bit 6 of value
    if (is_bit_set(value, 7) != is_bit_set(value, 6)) set_cc_bit(V_BIT);

    for (uint32_t i = 7 ; i > 0  ; i--) {
        if (is_bit_set(value, i - 1)) {
            value |= (1 << i);
        } else {
            value &= ~(1 << i);
        }
    }

    // Set bit 0 from the carry
    if (carry) {
        value |= 1;
    } else {
        value &= 0xFE;
    }

    set_cc_nz(value, false);
    return value;
}


/**
 * @brief Rotate right.
 *        Affects N, Z, C -- C becomes bit 0 of original operand.
 *
 * @param value: The value.
 *
 * @retval The result.
 */
uint8_t rotate_right(uint8_t value) {
    // C becomes bit 0 of original operand
    bool carry = is_cc_bit_set(C_BIT);
    reg.cc &= MASK_NZC;
    if (is_bit_set(value, 0)) set_cc_bit(C_BIT);

    for (uint32_t i = 0 ; i < 7 ; i++) {
        if (is_bit_set(value, i + 1)) {
            value |= (1 << i);
        } else {
            value &= ~(1 << i);
        }
    }

    // Set bit 7 from the carry
    if (carry) {
        value |= 0x80;
    } else {
        value &= 0x7F;
    }

    set_cc_nz(value, false);
    return value;
}


uint8_t decrement(uint8_t value) {
    // Subtract 1 from the operand
    // Affects: N, Z, V
    //          V set only if value is $80

    // See 'Programming the 6809' p 155
    clr_cc_bit(V_BIT);
    if (value == 0x80) set_cc_bit(V_BIT);

    uint8_t answer = value - 1;
    set_cc_nz(answer, false);
    return answer;
}

uint8_t increment(uint8_t value) {
    // Add 1 to the operand
    // Affects N, Z, V

    // See 'Programming the 6809' p 158
    clr_cc_bit(V_BIT);
    if (value == 0x7F) set_cc_bit(V_BIT);

    uint8_t answer = value + 1;
    set_cc_nz(answer, false);
    return answer;
}


uint8_t *set_reg_ptr(uint8_t reg_code) {
    switch(reg_code) {
        case 0x09:
            return &reg.b;
        case 0x0A:
            return &reg.cc;
        case 0x0B:
            return &reg.dp;
    }

    return &reg.a;
}

uint16_t *set_reg_16_ptr(uint8_t reg_code) {
    switch(reg_code) {
        case 0x01:
            return &reg.x;
        case 0x02:
            return &reg.y;
        case 0x03:
            return &reg.u;
        case 0x04:
            return &reg.s;
        case 0x05:
            return &reg.pc;
    }

    // Return a pointer to d
    return &reg.d;
}


void transfer_decode2(uint8_t reg_code, bool is_swap) {

    uint8_t source_reg = (reg_code & 0xF0) >> 4;
    uint8_t dest_reg = reg_code & 0x0F;

    if (source_reg < 0x08 && dest_reg > 0x05) return;
    if (source_reg > 0x05 && dest_reg < 0x08) return;

    if (source_reg > 0x05) {
        // 8-bit transfers
        uint8_t *src_ptr = set_reg_ptr(source_reg);
        uint8_t *dst_ptr = set_reg_ptr(dest_reg);
        uint8_t val = *dst_ptr;
        *dst_ptr = *src_ptr;
        if (is_swap) *src_ptr = val;
    } else {
        // 16-bit transfers

        // If the op touches D, use the temporary 16-bit register
        if (source_reg == 0x00 || dest_reg == 0x00) reg.d = (reg.a << 8) | (reg.b);

        uint16_t *src_16_ptr = set_reg_16_ptr(source_reg);
        uint16_t *dst_16_ptr = set_reg_16_ptr(dest_reg);
        uint16_t val_16 = *dst_16_ptr;
        *dst_16_ptr = *src_16_ptr;
        if (is_swap) *src_16_ptr = val_16;

        // If the op touched D, restore the components
        if (source_reg == 0x00 || dest_reg == 0x00) {
            reg.b = reg.d & 0xFF;
            reg.a = (reg.d >> 8) &0xFF;
        }
    }
}


void transfer_decode(uint8_t reg_code, bool is_swap) {
    // 'reg_code' contains an 8-bit number that identifies the two registers
    // to be swapped. Top four bits give source, bottom four bits give
    // the destination.

    uint8_t source_reg = (reg_code & 0xF0) >> 4;
    uint8_t dest_reg = reg_code & 0x0F;
    uint16_t d = 0;

    switch(source_reg) {
        case 0x00:
            // D - can only swap to X, Y, U, S, PC
            if (dest_reg > 0x05) return;
            d = exchange_16(((reg.a << 8) | reg.b), dest_reg);
            if (is_swap) {
                reg.a = (d >> 8) & 0xFF;
                reg.b = d & 0xFF;
            }
            break;
        case 0x01:
            // X - can only swap to D, Y, U, S, PC
            if (dest_reg > 0x05) return;
            if (is_swap) {
                reg.x = exchange_16(reg.x, dest_reg);
            } else {
                exchange_16(reg.x, dest_reg);
            }
            break;
        case 0x02:
            // Y - can only swap to D, X, U, S, PC
            if (dest_reg > 0x05) return;
            if (is_swap) {
                reg.y = exchange_16(reg.y, dest_reg);
            } else {
                exchange_16(reg.y, dest_reg);
            }
            break;
        case 0x03:
            // U - can only swap to D, X, Y, S, PC
            if (dest_reg > 0x05) return;
            if (is_swap) {
                reg.u = exchange_16(reg.u, dest_reg);
            } else {
                exchange_16(reg.u, dest_reg);
            }
            break;
        case 0x04:
            // S - can only swap to D, X, Y, U, PC
            if (dest_reg > 0x05) return;
            if (is_swap) {
                reg.s = exchange_16(reg.s, dest_reg);
            } else {
                exchange_16(reg.s, dest_reg);
            }
            break;
        case 0x05:
            // PC - can only swap to D, X, Y, U, S
            if (dest_reg > 0x05) return;
            if (is_swap) {
                reg.pc = exchange_16(reg.pc, dest_reg);
            } else {
                exchange_16(reg.pc, dest_reg);
            }
            break;
        case 0x08:
            // A - can only swap to B, CC, DP
            if (dest_reg < 0x08) return;
            if (is_swap) {
                reg.a = exchange(reg.a, dest_reg);
            } else {
                exchange(reg.a, dest_reg);
            }
            break;
        case 0x09:
            // B - can only swap to A, CC, DP
            if (dest_reg < 0x08) return;
            if (is_swap) {
                reg.b = exchange(reg.b, dest_reg);
            } else {
                exchange(reg.b, dest_reg);
            }
            break;
        case 0x0A:
            // CC - can only swap to A, B, DP
            if (dest_reg < 0x08) return;
            if (is_swap) {
                reg.cc = exchange(reg.cc, dest_reg);
            } else {
                exchange(reg.cc, dest_reg);
            }
            break;
        case 0x0B:
            // DPR - can only swap to A, B, CC
            if (dest_reg < 0x08) return;
            if (is_swap) {
                reg.dp = exchange(reg.dp, dest_reg);
            } else {
                exchange(reg.dp, dest_reg);
            }
            break;
    }
}


uint8_t exchange(uint8_t value, uint8_t reg_code) {
    // Returns the value *from* the register identified by 'reg_code'
    // after placing the passed value into that register
    uint8_t return_value = 0x00;
    switch(reg_code) {  // This is the DESTINATION register
        case 0x08: // A
            return_value = reg.a;
            reg.a = value;
            break;
        case 0x09: // B
            return_value = reg.b;
            reg.b = value;
            break;
        case 0x0A: // CC
            return_value = reg.cc;
            reg.cc = value;
            break;
        case 0x0B: // DPR
            return_value = reg.dp;
            reg.dp = value;
            break;
        default:
            // Incorrect Register - signal error
            // [self halt:@"Incorrect register in 8-bit swap"];
            return_value = 0xFF;
    }

    return return_value;
}


uint16_t exchange_16(uint16_t value, uint8_t reg_code) {
    // Returns the value *from* the register identified by 'reg_code'
    // after placing the passed value into that register
    uint16_t return_value = 0x0000;
    switch(reg_code) {  // This is the DESTINATION register
        case 0x00: // D
            return_value = (reg.a << 8) | reg.b;
            reg.a = (value >> 8) & 0xFF;
            reg.b = value & 0xFF;
            break;
        case 0x01: // X
            return_value = reg.x;
            reg.x = value;
            break;
        case 0x02: // Y
            return_value = reg.y;
            reg.y = value;
            break;
        case 0x03: // U
            return_value = reg.u;
            reg.u = value;
            break;
        case 0x04: // S
            return_value = reg.s;
            reg.s = value;
            break;
        case 0x05: // PC
            return_value = reg.pc;
            reg.pc = value;
            break;
        default:
            // Incorrect Register - signal error
            // [self halt:@"Incorrect register in 8-bit swap"];
            return_value = 0xFFFF;
    }

    return return_value;
}


void load_effective(uint16_t amount, uint8_t reg_code) {
    // Put passed value into an 8-bit register
    // Affects Z -- but only for X and Y registers
    uint16_t* reg_ptr = &reg.x;
    switch (reg_code) {
        case 0x01:
            reg_ptr = &reg.y;
        case 0x02:
            reg_ptr = &reg.s;
        case 0x03:
            reg_ptr = &reg.u;
    }

    if (reg_code < 2) {
        if (amount == 0) {
            set_cc_bit(Z_BIT);
        } else {
            clr_cc_bit(Z_BIT);
        }
    }

    *reg_ptr = amount;
}


void push(bool to_hardware, uint8_t post_byte) {
    // Push the specified registers (in 'post_byte') to the hardware or user stack
    // ('to_hardware' should be true for S, false for U)
    // See 'Programming the 6809' p.171-2
    uint16_t source = to_hardware ? reg.u : reg.s;
    uint16_t dest   = to_hardware ? reg.s : reg.u;

    if (is_bit_set(post_byte, 7)) {
        // Push PC
        dest--;
        set_byte(dest, (reg.pc & 0xFF));
        dest--;
        set_byte(dest, ((reg.pc >> 8) & 0xFF));
    }

    if (is_bit_set(post_byte, 6)) {
        // Push U/S
        dest--;
        set_byte(dest, (source & 0xFF));
        dest--;
        set_byte(dest, ((source >> 8) & 0xFF));
    }

    if (is_bit_set(post_byte, 5)) {
        // Push Y
        dest--;
        set_byte(dest, (reg.y & 0xFF));
        dest--;
        set_byte(dest, ((reg.y >> 8) & 0xFF));
    }

    if (is_bit_set(post_byte, 4)) {
        // Push X
        dest--;
        set_byte(dest, (reg.x & 0xFF));
        dest--;
        set_byte(dest, ((reg.x >> 8) & 0xFF));
    }

    if (is_bit_set(post_byte, 3)) {
        // Push DP
        dest--;
        set_byte(dest, reg.dp);
    }

    if (is_bit_set(post_byte, 2)) {
        // Push B
        dest--;
        set_byte(dest, reg.b);
    }

    if (is_bit_set(post_byte, 1)) {
        // Push A
        dest--;
        set_byte(dest, reg.a);
    }

    if (is_bit_set(post_byte, 0)) {
        // Push CC
        dest--;
        set_byte(dest, reg.cc);
    }

    if (to_hardware) {
        reg.s = dest;
    } else {
        reg.u = dest;
    }
}


void pull(bool from_hardware, uint8_t post_byte) {
    // Pull the specified registers (in 'post_byte') from the hardware or user stack
    // ('from_hardware' should be true for S, false for U)
    // See 'Programming the 6809' p.173-4
    uint16_t source = from_hardware ? reg.s : reg.u;
    uint16_t dest   = from_hardware ? reg.u : reg.s;

    if (is_bit_set(post_byte, 0)) {
        // Pull CC
        reg.cc = get_byte(source);
        source++;
    }

    if (is_bit_set(post_byte, 1)) {
        // Pull A
        reg.a = get_byte(source);
        source++;
    }

    if (is_bit_set(post_byte, 2)) {
        // Pull B
        reg.b = get_byte(source);
        source++;
    }

    if (is_bit_set(post_byte, 3)) {
        // Pull DP
        reg.dp = get_byte(source);
        source++;
    }

    if (is_bit_set(post_byte, 4)) {
        // Pull X
        reg.x = (get_byte(source) << 8);
        source++;
        reg.x |= get_byte(source);
        source++;
    }

    if (is_bit_set(post_byte, 5)) {
        // Pull Y
        reg.y = (get_byte(source) << 8);
        source++;
        reg.y |= get_byte(source);
        source++;
    }

    if (is_bit_set(post_byte, 6)) {
        // Pull S or U
        dest = (get_byte(source) << 8);
        source++;
        dest |= get_byte(source);
        source++;
    }

    if (is_bit_set(post_byte, 7)) {
        // Pull PC
        reg.pc = (get_byte(source) << 8);
        source++;
        reg.pc |= get_byte(source);
        source++;
    }

    if (from_hardware) {
        reg.s = source;
        reg.u = dest;
    } else {
        reg.u = source;
        reg.s = dest;
    }
}

void test(uint8_t value) {
    // Tests value for zero or negative
	// Affects N, Z, V
    //         V is always cleared
    clr_cc_nzv();
    set_cc_nz(value, false);
}


/*
 * ADDRESSING FUNCTIONS
 */

/**
 * @brief Calculate the effective 16-bit address based on the opcode's
 *        addressing mode -- this is the address of the data.
 *        On entry PC is pointing to the post byte.
 *
 * @note All of the called methods update PC.
 *
 * @param mode: The addressing mode.
 *
 * @retval The calculated address.
 */
uint16_t address_from_mode(uint8_t mode) {
    uint16_t address = 0;
    if (mode == MODE_IMMEDIATE) {
        // The effective address is the next byte,
        // to which PC is already pointing
        // NOTE Increment PC to avoid doing so for all 8-bit reads
        address = reg.pc;
        reg.pc++;
    } else if (mode == MODE_DIRECT) {
        // The effective address is MSB: DP, LSB: next byte,
        // to which PC is already pointing
        // NOTE Increment PC to avoid doing so for all 8-bit reads
        address = address_from_dpr(0);
        reg.pc++;
    } else if (mode == MODE_INDEXED) {
        // Indexed addressing, inc. extended indirect
        uint8_t post_byte = get_next_byte();
        address = indexed_address(post_byte);
    } else {
        // The effective address is stored in the next two bytes
        // NOTE Moves PC on 2
        address = address_from_next_two_bytes();
    }

    return address;
}


/**
 * @brief Read the bytes at regPC and regPC + 1 and returns them as a 16-bit address.
 *
 * @note `get_next_byte()` auto-increments regPC and handles rollover.
 *
 * @retval The calculated address.
 */
uint16_t address_from_next_two_bytes() {
    uint8_t msb = get_next_byte();
    uint8_t lsb = get_next_byte();
    return (msb << 8) | lsb;
}


/**
 * @brief Calculate the address composed from regDP (MSB) and the byte at
 *        regPC (LSB) plus any supplied offset. Does not increment regPC.
 *
 * @todo Should we increment regPC?
 *
 * @param offset: Any address offset to be applied.
 *
 * @retval The calculated address.
 */
uint16_t address_from_dpr(int16_t offset) {
    uint16_t address = reg.pc;
    address += offset;
    return (reg.dp << 8) | get_byte(address);
}


/**
 * @brief Calculate the target address for an indexed addressing op.
 *        This function increases the PC.
 *
 * @param post_byte: The byte indicating the indexing mode.
 *
 * @retval The calculated address.
 */
uint16_t indexed_address(uint8_t post_byte) {
    // Determine the source register
    uint8_t source_reg = ((post_byte & 0x60) >> 5);

    // Set 'returnValue' to the contents of the source register
    uint16_t address = register_value(source_reg);

    // Process the opcode encoded in the first five bits of the postbyte
    uint8_t op = post_byte & 0x1F;
    if ((post_byte & 0x80) == 0x00) {
        // 5-bit non-indirect offset
        int16_t offset = is_bit_set(op, 4) ? op - 0x20 : op;
        address += offset;
    } else {
        // All other opcodes have bit 7 set to 1
        uint8_t msb, lsb;
        int8_t value;
        int16_t value_16;
        switch(op) {
            case 0:
                // Auto-increment (,R+)
                address = register_value(source_reg);
                increment_register(source_reg, 1);
                break;
            case 1:
                // Auto-increment (,R++)
                address = register_value(source_reg);
                increment_register(source_reg, 2);
                break;
            case 2:
                // Auto-decrement (,-R)
                increment_register(source_reg, -1);
                address = register_value(source_reg);
                break;
            case 3:
                // Auto-decrement (,--R)
                increment_register(source_reg, -2);
                address = register_value(source_reg);
                break;
            case 4:
                // No offset (,R) - 'address' already set
                break;
            case 5:
                // Accumulator offset B; B is 2s-comp
                address += is_bit_set(reg.b, SIGN_BIT_8) ? reg.b - 256 : reg.b;
                break;
            case 6:
                // Accumulator offset A; A is 2s-comp
                address += is_bit_set(reg.a, SIGN_BIT_8) ? reg.a - 256 : reg.a;
                break;
            case 8:
                // 8-bit constant offset; offset is 2s-comp
                value = get_next_byte();
                address += is_bit_set(value, SIGN_BIT_8) ? value - 256 : value;
                break;
            case 9:
                // 16-bit constant offset; offset is 2s-comp
                msb = get_next_byte();
                lsb = get_next_byte();
                value_16 = (msb << 8) | lsb;
                address += is_bit_set(value_16, SIGN_BIT_16) ? value_16 - 65536 : value_16;
                break;
            case 11:
                // Accumulator offset D; D is a 2s-comp offset
                value_16 = (reg.a << 8) | reg.b;
                address += is_bit_set(value_16, SIGN_BIT_16) ? value_16 - 65536 : value_16;
                break;
            case 12:
                // PC relative 8-bit offset; offset is 2s-comp
                value = get_next_byte();
                address = reg.pc + (is_bit_set(value, SIGN_BIT_8) ? value - 256 : value);
                break;
            case 13:
                // regPC relative 16-bit offset; offset is 2s-comp
                msb = get_next_byte();
                lsb = get_next_byte();
                value_16 = (msb << 8) | lsb;
                address = reg.pc + (is_bit_set(value_16, SIGN_BIT_16) ? value_16 - 65536 : value_16);
                break;

            // From here on, the addressing is indirect: the Effective Address is a handle not a pointer
            // So we need to add a second memoryMap[] lookup to return the ultimate address we are pointing to

            case 17:
                // Indirect auto-increment + 2
                increment_register(source_reg, 2);
                break;
            case 19:
                // Indirect auto-decrement - 2
                increment_register(source_reg, -2);
                address = register_value(source_reg);
                break;
            case 20:
                // Indirect constant zero offset
                // address = [ram peek:address];
                break;
            case 21:
                // Indirect Accumulator offset B
                // eg. LDA [B,X]
                address += (is_bit_set(reg.b, SIGN_BIT_8) ? reg.b - 256 : reg.b);
                break;
            case 22:
                // Indirect Accumulator offset A
                // eg. LDA [A,Y]
                address += (is_bit_set(reg.a, SIGN_BIT_8) ? reg.a - 256 : reg.a);
                break;
            case 24:
                // Indirect constant 8-bit offset
                // eg. LDA [n,X]
                value = get_next_byte();
                address += (is_bit_set(value, SIGN_BIT_8) ? value - 256 : value);
                break;
            case 25:
                // Indirect constant 16-bit offset
                // eg. LDA [n,X]
                msb = get_next_byte();
                lsb = get_next_byte();
                value_16 = (msb << 8) | lsb;
                address += (is_bit_set(value_16, SIGN_BIT_16) ? value_16 - 65536 : value_16);
                break;
            case 27:
                // Indirect Accumulator offset D
                // eg. LDA [D,X]
                value_16 = (reg.a << 8) | reg.b;
                address += (is_bit_set(value_16, SIGN_BIT_16) ? value_16 - 65536 : value_16);
                break;
            case 28:
                // Indirect regPC relative 8-bit offset
                // eg. LDA [n,PCR]
                value = get_next_byte();
                address = reg.pc + (is_bit_set(value, SIGN_BIT_8) ? value - 256 : value);
                break;
            case 29:
                // Indirect regPC relative 16-bit offset
                // eg. LDX [n,PCR]
                msb = get_next_byte();
                lsb = get_next_byte();
                value_16 = (msb << 8) | lsb;
                address = reg.pc + (is_bit_set(value_16, SIGN_BIT_16) ? value_16 - 65536 : value_16);
                break;
            case 31:
                // Extended indirect
                // eg. LDA [n]
                msb = get_next_byte();
                lsb = get_next_byte();
                address = (msb << 8) | lsb;
                break;
        }

        // For indirect address, use 'address' as a handle
        if (op > 16) {
            // Keep current value of PC
            uint16_t pc = reg.pc;
            reg.pc = address;
            address = address_from_next_two_bytes();

            // Put original PC value back
            reg.pc = pc;
        }
    }

    return address;
}


/**
 * @brief Return the value of the specified index register.
 *
 * @param source_reg: The register to read.
 *
 * @retval The register's stored value.
 */
uint16_t register_value(uint8_t source_reg) {
    if (source_reg == REG_X) return reg.x;
    if (source_reg == REG_Y) return reg.y;
    if (source_reg == REG_U) return reg.u;
    return reg.s;
}


/**
 * @brief Increment/decrement a register by the specified amount.
 *
 * Calculate the offset from the Two's Complement of the 8- or 16-bit value.
 *
 * @param source_reg: The register to update.
 * @param amount:     The increment/decrement value.
 */
void increment_register(uint8_t source_reg, int16_t amount) {
    uint16_t *reg_ptr = &reg.x;
    if (source_reg == REG_Y) reg_ptr = &reg.y;
    if (source_reg == REG_U) reg_ptr = &reg.u;
    if (source_reg == REG_S) reg_ptr = &reg.s;

    uint16_t new_value = *reg_ptr;
    new_value += amount;
    *reg_ptr = new_value;
}


/**
 * @brief Set certain registers after RESET.
 *
 * See Zaks p.250
 */
void reset_registers() {
    // Zero DP
    reg.dp = 0;

    // Clear CC F and I bits
    reg.cc &= 0xAf;

    // Set PC from reset vector
    reg.pc = (mem[RESET_VECTOR] << 8) | mem[RESET_VECTOR + 1];
}


/**
 * @brief Zero all registers.
 */
void clear_all_registers() {
    reg.dp = 0;
    reg.cc = 0;
    reg.a = 0;
    reg.b = 0;
    reg.x = 0;
    reg.y = 0;
    reg.s = 0x8000;
    reg.u = 0x8000;
    reg.pc = 0x0000;
}

/**
 * @brief Interrupt service routine handler.
 *
 * @param irq: Interrupt type code.
 */
void process_interrupt(uint8_t irq) {
    if (irq == FIRQ_BIT) {
        clr_cc_bit(E_BIT);
        push(true, PUSH_PULL_CC_REG | PUSH_PULL_PC_REG);
        set_cc_bit(F_BIT);
        set_cc_bit(I_BIT);
        state.bus_state_pins = 0x02;
        reg.pc = (mem[FIRQ_VECTOR] << 8) | mem[FIRQ_VECTOR + 1];
        state.bus_state_pins = 0x00;
        flash_led(2);
    }

    if (irq == IRQ_BIT) {
        set_cc_bit(E_BIT);
        push(true, PUSH_PULL_ALL_REGS);
        set_cc_bit(I_BIT);
        state.bus_state_pins = 0x02;
        reg.pc = (mem[IRQ_VECTOR] << 8) | mem[IRQ_VECTOR + 1];
        state.bus_state_pins = 0x00;
        flash_led(4);
    }

    if (irq == NMI_BIT) {
        set_cc_bit(E_BIT);
        push(true, PUSH_PULL_ALL_REGS);
        set_cc_bit(F_BIT);
        set_cc_bit(I_BIT);
        state.bus_state_pins = 0x02;
        reg.pc = (mem[NMI_VECTOR] << 8) | mem[NMI_VECTOR + 1];
        state.bus_state_pins = 0x00;
        flash_led(6);
    }
}
