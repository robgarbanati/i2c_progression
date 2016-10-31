/*****************************************************************************
*
* @file LuaVM.c
*
* This file implements a small virtual machine
* based on the Lua VM.
*
* Copyright (c) 2007, 2008 Brandon Blodget
* Copyright (c) 2013 OLogic Inc.
* All rights reserved.
*
* Author: Brandon Blodget
*
*****************************************************************************/

#include "global.h"
#include "xprintf.h"
#include "lua_vm.h"
#include "lua_lib.h"
#include "lua_tables.h"

//
// Local defines and types specific to the virtual machine.
//

// Basic instruction format.
typedef enum
{
	iABC,
	iABx,
	iAsBx
} OpMode;

typedef enum
{
	//----------------------------------------------------------------------
	// name							args	description
	//------------------------------------------------------------------------
	OP_MOVE,				//	A B		R(A) := R(B)
	OP_LOADK,				//	A Bx	R(A) := Kst(Bx)
	OP_LOADBOOL,		//	A B C	R(A) := (Bool)B; if (C) pc++
	OP_LOADNIL,			//	A B		R(A) := ... := R(B) := nil
	OP_GETUPVAL,		//	A B		R(A) := UpValue[B]
	OP_GETGLOBAL,		//	A Bx	R(A) := Gbl[Kst(Bx)]
	OP_GETTABLE,		//	A B C	R(A) := R(B)[RK(C)]
	OP_SETGLOBAL,		//	A Bx	Gbl[Kst(Bx)] := R(A)
	OP_SETUPVAL,		//	A B		UpValue[B] := R(A)
	OP_SETTABLE,		//	A B C	R(A)[RK(B)] := RK(C)
	OP_NEWTABLE,		//	A B C	R(A) := {} (size = B,C)
	OP_SELF,				//	A B C	R(A+1) := R(B); R(A) := R(B)[RK(C)]
	OP_ADD,					//	A B C	R(A) := RK(B) + RK(C)
	OP_SUB,					//	A B C	R(A) := RK(B) - RK(C)
	OP_MUL,					//	A B C	R(A) := RK(B) * RK(C)
	OP_DIV,					//	A B C	R(A) := RK(B) / RK(C)
	OP_MOD,					//	A B C	R(A) := RK(B) % RK(C)
	OP_POW,					//	A B C	R(A) := RK(B) ^ RK(C)
	OP_UNM,					//	A B		R(A) := -R(B)
	OP_NOT,					//	A B		R(A) := not R(B)
	OP_LEN,					//	A B		R(A) := length of R(B)
	OP_CONCAT,			//	A B C	R(A) := R(B).. ... ..R(C)
	OP_JMP,					//	sBx		pc += sBx
	OP_EQ,					//	A B C	if ((RK(B) == RK(C)) ~= A) then pc++
	OP_LT,					//	A B C	if ((RK(B) <  RK(C)) ~= A) then pc++  
	OP_LE,					//	A B C	if ((RK(B) <= RK(C)) ~= A) then pc++  
	OP_TEST,				//	A C		if not (R(A) <=> C) then pc++
	OP_TESTSET,			//	A B C	if (R(B) <=> C) then R(A) := R(B) else pc++
	OP_CALL,				//	A B C	R(A), ... ,R(A+C-2) := R(A)(R(A+1), ... ,R(A+B-1))
	OP_TAILCALL,		//	A B C	return R(A)(R(A+1), ... ,R(A+B-1))
	OP_RETURN,			//	A B		return R(A), ... ,R(A+B-2)	(see note)
	OP_FORLOOP,			//	A			sBx	R(A)+=R(A+2);
									//				if R(A) <?= R(A+1) then { pc+=sBx; R(A+3)=R(A) }
	OP_FORPREP,			//	A			sBx	R(A)-=R(A+2); pc+=sBx
	OP_TFORLOOP,		//	A C		R(A+3), ... ,R(A+2+C) := R(A)(R(A+1), R(A+2));
									//				if R(A+3) ~= nil then R(A+2)=R(A+3) else pc++
	OP_SETLIST,			//	A B C	R(A)[(C-1)*FPF+i] := R(A+i), 1 <= i <= B
	OP_CLOSE,				//	A 		close all variables in the stack up to (>=) R(A)
	OP_CLOSURE,			//	A Bx	R(A) := closure(KPROTO[Bx], R(A), ... ,R(A+n))
	OP_VARARG				//	A B		R(A), R(A+1), ..., R(A+B-1) = vararg
} OpCode;

