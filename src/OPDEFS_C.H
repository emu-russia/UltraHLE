// Filename  : opdefs.h
// File Desc : Opcode Definitions

// Opcode Definitions

#define  ADD         32  // Add
#define  ADDfmt       0  // Floating Point Add
#define  ADDI         8  // Add Immediate
#define  ADDIU        9  // Add Immediate Unsigned
#define  ADDU        33  // Add Unsigned
#define  AND         36  // And
#define  ANDI        12  // And Immediate
#define  BC           8  // Branch on Coprocessor (COPx Sub OpCode)
#define  BCF          0  // Branch on Coprocessor False
#define  BCFL         2  // Branch on Coprocessor False Likely
#define  BCT          1  // Branch on Coprocessor True
#define  BCTL         3  // Branch on Coprocessor True Likely
#define  BEQ          4  // Branch on Equal
#define  BEQL        20  // Branch on Equal Likely
#define  BGEZ         1  // Branch on Greater than or Equal to Zero
#define  BGEZAL      17  // Branch on Greater than or Equal to Zero and Link
#define  BGEZALL     19  // Branch on Greater than or Equal to 0 and Link Likely
#define  BGEZL        3  // Branch on Greater than or Equal to Zero Likely
#define  BGTZ         7  // Branch on Greater than Zero
#define  BGTZL       23  // Branch on Greater than Zero Likely
#define  BLEZ         6  // Branch on Less than or Equal to Zero 
#define  BLEZL       22  // Branch on Less than or Equal to Zero Likely
#define  BLTZ         0  // Branch on Less than Zero
#define  BLTZAL      16  // Branch on Less than Zero and Link
#define  BLTZALL     18  // Branch on Less than Zero and Link Likely
#define  BLTZL        2  // Branch on Less than Zero Likely
#define  BNE          5  // Branch on Not Equal
#define  BNEL        21  // Branch on Not Equal Likely
#define  BREAK       13  // Breakpoint
#define  CACHE       47  // Cache Operation
#define  CF           2  // Move Control from Coprocessor (COPx Sub OpCode)
#define  COP0        16  // System Control Coprocessor Instructions
#define  COP1        17  // Floating Point Coprocessor Instructions
#define  COP2        18  // Reality Coprocessor Instructions
#define  CT           6  // Move Control to Coprocessor (COPx Sub OpCode)
#define  CVTDfmt     33  // Floating Point Convert to Double Float Point Format
#define  CVTSfmt     32  // Floating Point Convert to Single Float Point Format
#define  DADD        44  // Doubleword Add
#define  DADDI       24  // Doubleword Add Immediate
#define  DADDIU      25  // Doubleword Add Immediate Unsigned
#define  DADDU       45  // Doubleword Add Unsigned
#define  DDIV        30  // Doubleword Divide
#define  DDIVU       31  // Doubleword Divide Unsigned
#define  DIV         26  // Divide
#define  DIVfmt       3  // Floating Point Divide
#define  DIVU        27  // Divide Unsigned
#define  DMF          1  // Doubleword Move From Coprocessor (COP0 Sub OpCode)
#define  DMULT       28  // Doubleword Multiply
#define  DMULTU      29  // Doubleword Multiply Unsigned
#define  DSLL        56  // Doubleword Shift Left Logical
#define  DSLLV       20  // Doubleword Shift Left Logical Variable
#define  DSLL32      60  // Doubleword Shift Left Logical + 32
#define  DSRA        59  // Doubleword Shift Right Arithmetic
#define  DSRAV       23  // Doubleword Shift Right Arithmetic Variable
#define  DSRA32      63  // Doubleword Shift Right Arithmetic + 32
#define  DSRL        58  // Doubleword Shift Right Logical
#define  DSRLV       22  // Doubleword Shift Right Logical Variable
#define  DSRL32      62  // Doubleword Shift Right Logical + 32
#define  DSUB        46  // Doubleword Subtract
#define  DSUBU       47  // Doubleword Subtract Unsigned
#define  ERET        24  // Exception Return
#define  J            2  // Jump
#define  JAL          3  // Jump and Link
#define  JALR         9  // Jump and Link Register
#define  JR           8  // Jump Register
#define  LB          32  // Load Byte
#define  LBU         36  // Load Byte Unsigned
#define  LD          55  // Load Doubleword
#define  LDC1        53  // Load Doubleword to Coprocessor 1 (COP1)
#define  LDC2        54  // Load Doubleword to Coprocessor 2 (COP2)
#define  LDL         26  // Load Doubleword Left
#define  LDR         27  // Load Doubleword Right
#define  LH          33  // Load Halfword
#define  LHU         37  // Load Halfword Unsigned
#define  LL          48  // Load Linked
#define  LLD         52  // Load Linked Doubleword
#define  LUI         15  // Load Upper Immediate
#define  LW          35  // Load Word
#define  LWC1        49  // Load Word to Coprocessor 1 (COP1)
#define  LWC2        50  // Load Word to Coprocessor 2 (COP2)
#define  LWL         34  // Load Word Left
#define  LWR         38  // Load Word Right
#define  LWU         39  // Load Word Unsigned
#define  MF           0  // Move From Coprocessor (COPx Sub OpCode)
#define  MFHI        16  // Move from HI
#define  MFLO        18  // Move from LO
#define  MT           4  // Move to Coprocessor (COPx Sub OpCode)
#define  MTHI        17  // Move to HI
#define  MTLO        19  // Move to LO
#define  MULfmt       2  // Floating Point Multiply
#define  MULT        24  // Multiply
#define  MULTU       25  // Multiply Unsigned
#define  NOR         39  // Nor
#define  OR          37  // Or
#define  ORI         13  // Or Immediate
#define  REGIMM       1  // Register Immediate Instruction
#define  SB          40  // Store Byte
#define  SC          56  // Store Conditional
#define  SCD         60  // Store Conditional Doubleword
#define  SD          63  // Store Doubleword
#define  SDC1        61  // Store Doubleword from Coprocessor 1 (COP1)
#define  SDC2        62  // Store Doubleword from Coprocessor 2 (COP2)
#define  SDL         44  // Store Doubleword Left
#define  SDR         45  // Store Doubleword Right
#define  SH          41  // Store Halfword
#define  SLL          0  // Shift Left Logical
#define  SLLV         4  // Shift Left Logical Variable
#define  SLT         42  // Set on Less Than
#define  SLTI        10  // Set on Less than Immediate
#define  SLTIU       11  // Set on Less than Immediate Unsigned
#define  SLTU        43  // Set on Less than Unsigned
#define  SPECIAL      0  // Special Instructions
#define  SRA          3  // Shift Right Arithmetic
#define  SRAV         7  // Shift Right Arithmetic Variable
#define  SRL          2  // Shift Right Logical
#define  SRLV         6  // Shift Right Logical Variable
#define  SUB         34  // Subtract
#define  SUBU        35  // Subtract Unsigned
#define  SW          43  // Store Word
#define  SWC1        57  // Store Word from Coprocessor 1 (COP1)
#define  SWC2        58  // Store Word from Coprocessor 2 (COP2)
#define  SWL         42  // Store Word Left
#define  SWR         46  // Store Word Right
#define  SYNC        15  // Synchronise
#define  SYSCALL     12  // System Call
#define  TEQ         52  // Trap If Equal
#define  TEQI        12  // Trap If Equal Immediate
#define  TGE         48  // Trap If Greater Than or Equal
#define  TGEI         8  // Trap If Greater Than or Equal Immediate
#define  TGEIU        9  // Trap If Greater Than or Equal Immediate Unsigned
#define  TGEU        49  // Trap If Greater Than or Equal Unsigned
#define  TLBR         1  // Read Indexed TLB Entry
#define  TLBWI        2  // Write Indexed TLB Entry
#define  TLBWR        6  // Write Random TLB Entry
#define  TLBP         8  // Probe TLB for Matching Entry
#define  TLT         50  // Trap If Less Than
#define  TLTI        10  // Trap If Less Than Immediate
#define  TLTIU       11  // Trap If Less Than Immediate Unsigned
#define  TLTU        51  // Trap If Less Than Unsigned
#define  TNE         54  // Trap If Not Equal
#define  TNEI        13  // Trap If Not Equal Immediate
#define  TRUNCWfmt   13  // Floating Point Truncate to Single Fixed Point Format
#define  XOR         38  // Exclusive Or
#define  XORI        14  // Exclusive Or Immediate

// Floating Point Format Definitions

#define  FMT_S       16  // Size: Single, Format: Binary Floating Point
#define  FMT_D       17  // Size: Double, Format: Binary Floating Point
#define  FMT_W       20  // Size: Single, Format: 32-Bit Binary Fixed Point
#define  FMT_L       32  // Size: LongWord, Format: 64-Bit Binary Fixed Point
