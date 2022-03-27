#include <stdio.h>
#include "pico/stdlib.h"

#define DEBUG

#define PRINTLN()                                                              \
  do {                                                                         \
    printf("\n");                                                              \
  } while (0)

char *input = "R%?naeloob,?qe,!rdc-tes,raaaac,rdaaac,radaac,rddaac,raadac,rdadac,raddac,raaadc,rdaadc,radadc,rddadc,raaddc,rdaddc,radddc,rddddc,qmem,qssa,?regetni,=,>,=<,=>,?orez,?evitisop,?evitagen,xam,nim,oludom,mcl,rotaremun,rotanimoned,roolf,gniliec,etacnurt,dnuor,htgnel-gnirts,!tes-gnirts,gnirts-ekam,?=gnirts,?=<gnirts,?=>gnirts,gnirtsbus,dneppa-gnirts,ypoc-gnirts,!llif-gnirts,htgnel-rotcev,fer-rotcev,!tes-rotcev,rotcev-ekam,!llif-rotcev,cc/llac,etouq,!tes,fi,adbmal,nigeb,tel,dna,ro,dnoc,enifed,!rac-tes,esrever,vmem,rebmem,vssa,cossa,gnirts>-lobmys,lobmys>-gnirts,?neve,?ddo,redniamer,dcg,tpxe,gnirts>-rebmun,rebmun>-gnirts,fer-gnirts,?<gnirts,?>gnirts,?rotcev,tsil>-rotcev,?erudecorp,pam,hcae-rof,lave,rdddac,raac,radc,raaac,rdaac,radac,raadc,rdadc,raddc,rdddc,fer-tsil,?gnirts,rotcev>-tsil,enilwen,lper,dneppa,?tcejbo-foe,rahc-keep,yalpsid,+,?lobmys,sba,daer,etirw,?llun,htgnel,?lauqe,rddac,tneitouq,,,*,gnirts>-tsil,tsil>-gnirts,,ton,,-,rddc,rahc-daer,,,,rdac,<,,rac,,?riap,,rdc,snoc,?vqe,,,,,;8N!N8NBYOBYEZ/^8O~YL^YFBYT-vCvR3y]/7#YTN^z!TN8T;i&:DiU(ai&kkz!U(:kw'k!T:'_)YT:aA_E^~C^{!T,'^8T,YIlbA`^'`~?_E_~C_|!T58T9`^YT,ka_AaE`-YT5dAb@ai$E`^~C_|!TO#`kn88^~i$#`kn88^~i$#`kn88^~i$#`kn88^~YT+O^~?w+F^~?kI^~YT+^z!TA#YTOa_l{!T9#a_k#k_k~?iU(_{!>-b-:UfAdbw+k~CAaE`^|!T/-U:h-w4k-Uf~?iU(fd@aaa^}(!T7)i&^z!T8)YT7`^{!T6)YT8b`^|!?)YT6ca_xN|!-#b`n8T/f@i&@bwU'awU'`8T5@ea_`~YH_A`-ci$-cV@MdxH@AHcxLEHa-c@AHbxL~?EHbwU&~CAa_~?xH^-ci$-cK^-cKYT6V@MgxIwU'wU'YT7YT8`wU'xK~CMbHa~CAa_~?xI^-ci%-cK^-cKVi$@MdxJ^~CMbHa~CAa_~?xJ^8T/fMdH`E_`EH`~?xK^8>cAa_~?xL^#YT9ewT4#d~YDbYT;i&:UiU(MeYT:@@fi$i$akYC_nH`~?xM^-:DgZ.ecDfYAdboHa_~?xN^-YTAdYT,lbHbYAa_~K?xG_8T/f@i&@bwU'awU'`8T5@ea_`~YH_A`-ci$-cV@MdxH@AHcxLEHa-c@AHbxL~?EHbwU&~CAa_~?xH^-ci$-cK^-cKYT6V@MgxIwU'wU'YT7YT8`wU'xK~CMbHa~CAa_~?xI^-ci%-cK^-cKVi$@MdxJ^~CMbHa~CAa_~?xJ^8T/fMdH`E_`EH`~?xK^8>cAa_~?xL^#YT9ewT4#d~YDbYT;i&:UiU(MeYT:@@fi$i$akYC_nH`~?xM^-:DgZ.ecDfYAdboHa_~?xN^-YTAdYT,lbHbYAa_~K^~^?xO^#cHan~?xP^E_~C_#bYT,k``m~YH_|!T-3_BJ^{!O3uy!:'i$8:aA_BJ^8:aA_BJ^BJvS#~K?vS#_8:aA_BJ^8:aA_BJ^BJvS#~K^~^?vE^8:aA_BJvS5BJvS#~?u^8:aA_BJ^~P`E^~C^{!TB'i$'i$8TBA^BYEE^~C^BJvC~C^z!J8JZ9^8T-vS7vF~Z2^8EZ3^BJvF~Z4^8:i$R^~Z#^8JZ@^~YH^3vLBYTBA^BYEE^BJvK~C^8T-vLvK~YD^8T-vS;vF~?i%^8T-vS-vF~P^z!E8J^3vEBQi%R^BJvE~Z#^z!TC8TC8T2~?u^'^~Gk^Ly!T28T2BL'^8TC~?vR0^~G_vC'iU+~YL^YKy!T18T1@`^8T1@a^8T1@au~?vS5^L~?vS#^9E_~?vE^'i&~YL^Lz!T0)YT0^BL'i&~KKGvD`)YT0^BL'i&~KK^~^?vL_)YT0^BL'i&~K^~^?vK^YKy!TD)YTD^YF'i&BL~?vL^YT2y!F9?`'^~^^Z8^S@YT0^BL8<YT1i&BL~?vE^)@i&YFxPBL~?vJ^8PYF'i%BL~?vS;^'i$BL~?vS-^YKBL~?vF^8TDBL~?vK^'^~Gk^YT2y!K'^!U)^Ly!TE'^!U)iU*'^~?iU+^!U)^z!58TE^8TEYTP~?iU*^'^~?iU+^iU)y!U)iU*!L(iU+^z!S#7%YT4'_BYU#Oc^BYT@Ic^FFYT4i$zFFYT4i$z]0'i$90A`^BX$E_~C_{]1'i&)Z1Aa_X$E_~C_{!T;#l`^{]2YT.l!S$8T@YT3aF_^{!S%8PYT3k^z!S&8TLb`I^|!S'9$`I^{!S(i/]3i2!P#oYC_^z]4YT.o!S)8T@YT3aF_^{!S*8<YMi&R^z!S+8<YMR`R^{!TF8<a8TF@fZ7bb`a_Nl`~Ga_}'!S,8TFi&b`^|!TG'k'iU+~C_'l8TGAbA`'l~G`^'iU+~G__E`E^~C_~C^{!T<8TGR`R^{!S-89Z6`^{!S.89Z5`^{]50YT<a_k{]60kYT<`^{!S/(kYT<`^{!S08<YT3vC^z!S18TLb`I^|]79$`I^{!S2i/!;i2!<#nYC_^z]#YT.n!TH'_'i$'i$8THNNvR%`TbuA_~GvR/^~G_vR$E^~C^{!TI8THk^'i$~YD^z]8'i$87_k~^YTI^8TIA^~?vPE^'i$~YD^R^z!T='^8T=_`~Gak@b^YINTu``vR%Wu^{]98<YT=i&^8<@YT=i&N`kvP~Gk^z]:'^8=__~Z=`Z:Wm`T_^'l~?k_{!S3i'!S4i'!S5i'!S6i'!S7'lz!S8i'!S98=_WZ;``_YG`YG^'k~?k_{!T>8T>_Z<__'_~?k^{];8T>`^8T>__~G__YG`YG^{!S:8Ib^'^~?GkbGk`'k~?k^NT`a_W`^{]<87TWb``^{!G'^87_k~Gk^z!S;'_'^~G`^{!S<'^'_~G`^{]=89Z>^z]>(TWm`m^z!S=0k^z!S>0_kz!S?(k^z!S@89G`^{!SA89G__{!SB0__{!SCi(!SD89YT+^zBYU%ki#!U,Fi#!TJ'^!U,@iU,^YU$^8TJFa_'^~YB`F^I_~C_{]?8TJiU,^z]@i/!U$#m_i$z!HYT.m!TK'`8TK@ca`Nl^~G_k|!T38TKi&`^{]A'i$9AAa_'^~YBE__E_~C_{!SEjB]B'i$9BAa_'^~?E__E_~C_{]C'i$9CA`^'_~YBE`^~C_{!SFjD]D'i$9DA`^'_~?E`^~C_{!T?'^8T?Nl`A^~G`k{!TL9FaYT?`^|]$.YT?`^{!TM'_8TM@aE_A^~C^{]E8TMi&^z!M'_)YMaA_E^~C^{!C'k8IYCA_l~C^z!D(i&^z!SG9%F^z!SH9%I^z!SI9&F^z!SJ9&I^z!SK9'F^z!SL9'I^z!SM9(F^z!SN9(I^z!SO8AI^z!SP9)F^z!T#9)I^z!T$9*F^z!T%9*I^z!T&9+F^z!T'9+I^z]%6F^z]&6I^z]'9,F^z](9,I^z])1I^z]*9-F^z]+9-I^z],/I^z]-2I^z].8AF^z!A1F^z!6/F^z!12F^z!T(iU%]FiT@!*i/!.i2!)#k`^{!,YT.k!B'i$'i$'i$'i$8BIaI_~YBFaF_~YBOaO_~YT+`'i$~?pO_~YT+_'^~^?`^{!T)i(!T*89_'^~^?i%^z!9(i$^z!T.8T4'i$(bO^~YT+^zz!U*Nmk!U+Nlk!':lkl!+:lkm!4:lkn!T4:lko!T+:lkp!2:lkq!/:lkr!8:lks!T@:lkt!U%:lku!U#:lkv.!(:lkv/!0:lkv0!I:lkv1!7:lkv2!=:lkv3!@:lkv4!TP:lkv5!3:lkv6y"; // RVM code of the program

// an unsigned byte value for the REPL's code
typedef uint8_t byte;

// object
typedef uint32_t obj;

// a number
typedef int32_t num;

// a rib obj
#define RIB_NB_FIELDS 3
typedef struct {
  obj fields[RIB_NB_FIELDS];
} rib;

#define UNTAG(x) ((x) >> 1)
#define RIB(x) ((rib *)(x))
#define NUM(x) ((num)(UNTAG((num)(x))))
#define IS_NUM(x) ((x)&1)
#define IS_RIB(x) (!IS_NUM(x))
#define TAG_RIB(c_ptr) (((obj)(c_ptr)))
#define TAG_NUM(num) ((((obj)(num)) << 1) | 1)

#define PRIM1() obj x = pop()
#define PRIM2()                                                                \
  obj y = pop();                                                               \
  PRIM1()
#define PRIM3()                                                                \
  obj z = pop();                                                               \
  PRIM2()

#define CAR(x) RIB(x)->fields[0]
#define CDR(x) RIB(x)->fields[1]
#define TAG(x) RIB(x)->fields[2]
#define TOS CAR(stack)

#define NUM_0 (TAG_NUM(0))

#define INSTR_AP 0
#define INSTR_SET 1
#define INSTR_GET 2
#define INSTR_CONST 3
#define INSTR_IF 4
#define INSTR_HALT 5

