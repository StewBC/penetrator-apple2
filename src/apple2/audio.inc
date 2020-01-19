;-----------------------------------------------------------------------------
; audio.inc
; Part of penetrator, the zx spectrum game, made for Apple II
;
; Stefan Wessels, 2020
; This is free and unencumbered software released into the public domain.

;-----------------------------------------------------------------------------
.segment "CODE"

;-----------------------------------------------------------------------------
.proc serviceAudio
    
    lda audioMask                               ; get the active channels
    and audioFrame                              ; see if any channel needs to play this frame
    bne makeSounds                              ; make a sound if a channel needs audio
    rts 

makeSounds:
    bit Bit7Mask                                ; AUDIO_EXPLOSION
    bne bombDrop

    bit Bit6Mask                                ; AUDIO_FIRE
    bne fire

    bit Bit5Mask                                ; AUDIO_EXPLOSION
    bne explosion

    bit Bit8432Mask                             ; AUDIO_ENGINE - Not that bit 8 is erroneusly checked
    bne engine                                  ; only because I didn't want to make a new constant

    bit Bit1Mask                                ; AUDIO_UI_TICK
    bne ui

    rts 

explosion:
    ldy audioExplFrame                          ; use the frame as an index
    lda explDelay, y
    tax 
    ldy #$6
    jsr playNote

    inc audioExplFrame                          ; make sure off-screen ticks to end
    lda audioExplFrame
    cmp #%00000011
    bcc :+
    lda #AUDIO_MOTOR_PLAY
    sta audioFrame                              ; turn off the explosion
:
    rts 

bombDrop:
    ldy #$30
    ldx #$10
    jmp playNote

fire:
    ldy #$20
    ldx #$10
    jmp playNote

engine:
    bit Bit4Mask                                ; AUDIO_MOTOR_FWD
    beq :+
    ldy #$25
    ldx #$08
    jmp playNote
:
    bit Bit3Mask                                ; AUDIO_MOTOR_REW
    beq :+
    ldy #$35
    ldx #$08
    jmp playNote
:
    ldy #$30
    ldx #$08
    jmp playNote

ui:
    ldy #$40
    ldx #$08
    jmp playNote

.endproc

;-----------------------------------------------------------------------------
.proc playNote

    delay = tempBlock + 21

    sty delay

loop:
    lda SPEAKER
    ldy delay
:
    nop 
    nop 
    nop 
    nop 
    dey 
    bne :-                                      ; hold for the duration in y
    dex 
    bne loop                                    ; retrigger
    
    lda #AUDIO_EXPLOSION                        ; reset the audio frame (serviced)
    and audioFrame                              ; but keep explosion if it was set
    ora #AUDIO_MOTOR_PLAY                       ; and always add the MOTOR
    sta audioFrame

    rts 

.endproc 
