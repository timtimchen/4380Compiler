; ############################
; SC 4380 Project 4
; by Jinghong Chen
; ############################

;Data of the project

valX	.INT	-99
CNT		.INT	0
IntSize	.INT	4
MaxLen	.INT	30
ARRAY	.INT	0
		.INT	0
		.INT	0
		.INT	0
		.INT	0
		.INT	0
		.INT	0
		.INT	0
		.INT	0
		.INT	0
		.INT	0
		.INT	0
		.INT	0
		.INT	0
		.INT	0
		.INT	0
		.INT	0
		.INT	0
		.INT	0
		.INT	0
		.INT	0
		.INT	0
		.INT	0
		.INT	0
		.INT	0
		.INT	0
		.INT	0
		.INT	0
		.INT	0
		.INT	0
OverF	.INT	-999999
UnderF	.INT	-111111

newline	.BYT	10
space	.BYT	32
letterI	.BYT	'i'
letterS	.BYT	's'

str1	.BYT	'P'				;Please input a number: 
		.BYT	'l'
		.BYT	'e'
		.BYT	'a'
		.BYT	's'
		.BYT	'e'
		.BYT	32
		.BYT	'i'
		.BYT	'n'
		.BYT	'p'
		.BYT	'u'
		.BYT	't'
		.BYT	32
		.BYT	'a'
		.BYT	32
		.BYT	'n'
		.BYT	'u'
		.BYT	'm'
		.BYT	'b'
		.BYT	'e'
		.BYT	'r'
		.BYT	':'
		.BYT	32
strl1	.INT	23

str2	.BYT	'F'				;Fibonacci of 
		.BYT	'i'
		.BYT	'b'
		.BYT	'o'
		.BYT	'n'
		.BYT	'a'
		.BYT	'c'
		.BYT	'c'
		.BYT	'i'
		.BYT	32
		.BYT	'o'
		.BYT	'f'
		.BYT	32
strl2	.INT	13

;Code of the project

START
LOOP	LDR		R4, strl1		;print out string1
		LDA		R5, str1
print1	BRZ		R4, pend1
		LDB		R3, R5
		TRP		3
		ADI		R5, 1
		ADI		R4, -1
		JMP		print1
pend1	
		TRP		2				;get x
		STR		R3, valX		;store x for later print out
		BRZ		R3, QUIT1
; ready to call fib(x), test overflow
		MOV		R5, SP
		ADI		R5, -12			;Adjust for space needed
		CMP		R5, SL
		BLT		R5, OVERFLOW
; Create Activation Record and invoke fib(x)
		MOV		R3, FP			;save FP in R3
		MOV		FP, SP			;FP = SP
		ADI		SP, -4			;Adjust Stack Pointer for Return Address
		STR		R3, SP			;PFP = FP
		ADI		SP, -4			;Adjust Stack Pointer for PFP
		LDR		R5, valX
		STR		R5, SP			;pass x as parameter
		ADI		SP, -4			;Adjust Stack Pointer for x
		MOV		R1, PC
		ADI		R1, 36			;3 * FixLength = 36
		STR		R1, FP
		JMP		Fib
; put Fibonacci number into the ARRAY if it is not full
		LDR		R1, CNT
		ADI		R1, 2
		LDR		R2, MaxLen
		CMP		R2, R1			;check if ARRAY is full
		BLT		R2, OVERFLOW
		LDR		R1, CNT
		LDR		R5, IntSize
		MUL		R1, R5			;calculate the offset
		LDA		R3, ARRAY
		ADD		R3, R1			;pointer to ARRAY[CNT]
		LDR		R4, valX
		STR		R4, R3
		ADI		R3, 4			;pointing to the next slot
		LDR		R0, SP			;get result of fib(x)
		STR		R0, R3
		LDR		R1, CNT
		ADI		R1, 2
		STR		R1, CNT			;CNT += 2
		LDR		R4, strl2		;print out str2 and result of fib(x)
		LDA		R5, str2
print2	BRZ		R4, pend2
		LDB		R3, R5
		TRP		3
		ADI		R5, 1
		ADI		R4, -1
		JMP		print2
pend2			
		LDR		R3, valX
		TRP		1
		LDB		R3, space
		TRP		3
		LDB		R3, letterI
		TRP		3
		LDB		R3, letterS
		TRP		3
		LDB		R3, space
		TRP		3
		MOV		R3, R0
		TRP		1
		LDB		R3, newline
		TRP		3
		JMP		LOOP
