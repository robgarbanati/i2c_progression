#!armcc -E --device=DARMCM1
#define ROM_SIZE			0x12000
#define SRAM_START			0x20000000
#define SRAM_SIZE			0x2000
#define STACK_SIZE			0x400 

_ROM					0x00000	 ROM_SIZE
{
	_RESET		 +0
	{
		startup_N572_Keil.o(RESET, +FIRST)
	}
	_CODE		+0
	{
		* (+RO)
	}
	_STACK				SRAM_START UNINIT ALIGN 4 STACK_SIZE
	{
		startup_N572_Keil.o(STACK)
	}
	_SRAM				+0 ALIGN 4 (SRAM_SIZE-STACK_SIZE)
	{
		* (+RW, +ZI)
	}
	_HEAP				+0 ALIGN 4
	{
		* (HEAP)
	}
}
