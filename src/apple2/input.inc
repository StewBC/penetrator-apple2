;-----------------------------------------------------------------------------
; input.inc
; Part of penetrator, the zx spectrum game, made for Apple II
;
; Stefan Wessels, 2019
; This is free and unencumbered software released into the public domain.
;-----------------------------------------------------------------------------
.segment "CODE"

;-----------------------------------------------------------------------------
.proc inputReadJoystick

    joyMask = tempBlock + 21

    lda #0                                      ; assume no buttons
    sta joyMask                                 ; init the mask

    lda $c061                                   ; check RDBTN0
    cmp #$80                                    ; fully active
    bne :+                                      ; no, ignore
    lda #KEY_FIRE                               ; RDBTN0 maps to fire
    sta joyMask                                 ; init the mask

:
    lda $c062                                   ; check BUTN1
    cmp #$80                                    ; fully active
    bne :+                                      ; no, ignore
    lda joyMask
    ora #KEY_BOMB                               ; BUTN1 maps to Bomb
    sta joyMask                                 ; update the mask
    
:
    lda $c070                                   ; Reset the input

    ldy #0                                      ; zero the direction counters
    ldx #0

xchk:                                           ; get bit 8 clear in c064 and c065
    lda $c064                                   ; load left/right
    bpl goY                                     ; bit 8 not set?
    nop                                         ; wast time
    inx                                         ; count the ticks
    bne :+                                      ; overflow?
    dex                                         ; keep $ff
:   lda $c065                                   ; load up/down
    bmi ychk                                    ;  
    bpl xchk                                    ; 
ychk:
    iny                                         ; inc y counter
    bne :+                                      ; overflow?
    dey                                         ; keep $ff
:   jmp xchk                                    ; goX

goY:
    lda $c065                                   ; load up/down
    bmi ychk                                    ; bit 8 set?

    tya                                         ; y is the direction analog
    cmp #$60                                    ; >= $60 is down
    bcs down
    cmp #$10                                    ; < $10 is up, rest is deadzone
    bcs doX
    lda #KEY_UP                                 ; < $10 is KEY_UP digital
    bne :+
down:
    lda #KEY_DOWN                               ; >= $60 is KEY_DOWN digital
:
    ora joyMask                                 ; update the mask
    sta joyMask                                 ; for the Y direction

doX:
    txa                                         ; turn x into digital
    cmp #$60                                    ; >= $60 is right
    bcs right
    cmp #$10                                    ; < $10 is left, rest is deadzone
    bcs leave
    lda #KEY_LEFT                               ; < $10 is KEY_LEFT digital
    bne :+
right:
    lda #KEY_RIGHT                              ; > $60 is KEY_RIGHT
:
    ora joyMask 
    eor #$ff
    sta joyMask                                 ; update the joy Mask with X digital
    rts                                         ; return with joyMask in accumulator

leave:
    lda joyMask                                 ; return with joyMask in accumulator
    eor #$ff
    sta joyMask                                 ; update the joy Mask with X digital
    rts 

.endproc

;-----------------------------------------------------------------------------
; Reads the joystick and moves the player.
.proc inputGame

    zaRawInput = tempBlock + 1
    zaRawEor   = tempBlock + 2

    lda fireCoolDown 
    beq :+
    dec fireCoolDown
:
    jsr inputReadJoystick

	sta zaRawInput                              ; save the input
    eor #$ff                                    ; invert the bits
    sta zaRawEor                                ; save this state too

    lda zaRawInput                              ; no vertical move so process vertical joystick

up:
    bit Bit4Mask                                ; set when moving up
    bne down
    lda playerShipY                             ; and move the ship up 2 rows
    sec 
    sbc #3
    cmp #(SHIP_HEIGHT+WORLD_START)              ; limit at the top
    bcc preLeft                                 ; if too high, just ignore saving the move
    sta playerShipY                             ; save the new position
    bcs preLeft                                 ; up, so skip down and check horizontal