//===========================================================================
//  Notes:
//  (*) In OP_CALL, if (B == 0) then B = top. C is the number of returns - 1,
//      and can be 0: OP_CALL then sets `top' to last_result+1, so
//      next open instruction (OP_CALL, OP_RETURN, OP_SETLIST) may use `top'.
//
//  (*) In OP_VARARG, if (B == 0) then use actual number of varargs and
//      set top (like in OP_CALL with C == 0).
//
//  (*) In OP_RETURN, if (B == 0) then return up to `top'
//
//  (*) In OP_SETLIST, if (B == 0) then B = `top';
//      if (C == 0) then next `instruction' is real C
//
//  (*) For comparisons, A specifies what condition the test should accept
//      (true or false).
//
//  (*) All `skips' (pc++) assume that next instruction is a jump
//===========================================================================

typedef enum
{
  OpArgN,				// Argument is not used
  OpArgU,				// Argument is used
  OpArgR,				// Argument is a register or a jump offset
  OpArgK				// Argument is a constant or register/constant
} OpArgMask;

// Size and position of opcode arguments.
#define SIZE_C					9
#define SIZE_B					9
#define SIZE_Bx					(SIZE_C + SIZE_B)
#define SIZE_A					8
#define SIZE_OP					6
#define POS_OP					0
#define POS_A						(POS_OP + SIZE_OP)
#define POS_C						(POS_A + SIZE_A)
#define POS_B						(POS_C + SIZE_C)
#define POS_Bx					POS_C

// Creates a mask with 'n' 1 bits at position `p'.
#define MASK1(n, p)			((~((~(UINT32)0)<<n))<<p)

// Creates a mask with 'n' 0 bits at position `p'.
#define MASK0(n, p)			(~MASK1(n,p))

#define VM_MAX_ARG_Bx   ((1<<SIZE_Bx)-1)
#define VM_MAX_ARG_sBx  (VM_MAX_ARG_Bx>>1)         // `sBx' is signed
#define VM_MAX_ARG_A    ((1<<SIZE_A)-1)
#define VM_MAX_ARG_B    ((1<<SIZE_B)-1)
#define VM_MAX_ARG_C    ((1<<SIZE_C)-1)

// The following macros help to manipulate instructions.
#define VM_OPCODE(i)		((OpCode)(((i) >> POS_OP) & MASK1(SIZE_OP, 0)))
#define VM_ARG_A(i)			(int)(((i) >> POS_A) & MASK1(SIZE_A, 0))
#define VM_ARG_B(i)			(int)(((i) >> POS_B) & MASK1(SIZE_B, 0))
#define VM_ARG_C(i)			(int)(((i) >> POS_C) & MASK1(SIZE_C, 0))
#define VM_ARG_Bx(i)		(int)(((i) >> POS_Bx) & MASK1(SIZE_Bx, 0))
#define VM_ARG_sBx(i)		(VM_ARG_Bx(i) - VM_MAX_ARG_sBx)

// Macros to operate RK indices.

// This bit 1 means constant (0 means register).
#define BITRK						(1 << (SIZE_B - 1))

// Test whether value is a constant.
#define ISK(x)					((x) & BITRK)

// Gets the index of the constant.
#define INDEXK(r)				((int)(r) & ~BITRK)

