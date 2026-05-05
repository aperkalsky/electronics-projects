$NOMOD51
$GENONLY
$ERRORPRINT
$DEBUG
$MACRO(100)
	NAME    mus
;	This is a progam for electronic doorbell with different melodies.
; It is implemented on 80C32 microcontroller. It uses 2 timers of 80C32
; in the following modes:
;	Timer0 is set to Mode 1 (16-bit counter) and is used for note
; duration measurement.
;	Timer1 is set to Mode 2 (8-bit counter with auto-reload) and is
; used for note's frequency period measurement.
;	Ports 1 and 3 are set to output mode. P1.0 serves as an output
; for synthesized frequency. P3.0 disables reset while the melody is
; being played.
;
;	Register usage:
; --------------------- Bank 0 ------------------------
; R0: holds current reload value for TL0 register
; R1: holds current reload value for TH0 register
; R2: note counter (number of current note within melody, start from 1)
; R3: holds number of T0 reloads before note change (duration counter)
; R4: holds number of T1 reloads before change of frequency wavefront state
;     (octave value)
; R5: is ised as iterator for delay cycle
; R6: holds copy of DPL
; R7: holds copy of DPH
;
;	Memory usage:
; 20h - Melody playing statis word. Translates as following:
;	bMelPause(20.0) - Pause bit. Pause if set
;	bMelTerm(20.1) - Melody termination flag. Melody is finished if set
; 40h - 7Fh: Stack (SP is initialized to 40h)
; 30h - Load value for R4
$NOLIST
$INCLUDE(d:\mcs51asm\REG52.PDF)
$INCLUDE(MYDEF.PDF)
$LIST

; program body
ORG     0000h           ; Program starts from this address
        ajmp MAIN	      	; jump to the init header

ORG     0003h           ; External interrupt 0 ISR
        reti

ORG     000Bh           ; Timer0 ISR ( duration counter )
	djnz	R3, RELOAD_0	; one tick less		2
	clr	TR0		; stop timer 		1
	reti						;1
RELOAD_0:			; if time ish't expired
        acall   T0_RELOAD				;2
        reti						;1

ORG     0013h           ; External interrupt 1 ISR
        reti

ORG     001Bh           ; Timer1 ISR ( frequency counter )
	djnz	R4, T1_ISR_EXIT	; one tick less		2
	acall	OUT_MODIFY	; change output state	2
T1_ISR_EXIT:
	reti

ORG     0023h           ; Serial port interrupt
        reti

ORG     002Bh           ; Timer2 ISR
        reti

; Start of program area
ORG     0033h
;========================================================================
	; Timer0 reload routine
T0_RELOAD:
	clr	TR0		; stop timer
	mov	TL0, R0		; set low byte
	mov	TH0, R1		; set high byte
	setb	TR0		; turn timer on
	ret
	; End of Timer0 reload routine
;========================================================================
	; OUT_MODIFY routine - changes P1.0 state
OUT_MODIFY:
	cpl	P1.0		; invert P1.0 state
	mov	R4, 30h		; reload period duration counter
	ret
	; End of OUT_MODIFY routine

;========================================================================
	; Initialization routine
INIT:
	; Disable interrupts and arrange priorities
	mov	IE, #0          ; disable all interrupts
	clr     PT0             ; set low priority to Timer0 interrupt
        clr     PT1             ; set low priority to Timer1 interrupt
	; Initialize ports
        mov     P1, #0          ; initialize port 1 to output mode
        mov     P3, #0		; initialize port 3 to output mode
	; Initialize timers
	mov	TMOD, #21h	; set T0 to Mode 1, T1 to mode 2
				; clock is set to internal
	mov	TCON, #0        ; stop counters and clear overflow flags
	; Initialize registers
	mov	vMaxMelCnt, #37	; total number of melodies in memory
	; Enable interrupts
	mov	IE, #8Ah	; T0, T1 and global interrupts enable
	ret
	; End of initialization routine
;========================================================================
	; SET_MELODY - loads DPTR with start address of melody to play
SET_MELODY:
	; Define current melody index
	mov	A, vMelIndex	; load melody index from index register
	cjne	A,vMaxMelCnt,N_EQUAL	; cmp curr index to max value
	ajmp	CLR_INDEX	; if values are equal
N_EQUAL:			; it values are not equal
	jc	INDEX_READY	; if carry is set A is less, this is OK
CLR_INDEX:               	; if A is greater
	mov	vMelIndex, #0	; reset melody index
	mov	A, vMelIndex	; reload new melody index
INDEX_READY:
	clr	C		; clear carry flag if it was set
	; Convert it to melody table index
 	rl	A		; shift it to obtain m.b.t. index
	push	ACC		; save obtained index
	; Load DPTR with it
	mov	DPTR, #NAMES_TBL; set pointer to the melody browsing table
	movc	A, @A+DPTR	; load high byte of melody address
	mov	R7, A		; copy it to DPH mirror
	pop	ACC		; reload m.b.t. index
	inc	A		; increment it
	movc	A, @A+DPTR	; load low byte of melody address
	mov	R6, A		; copy it to DPL mirror
	clr	A		; reset accumulator (just to be perfect)
	acall	POP_DPTR	; reload DPTR from its mirror
	ret
	; End of SET_MELODY routine
;========================================================================
	; READ_NOTE - reads one note from note table and
	; loads T0 and T1 with data.
READ_NOTE:
	; read frequency
	clr	A		; reset accumulator
	movc	A, @A+DPTR	; read frequency code
	acall	PUSH_DPTR	; save DPTR (current index in note table)
	jnz	NO_PAUSE	; skip next line if not pause
	; set pause flag
	setb	bMelPause	; set pause flag
	sjmp	READ_DURAT	; go to duration reading procedure