#define PAIR_TAG TAG_NUM(0)
#define CLOSURE_TAG TAG_NUM(1)
#define SYMBOL_TAG TAG_NUM(2)
#define STRING_TAG TAG_NUM(3)
#define SINGLETON_TAG TAG_NUM(5)

// the only three roots allowed
obj stack = NUM_0;
obj pc = NUM_0;
obj FALSE = NUM_0;

// global, but not a root, referenced
obj symbol_table = NUM_0;

size_t pos = 0;

#define MAX_NB_OBJS 10000

rib heap_start[MAX_NB_OBJS*2];
//heap_start = obj[SPACE_SZ<<1];  


// GC
#define SPACE_SZ (MAX_NB_OBJS * RIB_NB_FIELDS)
#define heap_bot ((obj *)(heap_start))
#define heap_mid (heap_bot + (SPACE_SZ))
#define heap_top (heap_bot + (SPACE_SZ << 1))



#define EXIT_ILLEGAL_INSTR 6
#define EXIT_NO_MEMORY 7

#define vm_exit(code)                                                          \
  do {                                                                         \
    printf("VM EXIT: %d", (code));                                           \
    do { \
        sleep_ms(1000); \
    } while (1); \
  } while (0)



obj *alloc;
obj *alloc_limit;
obj *scan;


