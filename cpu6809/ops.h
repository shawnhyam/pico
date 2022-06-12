/*
 * e6809 for Raspberry Pi Pico
 * Opcode constant definitions
 *
 * @version     1.0.0
 * @author      smittytone
 * @copyright   2022
 * @licence     MIT
 *
 */
#ifndef _OPS_HEADER_
#define _OPS_HEADER_

// Opcodes

#define NEG_direct                   0x00 //
#define COM_direct                   0x03 //
#define LSR_direct                   0x04 //
#define ROR_direct                   0x06 //
#define ASR_direct                   0x07 //
#define ASL_direct                   0x08 //
#define ROL_direct                   0x09 //
#define DEC_direct                   0x0A //
#define INC_direct                   0x0C //
#define TST_direct                   0x0D //

#define JMP_direct                   0x0E // UNTESTED
#define CLR_direct                   0x0F //

#define NOP                          0x12 //
#define SYNC                         0x13
#define LBRA                         0x16 //
#define LBSR                         0x17 //
#define DAA                          0x19 // BUG
#define ORCC_immed                   0x1A //
#define ANDCC_immed                  0x1C //
#define SEX                          0x1D // UNTESTED
#define EXG_immed                    0x1E //
#define TFR_immed                    0x1F //

#define BRA                          0x20 // UNTESTED
#define BRN                          0x21 // UNTESTED
#define BHI                          0x22 // UNTESTED
#define BLS                          0x23 // UNTESTED
#define BHS                          0x24 // UNTESTED
#define BLO                          0x25 // UNTESTED
#define BNE                          0x26 // UNTESTED
#define BEQ                          0x27 // UNTESTED
#define BVC                          0x28 // UNTESTED
#define BVS                          0x29 // UNTESTED
#define BPL                          0x2A // UNTESTED
#define BMI                          0x2B // UNTESTED
#define BGE                          0x2C // UNTESTED
#define BLT                          0x2D // UNTESTED
#define BGT                          0x2E // UNTESTED
#define BLE                          0x2F // UNTESTED

#define LEAX_indexed                 0x30 //
#define LEAY_indexed                 0x31 //
#define LEAS_indexed                 0x32 //
#define LEAU_indexed                 0x33 //

#define PSHS_immed                   0x34 //
#define PULS_immed                   0x35 //
#define PSHU_immed                   0x36 //
#define PULU_immed                   0x37 //

#define RTS                          0x39 //
#define ABX                          0x3A //
#define RTI                          0x3B //
#define CWAI_immed                   0x3C //
#define MUL                          0x3D //
#define SWI                          0x3F // UNTESTED

#define NEGA                         0x40 // Note: Immediate addressing
#define COMA                         0x43 //
#define LSRA                         0x44 //
#define RORA                         0x46 //
#define ASRA                         0x47 //
#define ASLA                         0x48 //
#define ROLA                         0x49 //
#define DECA                         0x4A //
#define INCA                         0x4C //
#define TSTA                         0x4D //
#define CLRA                         0x4F //

#define NEGB                         0x50 // Note: Immediate addressing
#define COMB                         0x53 //
#define LSRB                         0x54 //
#define RORB                         0x56 //
#define ASRB                         0x57 //
#define ASLB                         0x58 //
#define ROLB                         0x59 //
#define DECB                         0x5A //
#define INCB                         0x5C //
#define TSTB                         0x5D //
#define CLRB                         0x5F //

#define NEG_indexed                  0x60 //
#define COM_indexed                  0x63 //
#define LSR_indexed                  0x64 //
#define ROR_indexed                  0x66 //
#define ASR_indexed                  0x67 //
#define ASL_indexed                  0x68 //
#define ROL_indexed                  0x69 //
#define DEC_indexed                  0x6A //
#define INC_indexed                  0x6C //
#define TST_indexed                  0x6D //
#define JMP_indexed                  0x6E //
#define CLR_indexed                  0x6F //

#define NEG_extended                 0x70
#define COM_extended                 0x73
#define LSR_extended                 0x74
#define ROR_extended                 0x76
#define ASR_extended                 0x77
#define ASL_extended                 0x78
#define ROL_extended                 0x79
#define DEC_extended                 0x7A
#define INC_extended                 0x7C
#define TST_extended                 0x7D
#define JMP_extended                 0x7E
#define CLR_extended                 0x7F

#define SUBA_immed                   0x80
#define CMPA_immed                   0x81
#define SBCA_immed                   0x82
#define SUBD_immed                   0x83
#define ANDA_immed                   0x84
#define BITA_immed                   0x85
#define LDA_immed                    0x86
#define EORA_immed                   0x88
#define ADCA_immed                   0x89
#define ORA_immed                    0x8A
#define ADDA_immed                   0x8B
#define CMPX_immed                   0x8C
#define BSR                          0x8D
#define LDX_immed                    0x8E

#define SUBA_direct                  0x90
#define CMPA_direct                  0x91
#define SBCA_direct                  0x92
#define SUBD_direct                  0x93
#define ANDA_direct                  0x94
#define BITA_direct                  0x95
#define LDA_direct                   0x96
#define STA_direct                   0x97
#define EORA_direct                  0x98
#define ADCA_direct                  0x99
#define ORA_direct                   0x9A
#define ADDA_direct                  0x9B
#define CMPX_direct                  0x9C
#define JSR_direct                   0x9D
#define LDX_direct                   0x9E
#define STX_direct                   0x9F

