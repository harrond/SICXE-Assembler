COPY     START   0            
FIRST    LDA     VAR
         LDB    #VAR
		 LDL     VAR
		 LDS    @VAR
		+LDT    #4096
		 LDX     VAR
		 LDCH    BUFFER,X
CALCUL   ADD     OPER
         DIV    #123
		 MUL    @OPER
		 SUB
		 AND     OPER
		 OR      OPER
		 SHIFTL  A,1
		 SHIFTR  S,3
		 ADDR    T,A
		 DIVR    A,T
		 MULR    S,A
		 SUBR    B,T
		 RMO     X,A
INOUT    HIO
         TD      INPUT
		 RD      INPUT
		 WD      OUTPUT
		 SIO
		 TIO
		 STI
STORE    STA     LENGTH
         STB     STV
         STL     STV
	     STCH    BUFFER,X
		 STS     STV
		 STT     STV
		 STX     LENGHT
		 STSW    STV,X
JUMP     J       @STORE
         JEQ     INOUT
		 JGT     ETC
		 JLT     CALCUL
		+JSUB    RUTIN
		 RSUB
ETC      CLEAR   A
         COMP    OPER
		 COMPR   A,T
		 LPS     LENGTH
		 SSK     STV
		 TIX     OPER
		 SVC     7
		 TIXR    T
VAR      BYTE    C'EOF'
OPER     WORD    4
LENGTH   RESW    1
STV      RESW    5
CH       RESB    1
INPUT    BYTE    X'f0' 
OUTPUT   BYTE    X'f5' 
BUFFER   RESB    4096
RUTIN    LDA     OPER
         END     FIRST