void init_heap() {
  //heap_start = malloc(sizeof(obj) * (SPACE_SZ << 1));

//   if (!heap_start) {
//     vm_exit(EXIT_NO_MEMORY);
//   }

  alloc = heap_bot;
  alloc_limit = heap_mid;
  stack = NUM_0;
}

// NULL is a pointer (0) but would represent NULL
// so it is never present in an obj field, and
// cannot be a number because it is even. This
// saves a couple of bytes v.s having STACK
// as the broken heart value
#define GC_COPIED_OBJ ((obj)NULL)



void copy() {
  obj o = *scan;
  // we sometime reference rib that are allocated in BSS,
  // we do not want to copy those
  if (IS_RIB(o)) {
    obj *ptr = RIB(o)->fields;
    obj field0 = ptr[0];
    obj copy;

    if (field0 == GC_COPIED_OBJ) {
      copy = ptr[1]; // copied, get new address
    } else {
      copy = TAG_RIB(alloc);
      *ptr++ = GC_COPIED_OBJ; // ptr points to CDR
      *alloc++ = field0;
      *alloc++ = *ptr++; // ptr points to TAG
      *alloc++ = *ptr;
      ptr[-1] = copy; // set forward ptr. Since it points to TAG, ptr[-1]
                      // rewrites the CDR
    }
    *scan = copy; // overwrite to new address.
  }
  scan++;
}

void gc() {
#ifdef DEBUG
  obj *from_space = (alloc_limit == heap_mid) ? heap_bot : heap_mid;

  size_t objc = alloc - from_space;
  printf("    --GC %d -> ", objc);
#endif

  // swap
  obj *to_space = (alloc_limit == heap_mid) ? heap_mid : heap_bot;
  alloc_limit = to_space + SPACE_SZ;

  alloc = to_space;

  // root: stack
  scan = &stack;
  copy();

  // root: pc
  scan = &pc;
  copy();

  // root: false
  scan = &FALSE;
  copy();

  // scan the to_space to pull all live references
  scan = to_space;
  while (scan != alloc) {
    copy();
  }

#ifdef DEBUG

  objc = alloc - to_space;
  printf("%d\n", objc);

#endif
}

obj pop() {
  obj x = CAR(stack);
  stack = CDR(stack);
  return x;
}

void push2(obj car, obj tag) {
  // default stack frame is (value, ->, NUM_0)
  *alloc++ = car;
  *alloc++ = stack;
  *alloc++ = tag;

  stack = TAG_RIB((rib *)(alloc - RIB_NB_FIELDS));

  if (alloc == alloc_limit) {
    gc();
  }
}

/**
 * Allocate a rib that is not kept on the stack (can be linked
 * from anywhere). The car and cdr can be live references to other
 * ribs.
 * @param car
 * @param cdr
 * @param tag
 * @return
 */
rib *alloc_rib(obj car, obj cdr, obj tag) {
  push2(car, cdr); // tag is set
  obj old_stack = CDR(stack);
  obj allocated = stack;

  CDR(allocated) = TAG(allocated);
  TAG(allocated) = tag;

  stack = old_stack;

  return RIB(allocated);
}