#define SUBA_indexed                 0xA0
#define CMPA_indexed                 0xA1
#define SBCA_indexed                 0xA2
#define SUBD_indexed                 0xA3
#define ANDA_indexed                 0xA4
#define BITA_indexed                 0xA5
#define LDA_indexed                  0xA6
#define STA_indexed                  0xA7
#define EORA_indexed                 0xA8
#define ADCA_indexed                 0xA9
#define ORA_indexed                  0xAA
#define ADDA_indexed                 0xAB
#define CMPX_indexed                 0xAC
#define JSR_indexed                  0xAD
#define LDX_indexed                  0xAE
#define STX_indexed                  0xAF

#define SUBA_extended                0xB0
#define CMPA_extended                0xB1
#define SBCA_extended                0xB2
#define SUBD_extended                0xB3
#define ANDA_extended                0xB4
#define BITA_extended                0xB5
#define LDA_extended                 0xB6
#define STA_extended                 0xB7
#define EORA_extended                0xB8
#define ADCA_extended                0xB9
#define ORA_extended                 0xBA
#define ADDA_extended                0xBB
#define CMPX_extended                0xBC
#define JSR_extended                 0xBD
#define LDX_extended                 0xBE
#define STX_extended                 0xBF

#define SUBB_immed                   0xC0
#define CMPB_immed                   0xC1
#define SBCB_immed                   0xC2
#define ADDD_immed                   0xC3
#define ANDB_immed                   0xC4
#define BITB_immed                   0xC5
#define LDB_immed                    0xC6
#define EORB_immed                   0xC8
#define ADCB_immed                   0xC9
#define ORB_immed                    0xCA
#define ADDB_immed                   0xCB
#define LDD_immed                    0xCC
#define LDU_immed                    0xCE

#define SUBB_direct                  0xD0
#define CMPB_direct                  0xD1
#define SBCB_direct                  0xD2
#define ADDD_direct                  0xD3
#define ANDB_direct                  0xD4
#define BITB_direct                  0xD5
#define LDB_direct                   0xD6
#define STB_direct                   0xD7
#define EORB_direct                  0xD8
#define ADCB_direct                  0xD9
#define ORB_direct                   0xDA
#define ADDB_direct                  0xDB
#define LDD_direct                   0xDC
#define STD_direct                   0xDD
#define LDU_direct                   0xDE
#define STU_direct                   0xDF

#define SUBB_indexed                 0xE0
#define CMPB_indexed                 0xE1
#define SBCB_indexed                 0xE2
#define ADDD_indexed                 0xE3
#define ANDB_indexed                 0xE4
#define BITB_indexed                 0xE5
#define LDB_indexed                  0xE6
#define STB_indexed                  0xE7
#define EORB_indexed                 0xE8
#define ADCB_indexed                 0xE9
#define ORB_indexed                  0xEA
#define ADDB_indexed                 0xEB
#define LDD_indexed                  0xEC
#define STD_indexed                  0xED
#define LDU_indexed                  0xEE
#define STU_indexed                  0xEF

#define SUBB_extended                0xF0
#define CMPB_extended                0xF1
#define SBCB_extended                0xF2
#define ADDD_extended                0xF3
#define ANDB_extended                0xF4
#define BITB_extended                0xF5
#define LDB_extended                 0xF6
#define STB_extended                 0xF7
#define EORB_extended                0xF8
#define ADCB_extended                0xF9
#define ORB_extended                 0xFA
#define ADDB_extended                0xFB
#define LDD_extended                 0xFC
#define STD_extended                 0xFD
#define LDU_extended                 0xFE
#define STU_extended                 0xFF

// Extended opcode start here

#define LBRN                         0x1021
#define LBHI                         0x1022
#define LBLS                         0x1023
#define LBHS                         0x1024
#define LBLO                         0x1025
#define LBNE                         0x1026
#define LBEQ                         0x1027
#define LBVC                         0x1028
#define LBVS                         0x1029
#define LBPL                         0x102A
#define LBMI                         0x102B
#define LBGE                         0x102C
#define LBLT                         0x102D

#define LBGT                         0x102E
#define LBLE                         0x102F
#define SWI2                         0x103F
#define CMPD_immed                   0x1083
#define CMPY_immed                   0x108C
#define LDY_immed                    0x108E
#define CMPD_direct                  0x1093
#define CMPY_direct                  0x109C
#define LDY_direct                   0x109E
#define STY_direct                   0x109F
#define CMPD_indexed                 0x10A3
#define CMPY_indexed                 0x10AC
#define LDY_indexed                  0x10AE
#define STY_indexed                  0x10AF
#define CMPD_extended                0x10B3
#define CMPY_extended                0x10BC
#define LDY_extended                 0x10BE
#define STY_extended                 0x10BF

#define LDS_immed                    0x10CE
#define LDS_direct                   0x10DE
#define STS_direct                   0x10DF
#define LDS_indexed                  0x10EE
#define STS_indexed                  0x10EF
#define LDS_extended                 0x10FE
#define STS_extended                 0x10FF

// Second extended opcodes

#define SWI3                         0x113F
#define CMPU_immed                   0x1183
#define CMPS_immed                   0x118C
#define CMPU_direct                  0x1193
#define CMPS_direct                  0x119C
#define CMPU_indexed                 0x11A3
#define CMPS_indexed                 0x11AC
#define CMPU_extended                0x11B3
#define CMPS_extended                0x11BC

#endif // _OPS_HEADER_