// Helpful macros
#define VM_REG(reg)			luaVMRegisters[luaVMFunctions[luaVMFunctionsIndex].stackStart+reg]
#define VM_STR(num)			(char *)(luaVMStrings + num)
#define VM_CONST(idx)		luaVMConstants[luaVMFunctions[luaVMFunctionsIndex].constStart+idx]
#define VM_GLOBAL(idx)	luaVMGlobals[idx]

//
// Virtual machine state variables.
//

// The program counter.
UINT16 luaVMPC;

// The program name.
UINT8 *luaVMProgramName;

// Current function index.
UINT8 luaVMFunctionsIndex;

// Current call stack index.
UINT32 luaVMCallStackIndex;

//
// Gloval virtual machine functions.
//

// Initialize and reset the virtual machine.
void luaVMInit(void)
{
	// Initialize the program pointer name.
	luaVMProgramName = luaVMStrings;

	// Initialize the virtual machine tables.
	luaVMTablesInit();

	// Reset the virtual machine.
	luaVMReset();
}

// Reset the virtual machine.
void luaVMReset(void)
{
  // reset pc
  luaVMFunctionsIndex = luaVMFunctionsSize - 1;
  luaVMPC = 0;
  luaVMCallStackIndex = 0;
}

// Reset the virtual machine and run program from the beginning.
void luaVMRun(void)
{
    // Reset the pc.
    luaVMReset();

		// Continue from here.
		luaVMCont();
}

// Runs the program from the current position.
void luaVMCont(void)
{
  int val;

	// Wait until the vm stops running or
	// The ESC key (0x1B) is pressed.
  while ((val = luaVMStep(0)) == 0);

	// Did we exit with error?
  if (val != 1)
	{
		xprintf("*** ERROR: %d\n", val);
  }
}