rib *alloc_rib2(obj car, obj cdr, obj tag) {
  push2(car, tag); // tag is set
  obj old_stack = CDR(stack);
  obj allocated = stack;

  CDR(allocated) = cdr;

  stack = old_stack;

  return RIB(allocated);
}

char get_byte() { return input[pos++]; }

num get_code() {
  num x = get_byte() - 35;
  return x < 0 ? 57 : x;
}

num get_int(num n) {
  num x = get_code();
  n *= 46;
  return x < 46 ? n + x : get_int(n + x - 46);
}

rib *list_tail(rib *lst, num i) {
  return (i == 0) ? lst : list_tail(RIB(lst->fields[1]), i - 1);
}

obj list_ref(rib *lst, num i) { return list_tail(lst, i)->fields[0]; }

obj get_opnd(obj o) {
  return (IS_NUM(o) ? list_tail(RIB(stack), NUM(o)) : RIB(o))->fields[0];
}

obj get_cont() {
  obj s = stack;

  while (!NUM(TAG(s))) {
    s = CDR(s);
  }

  return s;
}

#define TRUE (CAR(FALSE))
#define NIL (CDR(FALSE))

#ifdef DEBUG

void chars2str(obj o) {
  if (o != TAG_RIB(&NIL)) {
    printf("%c", (char)(NUM(RIB(o)->fields[0]) % 256));
    chars2str(RIB(o)->fields[1]);
  }
}

void sym2str(rib *c) { chars2str(RIB(c->fields[1])->fields[0]); }

void show_operand(obj o) {
  if (IS_NUM(o)) {
    printf("int %ld", NUM(o));
  } else {
    printf("sym ");
    sym2str(RIB(o));
  }
}

#endif

obj boolean(bool x) { return x ? CAR(FALSE) : FALSE; }

void prim(int no) {
  switch (no) {
  case 0: { // rib
    obj new_rib = TAG_RIB(alloc_rib(NUM_0, NUM_0, NUM_0));
    PRIM3();
    CAR(new_rib) = x;
    CDR(new_rib) = y;
    TAG(new_rib) = z;
    push2(new_rib, PAIR_TAG);
    break;
  }
  case 1: { // id
    PRIM1();
    push2(x, PAIR_TAG);
    break;
  }
  case 2: { // pop
    pop();
    true;
    break;
  }
  case 3: { // skip
    obj x = pop();
    pop();
    push2(x, PAIR_TAG);
    break;
  }
  case 4: { // close
    obj x = CAR(TOS);
    obj y = CDR(stack);
    TOS = TAG_RIB(alloc_rib(x, y, CLOSURE_TAG));
    break;
  }
  case 5: { // is rib?
    PRIM1();
    push2(boolean(IS_RIB(x)), PAIR_TAG);
    break;
  }
  case 6: { // field0
    PRIM1();
    push2(CAR(x), PAIR_TAG);
    break;
  }
  case 7: { // field1
    PRIM1();
    push2(CDR(x), PAIR_TAG);
    break;
  }
  case 8: { // field2
    PRIM1();
    push2(TAG(x), PAIR_TAG);
    break;
  }
  case 9: { // set field0
    PRIM2();
    push2(CAR(x) = y, PAIR_TAG);
    break;
  }
  case 10: { // set field1
    PRIM2();
    push2(CDR(x) = y, PAIR_TAG);
    break;
  }
  case 11: { // set field2
    PRIM2();
    push2(TAG(x) = y, PAIR_TAG);
    break;
  }
  case 12: { // eq
    PRIM2();
    push2(boolean(x == y), PAIR_TAG);
    break;
  }
  case 13: { // lt
    PRIM2();
    push2(boolean(x < y), PAIR_TAG);
    break;
  }
  case 14: { // add
    PRIM2();
    push2(x + y - 1, PAIR_TAG);
    break;
  }
  case 15: { // sub
    PRIM2();
    push2(x - y + 1, PAIR_TAG);
    break;
  }
  case 16: { // mul
    PRIM2();
    push2(TAG_NUM((NUM(x) * NUM(y))), PAIR_TAG);
    break;
  }
  case 17: { // div
    PRIM2();
    push2(TAG_NUM((NUM(x) / NUM(y))), PAIR_TAG);
    break;
  }
  case 18: { // getc
    int read;
    read = getchar();
    if (EOF == read) {
      vm_exit(0);
    }
    push2(TAG_NUM(read), PAIR_TAG);
    break;
  }
  case 19: { // putc
    PRIM1();
    putchar((char)NUM(x));
    fflush(stdout);
    push2(x, PAIR_TAG);
    break;
  }
  default: {
    vm_exit(EXIT_ILLEGAL_INSTR);
  }
  }
}

