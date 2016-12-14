COPY     START   0            
FIRST    LDA     DEC
         FLOAT
		 STF     FLO
         ADDF    FLO
		 STF     SF
		 SUBF    SF
		 LDF     FLO
         COMPF   SF
		 MULF    FLO
		 FIX       
		 DIVF    SF     
DEC      WORD    3
SF       RESB    6
FLO      RESB    6
         END     FIRST