// Runs the next command.	If print_op==1 then information about the 
// operation will be printed.
//
// Returns 0 for a normal operation
// Returns 1 for a break or end of program
// Returns >1 for error condition
INT32 luaVMStep(BOOL print_op)
{
	UINT32 argA;
	UINT32 argB;
	UINT32 argC;
	UINT32 argBx;
	UINT32 argSBx;
	UINT32 instruction;
	INT32 tmp1;
	INT32 tmp2;
	INT32 tmp3;
	INT32 tmp4;
	INT32 retVal = 0;
	INT32 values[MAX_PARAMS];	// For lua function params and returns.
	OpCode opcode;

	// Get the next instruction to execute.
	instruction = luaVMInstructions[luaVMFunctions[luaVMFunctionsIndex].codeStart + luaVMPC++];

	// Extract the opcode from the instruction.
	opcode = VM_OPCODE(instruction);

	argA = VM_ARG_A(instruction);
	argB = VM_ARG_B(instruction);
	argC = VM_ARG_C(instruction);
	argBx = VM_ARG_Bx(instruction);
	argSBx = VM_ARG_sBx(instruction);

	switch (opcode)
	{
		case OP_MOVE : // R(A) := R(B)
			if (print_op) xprintf("OP_MOVE %d %d\n\r", argA, argB);
			VM_REG(argA) = VM_REG(argB);
			break;
		case OP_LOADK : // R(A) := Kst(Bx)
			if (print_op) xprintf("OP_LOADK %d %d\n\r", argA, argBx);
			VM_REG(argA) = VM_CONST(argBx).constant;
		break;
		case OP_LOADBOOL : // A B C R(A) := (Bool)B; if (C) pc++
			if (print_op) xprintf("OP_LOADBOOL %d %d %d\n\r", argA, argB, argC);
			VM_REG(argA) = (argB==0) ? 0 : 1;
			if (argC)
			{
					luaVMPC++;
			}
			break;
		case OP_LOADNIL : // clears R(A) ... R(B)
			if (print_op) xprintf("OP_LOADNIL %d %d \n\r", argA, argB);
			do
			{
				VM_REG(argB--)=0;
			} while (argB>=argA);
			break;
		case OP_GETUPVAL : // A B		R(A) := UpValue[B]
			if (print_op) xprintf("*OP_GETUPVAL %d %d\n\r", argA, argB);
			break;
		case OP_GETGLOBAL : // A Bx R(A) := Gbl[Kst(Bx)]
			 if (print_op) xprintf("OP_GETGLOBAL %d %d\n\r", argA, argBx);
			 VM_REG(argA) = VM_GLOBAL(VM_CONST(argBx).globalIndex);
			break;
		case OP_GETTABLE : // A B C R(A) := R(B)[RK(C)]
			if (print_op) xprintf("*OP_GETTABLE %d %d %d\n\r", argA, argB, argC);
			break;
		case OP_SETGLOBAL : // A Bx Gbl[Kst(Bx)] := R(A)
			if (print_op) xprintf("OP_SETGLOBAL %d %d\n\r", argA, argBx);
			VM_GLOBAL(VM_CONST(argBx).globalIndex) = VM_REG(argA);
			break;
		case OP_SETUPVAL : // A B	 UpValue[B] := R(A)
			if (print_op) xprintf("*OP_SETUPVAL %d %d\n\r", argA, argB);
			break;
		case OP_SETTABLE : // A B C R(A)[RK(B)] := RK(C)
			if (print_op) xprintf("*OP_SETTABLE %d %d %d\n\r", argA, argB, argC);
			break;
		case OP_NEWTABLE : // A B C R(A) := {} (size = B,C)
			if (print_op) xprintf("*OP_NEWTABLE %d %d %d\n\r", argA, argB, argC);
			break;
		case OP_SELF : // A B C R(A+1) := R(B); R(A) := R(B)[RK(C)]
			if (print_op) xprintf("*OP_SELF %d %d %d\n\r", argA, argB, argC);
			break;
		case OP_ADD : // A B C	R(A) := RK(B) + RK(C)
			if (print_op) xprintf("OP_ADD %d %d %d \n\r", argA, argB, argC);
			tmp1 = (ISK(argB)) ? VM_CONST(INDEXK(argB)).constant : VM_REG(argB);
			tmp2 = (ISK(argC)) ? VM_CONST(INDEXK(argC)).constant : VM_REG(argC);
			VM_REG(argA) = tmp1 + tmp2;
			break;
		case OP_SUB : // A B C	R(A) := RK(B) - RK(C)
			if (print_op) xprintf("OP_SUB %d %d %d \n\r", argA, argB, argC);
			tmp1 = (ISK(argB)) ? VM_CONST(INDEXK(argB)).constant : VM_REG(argB);
			tmp2 = (ISK(argC)) ? VM_CONST(INDEXK(argC)).constant : VM_REG(argC);
			VM_REG(argA) = tmp1 - tmp2;
			break;
		case OP_MUL : // A B C	R(A) := RK(B) * RK(C)
			if (print_op) xprintf("OP_MUL %d %d %d \n\r", argA, argB, argC);
			tmp1 = (ISK(argB)) ? VM_CONST(INDEXK(argB)).constant : VM_REG(argB);
			tmp2 = (ISK(argC)) ? VM_CONST(INDEXK(argC)).constant : VM_REG(argC);
			VM_REG(argA) = tmp1 * tmp2;
			break;
		case OP_DIV : // A B C	R(A) := RK(B) / RK(C)
			if (print_op) xprintf("OP_DIV %d %d %d \n\r", argA, argB, argC);
			tmp1 = (ISK(argB)) ? VM_CONST(INDEXK(argB)).constant : VM_REG(argB);
			tmp2 = (ISK(argC)) ? VM_CONST(INDEXK(argC)).constant : VM_REG(argC);
			VM_REG(argA) = tmp1 / tmp2;
			break;
		case OP_MOD : // A B C	R(A) := RK(B) % RK(C)
			if (print_op) xprintf("OP_MOD %d %d %d \n\r", argA, argB, argC);
			tmp1 = (ISK(argB)) ? VM_CONST(INDEXK(argB)).constant : VM_REG(argB);
			tmp2 = (ISK(argC)) ? VM_CONST(INDEXK(argC)).constant : VM_REG(argC);
			VM_REG(argA) = tmp1 % tmp2;
			break;
		case OP_POW : // A B C R(A) := RK(B) ^ RK(C)
			if (print_op) xprintf("*OP_POW %d %d %d \n\r", argA, argB, argC);
			break;
			//tmp1 = (ISK(argB)) ? VM_CONST(INDEXK(argB)).constant : VM_REG(argB);
			//tmp2 = (ISK(argC)) ? VM_CONST(INDEXK(argC)).constant : VM_REG(argC);
			//tmp3 = tmp1 ** tmp2;
			//xprintf("%d ** %d = %d \n\r",tmp1,tmp2,tmp3);
			//VM_REG(argA) = tmp1 ** tmp2;
			//break;
		case OP_UNM : // A B		R(A) := -R(B)
			if (print_op) xprintf("OP_UNM %d %d\n\r", argA, argB);
			VM_REG(argA) = - VM_REG(argB);
			break;
		case OP_NOT : // A B		R(A) := not R(B)
			if (print_op) xprintf("OP_NOT %d %d\n\r", argA, argB);
			VM_REG(argA) = ~ VM_REG(argB);
			break;
		case OP_LEN : // A B		R(A) := length of R(B)
			if (print_op) xprintf("*OP_LEN %d %d\n\r", argA, argB);
			break;
		case OP_CONCAT : // A B C	 R(A) := R(B).. ... ..R(C)
			if (print_op) xprintf("*OP_CONCAT %d %d %d \n\r", argA, argB, argC);
			break;
		case OP_JMP : // sBx		pc+=sBx
			if (print_op) xprintf("OP_JMP %d\n\r", argSBx);
			luaVMPC += argSBx;
			break;
		case OP_EQ : // A B C	 if ((RK(B) == RK(C)) ~= A) then pc++
			if (print_op) xprintf("OP_EQ %d %d %d \n\r", argA, argB, argC);
			argB = (ISK(argB)) ? VM_CONST(INDEXK(argB)).constant : VM_REG(argB);
			argC = (ISK(argC)) ? VM_CONST(INDEXK(argC)).constant : VM_REG(argC);

			if (argB == argC && (argA==0)) {
					luaVMPC++;
			}
			break;
		case OP_LT : // A B C	 if ((RK(B) <	RK(C)) ~= A) then pc++
			if (print_op) xprintf("OP_LT %d %d %d \n\r", argA, argB, argC);
			argB = (ISK(argB)) ? VM_CONST(INDEXK(argB)).constant : VM_REG(argB);
			argC = (ISK(argC)) ? VM_CONST(INDEXK(argC)).constant : VM_REG(argC);
			if (argB < argC && (argA==0))
			{
				luaVMPC++;
			}
			if (argB >= argC && (argA==1))
			{
				luaVMPC++;
			}
			break;
		case OP_LE : // A B C	 if ((RK(B) <= RK(C)) ~= A) then pc++
			if (print_op) xprintf("OP_LE %d %d %d \n\r", argA, argB, argC);
			argB = (ISK(argB)) ? VM_CONST(INDEXK(argB)).constant : VM_REG(argB);
			argC = (ISK(argC)) ? VM_CONST(INDEXK(argC)).constant : VM_REG(argC);
			if (argB <= argC && (argA==0))
			{
				luaVMPC++;
			}
			if (argB > argC && (argA==1))
			{
				luaVMPC++;
			}
			break;
		case OP_TEST : // A C	 if not (R(A) <=> C) then pc++
			if (print_op) xprintf("OP_TEST %d %d\n\r", argA, argC);
			argA = VM_REG(argA);
			if (argA != argC)
			{
				luaVMPC++;
			}
			break;
		case OP_TESTSET : // A B C	if (R(B) <=> C) then R(A) := R(B) else pc++
			if (print_op) xprintf("OP_TESTSET %d %d %d \n\r", argA, argB, argC);
			argB = VM_REG(argB);
			if (argB == argC)
			{
				VM_REG(argA) = argB;
			}
			else
			{
				luaVMPC++;
			}
			break;
		case OP_CALL : // A B C R(A), ... ,R(A+C-2) := R(A)(R(A+1), ... ,R(A+B-1))
			if (print_op) xprintf("OP_CALL %d %d %d \n\r", argA, argB, argC);
			// Address of cfunc or lua_func proto num.
			tmp1 = VM_REG(argA);	
			if (print_op) xprintf("tmp1: %d \n\r",tmp1);
			// Get the parameters.	There are (B-1) params at most.
			if (argB >= MAX_PARAMS) {
				retVal = 4; // TOO Many Params
				break;
			}
			// Save the parameters to pass to the function.
			tmp4 = 0; // number of params
			if (argB >= 2)
			{
				// If B is 2 or more, there are (B-1) parameters.
				for (tmp2 = (INT32) argA + 1, tmp3 = 0; tmp2 < (INT32) (argA + argB); tmp2++, tmp3++)
				{
					values[tmp3] = VM_REG(tmp2);
					tmp4++; // num of params
				}
			}
			else if (argB == 0)
			{
				// If B is 0 params range from r(A+1) to top of stack.
				tmp4 = luaVMFunctions[luaVMFunctionsIndex].stackSize;	// num of params
				if (tmp4 > MAX_PARAMS)
				{
						tmp4 = MAX_PARAMS;
				}

				for (tmp2=argA+1,tmp3=0;tmp3<tmp4;tmp2++,tmp3++)
				{
						values[tmp3] = VM_REG(tmp2);
				}
			}

			// If the function reference (tmp1) is negative than it
			// is a builtin C function.	If it is zero or positive than
			// it is a user defined lua function.
			if (tmp1 < 0)
			{
				// C function index is a positive value.
				tmp1 = -tmp1;
				// Call the builtin C function.
				switch (tmp1)
				{
					// Output functions.
					case LUA_SYS_PRINT_NUM : xprintf("%d", values[0]); break;
					case LUA_SYS_PRINT_STR : xprintf("%s", VM_STR(values[0])); break;
					// IO functions.
					case LUA_SYS_SET_LED: luaSysSetLED((UINT8) values[0], (UINT8) values[1]); break;
					case LUA_SYS_GET_BUTTON: VM_REG(argA) = luaSysGetButton(); break;
					case LUA_SYS_DELAY: luaSysDelay((UINT32) values[0]); break;
				}
			}
			else
			{
				// Lua function so push current context onto call stack.
				if (luaVMCallStackIndex >= MAX_CALLSTACK_SIZE) {
					// STACK OVERFLOW
					retVal = 3;	
					break;
				}
				luaVMCallStack[luaVMCallStackIndex].pc = luaVMPC;
				luaVMCallStack[luaVMCallStackIndex].fpi = luaVMFunctionsIndex;
				luaVMCallStack[luaVMCallStackIndex].retReg = argA;
				luaVMCallStackIndex++;
				// Point at new function and start at the beginning.
				luaVMFunctionsIndex = tmp1;
				luaVMPC = 0;
				// Put params into new regs.
				for (tmp2=0; tmp2 < tmp4; tmp2++)
				{
					VM_REG(tmp2) = values[tmp2];
				}
			}
			break;
		case OP_TAILCALL : // A B C return R(A)(R(A+1), ... ,R(A+B-1))
			if (print_op) xprintf("*OP_TAILCALL %d %d %d \n\r", argA, argB, argC);
			break;
		case OP_RETURN : // A B return R(A), ... ,R(A+B-2)	(see note)
			if (print_op) xprintf("OP_RETURN %d %d\n\r", argA, argB);
			// If call stack is empty then return
			if (luaVMCallStackIndex <= 0)
			{
				retVal = 1;	// END of program
				break;
			}
			// Save return values in tmp variable. (B-1) max return values.
			if (argB >= MAX_PARAMS)
			{
				// TOO many return values
				retVal = 5; 
				break;
			}
			// Save the return values.
			tmp4 = 0; // Number of return values.
			if (argB >= 2)
			{
				// Bf B is 2 or more, there are (B-1) return values.
				for (tmp1 = (INT32) argA, tmp2 = 0; tmp1 < (INT32) (argA + argB - 1); tmp1++, tmp2++)
				{
					values[tmp2] = VM_REG(tmp1);
					tmp4++; // Num of return values.
				}
			}
			else if (argB == 0)
			{
				// FIXME: This is probably not quit right but may be ok
				// Need to add check for too many parameters here
				tmp4 = luaVMFunctions[luaVMFunctionsIndex].stackSize;	// num of return values
				if (tmp4 > MAX_PARAMS)
				{
					tmp4 = MAX_PARAMS;
				}
				// If B is 0 return values range from r(A) to top of stack.
				for (tmp1=argA,tmp2=0;tmp2<tmp4;tmp1++,tmp2++)
				{
					values[tmp2] = VM_REG(tmp1);
				}
			}
			// Pop stack
			luaVMCallStackIndex--;
			luaVMPC = luaVMCallStack[luaVMCallStackIndex].pc;
			luaVMFunctionsIndex = luaVMCallStack[luaVMCallStackIndex].fpi;
			tmp3 = luaVMCallStack[luaVMCallStackIndex].retReg;
			// Copy return values into registers.
			for (tmp2 = 0; tmp2 < tmp4; tmp2++)
			{
					VM_REG(tmp3 + tmp2) = values[tmp2];
			}
			break;
		case OP_FORLOOP : // A sBx	R(A)+=R(A+2)
			if (print_op) xprintf("OP_FORLOOP %d %d\n\r", argA, argSBx);
			VM_REG(argA) = VM_REG(argA) + VM_REG(argA + 2);
			// Is stepping neg or pos?
			if (VM_REG(argA + 2) < 0) {
				// Stepping is negative.
				if (VM_REG(argA) >= VM_REG(argA+1))
				{
						luaVMPC += argSBx;
						VM_REG(argA + 3) = VM_REG(argA);
				}
			}
			else
			{
				// Stepping is positive.
				if (VM_REG(argA) <= VM_REG(argA+1))
				{
					luaVMPC += argSBx;
					VM_REG(argA+3) = VM_REG(argA);
				}
			}
			break;
		case OP_FORPREP : // A sBx	R(A)-=R(A+2); pc+=sBx
			if (print_op) xprintf("OP_FORPREP %d %d\n\r", argA, argSBx);
			VM_REG(argA) = VM_REG(argA) - VM_REG(argA + 2);
			luaVMPC += argSBx;
			break;
		case OP_TFORLOOP : // A C	 R(A+3), ... ,R(A+2+C) := R(A)(R(A+1), R(A+2))
											 //			if R(A+3) ~= nil then R(A+2)=R(A+3) else pc++
			if (print_op) xprintf("*OP_TFORLOOP %d %d\n\r", argA, argC);
			break;
		case OP_SETLIST : // A B C	R(A)[(C-1)*FPF+i] := R(A+i), 1 <= i <= B
			if (print_op) xprintf("*OP_SETLIST %d %d %d \n\r", argA, argB, argC);
			break;
		case OP_CLOSE : // A		close all variables in the stack up to (>=) R(A)
			if (print_op) xprintf("*OP_CLOSE %d\n\r", argA);
			break;
		case OP_CLOSURE : // A Bx	 R(A) := closure(KPROTO[Bx], R(A), ... ,R(A+n))
			if (print_op) xprintf("OP_CLOSURE %d %d\n\r", argA, argBx);
			// copy the func proto number into reg(a)
			VM_REG(argA) = argBx;
			break;
		case OP_VARARG : // A B R(A), R(A+1), ..., R(A+B-1) = vararg
			if (print_op) xprintf("*OP_VARARG %d %d\n\r", argA, argB);
			break;
		default :
			xprintf("ERROR: Unsupported OpCode: %d. \n\r", opcode);
			retVal = 2;
			break;
	}

	return retVal;
}