void run() {
#define ADVANCE_PC()                                                           \
  do {                                                                         \
    pc = TAG(pc);                                                              \
  } while (0)
  while (1) {
    num instr = NUM(CAR(pc));
    switch (instr) {
    default: { // error
      vm_exit(EXIT_ILLEGAL_INSTR);
    }
    case INSTR_HALT: { // halt
      vm_exit(0);
    }
    case INSTR_AP: // call or jump
    {
      bool jump = TAG(pc) == NUM_0;
#ifdef DEBUG_I_CALL
      printf(jump ? "--- jump " : "--- call ");
      PRINTLN();
#endif
#define proc (get_opnd(CDR(pc)))
#define code CAR(proc)
      if (IS_NUM(code)) {
        prim(NUM(code));

        if (jump) {
          // jump
          pc = get_cont();
          CDR(stack) = CAR(pc);
        }
        pc = TAG(pc);
      } else {
        num argc = NUM(CAR(code));
        // Use the car of the PC to save the new PC
        CAR(pc) = CAR(get_opnd(CDR(pc)));

        //        pop();

        obj s2 = TAG_RIB(alloc_rib(NUM_0, proc, PAIR_TAG));

        for (int i = 0; i < argc; ++i) {
          s2 = TAG_RIB(alloc_rib(pop(), s2, PAIR_TAG));
        }

        obj c2 = TAG_RIB(list_tail(RIB(s2), argc));

        if (jump) {
          obj k = get_cont();
          CAR(c2) = CAR(k);
          TAG(c2) = TAG(k);
        } else {
          CAR(c2) = stack;
          TAG(c2) = TAG(pc);
        }

        stack = s2;

        obj new_pc = CAR(pc);
        CAR(pc) = TAG_NUM(instr);
        pc = TAG(new_pc);
      }
      break;
    }
#undef code
#undef proc
    case INSTR_SET: { // set
#ifdef DEBUG_I_CALL
      printf("--- set ");
      PRINTLN();
#endif
      obj x = pop();
      ((IS_NUM(CDR(pc))) ? list_tail(RIB(stack), NUM(CDR(pc))) : RIB(CDR(pc)))
          ->fields[0] = x;
      ADVANCE_PC();
      break;
    }
    case INSTR_GET: { // get
#ifdef DEBUG_I_CALL
      printf("--- get ");
      PRINTLN();
#endif
      push2(get_opnd(CDR(pc)), PAIR_TAG);
      ADVANCE_PC();
      break;
    }
    case INSTR_CONST: { // const
#ifdef DEBUG_I_CALL
      printf("--- const ");
      PRINTLN();
#endif
      push2(CDR(pc), PAIR_TAG);
      ADVANCE_PC();
      break;
    }
    case INSTR_IF: { // if
#ifdef DEBUG_I_CALL
      printf("--- if");
      PRINTLN();
#endif

      obj p = pop();
      if (p != FALSE) {
        pc = CDR(pc);
      } else {
        pc = TAG(pc);
      }
      break;
    }
    }
  }
#undef ADVANCE_PC
}

rib *symbol_ref(num n) { return RIB(list_ref(RIB(symbol_table), n)); }

