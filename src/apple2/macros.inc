;-----------------------------------------------------------------------------
; macros.inc
; Part of penetrator, the zx spectrum game, made for Apple II
;
; Stefan Wessels, 2019
; This is free and unencumbered software released into the public domain.

;-----------------------------------------------------------------------------
.macro printZAStr sx, sy

    .if .paramcount <> 2
    .error "Parameters issue for macro printZAStr"
    .endif

    .local textX, textY 
    
    textX = tempBlock + 10
    textY = tempBlock + 11

    lda sx
    sta textX
    lda sy
    sta textY
    jsr textOut

.endmacro 

;-----------------------------------------------------------------------------
.macro print szText, sx, sy

    .if .paramcount <> 3
    .error "Parameters issue for macro print"
    .endif

    .local zaStrL, zaStrH, textX, textY 
    
    zaStrL     = tempBlock + 1
    zaStrH     = tempBlock + 2

    lda #<szText
    sta zaStrL
    lda #>szText
    sta zaStrH 

    printZAStr sx, sy

.endmacro

;-----------------------------------------------------------------------------
.macro printBCD number, digits, force, sx, sy

    .if .paramcount <> 5
    .error "Parameters issue for macro printBCD"
    .endif

    .local zaStrL, zaStrH
    
    zaStrL     = tempBlock + 1
    zaStrH     = tempBlock + 2

    lda #<number
    sta zaStrL
    lda #>number
    sta zaStrH 
    ldy digits 
    ldx force
    jsr textBCDtoSZ

    print textNumber, sx, sy

.endmacro 

;-----------------------------------------------------------------------------
.macro printBig szText, sx, sy, wx, hy

    .if .paramcount <> 5
    .error "Parameters issue for macro printBig"
    .endif

    .local zaStrL, zaStrH, width, textX, textY, height
    
    zaStrL = tempBlock + 1
    zaStrH = tempBlock + 2
    textX  = tempBlock + 12
    textY  = tempBlock + 13
    width  = tempBlock + 14                    ; shared with drawPlotXY
    height = tempBlock + 15


    lda #<szText
    sta zaStrL
    lda #>szText
    sta zaStrH 
    lda sx 
    sta textX
    lda sy 
    sta textY
    lda wx 
    sta width 
    lda hy 
    sta height
    jsr textBigPrint

.endmacro

;-----------------------------------------------------------------------------
.macro wait time

    .if .paramcount <> 1
    .error "Parameters issue for macro wait"
    .endif

    .local loop, loopo

    lda time 
    sta tempBlock + 20
    ldx #0
loopo:
    ldy #$10
loop:
    dex 
    bne loop 
    dey 
    bne loop
    dec tempBlock + 20
    bne loopo

.endmacro 
