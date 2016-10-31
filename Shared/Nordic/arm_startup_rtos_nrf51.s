; Copyright (c) 2009 Nordic Semiconductor. All Rights Reserved.
;
; The information contained herein is property of Nordic Semiconductor ASA.
; Terms and conditions of usage are described in detail in NORDIC
; SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
;
; Licensees are granted free, non-transferable use of the information. NO
; WARRANTY of ANY KIND is provided. This heading must NOT be removed from
; the file.
 
; NOTE: Template files (including this one) are application specific and therefore 
; expected to be copied into the application project folder prior to its use!

; Description message

Stack_Size      EQU     0x640
                AREA    STACK, NOINIT, READWRITE, ALIGN=3
Stack_Mem       SPACE   Stack_Size
__initial_sp

Heap_Size       EQU     0x0

                AREA    HEAP, NOINIT, READWRITE, ALIGN=3
__heap_base
Heap_Mem        SPACE   Heap_Size
__heap_limit

                PRESERVE8
                THUMB

; Vector Table Mapped to Address 0 at Reset

                AREA    RESET, DATA, READONLY
                EXPORT  __Vectors
                EXPORT  __Vectors_End
                EXPORT  __Vectors_Size

__Vectors       DCD     __initial_sp              ; Top of Stack
                DCD     Reset_Handler             ; Reset Handler
                DCD     NMI_Wrapper               ; NMI Handler
                DCD     HardFault_Wrapper         ; Hard Fault Handler
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     SVC_Wrapper               ; SVCall Handler
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     PendSV_Wrapper            ; PendSV Handler
                DCD     SysTick_Wrapper           ; SysTick Handler

                ; External Interrupts
                DCD      POWER_CLOCK_Wrapper ;POWER_CLOCK
                DCD      RADIO_Wrapper ;RADIO
                DCD      UART0_Wrapper ;UART0
                DCD      SPI0_TWI0_Wrapper ;SPI0_TWI0
                DCD      SPI1_TWI1_Wrapper ;SPI1_TWI1
                DCD      0 ;Reserved
                DCD      GPIOTE_Wrapper ;GPIOTE
                DCD      ADC_Wrapper ;ADC
                DCD      TIMER0_Wrapper ;TIMER0
                DCD      TIMER1_Wrapper ;TIMER1
                DCD      TIMER2_Wrapper ;TIMER2
                DCD      RTC0_Wrapper ;RTC0
                DCD      TEMP_Wrapper ;TEMP
                DCD      RNG_Wrapper ;RNG
                DCD      ECB_Wrapper ;ECB
                DCD      CCM_AAR_Wrapper ;CCM_AAR
                DCD      WDT_Wrapper ;WDT
                DCD      RTC1_Wrapper ;RTC1
                DCD      QDEC_Wrapper ;QDEC
                DCD      LPCOMP_COMP_Wrapper ;LPCOMP_COMP
                DCD      SWI0_Wrapper ;SWI0
                DCD      SWI1_Wrapper ;SWI1
                DCD      SWI2_Wrapper ;SWI2
                DCD      SWI3_Wrapper ;SWI3
                DCD      SWI4_Wrapper ;SWI4
                DCD      SWI5_Wrapper ;SWI5
                DCD      0 ;Reserved
                DCD      0 ;Reserved
                DCD      0 ;Reserved
                DCD      0 ;Reserved
                DCD      0 ;Reserved
                DCD      0 ;Reserved


__Vectors_End

__Vectors_Size  EQU     __Vectors_End - __Vectors

; Start of area "EXCEPTION"
                AREA    EXCEPTION, CODE, READONLY            
; Dummy    Exception Wrappers
                
NMI_Wrapper     PROC
                EXPORT    NMI_Wrapper                [WEAK]
                LDR       R0, =NMI_ISR
                LDR       R0, [R0]
                BX        R0
                ENDP

HardFault_Wrapper\
                PROC
                EXPORT    HardFault_Wrapper          [WEAK]
                LDR       R0, =HardFault_ISR
                LDR       R0, [R0]
                BX        R0
                ENDP

SVC_Wrapper     PROC
                EXPORT    SVC_Wrapper                [WEAK]
                LDR       R0, =SVC_ISR
                LDR       R0, [R0]
                BX        R0
                ENDP

PendSV_Wrapper  PROC
                EXPORT    PendSV_Wrapper             [WEAK]
                LDR       R0, =PendSV_ISR
                LDR       R0, [R0]
                BX        R0
                ENDP

SysTick_Wrapper PROC
                EXPORT    SysTick_Wrapper            [WEAK]
                LDR       R0, =SysTick_ISR
                LDR       R0, [R0]
                BX        R0
                ENDP

POWER_CLOCK_Wrapper\
                PROC
                LDR       R0, =POWER_CLOCK_ISR
                LDR       R0, [R0]
                BX        R0
                ENDP
                
RADIO_Wrapper   PROC
                LDR       R0, =RADIO_ISR
                LDR       R0, [R0]
                BX        R0
                ENDP

UART0_Wrapper   PROC
                LDR       R0, =UART0_ISR
                LDR       R0, [R0]
                BX        R0
                ENDP

SPI0_TWI0_Wrapper\
                PROC
                LDR       R0, =SPI0_TWI0_ISR
                LDR       R0, [R0]
                BX        R0
                ENDP

SPI1_TWI1_Wrapper\
                PROC
                LDR       R0, =SPI1_TWI1_ISR
                LDR       R0, [R0]
                BX        R0
                ENDP

GPIOTE_Wrapper PROC
                LDR       R0, =GPIOTE_ISR
                LDR       R0, [R0]
                BX        R0
                ENDP

ADC_Wrapper     PROC
                LDR       R0, =ADC_ISR
                LDR       R0, [R0]
                BX        R0
                ENDP

TIMER0_Wrapper  PROC
                LDR       R0, =TIMER0_ISR
                LDR       R0, [R0]
                BX        R0
                ENDP

TIMER1_Wrapper  PROC
                LDR       R0, =TIMER1_ISR
                LDR       R0, [R0]
                BX        R0
                ENDP

TIMER2_Wrapper  PROC
                LDR       R0, =TIMER2_ISR
                LDR       R0, [R0]
                BX        R0
                ENDP

RTC0_Wrapper    PROC
                LDR       R0, =RTC0_ISR
                LDR       R0, [R0]
                BX        R0
                ENDP

TEMP_Wrapper    PROC
                LDR       R0, =TEMP_ISR
                LDR       R0, [R0]
                BX        R0
                ENDP

RNG_Wrapper     PROC
                LDR       R0, =RNG_ISR
                LDR       R0, [R0]
                BX        R0
                ENDP

ECB_Wrapper     PROC
                LDR       R0, =ECB_ISR
                LDR       R0, [R0]
                BX        R0
                ENDP

CCM_AAR_Wrapper PROC
                LDR       R0, =CCM_AAR_ISR
                LDR       R0, [R0]
                BX        R0
                ENDP

WDT_Wrapper     PROC
                LDR       R0, =WDT_ISR
                LDR       R0, [R0]
                BX        R0
                ENDP

RTC1_Wrapper    PROC
                LDR       R0, =RTC1_ISR
                LDR       R0, [R0]
                BX        R0
                ENDP

QDEC_Wrapper    PROC
                LDR       R0, =QDEC_ISR
                LDR       R0, [R0]
                BX        R0
                ENDP

LPCOMP_COMP_Wrapper\
                PROC
                LDR       R0, =LPCOMP_COMP_ISR
                LDR       R0, [R0]
                BX        R0
                ENDP

SWI0_Wrapper    PROC
                LDR       R0, =SWI0_ISR
                LDR       R0, [R0]
                BX        R0
                ENDP

SWI1_Wrapper    PROC
                LDR       R0, =SWI1_ISR
                LDR       R0, [R0]
                BX        R0
                ENDP

SWI2_Wrapper    PROC
                LDR       R0, =SWI2_ISR
                LDR       R0, [R0]
                BX        R0
                ENDP

SWI3_Wrapper    PROC
                LDR       R0, =SWI3_ISR
                LDR       R0, [R0]
                BX        R0
                ENDP

SWI4_Wrapper    PROC
                LDR       R0, =SWI4_ISR
                LDR       R0, [R0]
                BX        R0
                ENDP

