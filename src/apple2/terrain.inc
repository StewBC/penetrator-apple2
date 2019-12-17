;-----------------------------------------------------------------------------
; terrain.inc
; Part of penetrator, the zx spectrum game, made for Apple II
;
; Stefan Wessels, 2019
; This is free and unencumbered software released into the public domain.

;-----------------------------------------------------------------------------
.segment "CODE"

;-----------------------------------------------------------------------------
.proc terrainShow

    zaIndex    = tempBlock + 1
    zaWorldBot = tempBlock + 2
    zaWorldTop = tempBlock + 3
    zaSide     = tempBlock + 4

    ldy #WORLD_START                            ; start row
    lda #(WORLD_END - WORLD_START + 1)          ; no of rows to clear
    ldx backLayer                               ; inactive (back) layer
    jsr drawClearRows                           ; clean the part where the world must draw

    lda terrainOrigin
    lsr 
    sta zScreenCol

    ldy bufferDraw                              ; make enemyBuffer relative to screen col 0
    dey                                         ; but back up 1 to the last drawn height

    lda worldBBuffer, y                         ; prime the drawing with these heights
    sta zaWorldBot                              ; as the last height is where the col 0
    lda worldTBuffer, y                         ; draw needs to come from
    sta zaWorldTop

    iny                                         ; get back to col 0 equivalent index
    sty zaIndex                                 ; and save this index

loop:
    lda #0
    sta zaSide
    lda worldBBuffer, y
    ldy zaWorldBot                              ; get the last height
    sta zaWorldBot                              ; save new bottom as last
    jsr terrainDrawLeftColumn 

    inc zaSide
    ldy zaIndex
    lda worldTBuffer, y 
    ldy zaWorldTop                              ; get the last height
    sta zaWorldTop                              ; save new bottom as last
    cmp #9
    bcc :+
    jsr terrainDrawLeftColumn

:
    lda #0
    sta zaSide
    inc zaIndex
    ldy zaIndex 
    lda worldBBuffer, y
    ldy zaWorldBot                              ; get the last height
    sta zaWorldBot                              ; save new bottom as last
    jsr terrainDrawRightColumn 

    inc zaSide
    ldy zaIndex
    lda worldTBuffer, y 
    ldy zaWorldTop                              ; get the last height
    sta zaWorldTop                              ; save new bottom as last
    cmp #9
    bcc :+
    jsr terrainDrawRightColumn

:
    ldy zaIndex 
    cpy bufferInsert
    beq done
    iny 
    sty zaIndex
    inc zScreenCol
    bne loop

done:
    clc 
    rts 

.endproc

;-----------------------------------------------------------------------------
; y is last height, a is new height
.proc terrainDrawLeftColumn

    zaSide     = tempBlock + 4
    zaDiff     = tempBlock + 5
    zaDir      = tempBlock + 6                  ; 2 bytes used

    sty zaDiff
    sec 
    sbc zaDiff                                  ; get the delta (new - old)
    tax                                         ; save delta in x
    bcs eqdown                                  ; new .ge. old

up:                                             ; new < old so go up
    lda rowL, y
    adc zScreenCol
    sta writeUpStart + 1
    lda rowH, y
    adc zVramH
    sta writeUpStart + 2
    lda #%00000011                              ; start with a 2 pixel flat area
writeUpStart:
    sta PLACEHOLDER
    dey                                         ; go 1 row up
    inx 
    beq skup                                    ; if at new height now, skip the "up line" portion
:
    lda rowL, y                                 ; draw the "up line" portion now
    adc zScreenCol
    sta writeUp + 1
    lda rowH, y
    adc zVramH
    sta writeUp + 2
    lda #%00000100                              ; write a single pixel up the rows
writeUp:
    sta PLACEHOLDER
    dey                                         ; row up
    inx 
    bne :-                                      ; keep going till old .eq. new

skup:
    lda rowL, y                                 ; finish off the transition
    adc zScreenCol
    sta writeUpEnd + 1
    lda rowH, y
    adc zVramH
    sta writeUpEnd + 2
    lda #%00001000                              ; by writing a pixel to connect to new
writeUpEnd:
    sta PLACEHOLDER                             ; (which will become old, in next column, to draw from)

    lda #$ff
    ldx zaSide
    sta zaDir, x

    rts 

eqdown:
    clc                                         ; carry still set from subtract
    beq eq                                      ; if new .eq. old it's a flat line

down:                                           ; new > old so go down - see up, same but inc|dec is reversed
    lda rowL, y
    adc zScreenCol
    sta writeDownStart + 1
    lda rowH, y
    adc zVramH
    sta writeDownStart + 2
    lda #%00000011
writeDownStart:
    sta PLACEHOLDER
    iny 
    dex 
    beq skdn

:
    lda rowL, y
    adc zScreenCol
    sta writeDown + 1
    lda rowH, y
    adc zVramH
    sta writeDown + 2
    lda #%00000100
writeDown:
    sta PLACEHOLDER
    iny 
    dex 
    bne :-

skdn:
    lda rowL, y
    adc zScreenCol
    sta writeDownEnd + 1
    lda rowH, y
    adc zVramH
    sta writeDownEnd + 2
    lda #%00001000