down:
    bit Bit3Mask                                ; ser for joystick down
    bne left
    lda playerShipY                             ; move the ship 2 rows down
    clc 
    adc #3
    cmp #(WORLD_END+1)                          ; limit at the bottom
    bcs preBomb                                 ; skip saving the new position if too low
    sta playerShipY                             ; new position okay so save it

preLeft:
    lda zaRawInput                              ; no horizontal movement so process horiz joystick

left:
	bit Bit2Mask                                ; left sets this bit
    bne right                                   ; not set see if right is set
    ldx playerShipX                             ; the ship should move left
    dex                                         ; move left one column
    bmi preBomb                                 ; if off-screen to the left, ignore
    stx playerShipX                             ; save the movement
    lda audioFrame
    ora #AUDIO_MOTOR_REW
    sta audioFrame
    bne preBomb

right:
    bit Bit1Mask                                ; bit set when right joystick active
    bne bomb                                    ; if nothing, go process player movement

    ldx playerShipX                             ; get the ship position
    inx                                         ; move one column to the right
    cpx #(((XSIZE/3)*2) - SHIP_WIDTH - 1)       ; limit to two-thirds toward the right
    bcs preBomb                                 ; if over limit, ignore
    stx playerShipX                             ; save movement
    lda audioFrame
    ora #AUDIO_MOTOR_FWD
    sta audioFrame

preBomb:
    lda zaRawInput

bomb:
	bit Bit8Mask                                ; Button_A = bomb
    bne fire
    lda lastInput                               ; debounce the key for single shot
    and zaRawEor 
    and Bit8Mask
    beq nobomb                                  ; bomb key not down
    lda playerShipY                             ; bomb requested but can't
    cmp #(WORLD_END-BOMB_HEIGHT)                ; drop it when there's no room
    bcs nobomb                                  ; before the world end (else it draws out of bounds)
    ldx #NUM_BOMBS-1                            ; iterate over all bombs
:
    lda bombY, x
    beq drop                                    ; find one that's available
    dex 
    bpl :-
    bmi nobomb                                  ; none available
drop:
    lda playerShipX                             ; bomb is relative to player
    clc 
    adc #2
    sta bombX, x
    lda playerShipY
    adc #6
    sta bombY, x
    lda #BOMB_FRWRD_FRMS                        ; set it to fly forward
    sta bombDir, x
    lda audioFrame
    ora #AUDIO_BOMB_DROP
    sta audioFrame
nobomb:
    lda zaRawInput

fire:
	bit Bit7Mask                                ; Button_B = fire
    bne joyDone
    lda fireCoolDown 
    bne joyDone
    lda lastInput                               ; debounce the key for single shot
    and zaRawEor 
    and Bit7Mask
    beq joyDone
    lda playerShipX                             ; Put even/odd into carry
    lsr 
    lda playerShipX                             ; bullets start relative to ship
    adc #4                                      ; 4 for even, 5 for odd
    adc bulletIndex
    tax 
    lda playerShipY
    sec 
    sbc #4
    sta bulletsBuffer, x                        ; put height into 3 columns
    lda #3
    sta fireCoolDown
    lda audioFrame
    ora #AUDIO_FIRE
    sta audioFrame

joyDone:
    lda zaRawInput                              ; remember the key-state for next time
    sta lastInput                               ; so that fire and bombs can be debounced

    lda KBD                                     ; leave with a key in the accumulator
    bit KBDSTRB                                 ; reset the key

    clc                                         ; keep carry clear between routines
    rts 

.endproc

;-----------------------------------------------------------------------------
.proc inputEdit

    rawInput  = tempBlock + 17
    rawEor    = tempBlock + 18
    brushType = tempBlock + 16

    lda KBD
    bit Bit8Mask 
    beq :+
    bit KBDSTRB
    and #$7f
    bne :++
:
    lda #0
:
    pha 

    jsr inputReadJoystick
    sta rawInput                                ; save the input state

    pla 
    pha 
    cmp #' '
    bne :+
    lda rawInput
    eor #KEY_SELECT
    sta rawInput
    pla 
    pha 

:
    cmp #'B'
    bne :+
    lda rawInput
    eor #KEY_START
    sta rawInput

:
    lda rawInput
    eor #$ff                                    ; invert the bits so 1 means button down
    sta rawEor                                  ; save the inverted bits
    bit Bit6Mask                                ; see if select button is held
    bne Button_B                                ; if it is down, key processing done
    and lastInput                               ; otherwise debounce the keys
    sta rawEor                                  ; and save this state as the key state

 Button_B:
 	bit Bit7Mask                               ; Button_B
    beq Button_A
    lda brushType
    bne radar
    ora #KEY_RIGHT                              ; when pressed, move right as well
    sta rawEor
    jsr editSetTerrainBottom
    jmp :+

radar:
    jsr editSetRadar
:
    lda rawEor                                  ; no horizontal movement so process horiz joystick

Button_A:
	bit Bit8Mask                                ; Button_A
    beq up
    lda brushType
    bne missile
    ora #KEY_RIGHT
    sta rawEor
    jsr editSetTerrainTop
    jmp :+

missile:
    jsr editSetMissile
:
    lda rawEor                                  ; no horizontal movement so process horiz joystick

up:
    bit Bit4Mask                                ; set when moving up
    beq down
    ldx playerShipY 
    cpx #9
    bcc :+
    dex 
    stx playerShipY 
:
    lda rawEor                                  ; no horizontal movement so process horiz joystick

down:
    bit Bit3Mask                                ; ser for joystick down
    beq left
    ldx playerShipY 
    cpx #WORLD_END
    bcs :+
    inx 
    stx playerShipY 
:
    lda rawEor                                  ; no horizontal movement so process horiz joystick

left:
	bit Bit2Mask                                ; left sets this bit
    beq right                                   ; not set see if right is set
    jsr editGameWorldLeft
    lda rawEor                                  ; no horizontal movement so process horiz joystick

right:
    bit Bit1Mask                                ; bit set when right joystick active
    beq Button_Start
    jsr editGameWorldRight

Button_Start:
    lda rawInput                                ; This is a toggle so never auto-repeat
    eor #$ff                                    ; start from source
    and lastInput                               ; and debounce
    bit Bit5Mask                                ; test if Start button down
    beq joyDone
    lda brushType                               ; if down, toggle the brush type between terrain and enemies
    eor #1
    sta brushType 
    jsr uiShowEditLabels                        ; update the HUD

joyDone:
    lda rawInput                                ; preserve this input for next frame as last input
    sta lastInput                               ; so that fire and bombs can be debounced

    pla                                         ; get the keyboard in accumulator
    clc                                         ; keep carry clear between routines
    rts 

.endproc

;-----------------------------------------------------------------------------
; Checks for a user input on keyboard or joystick
; upon exit - acc 0 means nothing, <> 0 means user input detected
.proc inputCheckForInput

    rawInput = tempBlock + 20                   ; internal

    lda KBD
    bit Bit8Mask
    bne done                                    ; if there's a key then done here

    jsr inputReadJoystick                       ; check the joystick
    sta rawInput                                ; keep the raw bits
    eor #$ff                                    ; invert
    and lastInput                               ; debounce
    pha                                         ; save this
    lda rawInput                                ; back up the raw to the last
    sta lastInput
    pla                                         ; and get back the debounced bits
    beq done
    ldx #1
:
    lsr 
    bcs :+
    inx 
    bne :-
:
    txa 
    ; rts 

done:
    and #$7f
    beq :+
    pha 
    lda #AUDIO_UI_TICK
    sta audioFrame
    jsr serviceAudio
    pla 
:
    rts                                         ; upon return 0 means no interaction, anything else is a user input

.endproc