SWI5_Wrapper    PROC
                LDR       R0, =SWI5_ISR
                LDR       R0, [R0]
                BX        R0
                ENDP

Default_Handler PROC                    
                EXPORT    NMI_Handler                [WEAK] 
                EXPORT    HardFault_Handler          [WEAK] 
                EXPORT    SVC_Handler                [WEAK] 
                EXPORT    PendSV_Handler             [WEAK] 
                EXPORT    SysTick_Handler            [WEAK]
HardFault_Handler
NMI_Handler
SVC_Handler
PendSV_Handler
SysTick_Handler
                EXPORT    POWER_CLOCK_IRQHandler     [WEAK]
                EXPORT    RADIO_IRQHandler           [WEAK]
                EXPORT    UART0_IRQHandler           [WEAK]
                EXPORT    SPI0_TWI0_IRQHandler       [WEAK]
                EXPORT    SPI1_TWI1_IRQHandler       [WEAK]
                EXPORT    GPIOTE_IRQHandler          [WEAK]
                EXPORT    ADC_IRQHandler             [WEAK]
                EXPORT    TIMER0_IRQHandler          [WEAK]
                EXPORT    TIMER1_IRQHandler          [WEAK]
                EXPORT    TIMER2_IRQHandler          [WEAK]
                EXPORT    RTC0_IRQHandler            [WEAK]
                EXPORT    TEMP_IRQHandler            [WEAK]
                EXPORT    RNG_IRQHandler             [WEAK]
                EXPORT    ECB_IRQHandler             [WEAK]
                EXPORT    CCM_AAR_IRQHandler         [WEAK]
                EXPORT    WDT_IRQHandler             [WEAK]
                EXPORT    RTC1_IRQHandler            [WEAK]
                EXPORT    QDEC_IRQHandler            [WEAK]
                EXPORT    LPCOMP_COMP_IRQHandler     [WEAK]
                EXPORT    SWI0_IRQHandler            [WEAK]
                EXPORT    SWI1_IRQHandler            [WEAK]
                EXPORT    SWI2_IRQHandler            [WEAK]
                EXPORT    SWI3_IRQHandler            [WEAK]
                EXPORT    SWI4_IRQHandler            [WEAK]
                EXPORT    SWI5_IRQHandler            [WEAK]
POWER_CLOCK_IRQHandler
RADIO_IRQHandler
UART0_IRQHandler
SPI0_TWI0_IRQHandler
SPI1_TWI1_IRQHandler
GPIOTE_IRQHandler
ADC_IRQHandler
TIMER0_IRQHandler
TIMER1_IRQHandler
TIMER2_IRQHandler
RTC0_IRQHandler
TEMP_IRQHandler
RNG_IRQHandler
ECB_IRQHandler
CCM_AAR_IRQHandler
WDT_IRQHandler
RTC1_IRQHandler
QDEC_IRQHandler
LPCOMP_COMP_IRQHandler
SWI0_IRQHandler
SWI1_IRQHandler
SWI2_IRQHandler
SWI3_IRQHandler
SWI4_IRQHandler
SWI5_IRQHandler
                B        .
                ENDP
                ALIGN
; End of area "EXCEPTION"

; Start of area "STARTUP"                 
                AREA    STARTUP, CODE, READONLY        
; Reset Handler
NRF_POWER_RAMON_ADDRESS           EQU   0x40000524  ; NRF_POWER->RAMON address
NRF_POWER_RAMON_RAMxON_ONMODE_Msk EQU   0xF         ; All RAM blocks on in onmode bit mask

Reset_Handler   PROC
                EXPORT  Reset_Handler             [WEAK]
                IMPORT  SystemInit
                IMPORT  __main
                LDR     R0, =NRF_POWER_RAMON_ADDRESS
                LDR     R2, [R0]
                MOVS    R1, #NRF_POWER_RAMON_RAMxON_ONMODE_Msk
                ORRS    R2, R2, R1
                STR     R2, [R0]
                LDR     R0, =SystemInit
                BLX     R0
                LDR     R0, =__main
                BX      R0
                ENDP

; User Initial Stack & Heap

                IF      :DEF:__MICROLIB
                
                EXPORT  __initial_sp
                EXPORT  __heap_base
                EXPORT  __heap_limit

                ELSE

                IMPORT  __use_two_region_memory
                EXPORT  __user_initial_stackheap