writeDownEnd:
    sta PLACEHOLDER

    lda #$01
    ldx zaSide
    sta zaDir, x

    rts 

eq:                                             ; flat line from old to new when equal
    lda rowL, y
    adc zScreenCol
    sta writeFlat + 1
    lda rowH, y
    adc zVramH
    sta writeFlat + 2
    lda #%00001111 
writeFlat:                                      ; 4 pixels in a line
    sta PLACEHOLDER

    lda #$00
    ldx zaSide
    sta zaDir, x

    rts 

.endproc

;-----------------------------------------------------------------------------
; y is last height, a is new height
.proc terrainDrawRightColumn

    zaSide     = tempBlock + 4
    zaDiff     = tempBlock + 5
    zaDir      = tempBlock + 6                  ; 2 bytes used
    zaSaveX    = tempBlock + 8

    sty zaDiff
    sec 
    sbc zaDiff                                  ; get the delta (new - old)
    tax                                         ; save delta in x
    bcc up
    jmp eqdown                                  ; new .ge. old

up:                                             ; new < old so go up
    lda rowL, y
    adc zScreenCol
    sta writeUpStart + 1
    sta writeUpStart + 4
    lda rowH, y
    adc zVramH
    sta writeUpStart + 2
    sta writeUpStart + 5
    lda #%00010000                              ; start with a 2 pixel flat area
writeUpStart:
    ora PLACEHOLDER
    sta PLACEHOLDER
    dey                                         ; go 1 row up
    inx 
    beq skup                                    ; if at new height now, skip the "up line" portion

    stx zaSaveX
    ldx zaSide
    lda zaDir, x
    ldx zaSaveX
    cmp #1
    bne fastUp
    clc 
:
    lda rowL, y                                 ; draw the "up line" portion now
    adc zScreenCol
    sta writeUp + 1
    sta writeUp + 4
    lda rowH, y
    adc zVramH
    sta writeUp + 2
    sta writeUp + 5
    lda #%00100000                              ; write a single pixel up the rows
writeUp:
    ora PLACEHOLDER
    sta PLACEHOLDER
    dey                                         ; row up
    inx 
    bne :-                                      ; keep going till old .eq. new
    beq skup

fastUp:
    clc 
:
    lda rowL, y                                 ; draw the "up line" portion now
    adc zScreenCol
    sta writeUpFast + 1
    lda rowH, y
    adc zVramH
    sta writeUpFast + 2
    lda #%00100000                              ; write a single pixel up the rows
writeUpFast:
    sta PLACEHOLDER
    dey                                         ; row up
    inx 
    bne :-                                      ; keep going till old .eq. new

skup:
    lda rowL, y                                 ; finish off the transition
    adc zScreenCol
    sta writeUpEnd + 1
    sta writeUpEnd + 4
    lda rowH, y
    adc zVramH
    sta writeUpEnd + 2
    sta writeUpEnd + 5
    lda #%01000000                              ; by writing a pixel to connect to new
writeUpEnd:
    ora PLACEHOLDER
    sta PLACEHOLDER                             ; (which will become old, in next column, to draw from)
    
    rts 

eqdown:
    clc                                         ; carry still set from subtract
    bne down                                    ; if new .eq. old it's a flat line
    jmp eq

down:                                           ; new > old so go down - see up, same but inc|dec is reversed
    lda rowL, y
    adc zScreenCol
    sta writeDownStart + 1
    sta writeDownStart + 4
    lda rowH, y
    adc zVramH
    sta writeDownStart + 2
    sta writeDownStart + 5
    lda #%00010000
writeDownStart:
    ora PLACEHOLDER
    sta PLACEHOLDER
    iny 
    dex 
    beq skdn

    stx zaSaveX
    ldx zaSide
    lda zaDir, x
    ldx zaSaveX
    cmp #$ff
    bne fastDown
    clc 

:
    lda rowL, y
    adc zScreenCol
    sta writeDown + 1
    sta writeDown + 4
    lda rowH, y
    adc zVramH
    sta writeDown + 2
    sta writeDown + 5
    lda #%00100000
writeDown:
    ora PLACEHOLDER
    sta PLACEHOLDER
    iny 
    dex 
    bne :-
    beq skdn

fastDown:
    clc 
:
    lda rowL, y
    adc zScreenCol
    sta writeDownFast + 1
    lda rowH, y
    adc zVramH
    sta writeDownFast + 2
    lda #%00100000
writeDownFast:
    sta PLACEHOLDER
    iny 
    dex 
    bne :-

skdn:
    lda rowL, y
    adc zScreenCol
    sta writeDownEnd + 1
    sta writeDownEnd + 4
    lda rowH, y
    adc zVramH
    sta writeDownEnd + 2
    sta writeDownEnd + 5
    lda #%01000000
writeDownEnd:
    ora PLACEHOLDER
    sta PLACEHOLDER

    rts 

eq:                                             ; flat line from old to new when equal
    lda rowL, y
    adc zScreenCol
    sta writeFlat + 1
    sta writeFlat + 4
    lda rowH, y
    adc zVramH
    sta writeFlat + 2
    sta writeFlat + 5
    lda #%01110000 
writeFlat:                                      ; 4 pixels in a line
    ora PLACEHOLDER
    sta PLACEHOLDER

    rts 

.endproc