NO_PAUSE:			; if it is non-zero note
	; clear pause flag
	clr	bMelPause	; clear pause flag
	push	ACC		; save accumulator contents
	; load Table A address
	mov	DPTR, #TABLE_A	; set DPTR to note browsing table
	; calculate Table A index
	anl	A, #0F0h	; mask off low nibble
	swap	A		; swap nibbles of accumulator
	; read note value
	movc	A, @A+DPTR	; read note value from Table A
	; load T1 registers with it
	mov	TL1, A		; copy it to TL1
	mov	TH1, A		; copy it to TH1
	; load Table B index
	mov	DPTR, #TABLE_B	; set DPTR to octave browsing table
	; calculate Table B index
	pop	ACC		; take initial frequency code back
	anl	A, #00Fh	; mask off high nibble
	dec	A		; decrement value by 1, this gives us the index
	; read octave value
	movc	A, @A+DPTR	; read octave value from Table B
	; load octave counter register and its loader with it
	mov	30h, A		; copy it to the loader
	mov	R4, A		; copy it to R4
READ_DURAT:
	; read second byte
	acall	POP_DPTR	; get current music table index
	inc	DPTR		; increment it
	clr	A		; reset accumulator
	movc	A, @A+DPTR	; read byte from music table
	jnz	NOT_FINISHED	; if != 0, melody isn't expired
	setb	bMelTerm	; if finished, set termination flag
	sjmp	R_N_END		; go to end of function
NOT_FINISHED:
	; save DPTR value
	acall	PUSH_DPTR	; save current music table index
	; load table C address
	mov	DPTR, #TABLE_C	; set DPTR to duration value browsing table
	; save accumulator contents
	push	ACC		; push accumulator onto stack
	; calculate table C index
	anl	A, #0E0h	; mask off five low bits
	swap	A		; swap nibbles - this will give us index*2
	push	ACC		; save table C index
	; read two bytes of duration value from Table C
	movc	A, @A+DPTR	; read high byte of duration value
	mov	R1, A		; copy it to buffer register
	mov	TH0, A		; load it to timer
	pop	ACC		; recover table C index
	inc	A		; increment it
	movc	A, @A+DPTR	; read low byte of duration value
	mov	R0, A		; copy it to buffer register
	mov	TL0, A		; load it to timer
	; load Table D address
	mov	DPTR, #TABLE_D	; set DPTR to duration index browsing table
	; calculate table D index
	pop	ACC		; reload accumulator with duration code
	anl	A, #01Fh	; mask off three upper bits
	; read number of duration counter reloads from table D
	movc	A, @A+DPTR	; read byte from table D
	; load duration counter register with it
	mov	R3, A		; copy it to R3
	; get music table address back
	acall	POP_DPTR	; get last music table address
	inc	DPTR		; set DPTR to the next note in the table
R_N_END:
	ret
	; End of READ_NOTE routine
;========================================================================
	; PUSH_DPTR - saves Data Pointer value
PUSH_DPTR:
	mov	R6, DPL		; save low byte of DPTR
	mov	R7, DPH		; save high byte of DPTR
	ret
	; End of PUSH_DPTR routine
;========================================================================
	; POP_DPTR - reloads Data Pointer vith saved value
POP_DPTR:
	mov	DPH, R7		; recover high byte of DPTR
	mov	DPL, R6		; recover low byte of DPTR
	ret
	; End of POP_DPTR routine
;========================================================================
	; PLAY_NOTE - melody playing routine. Returns when note
	; duration time is expired. It is implied that duration
	; and note frequency values are already loaded into timers.
PLAY_NOTE:
	setb	TR0		; run duration timer
	jb	bMelPause, WAIT_LOOP; if it's pause, don't run Timer1
	setb	TR1		; run frequency timer
WAIT_LOOP:
	nop			; waste a time
	cjne	R3,#0, WAIT_LOOP; if duration isn't expired, continue
	clr	TR0		; stop duration counter
	jb	bMelPause, P_N_EXIT ; if it was pause, nothing else to do
	clr	TR1		; otherwise, stop also freq. counter
P_N_EXIT:
        clr     P1.0            ; set output to 0
	acall DELAY		; make a short pause (1/4 ms)
	acall DELAY		; make a short pause (1/4 ms)
	acall DELAY		; make a short pause (1/4 ms)
	ret
	; End of PLAY_NOTE routine
;========================================================================
	; DELAY routine - makes a short delay between notes
DELAY:
        mov     R5, #0FFh       ; load loop iterator
WASTE_LOOP:                     ; make a pause between notes
        nop
        nop
        djnz    R5, WASTE_LOOP
	ret
	; End of Delay routine
;========================================================================
	; CHOOSE_NEXT routine - set melody index with new value
CHOOSE_NEXT:
	inc	vMelIndex	; for meaning time just increment it
	ret
	; end of CHOOSE_NEXT routine
;========================================================================
;
; Main body
;
;========================================================================
MAIN:
	; Initialize Stack Pointer
	mov	SP, #0040h	; set SP to the second row of scratch pad area
	; Initialize internal hardware
	acall 	INIT		; initialize internal ports, timers, etc
	; Choose melody to play
	acall	SET_MELODY	; choose melody to play
	; Initialize flags
	clr	bMelTerm	; clear melody expiration bit
	mov	R2, #0		; reset note counter
	; Melody playing loop
PLAY_LOOP:
	acall	READ_NOTE	; read 2 bytes of melody (one note)
	inc	R2		; increment note counter
	jb	bMelTerm,TERM_PLAY; check whether end of melody is reached
	acall	PLAY_NOTE	; call note playing routine
	ajmp	PLAY_LOOP	; go to next note
	; Termination of melody
TERM_PLAY:
	acall	CHOOSE_NEXT	; choose next melody to play
	setb	P1.0		; set output pin to HIGH state
	setb	P3.0		; enable reset pulse
	orl	PCON, #02h	; set power down mode
	; It will never come here
DUMMY_LOOP:
	nop
	ajmp	DUMMY_LOOP	; just enter infinite loop
	; End of main body
