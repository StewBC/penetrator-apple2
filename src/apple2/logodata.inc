;-----------------------------------------------------------------------------
; logodata.inc
; Part of penetrator, the zx spectrum game, made for Apple II
;
; Stefan Wessels, 2019
; This is free and unencumbered software released into the public domain.

;-----------------------------------------------------------------------------
.segment "RODATA"

;-----------------------------------------------------------------------------
; x/y coordinates for lines that spell out Penetrator
dataLogoLines:
.byte $00, $0C, $00, $26
.byte $01, $0D, $07, $13
.byte $06, $14, $01, $19
.byte $02, $21, $08, $1C
.byte $09, $1B, $02, $1B
.byte $02, $1C, $05, $22
.byte $06, $22, $0B, $1D
.byte $0C, $1C, $0C, $17
.byte $0D, $17, $10, $1F
.byte $11, $1E, $11, $16
.byte $14, $1B, $19, $15
.byte $19, $14, $14, $14
.byte $14, $15, $13, $16
.byte $13, $17, $17, $1D
.byte $17, $1C, $1B, $16
.byte $1B, $15, $1D, $09
.byte $1E, $0A, $1E, $1A
.byte $1D, $14, $21, $14
.byte $22, $15, $23, $16
.byte $22, $1B, $24, $12
.byte $25, $11, $28, $11
.byte $28, $12, $29, $13
.byte $25, $19, $26, $17
.byte $27, $16, $29, $16
.byte $29, $17, $2C, $1C
.byte $2B, $1D, $29, $1F
.byte $28, $1F, $27, $1F
.byte $26, $1E, $25, $1D
.byte $25, $1C, $27, $1A
.byte $28, $1A, $29, $1A
.byte $2C, $1C, $2E, $16
.byte $2E, $15, $30, $0E
.byte $31, $10, $31, $22
.byte $2F, $1A, $34, $1A
.byte $32, $21, $35, $1B
.byte $36, $1B, $39, $1E
.byte $39, $1F, $36, $22
.byte $36, $22, $34, $20
.byte $38, $26, $3A, $1C
.byte $3B, $1B, $3C, $1B
.byte $3D, $1B, $3E, $1B
.byte $3E, $1C, $3F, $1D
dataLogoLinesEnd:
