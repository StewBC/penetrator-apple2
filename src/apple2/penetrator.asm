;-----------------------------------------------------------------------------
; penetrator, the zx spectrum game, made for Apple II
;
; Stefan Wessels, 2019
; This is free and unencumbered software released into the public domain.
;
; Use the ca65 assembler and make to build

;-----------------------------------------------------------------------------
; assembler directives
.debuginfo on
.listbytes unlimited

;-----------------------------------------------------------------------------
.segment "CODE"

;-----------------------------------------------------------------------------
jmp main                                        ; This ends up at $080d (sys 2061's target)

;-----------------------------------------------------------------------------
.include "defs.inc"                             ; constants
.include "macros.inc"                           ; vpoke, vpeek, print* & wait.
.include "zpvars.inc"                           ; Zero Page usage (variables)
.include "input.inc"                            ; Keyboard/Joystick routines
.include "ui.inc"                               ; The front-end code
.include "edit.inc"                             ; In-game editor
.include "game.inc"                             ; All gameplay code
.include "terrain.inc"                          ; Code that writes the terrain to HRG
.include "draw.inc"                             ; Code that writes to HRG
.include "text.inc"                             ; The text and text printing code
.include "file.inc"                             ; LOAD / SAVE routine
.include "variables.inc"                        ; Game Variables (DATA segment)
.include "trndata.inc"                          ; Terrain triplets
.include "fontdata.inc"                         ; The ZA Spectrum font as 2bpp, 2 bytes/char
.include "logodata.inc"                         ; Lines to spell Penetrator and intro graphic
.include "rodata.inc"                           ; Read Only (RODATA segment sprites, etc)
.include "logo.inc"

;-----------------------------------------------------------------------------
.segment "CODE"
.proc main

    jsr mainGameSetup
:
    jsr inputCheckForInput                      ; wait for user interaction
    beq :-
    jsr drawPresent
:
    jsr uiTitleScreen
    jsr uiMainMenu
    jmp :-

.endproc

;-----------------------------------------------------------------------------
.proc mainGameSetup

    ldx #((textHSEnd-textHS) - 1)               ; empty the high score names to spaces
store:
    lda textHS, x
    beq :+                                      ; skip the null terminators
    lda #$20                                    ; load the space (" ") character
    sta textHS, x                               ; write it to the text areas
:
    dex 
    bpl store

    ldx #((highScoresEnd-highScores) - 1)       ; set high score table scores to 0
    lda #$0
:
    sta highScores, x
    dex 
    bpl :-

    ldx #((BitMasksEnd - BitMasks) - 1)
:
    lda BitMasks, x
    sta Bit1Mask, x
    dex 
    bpl :-

    rts 

.endproc