QUIT1
; part II of the Program
		LDB		R3, newline
		TRP		3
		LDA		R0, ARRAY		;pointer head -> ARRAY[0]
		LDR		R1, CNT
		LDR		R2, IntSize
		ADI		R1, -1
		MUL		R1, R2
		ADD		R1, R0			;pointer tail -> ARRAY[CNT-1]
While1	MOV		R2, R1
		CMP		R2, R0			;compare two pointers
		BLT		R2, Wend1
		BRZ		R2, Wend1
		LDR		R3, R0			;print ARRAY[head]
		TRP		1
		LDB		R3, space
		TRP		3
		LDR		R3, R1			;print ARRAY[tail]
		TRP		1
		LDB		R3, space
		TRP		3
		ADI		R0, 4			;move head pointer
		ADI		R1, -4			;move tail pointer
		JMP		While1
Wend1	LDB		R3, newline
		TRP		3
END		TRP		0

OVERFLOW
		LDB		R3, newline
		TRP		3
		LDR		R3, OverF
		TRP		1
		LDB		R3, newline
		TRP		3
		TRP		0
UNDERFLOW
		LDB		R3, newline
		TRP		3
		LDR		R3, UnderF
		TRP		1
		LDB		R3, newline
		TRP		3
		TRP		0
		
Fib		MOV		R5, SP
		ADI		R5, -4			;test overflow
		CMP		R5, SL
		BLT		R5, OVERFLOW
		ADI		SP, -4			;reserve for local variable
		MOV		R7, FP
		ADI		R7, -8
		LDR		R0, R7			;read parameter x
		SUB		R1, R1
		ADI		R1, 1			;set R1 = 1
		CMP		R0, R1
		BGT		R0, GT1			;branch when x > 1
		LDR		R0, R7			;read parameter x
		CMP		R0, R1
		BLT		R0, LT1			;branch when x < 1
		MOV		R0, R1			;return result = 1 when x = 1
		JMP		RTN1
LT1		SUB		R0, R0			;return result = 0 when x < 1
		JMP		RTN1
GT1		
; ready to call fib(x - 1), test overflow
		MOV		R5, SP
		ADI		R5, -12			;Adjust for space needed
		CMP		R5, SL
		BLT		R5, OVERFLOW
; Create Activation Record and invoke fib(x - 1)
		MOV		R3, FP			;save FP in R3
		MOV		FP, SP			;FP = SP
		ADI		SP, -4			;Adjust Stack Pointer for Return Address
		STR		R3, SP			;PFP = FP
		ADI		SP, -4			;Adjust Stack Pointer for PFP
		LDR		R5, R7			;read parameter x
		ADI		R5, -1			;calculate x - 1
		STR		R5, SP			;pass x as parameter
		ADI		SP, -4			;Adjust Stack Pointer for x
		MOV		R1, PC
		ADI		R1, 36			;3 * FixLength = 36
		STR		R1, FP
		JMP		Fib
		LDR		R0, SP			;get result of fib(x - 1)
		MOV		R7, FP
		ADI		R7, -12
		STR		R0, R7			;store result of fib(x - 1) to local variable
		ADI		R7, 4
; ready to call fib(x - 2), test overflow
		MOV		R5, SP
		ADI		R5, -12			;Adjust for space needed
		CMP		R5, SL
		BLT		R5, OVERFLOW
; Create Activation Record and invoke fib(x - 2)
		MOV		R3, FP			;save FP in R3
		MOV		FP, SP			;FP = SP
		ADI		SP, -4			;Adjust Stack Pointer for Return Address
		STR		R3, SP			;PFP = FP
		ADI		SP, -4			;Adjust Stack Pointer for PFP
		LDR		R5, R7			;read parameter x
		ADI		R5, -2			;calculate x - 2
		STR		R5, SP			;pass x as parameter
		ADI		SP, -4			;Adjust Stack Pointer for x
		MOV		R1, PC
		ADI		R1, 36			;3 * FixLength = 36
		STR		R1, FP
		JMP		Fib
		LDR		R0, SP			;get result of fib(x - 2)
		MOV		R1, FP
		ADI		R1, -12
		LDR		R2, R1			;get result of fib(x - 1)
		ADD		R0, R2
; get ready to return
RTN1	MOV		SP, FP			;De-allocate (SP = FP)
		MOV		R5, FP
		CMP 	R5, SB			;Test for Underflow (SP > SB)
		BGT		R5, UNDERFLOW
; Set Previous Frame to Current Frame and Return
		LDR		R5, FP			;Return Address Pointed to by FP
		MOV		R6, FP
		ADI		R6, -4			;get PFP
		LDR		FP, R6			;FP = PFP
		STR		R0, SP			;return the result 
		JMR		R5				;Jump to Address in Register R5
		
		