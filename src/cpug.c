#include "ultra.h"
#include "opdefs_c.h"

#define GETREGS \
        rs=&st.g[OP_RS(opcode)].d; \
        rt=&st.g[OP_RT(opcode)].d; \
        rd=&st.g[OP_RD(opcode)].d;

#define GETREGSIMM \
        rs=&st.g[OP_RS(opcode)].d; \
        rd=&st.g[OP_RT(opcode)].d; \
        imm=SIGNEXT16(OP_IMM(opcode)); \
        rt=&imm;

#define GETREGSBR \
        rs=&st.g[OP_RS(opcode)].d; \
        rt=&st.g[OP_RT(opcode)].d;

#define GETREGSIMMUNS \
        rs=&st.g[OP_RS(opcode)].d; \
        rd=&st.g[OP_RT(opcode)].d; \
        imm=OP_IMM(opcode); \
        rt=&imm;

   // DMULTU Opcode
   
   static void op_dmultu( int reg1, int reg2 )
   {
      qreg a, b;

      a = st.g[reg1];
      b = st.g[reg2];

      st.mlo.q = (unsigned __int64)a.q * (unsigned __int64)b.q;
      st.mhi.q = 0;
   }

   // DDIVU Opcode

   static void op_ddivu( int reg1, int reg2 )
   {
      qreg a, b;

      a = st.g[reg1];
      b = st.g[reg2];

      if( !b.q )
      {
         st.mlo.q = 0;
         st.mhi.q = 0;
      }
      else
      {
         st.mlo.q = (unsigned __int64)a.q / (unsigned __int64)b.q;
         st.mhi.q = (unsigned __int64)a.q % (unsigned __int64)b.q;
      }
   }

   // DDIV Opcode

   static void op_ddiv( int reg1, int reg2 )
   {
      qreg a, b;

      a = st.g[reg1];
      b = st.g[reg2];
    
      if( !b.q )
      {
         st.mlo.q = 0;
         st.mhi.q = 0;
      }
      else
      {
         st.mlo.q = (__int64)a.q / (__int64)b.q;
         st.mhi.q = (__int64)a.q % (__int64)b.q;
      }
   }

   // Get Opcode at Memory Address

   __inline dword op_memaddr( dword opcode )
   {
      dword a;

      a = OP_IMM(opcode);
      a = SIGNEXT16(a);
      a += st.g[OP_RS(opcode)].d;
    
      return(a);
   }

   // Read Memory

   static void op_readmem( dword opcode, int bytes )
   {
      int a, *d;

      a = op_memaddr(opcode);

      if( bytes > 0x10 )
      { // FPU
         d = &st.f[OP_RT(opcode)].d;
         bytes -= 0x10;
      }
      else
      {
        d = &st.g[OP_RT(opcode)].d;
      }

      cpu_notify_readmem( a, bytes );

      switch( bytes )
      {
         case -1:

            x = mem_read8(a);
            d[0] = SIGNEXT8(x);
        
            break;
    
         case 1:
        
            x = mem_read8(a);
            d[0] = x;
        
            break;
    
         case -2:
        
            x = mem_read16(a);
            d[0] = SIGNEXT16(x);
        
            break;
    
         case 2:
        
            x = mem_read16(a);
            d[0] = x;
        
            break;
    
         case -4:
         case 4:
        
            x = mem_read32(a);
            d[0] = x;
        
            break;
    
         case 8:
         case -8:
        
            d[1] = mem_read32(a);
            d[0] = mem_read32(a+4);
        
            break;
      }
   }

   // Write Memory

   static void op_writemem( dword opcode, int bytes )
   {
      int a, x, *d;
    
      a = op_memaddr(opcode);
    
      if( bytes > 0x10 )
      { // FPU
         d = &st.f[OP_RT(opcode)].d;
         bytes -= 0x10;
      }
      else
      {
         d = &st.g[OP_RT(opcode)].d;
      }

      switch( bytes )
      {
         case 1:
        
            mem_write8( a, d[0] );
        
            break;
    
         case 2:
        
            mem_write16( a, d[0] );
        
            break;
    
         case 4:
        
            mem_write32( a, d[0] );
        
            break;
    
         case 8:
        
            mem_write32( a, d[1] );
            mem_write32( a+4, d[0] );
        
            break;
      }

      cpu_notify_writemem( a, bytes );
   }

   // Read/Write Memory Left and Right

   static void op_rwmemrl( dword opcode, int write, int right )
   {
      dword x, y, a, s, m;

      a = op_memaddr(opcode);
      s = a & 3;
      a &= ~3;
    
      if( write )
      {
         x = mem_read32(a);
         y = st.g[OP_RT(opcode)].d;
        
         if( right )
         {
            m = 0x00ffffff >> (s * 8);
            y <<= (3 - s) * 8;
            x &= m;
            x |= y;
         }
         else
         {
            m = 0xffffff00 << ((3 - s) * 8);
            y >>= s * 8;
            x &= m;
            x |= y;
         }
        
         mem_write32( a, x );
      }
      else
      {
         x = st.g[OP_RT(opcode)].d;
         y = mem_read32(a);
        
         if( right )
         {
            m = 0xffffff00 << (s * 8);
            y >>= (3 - s) * 8;
            x &= m;
            x |= y;
         }
         else
         {
            m = 0x00ffffff >> ((3 - s) * 8);
            y <<= s * 8;
            x &= m;
            x |= y;
         }
        
         st.g[OP_RT(opcode)].d = x;
      }
   }

   // MULT Opcode

   static void op_mult( int a, int b )
   {
      int lo, hi;
    
      _asm {
         mov  eax,a
         mov  edx,b
         imul edx
         mov  lo,eax
         mov  hi,edx
      }
    
      st.mlo.d = lo;
      st.mhi.d = hi;
   }

   // MULTU Opcode

   static void op_multu( int a, int b )
   {
      int lo, hi;
    
      _asm {
         mov  eax,a
         mov  edx,b
         mul  edx
         mov  lo,eax
         mov  hi,edx
      }
    
      st.mlo.d = lo;
      st.mhi.d = hi;
   }

   // DIV Opcode

   static void op_div( int a, int b )
   {
      int lo, hi;
    
      if( !b )
      {
         error( "RM - Divide by Zero" );
         st.mlo.d = 0;
         st.mhi.d = 0;
         return;
      } 
    
      _asm {
         mov  eax,a
         cdq
         mov  ecx,b
         idiv ecx
         mov  lo,eax
         mov  hi,edx
      }
    
      st.mlo.d = lo;
      st.mhi.d = hi;
   }

   // DIVU Opcode

   static void op_divu( int a, int b )
   {
      int lo, hi;
    
      if( !b )
      {
         error( "RM - Divide by Zero" );
         st.mlo.d = 0;
         st.mhi.d = 0;
         return;
      }

      _asm {
         mov  eax,a
         xor  edx,edx
         mov  ecx,b
         div  ecx
         mov  lo,eax
         mov  hi,edx
      }
    
      st.mlo.d = lo;
      st.mhi.d = hi;
   }

   // Execution Jump

   static void op_jump( dword opcode, int link, int reg )
   {
      int to;

      st.branchtype = BRANCH_NORMAL;

      if( reg == -1 )
      {
         to = ((OP_TAR(opcode) << 2) & 0x0fffffff) |
              (st.pc & 0xf0000000);
      }
      else
      {
         to = st.g[reg].d;
         if( reg == 31 ) 
            st.branchtype = BRANCH_RET;
      }

      if( link )
      { // Link
         st.branchtype = BRANCH_CALL;
         st.g[31].d = st.pc + 8;
      }

      st.branchdelay = 2;
      st.branchto = to;
   }

   // Execution Branch

   static void op_branch( dword opcode, int doit, int likely, int link )
   {
      if( doit )
      {
         int imm = SIGNEXT16(OP_IMM(opcode));
        
         if( link )
         { // Link
            st.g[31].d = st.pc + 8;
            st.branchtype = BRANCH_CALL;
         }
         else 
            st.branchtype = BRANCH_NORMAL;
        
         imm <<= 2;
         imm += st.pc + 4;
         st.branchdelay = 2;
         st.branchto = imm;
      }
      else
      {
         if( likely )
         {
            st.pc += 4; // Likely, Skip Delay Slot Instruction
         }
      }
   }

   // Read Double
   
   double readdouble( int reg )
   {
      dword x[2];
      
      x[0] = st.f[reg+0].d;
      x[1] = st.f[reg+1].d;
    
      return( *(double *)x );
   }

   // Write Double

   void writedouble( int reg, double value )
   {
      dword x[2];
      *(double *)x = value;
      st.f[reg+0].d = x[0];
      st.f[reg+1].d = x[1];
   }

   // FPU Opcode Execution

   static void op_fpu( dword opcode )
   {
      int fmt = OP_RS(opcode);
      int op = OP_FUNC(opcode);

      // 0x00-0x3f = basic ops
      // 0x40-0x7F = BC ops
      // 0x80-0x8F = BC ops
    
      if( fmt < 8 )
      { // Move Ops
        int rt, fs;

        rt = OP_RT(opcode);
        fs = OP_RD(opcode);
        
        switch( fmt )
        {
            case 0: // MFC1
            
               st.g[rt].d = st.f[fs].d;
            
               break;
        
            case 4: // MTC1
            
               st.f[fs].d = st.g[rt].d;
            
               break;
        
            case 2:
            case 6:
               
               // Ignore CFC0 CTC0
            
               break;
        
            default:
            
               error( "RM - Unimplemented FPU-Move Opcode" );
            
               break;
         }
      }
      else 
         if( fmt == 8 )
         { // BC Ops (Branch)
            int rt = OP_RT(opcode);
            int ontrue = rt & 1;
            int likely = (rt & 2) >> 1;
            int flag;

            if( ontrue ) 
               flag = st.fputrue;
            else       
               flag = !st.fputrue;

            op_branch( opcode, flag, likely, 0 );
         }
         else
         { // Generic Ops
            double  r, a, b;
            int     storer = 1;
         
            if( fmt == FMT_S )
            { 
               a = (double)st.f[OP_RD(opcode)].f;
               b = (double)st.f[OP_RT(opcode)].f;
            }
            else 
               if( fmt == FMT_D )
               { 
                  a = readdouble(OP_RD(opcode));
                  b = readdouble(OP_RT(opcode));
               }
               else
               {
                  if( op != 33 && op != 32) 
                     op = 255; // Force Error
               }
        
            switch( op )
            {
               case 0: // ADD
            
                  r = a + b;
               
                  break;
        
               case 1: // SUB
            
                  r = a - b;
            
                  break;
        
               case 2: // MUL
            
                  r = a * b;
            
                  break;
        
               case 3: // DIV
            
                  r = a / b;
            
                  break;
        
               case 4: // SQRT
            
                  r = sqrt(a);
            
                  break;
        
               case 5: // ABS [not used]
            
                  r = fabs(a);
            
                  break;
        
               case 7: // NEG
            
                  r = -a;
            
                  break;
        
               case 48: // C.F
               case 49: // C.UN
               case 56: // C.SF
               case 57: // C.NGLE
            
                  storer = 0;
                  st.fputrue = 0;
            
                  break;
        
               case 50: // C.EQ   [actually used]
               case 51: // C.UEQ
               case 58: // C.SEQ
               case 59: // C.NGL
            
                  storer = 0;
                  st.fputrue = (a == b);
            
                  break;
        
               case 60: // C.LT   [actually used]
               case 52: // C.OLT
               case 53: // C.ULT
               case 61: // C.NGE
            
                  storer = 0;
                  st.fputrue = (a < b);
            
                  break;
        
               case 62: // C.LE   [actually used]
               case 54: // C.OLE
               case 55: // C.ULE
               case 63: // C.NGT
            
                  storer = 0;
                  st.fputrue = (a <= b);
            
                  break;
        
               // Convert & Move
        
               case 6: // MOV
            
                  storer = 0;
                  if( fmt == FMT_D || fmt == 21 )
                  { // qword
                     st.f[OP_SHAMT(opcode)+0].d = st.f[OP_RD(opcode)+0].d;
                     st.f[OP_SHAMT(opcode)+1].d = st.f[OP_RD(opcode)+1].d;
                  }
                  else
                  { // dword
                     st.f[OP_SHAMT(opcode)].d = st.f[OP_RD(opcode)].d;
                  }
               
                  break;

               case 12: // ROUND.W [not used]
            
                  storer = 0;
                  st.f[OP_SHAMT(opcode)].d = (int)(a);
            
                  break;
        
               case 13: // TRUN.W [used]
            
                  storer = 0;
                  st.f[OP_SHAMT(opcode)].d = (int)(a);
            
                  break;
        
               case 14: // CEIL.W [not used]
            
                  storer = 0;
                  st.f[OP_SHAMT(opcode)].d = ceil(a);
            
                  break;
        
               case 15: // FLOOR.W [not used]
            
                  storer = 0;
                  st.f[OP_SHAMT(opcode)].d = floor(a);
            
                  break;
   
               case 32: // CVT.S [used]
            
                  storer = 0;
                  if( fmt == 20 )      
                     st.f[OP_SHAMT(opcode)].f = st.f[OP_RD(opcode)].d;
                  else 
                     if( fmt == 17 ) 
                        st.f[OP_SHAMT(opcode)].f = readdouble(OP_RD(opcode));
                     else
                     {
                        error( "RM - Unimplemented FPU-CVT-Opcode" );
                     }
                  
                  break;
        
               case 33: // CVT.D [used]
            
                  storer=0;
                  if( fmt == 20 )      
                     writedouble(OP_SHAMT(opcode), st.f[OP_RD(opcode)].d);
                  else 
                     if( fmt == 16 ) 
                        writedouble(OP_SHAMT(opcode), st.f[OP_RD(opcode)].f);
                     else
                     {
                        error( "RM - Unimplemented FPU-CVT-Opcode" );
                     }
            
                  break;
        
               case 36: // CVT.W
            
                  storer = 0;
                  if( fmt == 17 )      
                     st.f[OP_SHAMT(opcode)].d = a;
                  else 
                     if( fmt == 16 ) 
                        st.f[OP_SHAMT(opcode)].d = (int)a;
                     else
                     {
                        error( "RM - Unimplemented FPU-CVT-Opcode" );
                     }
            
                  break;
        
               default:
            
                  storer = 0;
                  error( "RM - Unimplemented FPU-Opcode" );
            
                  break;
            }

            if( storer )
            {
               if( fmt == 16 )
               { // Single
                  st.f[OP_SHAMT(opcode)].f = (float)r;
               }
               else 
                  if( fmt == 17 )
                  { // Double
                     writedouble( OP_SHAMT(opcode), r );
                  }
            }
      }
   }


   // System Control Coprocessor Opcode Execution

   static void op_scc( dword opcode )
   {
      switch( OP_RS(opcode) )
      {
         case MF:
         case DMF:

            st.g[OP_RT(opcode)].d = st.mmu[OP_RD(opcode)].d;

            break;

         case MT:

            // If write to the Compare Register, Clear the Timer Interrupt
            // in the Cause Register

            if( OP_RD(opcode) == 11 )
               st.mmu[13].d &= ~0x00008000;


            // If write to the Cause Register, mask out all bits except
            // IP0 & IP1

            if( OP_RD(opcode) == 13 )
            {
               st.mmu[13].d = st.g[OP_RT(opcode)].d & 0x00000300;
               return;
            }

            st.mmu[OP_RD(opcode)].d = st.g[OP_RT(opcode)].d;

            // Later Implementation:
            // If Status Register written to and that write included any
            // interrupt bits then check for CPU Interrupts.

            break;

         default:

            error( "RM - Unimplemented COP0-MMU Opcode" );
            print( "RM - Opcode (0x%08X)\n", opcode );
            print( "RM - rs (0x%02X)\n", OP_RS(opcode) );

            break;
      }
   }

   // Execute Main Processor Opcodes

   static void op_main( dword opcode )
   {
      int op,flag;
      int *rs,*rt,*rd,imm;

      op = OP_OP(opcode);
      if( op == 0 ) 
         op = OP_FUNC(opcode) + 0x40;
      else
         if( op == 1) 
            op = OP_RT(opcode) + 0x80;

      switch( op )
      {
         case COP0:

            op_scc( opcode );

            break;

         // ********** Ignored Coprocessor Stuff ********** 

         case COP2:

            error( "RM - Unimplemented COP2 Opcode" );

            break;
    
         // ********** Special Operations ********** 

         case OP_PATCH: // *PATCH*

            op_patch( OP_IMM(opcode) );
            st.bailout -= 100; // Approximate Every O/S Routine takes 100 Cycles

            break;

         case OP_GROUP: // *GROUP*

            error( "RM - Reserved Group-Opcode Encountered in CPUC" );

            break;

         // ********** Loads ********** 

         case LB:

            op_readmem( opcode, -1 );

            break;

         case LBU:

            op_readmem( opcode, 1 );
        
            break;

         case LH:

            op_readmem( opcode, -2 );
         
            break;

         case LHU:

            op_readmem( opcode, 2 );
        
            break;

         case LW:
         case LWU:

            op_readmem( opcode, 4 );
        
            break;

         case LWL:

            op_rwmemrl( opcode, 0, 0 );

            break;

         case LWR:
        
            op_rwmemrl( opcode, 0, 1 );

            break;

         case LD:
         case LDL:
         case LDR:

            //print( "RM - Doubleword Load at (%08X)\n", st.pc );
            op_readmem( opcode, 8 );
        
            break;
    
         // ********** Stores **********

         case SB:
        
            op_writemem( opcode, 1 );
        
            break;

         case SH:
        
            op_writemem( opcode, 2 );
        
            break;
    
         case SW:
        
            op_writemem( opcode, 4 );
        
            break;
    
         case SWL:
        
            op_rwmemrl( opcode, 1, 0 );
        
            break;
    
         case SWR:
        
            op_rwmemrl( opcode, 1, 1 );
        
            break;

         case SD: 
         case SDL:
         case SDR:

            //print( "RM - Doubleword Store at (%08X)\n", st.pc );
            op_writemem( opcode, 8 );
        
            break;

         // ********** Arithmetic **********

         case ADDI:
         case ADDIU:
        
            GETREGSIMM;
            *rd = *rs + *rt;
        
            break;

         case SLTI:
        
            GETREGSIMM;
            *rd = ( (int)*rs < (int)*rt );
        
            break;
    
         case SLTIU:
        
            GETREGSIMM;
            *rd = ( (unsigned)*rs < (unsigned)*rt );
        
            break;
    
         case ANDI:
        
            GETREGSIMMUNS;
            *rd = *rs & *rt;
        
            break;
    
         case ORI:
        
            GETREGSIMMUNS;
            *rd = *rs | *rt;
        
            break;
    
         case XORI:
        
            GETREGSIMMUNS;
            *rd = *rs ^ *rt;
        
            break;
    
         case LUI:
        
            GETREGSIMM;
            *rd = *rt << 16;
        
            break;
    
         case 0x40+SLL:
        
            GETREGS;
            *rd = *rt << OP_SHAMT(opcode);
        
            break;

         case 0x40+SRL:
        
            GETREGS;
            *rd = (unsigned)*rt >> OP_SHAMT(opcode);
        
            break;

         case 0x40+SRA:
        
            GETREGS;
            *rd = (int)*rt >> OP_SHAMT(opcode);
        
            break;
    
         case 0x40+SLLV:
        
            GETREGS;
            *rd = *rt << *rs;
        
            break;
    
         case 0x40+SRLV:
        
            GETREGS;
            *rd = (unsigned)*rt >> *rs;
        
            break;
    
         case 0x40+SRAV:
        
            GETREGS;
            *rd = (int)*rt >> *rs;
        
            break;
    
         case 0x40+SYSCALL:
        
            exception( "RM - Opcode Syscall" );
        
            break;
    
         case 0x40+BREAK:
        
            exception( "RM - Opcode Break" );
        
            break;

         case 0x40+MFHI:
        
            GETREGS;
            rd[0] = st.mhi.d2[0];
            rd[1] = st.mhi.d2[1];
        
            break;
    
         case 0x40+MTHI:
        
            GETREGS;
            st.mhi.d2[0] = rs[0];
            st.mhi.d2[1] = rs[1];
        
            break;
    
         case 0x40+MFLO:
        
            GETREGS;
            rd[0] = st.mlo.d2[0];
            rd[1] = st.mlo.d2[1];
        
            break;
    
         case 0x40+MTLO:
        
            GETREGS;
            st.mlo.d2[0] = rs[0];
            st.mlo.d2[1] = rs[1];
        
            break;
    
         case 0x40+MULT:
        
            GETREGS;
            op_mult( *rs, *rt );
        
            break;
    
         case 0x40+MULTU:
        
            GETREGS;
            op_multu( *rs, *rt );
        
            break;
    
         case 0x40+DIV:
        
            GETREGS;
            op_div( *rs, *rt );
        
            break;
    
         case 0x40+DIVU:
        
            GETREGS;
            op_divu( *rs, *rt );
        
            break;
    
         case 0x40+DMULT:
        
            op_dmultu( OP_RS(opcode), OP_RT(opcode) );
            logi( "RM - Doubleworld DMULT at (%08X)\n", st.pc );
        
            break;
    
         case 0x40+DMULTU:
        
            op_dmultu( OP_RS(opcode), OP_RT(opcode) );
            logi( "RM - Doubleworld DMULTU at (%08X)\n", st.pc );
        
            break;
    
         case 0x40+DDIV:
        
            op_ddiv( OP_RS(opcode), OP_RT(opcode) );
            logi( "RM - Doubleworld DDIV at (%08X)\n", st.pc );
        
            break;
    
         case 0x40+DDIVU:
        
            op_ddivu( OP_RS(opcode), OP_RT(opcode) );
            logi( "RM - Doubleworld DDIVU at (%08X)\n", st.pc );
        
            break;
    
         case 0x40+ADD:
         case 0x40+ADDU:
        
            GETREGS;
            *rd = *rs + *rt;
        
            break;
    
         case 0x40+SUB:
         case 0x40+SUBU:
        
            GETREGS;
            *rd = *rs - *rt;
        
            break;
    
         case 0x40+AND:
        
            GETREGS;
            *rd = *rs & *rt;
        
            break;
    
         case 0x40+OR:
        
            GETREGS;
            *rd = *rs | *rt;
        
            break;
         
         case 0x40+XOR:
        
            GETREGS;
            *rd = *rs ^ *rt;
        
            break;
    
         case 0x40+NOR:
        
            GETREGS;
            *rd = ~( *rs | *rt );
        
            break;
    
         case 0x40+SLT:
        
            GETREGS;
            *rd = ( (int)*rs < (int)*rt );
        
            break;
    
         case 0x40+SLTU:
        
            GETREGS;
            *rd = ( (unsigned)*rs < (unsigned)*rt );
        
            break;
    
         // ********** Jump and Branch **********
         
         case J:
        
            op_jump( opcode, 0, -1 );
        
            break;
    
         case JAL:
        
            op_jump( opcode, 1, -1 );
        
            break;
    
         case BEQ:
        
            GETREGSBR;
            flag = *rs == *rt;
            op_branch( opcode, flag, 0, 0 );
        
            break;
    
         case BNE:
        
            GETREGSBR;
            flag = *rs != *rt;
            op_branch( opcode, flag, 0, 0 );
        
            break;
    
         case BLEZ:
        
            GETREGSBR;
            flag = (int)*rs <= 0;
            op_branch( opcode, flag, 0, 0 );
        
            break;
    
         case BGTZ:
        
            GETREGSBR;
            flag = (int)*rs > 0;
            op_branch( opcode, flag, 0, 0 );
        
            break;
    
         case BEQL:
        
            GETREGSBR;
            flag = *rs == *rt;
            op_branch( opcode, flag, 1, 0 );
        
            break;
    
         case BNEL:
        
            GETREGSBR;
            flag = *rs != *rt;
            op_branch( opcode, flag, 1, 0 );
        
            break;
    
         case BLEZL:
        
            GETREGSBR;
            flag = (int)*rs <= 0;
            op_branch( opcode, flag, 1, 0 );
        
            break;
    
         case BGTZL:
        
            GETREGSBR;
            flag = (int)*rs > 0;
            op_branch( opcode, flag, 1, 0 );
        
            break;
    
         case 0x40+JR:
        
            op_jump( opcode, 0, OP_RS(opcode) );
        
            break;
    
         case 0x40+JALR:
        
            op_jump( opcode, 1, OP_RS(opcode) );
        
            break;
    
         case 0x80+BLTZ:
        
            GETREGSBR;
            flag = (int)*rs < 0;
            op_branch( opcode, flag, 0, 0 );
        
            break;
    
         case 0x80+BGEZ:
        
            GETREGSBR;
            flag = (int)*rs >= 0;
            op_branch( opcode, flag, 0, 0 );
        
            break;
    
         case 0x80+BLTZL:
        
            GETREGSBR;
            flag = (int)*rs < 0;
            op_branch( opcode, flag, 1, 0 );
        
            break;
    
         case 0x80+BGEZL:
        
            GETREGSBR;
            flag = (int)*rs >= 0;
            op_branch( opcode, flag, 1, 0 );
        
            break;
    
         case 0x80+BLTZAL:
        
            GETREGSBR;
            flag = (int)*rs < 0;
            op_branch( opcode, flag, 0, 1 );
         
            break;
    
         case 0x80+BGEZAL:
        
            GETREGSBR;
            flag = (int)*rs >= 0;
            op_branch( opcode, flag, 0, 1 );
        
            break;
    
         case 0x80+BLTZALL:
        
            GETREGSBR;
            flag = (int)*rs < 0;
            op_branch( opcode, flag, 1, 1 );
        
            break;
    
         case 0x80+BGEZALL:
        
            GETREGSBR;
            flag = (int)*rs >= 0;
            op_branch( opcode, flag, 1, 1 );
        
            break;
    
         case CACHE:
        
            break;

         // ********** Floating-Point Unit **********
         
         case COP1:
        
            op_fpu( opcode );
        
            break;

         case LDC1:
        
            op_readmem( opcode, 0x18 );
        
            break;
    
         case SDC1:
        
            op_writemem( opcode, 0x18 );
        
            break;
    
         case LWC1:
        
            op_readmem( opcode, 0x14 );
        
            break;
    
         case SWC1:
        
            op_writemem( opcode, 0x14 );
        
            break;
    
         // ********** Nothing I Recognise **********
         
         default:
        
            error( "RM - Unimplemented CPU Opcode" );

            break;
      }
   }

   // Call 'C' Opcode Execution

   void c_execop( dword opcode )
   {
      op_main( opcode );

      st.pc += 4;

      if( st.branchdelay > 0 )   
      {
         if( !--st.branchdelay )
         {
            cpu_notify_branch( st.branchto, st.branchtype );
            st.pc = st.branchto;
         }
      }
   }

   // Call 'C' MIPS Emulation Execution

   void c_exec( void )
   {
      while( st.bailout > 0 )
      {
         c_execop( mem_readop( st.pc ) );
         st.bailout--;
         cpu_notify_pc( st.pc );
      }
   }

   