__user_initial_stackheap

                LDR     R0, = Heap_Mem
                LDR     R1, = (Stack_Mem + Stack_Size)
                LDR     R2, = (Heap_Mem + Heap_Size)
                LDR     R3, = Stack_Mem
                BX      LR

                ALIGN

                ENDIF

                ALIGN
; End of area "STARTUP"

; Start of area "IRQTABLE"			
				AREA	IRQTABLE, DATA, READWRITE
                IMPORT  OS_Tick_Handler
NMI_ISR         DCD     NMI_Handler
HardFault_ISR   DCD     HardFault_Handler
SVC_ISR         DCD     SVC_Handler
PendSV_ISR      DCD     PendSV_Handler
SysTick_ISR     DCD     SysTick_Handler
;    maximum    of 32 External Interrupts are possible
POWER_CLOCK_ISR DCD     POWER_CLOCK_IRQHandler
RADIO_ISR       DCD     RADIO_IRQHandler
UART0_ISR       DCD     UART0_IRQHandler
SPI0_TWI0_ISR   DCD     SPI0_TWI0_IRQHandler
SPI1_TWI1_ISR   DCD     SPI1_TWI1_IRQHandler
GPIOTE_ISR      DCD     GPIOTE_IRQHandler
ADC_ISR         DCD     ADC_IRQHandler
TIMER0_ISR      DCD     TIMER0_IRQHandler
TIMER1_ISR      DCD     TIMER1_IRQHandler
TIMER2_ISR      DCD     TIMER2_IRQHandler
RTC0_ISR        DCD     RTC0_IRQHandler
TEMP_ISR        DCD     TEMP_IRQHandler
RNG_ISR         DCD     RNG_IRQHandler
ECB_ISR         DCD     ECB_IRQHandler
CCM_AAR_ISR     DCD     CCM_AAR_IRQHandler
WDT_ISR         DCD     WDT_IRQHandler
RTC1_ISR        DCD     OS_Tick_Handler
QDEC_ISR        DCD     QDEC_IRQHandler
LPCOMP_COMP_ISR DCD     LPCOMP_COMP_IRQHandler
SWI0_ISR        DCD     SWI0_IRQHandler
SWI1_ISR        DCD     SWI1_IRQHandler
SWI2_ISR        DCD     SWI2_IRQHandler
SWI3_ISR        DCD     SWI3_IRQHandler
SWI4_ISR        DCD     SWI4_IRQHandler
SWI5_ISR        DCD     SWI5_IRQHandler
				ALIGN
; End of area "IRQTABLE"	
				EXPORT    NMI_ISR
                EXPORT    HardFault_ISR
                EXPORT    SVC_ISR
                EXPORT    PendSV_ISR
                EXPORT    SysTick_ISR
                EXPORT    POWER_CLOCK_ISR 
                EXPORT    RADIO_ISR       
                EXPORT    UART0_ISR       
                EXPORT    SPI0_TWI0_ISR   
                EXPORT    SPI1_TWI1_ISR   
                EXPORT    GPIOTE_ISR      
                EXPORT    ADC_ISR         
                EXPORT    TIMER0_ISR      
                EXPORT    TIMER1_ISR      
                EXPORT    TIMER2_ISR      
                EXPORT    RTC0_ISR        
                EXPORT    TEMP_ISR        
                EXPORT    RNG_ISR         
                EXPORT    ECB_ISR         
                EXPORT    CCM_AAR_ISR     
                EXPORT    WDT_ISR         
                EXPORT    RTC1_ISR        
                EXPORT    QDEC_ISR        
                EXPORT    LPCOMP_COMP_ISR 
                EXPORT    SWI0_ISR        
                EXPORT    SWI1_ISR        
                EXPORT    SWI2_ISR        
                EXPORT    SWI3_ISR        
                EXPORT    SWI4_ISR        
                EXPORT    SWI5_ISR        

; Start of area "JMPTABLE"            
                AREA    JMPTABLE, DATA, READONLY
appJumpAddress  DCD     Reset_Handler
                ALIGN
; End of area "JMPTABLE"
                EXPORT    appJumpAddress

                END