obj lst_length(obj list) {
  size_t l = 0;

  while (IS_RIB(list) && NUM(TAG(list)) == 0) {
    ++l;
    list = CDR(list);
  }

  return TAG_NUM(l);
}

rib *create_sym(obj name) {
  rib *list = alloc_rib(name, lst_length(name), STRING_TAG);
  rib *sym = alloc_rib(FALSE, TAG_RIB(list), SYMBOL_TAG);
  rib *root = alloc_rib(TAG_RIB(sym), symbol_table, PAIR_TAG);
  return root;
}

void build_sym_table() {
  num n = get_int(0);

  while (n > 0) {
    n--;
    symbol_table = TAG_RIB(create_sym(NIL));
  }

  obj accum = NIL;

  while (1) {
    byte c = get_byte();

    if (c == 44) {
      symbol_table = TAG_RIB(create_sym(accum));
      accum = NIL;
      continue;
    }

    if (c == 59)
      break;

    accum = TAG_RIB(alloc_rib(TAG_NUM(c), TAG_RIB(accum), PAIR_TAG));
  }

  symbol_table = TAG_RIB(create_sym(accum));
}

void set_global(obj c) {
  CAR(CAR(symbol_table)) = c;
  symbol_table = CDR(symbol_table);
}

void decode() {
  int weights[6] = {20, 30, 0, 10, 11, 4};

  obj n;
  int d;
  int op;

  while (1) {
    num x = get_code();
    n = x;
    op = -1;

    while (n > 2 + (d = weights[++op])) {
      n -= d + 3;
    }

    if (x > 90) {
      op = INSTR_IF;
      n = pop();
    } else {
      if (!op) {
        push2(NUM_0, NUM_0);
      }

      if (n >= d) {
        n = (n == d) ? TAG_NUM(get_int(0))
                     : TAG_RIB(symbol_ref(get_int(n - d - 1)));
      } else {
        n = (op < 3) ? TAG_RIB(symbol_ref(n)) : TAG_NUM(n);
      }

      if (op > 4) {
        n = TAG_RIB(
            alloc_rib(TAG_RIB(alloc_rib2(n, NUM_0, pop())), NIL, CLOSURE_TAG));
        if (stack == NUM_0) {
          break;
        }
        op = INSTR_CONST;
      } else if (op > 0) {
        op--;
      } else {
        op = 0;
      }
    }

    rib *c = alloc_rib(TAG_NUM(op), n, 0);
    c->fields[2] = TOS;
    TOS = TAG_RIB(c);
  }

  pc = TAG(CAR(n));
}

void setup_stack() {
  push2(NUM_0, PAIR_TAG);
  push2(NUM_0, PAIR_TAG);

  obj first = CDR(stack);
  CDR(stack) = NUM_0;
  TAG(stack) = first;

  CAR(first) = TAG_NUM(INSTR_HALT);
  CDR(first) = NUM_0;
  TAG(first) = PAIR_TAG;
}


void init() {
  init_heap();

  FALSE = TAG_RIB(alloc_rib(TAG_RIB(alloc_rib(NUM_0, NUM_0, SINGLETON_TAG)),
                            TAG_RIB(alloc_rib(NUM_0, NUM_0, SINGLETON_TAG)),
                            SINGLETON_TAG));

  build_sym_table();
  decode();

  set_global(
      TAG_RIB(alloc_rib(NUM_0, symbol_table, CLOSURE_TAG))); /* primitive 0 */
  set_global(FALSE);
  set_global(TRUE);
  set_global(NIL);

  setup_stack();

  run();
}


int main() {    
    stdio_init_all();   
    init();
}




#if 0
// #pragma clang diagnostic push
// #pragma clang diagnostic ignored "-Wunknown-pragmas"
// #pragma ide diagnostic ignored "ConstantFunctionResult"

// basic def. of a boolean







#ifndef NOSTART

int main() { init(); }

#endif

#pragma clang diagnostic pop
#endif