;========================================================================
;
; Note and duration conversion tables
;
;========================================================================
; Table(A) of values loaded in Timer1 (frequency definitions)
TABLE_A:
DB 	011h,01Fh,02Bh,037h,042h,04Dh,057h,061h,06Ah,072h,07Ah,081h
DB	0	; delimiter byte
; End of table A
;========================================================================
; Table(B) holding octave information - number of Timer1 reload cycles
; before P1.0 output level state. These values are loaded in R4 and 30h
TABLE_B:
DB	001h,002h,004h,008h,010h,020h
DB	0	; delimiter byte
; End of table B
;========================================================================
; Table(C) of duration coefficients loaded in Timer0 (duration definitions)
; These values are stored in the following format: TH0:TL0
TABLE_C:
DB	0B5h, 0C8h
DB	09Eh, 058h
DB	06Bh, 090h
DB	03Ch, 0B0h
DB	06Dh, 084h
DB	0	; delimiter byte
; End of table C
;========================================================================
; Table(D) holdind number of Timer0 reload cycles before achieving of given
; duration. These values are loaded in R3
TABLE_D:
DB	01h,02h,03h,04h,05h,06h,07h,08h,09h,0Ah,0Bh,0Ch
DB	0Eh,10h,12h,14h,18h,1Ch,20h,24h,28h,30h,38h,40h,48h
DB	0	; delimiter byte
; End of table D
;========================================================================
; Table NAMES_TBL of start addresses of all the melodies
NAMES_TBL:
DW	EXODUS, BACH1P, FIDDLERE, CIRCUS, KING, PETROV, SOLFE, SOMETHIN
DW	BACH3P, CLAVIER, FANTASY, GYPSY, FIDDLER, GAVOTTE, GODFATHE
DW	INV14V, HAVA, MAMA, MINUET_D, PRELUDE, RUSSIAN, XMASTREE, FASCINAT
DW	MOON_A, GYPSY_E, HAVA_E, INV14V_E, INV14V_N, INV14V_X, INV14V_Y
DW	MINUET_X, MOON_B, MOON_D, MOON_C, MOON_E, MOON_F, MOON_G
; End of table NAMES_TBL
;========================================================================
;
; Melody tables
;
EXODUS:
DB      075h,065h, 004h,06Eh, 074h,065h, 054h,06Eh, 004h,065h, 034h,065h
DB      054h,065h, 024h,068h, 0A5h,062h, 004h,06Eh, 074h,065h, 0A4h,06Eh
DB      094h,065h, 0A4h,065h, 003h,065h, 094h,068h, 054h,062h, 074h,070h
DB      0, 0
BACH1P:
DB      093h,063h, 002h,061h, 093h,061h, 043h,063h, 093h,061h, 043h,061h
DB      003h,063h, 043h,061h, 003h,061h, 094h,067h, 044h,061h, 094h,061h
DB      003h,061h, 094h,061h, 0B4h,061h, 094h,061h, 0B4h,061h, 094h,061h
DB      084h,061h, 0B4h,061h, 023h,061h, 0B4h,061h, 003h,061h, 0B4h,061h
DB      003h,061h, 094h,061h, 093h,063h, 002h,061h, 093h,061h, 043h,063h
DB      093h,061h, 043h,061h, 003h,063h, 043h,061h, 003h,061h, 094h,067h
DB      003h,061h, 0B4h,061h, 003h,063h, 003h,061h, 0B4h,061h, 003h,063h
DB      093h,063h, 003h,062h, 023h,060h, 003h,063h, 0B4h,063h
DB      0, 0
FIDDLERE:
DB      044h,065h, 054h,065h, 074h,065h, 084h,065h, 084h,065h, 084h,065h
DB      084h,065h, 084h,065h, 0A4h,062h, 003h,062h, 0A4h,065h, 084h,065h
DB      074h,065h, 074h,062h, 074h,062h, 074h,062h, 074h,062h, 074h,062h
DB      074h,062h, 074h,065h, 000h,065h, 000h,065h, 074h,065h, 054h,065h
DB      054h,065h, 054h,065h, 054h,065h, 034h,065h, 024h,065h, 004h,065h
DB      034h,065h, 074h,070h
DB      0, 0
CIRCUS:
DB      0B3h,060h, 005h,060h, 002h,081h, 000h,060h, 075h,060h, 0B3h,081h
DB      000h,060h, 073h,060h, 0A3h,060h, 000h,060h, 0B3h,060h, 075h,060h
DB      0A3h,060h, 000h,060h, 093h,060h, 005h,060h, 083h,081h, 000h,060h
DB      075h,060h, 073h,081h, 000h,060h, 073h,060h, 063h,062h, 075h,060h
DB      073h,081h, 000h,060h, 005h,060h, 093h,081h, 000h,060h, 075h,060h
DB      083h,081h, 000h,060h, 073h,060h, 073h,060h, 000h,060h, 083h,060h
DB      075h,060h, 073h,060h, 000h,060h, 063h,060h, 005h,060h, 053h,081h
DB      000h,060h, 075h,060h, 043h,081h, 000h,060h, 073h,060h, 033h,062h
DB      075h,060h, 043h,081h, 000h,060h, 0B3h,060h, 073h,081h, 000h,060h
DB      075h,060h, 053h,060h, 000h,060h, 053h,060h, 073h,060h, 013h,062h
DB      075h,060h, 023h,081h, 000h,060h, 0B3h,060h, 073h,081h, 000h,060h
DB      075h,060h, 053h,060h, 000h,060h, 053h,060h, 073h,060h, 013h,062h
DB      075h,060h, 023h,081h, 000h,060h, 055h,060h, 0B4h,060h, 000h,060h
DB      003h,060h, 045h,060h, 013h,060h, 000h,060h, 023h,060h, 025h,060h
DB      033h,060h, 000h,060h, 043h,060h, 005h,060h, 053h,060h, 000h,060h
DB      063h,060h, 0B3h,060h, 073h,060h, 000h,060h, 083h,060h, 093h,060h
DB      093h,060h, 000h,060h, 0B3h,060h, 073h,060h, 093h,062h, 0B3h,060h
DB      073h,081h
DB      0, 0
KING:
DB      023h,063h, 043h,063h, 053h,063h, 073h,063h, 093h,063h, 053h,063h
DB      093h,067h, 083h,063h, 043h,063h, 083h,067h, 073h,063h, 033h,063h
DB      073h,067h, 023h,063h, 043h,063h, 053h,063h, 073h,063h, 093h,063h
DB      053h,063h, 093h,063h, 022h,063h, 002h,063h, 093h,063h, 053h,063h
DB      093h,063h, 002h,06Dh
DB      0, 0
PETROV:
DB      043h,067h, 033h,063h, 043h,067h, 000h,067h, 053h,067h, 003h,063h
DB      043h,067h, 000h,067h, 033h,063h, 043h,063h, 053h,063h, 043h,063h
DB      023h,063h, 003h,063h, 0B4h,063h, 094h,063h, 003h,06Bh, 0B4h,067h
DB      000h,067h, 023h,067h, 013h,063h, 023h,067h, 000h,067h, 053h,067h
DB      0B4h,063h, 023h,067h, 000h,067h, 053h,070h, 043h,063h, 023h,063h
DB      0B4h,063h, 003h,06Dh
DB      0, 0
SOLFE:
DB      034h,061h, 004h,061h, 034h,061h, 074h,061h, 003h,061h, 033h,061h
DB      023h,061h, 003h,061h, 0B4h,061h, 074h,061h, 0B4h,061h, 023h,061h
DB      073h,061h, 053h,061h, 033h,061h, 023h,061h, 033h,061h, 003h,061h
DB      033h,061h, 073h,061h, 002h,061h, 032h,061h, 022h,061h, 002h,061h
DB      022h,061h, 002h,061h, 0B3h,061h, 093h,061h, 073h,061h, 053h,061h
DB      033h,061h, 023h,061h, 033h,061h, 003h,061h, 033h,061h, 073h,061h
DB      002h,061h, 032h,061h, 022h,061h, 002h,061h, 0B3h,061h, 073h,061h
DB      0B3h,061h, 022h,061h, 072h,061h, 052h,061h, 032h,061h, 022h,061h
DB      032h,061h, 002h,061h, 032h,061h, 072h,061h, 001h,061h, 031h,061h
DB      021h,061h, 001h,061h, 021h,061h, 001h,061h, 0B2h,061h, 092h,061h
DB      072h,061h, 052h,061h, 032h,065h
DB      0, 0
SOMETHIN:
DB      003h,067h, 094h,063h, 044h,06Dh, 000h,067h, 044h,063h, 094h,063h
DB      003h,063h, 0B4h,063h, 094h,063h, 0B4h,067h, 084h,063h, 044h,06Dh
DB      000h,06Dh, 094h,063h, 0A4h,063h, 094h,063h, 074h,063h, 044h,063h
DB      024h,063h, 014h,06Dh, 000h,063h, 014h,067h, 024h,067h, 044h,067h
DB      074h,063h, 054h,063h, 044h,063h, 074h,06Dh
DB      0, 0
BACH3P:
DB      024h,063h, 094h,063h, 054h,063h, 024h,063h, 094h,063h, 054h,063h
DB      024h,063h, 094h,063h, 054h,063h, 024h,063h, 094h,063h, 054h,063h
DB      024h,063h, 0A4h,063h, 074h,063h, 024h,063h, 0A4h,063h, 074h,063h
DB      024h,063h, 0A4h,063h, 074h,063h, 024h,063h, 0A4h,063h, 074h,063h
DB      014h,063h, 074h,063h, 044h,063h, 014h,063h, 074h,063h, 044h,063h
DB      014h,063h, 0A4h,063h, 074h,063h, 044h,063h, 094h,063h, 074h,063h
DB      054h,063h, 024h,063h, 054h,063h, 094h,063h, 054h,063h, 094h,063h
DB      023h,063h, 094h,063h, 023h,063h, 053h,063h
DB      0, 0
CLAVIER:
DB      0B5h,062h, 024h,062h, 074h,062h, 023h,062h, 073h,062h, 074h,062h
DB      023h,062h, 073h,062h, 0B5h,062h, 004h,062h, 044h,062h, 074h,062h
DB      003h,062h, 044h,062h, 074h,062h, 003h,062h, 0B5h,062h, 004h,062h
DB      044h,062h, 074h,062h, 003h,062h, 044h,062h, 074h,062h, 003h,062h
DB      095h,062h, 004h,062h, 044h,062h, 074h,062h, 003h,062h, 044h,062h
DB      074h,062h, 003h,062h, 095h,062h, 004h,062h, 044h,062h, 074h,062h
DB      003h,062h, 044h,062h, 074h,062h, 003h,062h, 025h,062h, 095h,062h
DB      024h,062h, 064h,062h, 003h,062h, 024h,062h, 064h,062h, 003h,062h
DB      025h,062h, 095h,062h, 024h,062h, 064h,062h, 003h,062h, 024h,062h
DB      064h,062h, 003h,062h, 075h,062h, 0B5h,062h, 024h,062h, 074h,062h
DB      0B4h,062h, 024h,062h, 074h,062h, 0B4h,062h
DB      0, 0
FANTASY:
DB      022h,062h, 093h,062h, 053h,062h, 093h,062h, 053h,062h, 023h,062h
DB      022h,062h, 093h,062h, 053h,062h, 093h,062h, 053h,062h, 023h,062h
DB      043h,062h, 093h,062h, 043h,062h, 013h,062h, 043h,062h, 013h,062h
DB      094h,062h, 013h,062h, 043h,062h, 0A4h,062h, 094h,062h, 074h,062h
DB      054h,062h, 094h,062h, 023h,062h, 094h,062h, 023h,062h, 053h,062h
DB      023h,062h, 053h,062h, 093h,062h, 053h,062h, 093h,062h, 022h,065h
DB      0, 0
GYPSY:
DB      075h,062h, 074h,062h, 0A4h,062h, 023h,062h, 043h,065h, 023h,065h
DB      055h,062h, 054h,062h, 094h,062h, 023h,062h, 043h,065h, 023h,065h
DB      045h,062h, 054h,062h, 094h,062h, 013h,062h, 043h,065h, 013h,065h
DB      023h,062h, 053h,062h, 043h,062h, 023h,062h, 094h,062h, 054h,062h
DB      044h,062h, 024h,062h, 075h,062h, 074h,062h, 0A4h,062h, 023h,062h
DB      043h,062h, 023h,062h, 043h,062h, 023h,062h, 055h,062h, 054h,062h
DB      094h,062h, 023h,062h, 043h,062h, 023h,062h, 043h,062h, 023h,062h
DB      045h,062h, 074h,062h, 094h,062h, 013h,062h, 043h,062h, 013h,062h
DB      043h,062h, 013h,062h, 023h,065h, 094h,065h, 054h,065h, 024h,065h
DB      0, 0
FIDDLER:
DB      074h,062h, 054h,062h, 074h,062h, 054h,062h, 044h,065h, 004h,065h
DB      000h,065h, 044h,062h, 054h,062h, 074h,062h, 054h,062h, 074h,062h
DB      054h,062h, 044h,062h, 054h,062h, 074h,062h, 094h,062h, 0A4h,062h
DB      094h,062h, 0A4h,062h, 094h,062h, 074h,065h
DB      0, 0
GAVOTTE:
DB      075h,061h, 022h,061h, 075h,061h, 042h,061h, 074h,060h, 0B4h,060h
DB      023h,060h, 022h,060h, 074h,060h, 0B4h,060h, 023h,060h, 0B3h,060h
DB      025h,061h, 002h,061h, 025h,061h, 022h,061h, 064h,060h, 003h,060h
DB      023h,060h, 002h,060h, 064h,060h, 003h,060h, 023h,060h, 093h,060h
DB      075h,063h, 073h,063h, 074h,060h, 0B4h,060h, 023h,060h, 062h,060h
DB      074h,060h, 0B4h,060h, 023h,060h, 072h,060h, 075h,063h, 073h,063h
DB      074h,061h, 0B4h,061h, 023h,061h, 000h,061h, 095h,061h, 002h,061h
DB      095h,061h, 022h,061h, 003h,060h, 063h,060h, 093h,060h, 002h,060h
DB      003h,060h, 063h,060h, 093h,060h, 093h,060h, 075h,061h, 0B3h,061h
DB      075h,061h, 002h,061h, 074h,060h, 0B4h,060h, 023h,060h, 0B3h,060h
DB      074h,060h, 0B4h,060h, 023h,060h, 073h,060h, 025h,063h, 093h,063h
DB      094h,060h, 023h,060h, 063h,060h, 012h,060h, 094h,060h, 023h,060h
DB      063h,060h, 022h,060h, 025h,063h, 023h,067h
DB      0, 0
GODFATHE:
DB      044h,067h, 094h,067h, 003h,067h, 0B4h,067h, 094h,067h, 003h,067h
DB      094h,067h, 0B4h,067h, 094h,067h, 054h,067h, 074h,067h, 044h,070h
DB      000h,067h, 044h,067h, 094h,067h, 003h,067h, 0B4h,067h, 094h,067h
DB      003h,067h, 094h,067h, 0B4h,067h, 094h,067h, 054h,067h, 044h,067h
DB      024h,070h
DB      0, 0
INV14V:
DB      0A3h,081h, 002h,081h, 022h,081h, 002h,081h, 0A3h,062h, 052h,062h
DB      022h,062h, 0A2h,062h, 052h,062h, 022h,062h, 052h,081h, 032h,081h
DB      022h,081h, 032h,081h, 052h,062h, 0A3h,062h, 022h,062h, 053h,062h
DB      083h,062h, 073h,062h, 033h,081h, 053h,081h, 073h,081h, 053h,081h
DB      033h,062h, 0A3h,062h, 073h,062h, 032h,062h, 0A3h,062h, 073h,062h
DB      0A3h,081h, 083h,081h, 073h,081h, 083h,081h, 0A3h,062h, 033h,062h
DB      073h,062h, 003h,062h, 033h,062h, 094h,062h, 003h,081h, 023h,081h
DB      033h,081h, 023h,081h, 003h,062h, 093h,062h, 053h,062h, 002h,062h
DB      093h,062h, 032h,062h, 053h,081h, 073h,081h, 093h,081h, 073h,081h
DB      053h,062h, 002h,062h, 093h,062h, 052h,062h, 002h,062h, 022h,06Bh
DB      0, 0
HAVA:
DB      024h,065h, 024h,065h, 000h,081h, 064h,062h, 034h,062h, 024h,062h
DB      064h,065h, 064h,065h, 000h,081h, 094h,062h, 074h,062h, 064h,062h
DB      074h,065h, 074h,065h, 000h,081h, 0A4h,062h, 094h,062h, 074h,062h
DB      064h,065h, 034h,065h, 024h,06Bh, 024h,065h, 024h,065h, 000h,081h
DB      064h,062h, 034h,062h, 024h,062h, 064h,065h, 064h,065h, 000h,081h
DB      094h,062h, 074h,062h, 064h,062h, 074h,065h, 074h,065h, 000h,081h
DB      0A4h,062h, 094h,062h, 074h,062h, 064h,065h, 034h,065h, 024h,06Bh
DB      0, 0
MAMA:
DB      093h,061h, 073h,061h, 053h,061h, 043h,061h, 023h,067h, 093h,061h
DB      073h,061h, 053h,061h, 043h,061h, 023h,067h, 023h,063h, 0A3h,06Bh
DB      023h,063h, 0A3h,06Bh, 073h,061h, 053h,061h, 043h,061h, 023h,061h
DB      003h,067h, 073h,061h, 053h,061h, 043h,061h, 023h,061h, 003h,067h
DB      003h,063h, 093h,06Bh, 003h,063h, 093h,06Bh
DB      0, 0
MINUET_D:
DB      093h,067h, 052h,063h, 042h,063h, 022h,063h, 012h,063h, 022h,067h
DB      093h,067h, 0A3h,067h, 013h,063h, 043h,063h, 073h,063h, 0A3h,063h
DB      093h,063h, 073h,063h, 053h,067h, 043h,063h, 053h,063h, 023h,067h
DB      053h,067h, 0A3h,063h, 093h,063h, 022h,063h, 002h,063h, 052h,067h
DB      042h,063h, 022h,063h, 002h,063h, 0A3h,063h, 093h,063h, 0A3h,061h
DB      002h,061h, 053h,067h, 043h,067h, 053h,06Dh
DB      0, 0
PRELUDE:
DB      002h,061h, 0B3h,061h, 002h,061h, 073h,061h, 002h,061h, 0B3h,061h
DB      002h,061h, 043h,061h, 003h,061h, 0B4h,061h, 003h,061h, 074h,061h
DB      003h,061h, 0B4h,061h, 003h,061h, 074h,061h, 002h,061h, 0B3h,061h
DB      002h,061h, 073h,061h, 002h,061h, 0B3h,061h, 002h,061h, 093h,061h
DB      053h,061h, 043h,061h, 053h,061h, 003h,061h, 053h,061h, 043h,061h
DB      053h,061h, 023h,061h, 022h,061h, 002h,061h, 022h,061h, 093h,061h
DB      022h,061h, 002h,061h, 022h,061h, 0B3h,061h, 073h,061h, 063h,061h
DB      073h,061h, 023h,061h, 073h,061h, 063h,061h, 073h,061h, 043h,061h
DB      042h,061h, 022h,061h, 042h,061h, 0B3h,061h, 042h,061h, 022h,061h
DB      042h,061h, 002h,061h, 093h,061h, 083h,061h, 093h,061h, 043h,061h
DB      093h,061h, 083h,061h, 093h,062h
DB      0, 0
RUSSIAN:
DB      003h,062h, 003h,062h, 003h,062h, 0B4h,062h, 094h,062h, 044h,062h
DB      024h,062h, 024h,062h, 0B4h,062h, 094h,062h, 074h,062h, 054h,062h
DB      044h,062h, 044h,062h, 044h,062h, 054h,062h, 064h,062h, 084h,062h
DB      094h,06Bh, 044h,062h, 000h,062h, 003h,062h, 003h,062h, 003h,062h
DB      0B4h,062h, 094h,062h, 044h,062h, 024h,062h, 024h,062h, 0B4h,062h
DB      094h,062h, 074h,062h, 054h,062h, 074h,062h, 074h,062h, 074h,062h
DB      084h,062h, 094h,062h, 0B4h,062h, 003h,06Bh, 094h,062h
DB      0, 0
XMASTREE:
DB      004h,067h, 054h,065h, 054h,061h, 054h,065h, 000h,020h, 074h,065h
DB      094h,065h, 094h,061h, 094h,06Bh, 000h,020h, 094h,063h, 074h,063h
DB      094h,063h, 0A4h,067h, 044h,067h, 074h,067h, 054h,067h, 003h,063h
DB      003h,063h, 094h,063h, 023h,06Bh, 003h,063h, 003h,063h, 0A4h,063h
DB      0A4h,06Bh, 0A4h,063h, 0A4h,063h, 074h,063h, 003h,06Bh, 0A4h,063h
DB      0A4h,063h, 094h,063h, 094h,067h
DB      0, 0
FASCINAT:
DB      0B5h,063h, 004h,063h, 044h,063h, 074h,063h, 003h,070h, 0B4h,06Dh
DB      003h,067h, 0B4h,072h, 0B5h,063h, 004h,063h, 044h,063h, 074h,063h
DB      003h,067h, 0B4h,067h, 003h,067h, 0B4h,067h, 094h,06Bh, 0B4h,063h
DB      094h,072h, 014h,063h, 024h,063h, 054h,063h, 094h,063h, 023h,072h
DB      014h,063h, 024h,063h, 054h,063h, 094h,063h, 003h,072h, 014h,063h
DB      024h,063h, 054h,063h, 094h,063h, 023h,067h, 003h,067h, 0B4h,067h
DB      094h,067h, 054h,06Bh, 024h,063h, 004h,06Dh, 0B5h,06Dh
DB      0, 0
MOON_A:
DB      085h,081h, 014h,081h, 044h,081h, 084h,081h, 014h,081h, 044h,081h
DB      084h,081h, 013h,081h, 044h,081h, 084h,081h, 013h,081h, 043h,081h
DB      084h,081h, 013h,081h, 043h,081h, 083h,081h, 013h,081h, 043h,081h
DB      083h,081h, 012h,081h, 043h,081h, 083h,081h, 012h,081h, 042h,081h
DB      083h,081h, 012h,081h, 042h,081h, 082h,062h, 082h,062h, 000h,081h
DB      085h,081h, 004h,081h, 034h,081h, 084h,081h, 004h,081h, 034h,081h
DB      084h,081h, 003h,081h, 034h,081h, 084h,081h, 003h,081h, 033h,081h
DB      084h,081h, 004h,081h, 034h,081h, 083h,081h, 003h,081h, 033h,081h
DB      083h,081h, 002h,081h, 033h,081h, 083h,081h, 002h,081h, 032h,081h
DB      083h,081h, 002h,081h, 032h,081h, 082h,062h, 082h,062h, 014h,081h
DB      054h,081h, 084h,081h, 013h,081h, 054h,081h, 084h,081h, 013h,081h
DB      053h,081h, 084h,081h, 013h,081h, 053h,081h, 083h,081h, 013h,081h
DB      053h,081h, 083h,081h, 012h,081h, 053h,081h, 083h,081h, 012h,081h
DB      052h,081h, 083h,081h, 012h,081h, 052h,081h, 082h,081h, 012h,081h
DB      052h,081h, 082h,081h, 011h,081h, 011h,062h
DB      0, 0
GYPSY_E:
DB      003h,065h, 0A4h,065h, 003h,065h, 0A4h,065h, 0A4h,065h, 094h,065h
DB      094h,06Bh, 094h,065h, 074h,065h, 094h,065h, 074h,065h, 074h,065h
DB      054h,065h, 054h,06Bh, 044h,065h, 044h,065h, 003h,065h, 0A4h,065h
DB      0A4h,065h, 094h,065h, 094h,06Bh, 094h,065h, 094h,065h, 053h,065h
DB      013h,065h, 043h,065h, 023h,065h, 023h,06Bh
DB      0, 0
HAVA_E:
DB      074h,06Bh, 000h,081h, 074h,06Bh, 000h,081h, 074h,065h, 0A4h,065h
DB      0A4h,06Bh, 074h,062h, 074h,062h, 0A4h,062h, 094h,062h, 074h,062h
DB      0A4h,062h, 094h,062h, 074h,062h, 074h,062h, 074h,062h, 0A4h,062h
DB      094h,062h, 074h,062h, 0A4h,062h, 094h,062h, 074h,062h, 094h,062h
DB      094h,062h, 003h,062h, 0A4h,062h, 094h,062h, 003h,062h, 0A4h,062h
DB      094h,062h, 094h,062h, 094h,062h, 003h,062h, 0A4h,062h, 094h,062h
DB      003h,062h, 0A4h,062h, 094h,062h, 000h,062h, 094h,062h, 023h,065h
DB      094h,062h, 094h,062h, 023h,065h, 094h,062h, 003h,062h, 0A4h,062h
DB      094h,062h, 074h,06Bh
DB      0, 0
INV14V_E:
DB      072h,081h, 052h,081h, 032h,081h, 052h,081h, 072h,062h, 002h,065h
DB      000h,065h, 000h,062h, 052h,081h, 032h,081h, 022h,081h, 032h,081h
DB      052h,062h, 0A3h,065h, 000h,065h, 000h,062h, 033h,081h, 023h,081h
DB      003h,081h, 023h,081h, 033h,062h, 093h,062h, 002h,081h, 0A3h,081h
DB      093h,081h, 0A3h,081h, 002h,062h, 053h,065h
DB      0, 0
INV14V_N:
DB      003h,081h, 023h,081h, 033h,081h, 023h,081h, 003h,062h, 002h,068h
DB      0A3h,062h, 083h,062h, 053h,081h, 073h,081h, 083h,081h, 073h,081h
DB      053h,062h, 052h,068h, 032h,062h, 022h,062h, 0A3h,081h, 002h,081h
DB      022h,081h, 002h,081h, 0A3h,062h, 0A2h,068h, 082h,062h, 072h,062h
DB      0A2h,081h, 082h,081h, 072h,081h, 082h,081h, 0A2h,062h, 032h,062h
DB      072h,081h, 052h,081h, 032h,081h, 052h,081h, 072h,062h, 002h,062h
DB      032h,081h, 022h,081h, 002h,081h, 022h,081h, 032h,062h, 093h,062h
DB      002h,081h, 022h,081h, 032h,081h, 022h,068h
DB      0, 0
INV14V_X:
DB      053h,081h, 073h,081h, 093h,081h, 073h,081h, 053h,062h, 002h,062h
DB      093h,062h, 052h,062h, 002h,062h, 092h,062h, 032h,081h, 022h,081h
DB      002h,081h, 022h,081h, 032h,062h, 093h,062h, 002h,062h, 063h,062h
DB      093h,062h, 0A3h,065h, 022h,065h, 0A3h,065h, 073h,065h, 083h,065h
DB      052h,065h, 093h,065h, 053h,065h, 073h,062h, 003h,081h, 023h,081h
DB      033h,081h, 023h,081h, 003h,062h, 073h,062h, 033h,062h, 002h,062h
DB      073h,062h, 022h,062h, 083h,081h, 073h,081h, 053h,081h, 073h,081h
DB      083h,062h, 023h,062h, 053h,062h, 0B4h,062h, 073h,062h, 033h,065h
DB      0, 0
INV14V_Y:
DB      0A3h,081h, 002h,081h, 022h,081h, 002h,081h, 0A3h,062h, 052h,062h
DB      022h,062h, 0A2h,062h, 052h,062h, 022h,062h, 052h,081h, 032h,081h
DB      022h,081h, 032h,081h, 052h,062h, 0A3h,062h, 032h,062h, 0A3h,062h
DB      032h,062h, 073h,062h, 033h,081h, 053h,081h, 073h,081h, 053h,081h
DB      033h,062h, 0A3h,062h, 073h,062h, 032h,062h, 0A3h,062h, 073h,062h
DB      0A3h,081h, 083h,081h, 073h,081h, 083h,081h, 0A3h,062h, 033h,065h
DB      032h,065h, 032h,062h, 032h,081h, 022h,081h, 002h,081h, 022h,081h
DB      032h,062h, 033h,065h, 032h,065h, 032h,062h, 002h,081h, 022h,081h
DB      032h,081h, 022h,081h, 002h,062h, 052h,062h, 022h,081h, 002h,081h
DB      0A3h,081h, 002h,081h, 022h,062h, 053h,062h, 0A3h,062h, 002h,062h
DB      083h,062h, 0A3h,070h
DB      0, 0
MINUET_X:
DB      092h,067h, 053h,063h, 092h,063h, 072h,063h, 052h,063h, 042h,061h
DB      052h,061h, 072h,063h, 002h,06Dh, 052h,067h, 023h,063h, 052h,063h
DB      042h,063h, 022h,063h, 012h,061h, 022h,061h, 042h,063h, 093h,06Dh
DB      093h,063h, 0B3h,063h, 012h,063h, 022h,063h, 042h,063h, 052h,063h
DB      072h,063h, 042h,063h, 012h,063h, 0A2h,063h, 092h,063h, 072h,063h
DB      052h,061h, 042h,061h, 022h,063h, 042h,067h, 012h,067h, 022h,070h
DB      0, 0
MOON_B:
DB      083h,081h, 082h,081h, 083h,081h, 082h,081h, 0A3h,081h, 082h,081h
DB      002h,081h, 082h,081h, 012h,081h, 082h,081h, 032h,081h, 082h,081h
DB      002h,081h, 082h,081h, 032h,081h, 082h,081h, 012h,081h, 082h,081h
DB      062h,081h, 082h,081h, 042h,081h, 082h,081h, 032h,081h, 082h,081h
DB      012h,081h, 082h,081h, 002h,081h, 082h,081h, 093h,081h, 072h,081h
DB      083h,081h, 082h,081h, 083h,081h, 082h,081h, 083h,081h, 082h,081h
DB      0A3h,081h, 082h,081h, 002h,081h, 082h,081h, 012h,081h, 082h,081h
DB      032h,081h, 082h,081h, 002h,081h, 082h,081h, 032h,081h, 082h,081h
DB      012h,081h, 082h,081h, 062h,081h, 082h,081h, 042h,081h, 082h,081h
DB      032h,081h, 082h,081h, 012h,081h, 082h,081h, 002h,081h, 082h,081h
DB      093h,081h, 072h,081h, 083h,081h, 082h,081h, 093h,081h, 072h,081h
DB      083h,081h, 082h,081h, 093h,081h, 072h,081h, 083h,081h, 082h,081h
DB      093h,081h, 072h,081h, 083h,081h, 082h,081h, 093h,081h, 072h,081h
DB      082h,065h, 084h,06Eh
DB      0, 0
MOON_D:
DB      092h,081h, 042h,081h, 062h,081h, 082h,081h, 092h,081h, 0B2h,081h
DB      011h,081h, 0B2h,081h, 092h,081h, 042h,081h, 062h,081h, 082h,081h
DB      092h,081h, 0B2h,081h, 011h,081h, 0B2h,081h, 092h,081h, 042h,081h
DB      062h,081h, 082h,081h, 092h,081h, 0B2h,081h, 011h,081h, 0B2h,081h
DB      092h,081h, 042h,081h, 062h,081h, 082h,081h, 092h,081h, 0B2h,081h
DB      011h,081h, 021h,081h, 041h,081h, 031h,081h, 041h,081h, 031h,081h
DB      041h,081h, 011h,081h, 0B2h,081h, 092h,081h, 082h,06Bh, 0B2h,000h
DB      082h,060h, 0A2h,081h, 082h,081h, 0A2h,081h, 082h,081h, 0A2h,081h
DB      082h,081h, 0A2h,081h, 082h,065h
DB      0, 0
MOON_C:
DB      0B3h,063h, 083h,081h, 073h,040h, 083h,040h, 0A3h,040h, 083h,063h
DB      073h,068h, 073h,062h, 032h,063h, 073h,081h, 0A3h,065h, 083h,068h
DB      083h,062h, 032h,063h, 083h,081h, 0B3h,065h, 0A3h,068h, 0A3h,062h
DB      032h,063h, 0A3h,081h, 0B3h,062h, 031h,065h, 031h,065h, 031h,062h
DB      0B2h,062h, 082h,062h, 082h,062h, 072h,065h, 072h,065h, 072h,062h
DB      031h,062h, 072h,062h, 0A2h,062h, 082h,065h, 082h,065h, 082h,062h
DB      031h,062h, 082h,062h, 0B2h,062h, 0A2h,065h, 0A2h,065h, 0A2h,062h
DB      031h,062h, 0A2h,062h, 001h,06Bh, 011h,06Bh, 0A2h,065h, 0B2h,06Eh
DB      082h,06Bh, 092h,06Bh, 052h,000h, 072h,060h, 052h,081h, 072h,081h
DB      052h,081h, 072h,081h, 082h,06Eh
DB      0, 0
MOON_E:
DB      083h,062h, 0B3h,062h, 0B3h,062h, 0B3h,062h, 0B3h,062h, 0B3h,062h
DB      0A3h,062h, 083h,062h, 073h,062h, 032h,062h, 032h,062h, 032h,062h
DB      032h,062h, 032h,062h, 032h,062h, 032h,062h, 032h,062h, 0B3h,062h
DB      0B3h,062h, 0B3h,062h, 0B3h,062h, 0B3h,062h, 0A3h,062h, 083h,062h
DB      073h,062h, 032h,062h, 032h,062h, 032h,062h, 032h,062h, 032h,062h
DB      032h,062h, 032h,062h, 032h,062h, 032h,062h, 032h,062h, 0B3h,062h
DB      000h,062h, 042h,062h, 042h,062h, 012h,062h, 000h,062h, 032h,062h
DB      032h,062h, 0B3h,062h, 000h,062h, 032h,062h, 032h,062h, 0A3h,062h
DB      0B3h,062h
DB      0, 0
MOON_F:
DB      0B4h,081h, 083h,081h, 033h,081h, 083h,081h, 0B4h,081h, 083h,081h
DB      033h,081h, 083h,081h, 0B4h,081h, 083h,081h, 033h,081h, 083h,081h
DB      0B4h,081h, 083h,081h, 033h,081h, 083h,081h, 003h,081h, 063h,081h
DB      033h,081h, 063h,081h, 003h,081h, 063h,081h, 033h,081h, 063h,081h
DB      003h,081h, 063h,081h, 033h,081h, 063h,081h, 003h,081h, 063h,081h
DB      033h,081h, 063h,081h, 043h,081h, 085h,081h, 014h,081h, 044h,081h
DB      084h,081h, 014h,081h, 044h,081h, 084h,081h, 013h,081h, 044h,081h
DB      084h,081h, 013h,081h, 043h,081h, 084h,081h, 013h,081h, 043h,081h
DB      083h,081h, 013h,081h, 043h,081h, 083h,081h, 012h,081h, 043h,081h
DB      083h,081h, 012h,081h, 042h,081h, 083h,081h, 012h,081h, 042h,081h
DB      082h,062h, 082h,062h
DB      0, 0
MOON_G:
DB      082h,068h, 082h,062h, 092h,062h, 082h,062h, 062h,062h, 062h,062h
DB      082h,062h, 062h,062h, 042h,062h, 042h,062h, 062h,062h, 042h,062h
DB      032h,062h, 032h,062h, 042h,062h, 032h,062h, 012h,062h, 012h,062h
DB      032h,062h, 012h,062h, 002h,06Bh, 012h,06Bh, 003h,040h, 063h,040h
DB      032h,063h, 032h,06Bh, 012h,065h, 002h,065h, 002h,065h, 012h,065h
DB      012h,065h, 003h,040h, 063h,040h, 032h,063h, 032h,06Bh, 043h,062h
DB      012h,062h, 063h,062h, 002h,062h, 063h,062h, 002h,062h, 043h,062h
DB      012h,062h, 043h,062h, 012h,062h, 063h,062h, 032h,062h, 063h,062h
DB      032h,062h, 043h,062h, 012h,062h, 043h,062h, 012h,062h, 094h,040h
DB      013h,040h, 033h,040h, 093h,040h, 093h,062h, 093h,06Eh, 084h,040h
DB      003h,040h, 033h,040h, 083h,040h, 083h,062h, 083h,06Eh, 084h,040h
DB      003h,040h, 033h,040h, 083h,040h, 083h,062h, 083h,06Eh
DB      0, 0
;========================================================================
END