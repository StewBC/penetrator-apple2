;-----------------------------------------------------------------------------
; game.inc
; Part of penetrator, the zx spectrum game, made for Apple II
;
; Stefan Wessels, 2019
; This is free and unencumbered software released into the public domain.

;-----------------------------------------------------------------------------
.segment "CODE"

;-----------------------------------------------------------------------------
.proc gamePlay

    jsr gameInit

outerloop:
    lda lives 
    bne :+
    rts                                         ; no lives left = Game Over
:
    jsr gameLevelInit                           ; prep whatever level the active player is at
    jsr gameShowPlayer                          ; show the pre-play who's turn message
    jsr uiShowGameLabels                        ; show the lines for score, danger, etc.

preamble:                                       ; bring terrain on-screen with no clock delay
    jsr drawPresent
    jsr gameWorldMove
    jsr gameWorldMove
    jsr terrainShow                             ; show terrain 1st because it collides with nothing
    jsr drawEnemies                             ; enemy missiles collide with terrain

    dec terrainOrigin                           ; get the start of terrain drawing to col 0
    dec terrainOrigin                           ; get the start of terrain drawing to col 0
    bpl preamble                                ; and draw even at 0
    inc terrainOrigin                           ; origin is now -1 so set it back to 0
    jsr gamePlaceShip                           ; find a safe spot for the player to spawn
    jsr drawPlayer                              ; now show the player ship
    jsr drawPresent                             ; flip layers 0/1 (bring backLayer to visible)

    bit KBDSTRB
    wait #$40                                   ; give the user a moment to get ready

loop:
    jsr inputGame                               ; read joystick and move player, check for pause key
    cmp #$9b                                    ; check for ESC
    bne skip                                    ; if not, check for pause
    rts                                         ; on ESC, just quit

skip:
    lda playerDead                              ; if the player is dead the world doesn't move
    bne :+
    jsr gameWorldMove                           ; scroll the world
    inc bufferDraw                              ; and move the draw start also
    jsr gameCheckStage                          ; see if a stage is ending
:
    lda flipFlop                                ; the flipFlop is 0->1->0->1, etc.
    eor #1
    sta flipFlop

    jsr gameAI                                  ; run the "AI" - this moves enemies and animates everything, etc.

next:
    dec bulletIndex                             ; Bullets move every frame (2x for 1 world move)
    dec bulletIndex

draw:
    jsr terrainShow                             ; show terrain 1st because it collides with nothing
    jsr drawEnemies                             ; enemy missiles collide with terrain
    jsr drawPlayer                              ; player collides with enemies only
    jsr drawBullets                             ; checks for enemy collision
    jsr drawBombs                               ; checks for enemy collision (but won't destroy player)
    jsr drawExplosions                          ; explosions draw over everything and collide with nothing

    lda updateHUD                               ; only update the hud when it changes
    beq delay
    jsr uiUpdateGameHud

delay:
    jsr drawPresent                             ; flip layers 0/1 (bring backLayer to visible)

    lda victory                                 ; when set, either a nuke or back home
    bne win
    jsr serviceAudio                            ; run the audio
    lda playerDead
    beq loop                                    ; playerDead = 0 means alive so keep going

died:
    dec playerDead                              ; the counter holds the explosion state
    bne skip                                    ; don't get user input
    jsr gameNextPlayer                          ; switch players if a 2p game
    jmp outerloop

win:
    jsr gameWinSequence                         ; flash screen and show the win screen (wow or home)
    jmp outerloop

.endproc 

;-----------------------------------------------------------------------------
.proc gameInit

    jsr drawClearScreen                         ; make sure the screen is clear before adjusting the

    lda #NUM_LIVES                              ; lives to start
    sta lives

    lda #0                                      ; init some variables - stage is set before this
    sta direction                               ; go to the right
    sta activePlayer                            ; player 1 is always 1st to go
    sta score 
    sta score + 1
    sta score + 2 
    
    ldx #NUM_STAGES-1                           ; mark all stages as all rockets
:
    sta makeMonster, x
    dex 
    bpl :-

    lda #NUM_SKIP_MONST                         ; make this stage (NUM_SKIP_MONST) be a monster stage
    sta monsterCnt

    ldx #3                                      ; set stage 3 (zero based) as some rockets become monsters
    sta makeMonster, x


    ldy #0                                      ; init player 1
    jsr gameStatsToPlayer

    ldy #1                                      ; init player 2
    jmp gameStatsToPlayer

.endproc 

;-----------------------------------------------------------------------------
.proc gameLevelInit

    lda #0                                      ; Lots of values need a 0 init
    sta victory
    sta playerDead
    sta zCollision
    sta stopScrolling
    sta bufferInsert
    sta dangerTickIdx
    sta moveHorz
    sta moveVert
    sta flipFlop
    sta pause
    sta fireCoolDown

    tax                                         ; clear the buffers for tracking objects
:
    sta worldBBuffer, x 
    sta worldTBuffer, x 
    sta enemyBuffer, x 
    sta enemyHgtBuffer, x 
    sta explosionBuffer, x
    sta bulletsBuffer, x 
    dex 
    bne :-

    ldx #(bombDir + (NUM_BOMBS - 1) - bombX)    ; clear the bomb area
:
    sta bombX, x 
    dex 
    bpl :-

    lda #XSIZE-1                                ; place where terrain will start drawing (right hand)
    sta terrainOrigin

    lda #AUDIO_MOTOR_PLAY                       ; Init th eaudio-frame with the engine
    sta audioFrame

    lda #$ff 
    sta lastInput                               ; set no keys down all 1's
    sta nextStage

    lda #DANGER_TICKS
    sta dangerTickCount

    ldx #(textEnd-textStage)-1                  ; empty the text areas to spaces
store:
    lda textStage, x
    beq :+                                      ; skip the null terminators
    lda #$20                                    ; load the space (" ") character
    sta textStage, x                            ; write it to the text areas
:
    dex 
    bpl store

    jsr gameFindStage                           ; point zWorldPtr at the stage start
    jsr gameWorldMove                           ; prime the draw buffers
    lda bufferInsert
    sta bufferDraw                              ; point the tail at the head
    inc bufferDraw                              ; advance the tail (this will correct itself)

    rts 

.endproc 

;-----------------------------------------------------------------------------
.proc gameShowPlayer

    jsr drawClearScreen

    print  textPlayer, #(08), #(8*08)
    lda activePlayer
    clc 
    adc #$31
    sta textPlayerNum
    print  textPlayerNum, #(23), #(8*08)
    jsr drawPresent

    wait #$40

    jmp drawClearScreen

.endproc 

;-----------------------------------------------------------------------------
.proc gamePlaceShip
    
    bottom = tempBlock
    top    = tempBlock + 1

    lda #$ff                                    ; init markers with extremes
    sta bottom 
    lda #0
    sta top

    lda #(XSIZE/3)                              ; some way into the screen
    sta playerShipX                             ; put the ship
    clc 
    adc bufferDraw                              ; make worldBBuffer relative to screen col 0
    tay 
    ldx #SHIP_WIDTH
loop:
    lda worldBBuffer, y                         ; get the bottom at this col under the ship
    cmp bottom                                  ; see if it's higher than the marker
    bcs :+
    sta bottom                                  ; is higher, update the marker
:
    lda worldTBuffer, y                         ; get the top above the ship
    cmp top                                     ; see if it's lower than the marker
    bcc :+
    sta top                                     ; if lower, update the top marker
:
    iny 
    dex 
    bne loop                                    ; do for all terrain under/above the ship
    lda bottom                                  ; put the ship SHIP_START above the bottom
    sec 
    sbc #SHIP_START 
    sta playerShipY
    sec                                         ; see how high the top is now
    sbc #SHIP_HEIGHT
    cmp top                                     ; see if the top is now above the world top
    bcs done 
    lda #SHIP_HEIGHT                            ; if it is, adjust the ship down to
    adc top                                     ; be directly under the top - this happens in narrow passages
    sta playerShipY

done:
    rts 

.endproc 

;-----------------------------------------------------------------------------
; Manage the nextStage counter based on the stage variable changing
; This variable is needed because the stage changes relative to the marker
; based on direction.
.proc gameCheckStage 

    ldy #2                                      ; check for the current stage end marker
    lda (zWorldPtr), y 
    bit Bit7Mask                                ; end of/start of stage marker
    beq noMarker                                ; not set means no marker
    
    lda direction                               ; there is a marker - see which direction
    bne :+
    lda #XSIZE
:
    sta nextStage

noMarker:
    lda nextStage                               ; now deal with the nextStage marker counter, if set
    bmi done                                    ; -1 means nothing to do

    lda stopScrolling                           ; if stopScrolling is set
    bne done                                    ; then the stage counter does not count down
    dec nextStage                               ; count down to -1 eventually
    bne done                                    ; non-zero - nothing to do

    dec nextStage                               ; at zero - take action and set to -1 so this inly triggers once
    sty updateHUD                               ; y = 2 so force HUD update draws for the 2 buffers
    lda direction                               ; based on direction, adjust the stage counter
    beq stageUp                                 ; going right

stagedown:
    dec stage                                   ; going left so dec stage
    bpl done                                    ; stage 0 and above - keep going

    lda #0                                      ; stage -1 means back home,
    sta enemyHit                                ; add score for a non-flying nuke (1000 pts)
    jsr gameAddScore

    lda #1                                      ; mark the "home run" as a victory
    sta victory
    rts 

stageUp:
    inc stage                                   ; going right - stage up

done:
    rts 
.endproc 

;-----------------------------------------------------------------------------
; This processes the buffers for enemies, bullets, explosions and moves the bombs.
.proc gameAI

    zaBombHeight   = tempBlock + 1
    zaTempFrame    = tempBlock + 2
    zaTempHeight   = tempBlock + 2
    zaEnd          = tempBlock + 3

    lda inFlight + 1                            ; launches counted last frame
    sta inFlight                                ; use it this frame
    lda #0                                      ; reset the counter from last frame
    sta inFlight + 1                            ; to count again in this frame

    ldy bufferInsert                            ; this is the end of the world
    iny                                         ; but radar's can extend up to 3 beyond
    iny                                         ; and the end is 1 past the last
    iny                                         ; thing to process
    iny 
    sty zaEnd                                   ; so mark the end of processing
    ldy bufferDraw                              ; make enemyBuffer relative to screen col 0

loop:
    lda enemyBuffer, y
    beq explosion                               ; ignore blank columns

    bit Bit12Mask                               ; check if it needs animating
    beq :+                                      ; ignore monsters (%nnnnnn00)

    adc #%00010000                              ; advance to next anim frame (radars & missiles)
    and #%01110000                              ; restrict to these 3 bits
    sta zaTempFrame                             ; save only the frame counter
    lda enemyBuffer, y                          ; get the original
    and #%10001111                              ; mask out the frame
    ora zaTempFrame                             ; add in the new frame (and thus preserve bit 8)
    sta enemyBuffer, y                          ; save this enemy now (post anim update)

:
    bit Bit8432Mask                             ; check if this is a radar, 2nd column of a missile or a launched missile
    bne doHeight                                ; skip it if it is one of those things
    
    lda inFlight                                ; see how many rockets are in-flight
    cmp #NUM_INFLIGHT * 2                       ; limit to 3 (both halves count so looks like 6 launches)
    bcs :+

    lda enemyHgtBuffer, y                       ; this is the left hand column of a missile - see if it needs to launch
    sec 
    sbc playerShipY                             ; get the distance from the player
    lsr                                         ; put it in the same units as the X
    lsr 
    cmp #31                                     ; distance cut off - > delta then rockets don't launch
    bcs :+
    sta tempBlock + 4                           ; approx 4/3 * height
    lsr tempBlock + 4                           ; since the missiles move 3 rows up for every
    lsr tempBlock + 4                           ; 4 pixels the world moves
    clc 
    adc tempBlock + 4
    sta zaTempHeight                            ; save it
    tya                                         ; get the distance of this rocket from the player in X
    sec                                         ; start by calculating the screen X position of the rocket
    sbc bufferDraw                              ; make enemyBuffer relative to screen col 0
    sbc playerShipX                             ; and then subtracting the screen x position of the ship
    bcc :+                                      ; player is to the right so ignore
    cmp zaTempHeight                            ; is dist y to missile = dist x to missile (in column units?)
    beq launch                                  ; if equal, do a launch
:
    lda enemyBuffer, y 
    bne doHeight                                ; no match, so no launch

launch:
    iny                                         ; mark the right hand column as launched
    lda enemyBuffer, y
    ora #%10000000
    sta enemyBuffer, y
    dey 

    lda enemyBuffer, y                          ; mark left column as launched
    ora #%10000000
    sta enemyBuffer, y

doHeight:
    bit Bit8Mask                                ; see if this needs height adjustment
    beq explosion                               ; don't move non-launched stuff

    inc inFlight + 1                            ; count launched rockets
    bit Bit1Mask                                ; see if this is a missile
    bne :+                                      ; this is a missile
    bit Bit34Mask                               ; don't do anything unless this is
    bne explosion                               ; the 1st column of the monster
    jsr gameMonsterAI                           ; This is a monster - run its AI
    jmp explosion

:
    lda enemyHgtBuffer, y
    sec 
    sbc #3
    cmp #WORLD_START
    bcs :+                                      ; out of existence at the top, kill it off
    lda #0
    sta enemyBuffer, y
:
    sta enemyHgtBuffer, y

explosion:
    lda explosionBuffer, y                      ; advance the explosion sprite frame
    beq nextBufferEntry
    clc 
    adc #1
    bit Bit3Mask                                ; at frame 4, stop the explosion
    beq :+
    lda #0
:
    sta explosionBuffer, y

nextBufferEntry:                                ; advance processing to the next enemy
    iny 
    cpy zaEnd                                   ; see if all enemies processed
    clc 
    beq doBombs
    jmp loop

doBombs:
    ldx #NUM_BOMBS - 1                          ; move the bombs

loopb:
    lda bombY, x 
    beq nextb                                   ; 0 in Y means inactive
    clc 
    adc #3                                      ; move down 4 rows
    sta bombY, x 

    lda bombDir, x                              ; get the travel direction
    beq :+                                      ; if 0, it's going straight down
    dec bombDir, x                              ; decrease the columns it will move forward for
    inc bombX, x                                ; <> 0, still moving forward, inc it's x position
    bne skbd                                    ; always (jmp)
:
    lda stopScrolling                           ; see if the world is moving
    bne skbd                                    ; if not, don't adjust the bomb X
    dec bombX, x                                ; scroll bomb with world to go straight down
skbd:
    lda bombX, x                                ; get the screen column
    bmi bol                                     ; if < 0 then bomb out left hand side of screen
    adc bufferDraw                              ; make enemyBuffer relative to screen col 0
    tay 
    lda worldBBuffer, y                         ; get the terrain bottom height in this col
    sec 
    sbc bombY, x                                ; subtract the bomb height
    bcs nextb                                   ; if the bomb < terrain (carry set), all is well
bol:
    lda #0                                      ; bomb too low, write 0 into the bomb height
    sta bombY, x

nextb:
    dex 
    bpl loopb                                   ; repeat for all bombs

    clc 
    rts 
.endproc 


;-----------------------------------------------------------------------------
; This moves the monster up and down between the top and bottom of the world
; where it will pause for a moment.  When the monster overlaps the player, it
; will also pause for a moment before going back to the bounce
; called with y the index in enemyBuffer to the 1st strip of the monster and
; acc the value in enemyBuffer, y
.proc gameMonsterAI

    zaTempIndex    = tempBlock + 2              ; y reg saved as Index to 1st strip in monster

    and #%01100000                              ; check if the stop counter has a value
    beq moving                                  ; if not, monster is moving

paused:                                         ; the monster is at top/bottom or overlapped player
    lda #%00100000
    adc enemyBuffer, y                          ; add to the monster value
    ora #%10000000                              ; make sure the flying bit stays on
    sta enemyBuffer, y                          ; save the update
    rts 

moving:                                         ; the monster is bouncing up or down
    sty zaTempIndex                             ; save the index - need to work on 3 strips
    ldx #MONSTER_WIDTH                          ; load x with the width (3)
    lda enemyBuffer, y                          ; reload the enemy (acc destroyed by and above)
    bit Bit5Mask                                ; bit 5 is direction - 0 is up and 1 is down
    bne down 

up:
    lda enemyHgtBuffer, y                       ; monster is at same height in all its columns
    sec 
    sbc #MONSTER_SPEED                          ; move up to where it would be
    sbc #MONSTER_HEIGHT                         ; but need the top of the monster to check if it hit the ceiling

    dey                                         ; adjust for alignment with world buffer

chktop:
    cmp worldTBuffer, y                         ; compare to the world ceiling height
    beq changedir                               ; if equal, change direction don't go up
    bcc changedir                               ; if less than, also don't go up but change dir
    iny                                         ; do for all the monster strips
    dex 
    bne chktop

    ldx #MONSTER_WIDTH                          ; not in ceiling collision so adjust height
    ldy zaTempIndex                             ; start at the 1st strip

:
    lda enemyHgtBuffer, y                       ; move the strips upwards
    sec 
    sbc #MONSTER_SPEED                          ; speed at which monster moves
    sta enemyHgtBuffer, y                       ; save updated position
    iny                                         ; repeat for all strips
    dex 
    bne :-

chkplayerhgt:                                   ; see if the monster is now overlapping the enemy ship
    clc 
    ldy zaTempIndex                             ; get the 1st strip index
    ldx #MONSTER_SPEED                          ; check as many columns as moved (faster for than overlap calc)
:
    cmp playerShipY                             ; is it matching the player
    beq stop                                    ; if yes, stop the monster
    adc #1                                      ; try each position in the step
    dex                                         ; and repeat for all increments in a monster step (speed)
    bne :-
    rts 

changedir: 
    ldy zaTempIndex                             ; go back to the 1st strip
    lda enemyBuffer, y                          ; get the enemy
    eor #%00110000                              ; turn on stop and change dir in one go
    sta enemyBuffer, y                          ; save
    rts 

stop:
    lda enemyBuffer, y                          ; get the 1st strip
    ora #%00100000                              ; set stop counter to 1
    sta enemyBuffer, y                          ; save
    rts 

down:
    lda enemyHgtBuffer, y                       ; monster is at same height in all its columns
    clc 
    adc #MONSTER_SPEED                          ; height down by speed to where it would go

    ldx #MONSTER_WIDTH                          ; check for a bottom collision
    dey                                         ; adjust for the world offset
chkbot:
    cmp worldBBuffer, y                         ; get the world below this strip
    bcs changedir                               ; if the world is higher than the base of the monster, change direction
    iny                                         ; check all strips
    dex 
    bne chkbot

    ldx #MONSTER_WIDTH                          ; height okay, so finally move the monster
    ldy zaTempIndex                             ; starting at strip 1

:
    lda enemyHgtBuffer, y                       ; get the strip height
    clc 
    adc #MONSTER_SPEED                          ; adjust the height
    sta enemyHgtBuffer, y                       ; save it
    iny                                         ; do for all strips
    dex 
    bne :-
    beq chkplayerhgt                            ; JuMP to see if the monster and player now overlap

done:
    rts 

.endproc 

;-----------------------------------------------------------------------------
.proc gameWorldMove

    jsr gameUpdateDangerBar                     ; when the world moves, the danger bar needs to be updated

    clc 
    inc bufferInsert                            ; advance where buffer data is inserted
    ldx bufferInsert                            ; get the new right edge
    inx                                         ; step past where radar columns might be
    inx 
    inx 
    lda #0                                      ; write a null to kill old circular data
    sta enemyBuffer, x 
    sta enemyHgtBuffer, x 
    ldx bufferInsert                            ; get the right edge again

    lda direction                               ; world prt moves based on level forward/reverse direction
    bne left

    clc                                         ; moving through to the right, advance the world 1 column
    lda zWorldPtr
    adc #3
    sta zWorldPtr
    bcc :+
    inc zWorldPtr + 1
    clc 
:
    ldy #0                                      ; get the world bottom
    lda (zWorldPtr), y 
    sta worldBBuffer, x 

    iny                                         ; get the world top
    lda (zWorldPtr), y 
    sta worldTBuffer, x

    lda terrainOrigin                           ; if the terrain is still scrolling in
    bne done                                    ; then don't populate enemies

    iny                                         ; get the enemy flags
    lda (zWorldPtr), y                          ; get the enemy flag
    beq done                                    ; no enemies, carry on
    jmp gameWorldFlag                           ; populate the world with enemies

left:                                           ; to the left (in the data, back to front), 1 column "subtract"
    sec 
    lda zWorldPtr
    sbc #$03
    sta zWorldPtr
    bcs :+
    dec zWorldPtr + 1

:
    clc                                         ; make carry clear since gameWorldFlag expects to be called with carry clear
    ldy #6                                      ; get the world bottom
    lda (zWorldPtr), y 
    sta worldBBuffer, x 
    
    iny                                         ; get the world top
    lda (zWorldPtr), y 
    sta worldTBuffer, x

    lda terrainOrigin                           ; if the terrain is still scrolling in
    bne done                                    ; then don't populate enemies

    ldy #8                                      ; look ahead 1 col
    lda (zWorldPtr), y                          ; get the enemy flag
    bit Bit1Mask                                ; is this a missile
    beq radar                                   ; not a missile, maybe there's a radar coming?
    jmp gameWorldFlag                           ; this is a missile - go add it

radar:
    ldy #2                                      ; Look ahead even further for a radar coming on-screen
    lda (zWorldPtr), y                          ; get the enemy flag
    bit Bit2Mask                                ; is this a radar
    beq done                                    ; no, then nothing more to do
    jmp gameWorldFlag                           ; yes - add the radar

done:
    rts 

.endproc

;-----------------------------------------------------------------------------
.proc gameFindStage

    zaStage = tempBlock + 1

    ldy #2                                      ; world data is bottom, top and then flags.  need flags in this routine

    lda direction                               ; world prt moves based on level forward/reverse direction
    bne left

    lda #<worldDataStart                        ; forward (right) - init with the start of the data
    sta zWorldPtr
    lda #>worldDataStart
    sta zWorldPtr + 1

    lda stage                                   ; see how many stage markers to skip
    beq done                                    ; stage 0 - skip none, start of data is good
    sta zaStage 

rloop:
    lda (zWorldPtr), y                          ; get the flags
    bit Bit7Mask                                ; see if it's a stage marker
    beq :+
    dec zaStage                                 ; found a stage but step once more not to trigger in-game

:
    lda zWorldPtr                               ; step the pointer over the data triplet
    clc 
    adc #3
    sta zWorldPtr
    bcc :+
    inc zWorldPtr + 1
:
    jsr gameUpdateDangerBar                     ; update the danger meter
    lda zaStage                                 ; now check if the stage is found
    beq done                                    ; and exit if it has
    bne rloop

left:                                           ; world is moving backwards so
    lda #<(worldDataEnd - 3)                    ; start at the end of the data, looking
    sta zWorldPtr                               ; at the 1st triplet of data
    lda #>(worldDataEnd - 3)
    sta zWorldPtr + 1

    lda #(NUM_STAGES-1)                         ; see how many stage markers to skip
    sec 
    sbc stage 
    beq done
    sta zaStage                                 ; save the counter

lloop:
    lda (zWorldPtr), y                          ; get the flags
    bit Bit7Mask                                ; and see if it's a stage flag
    beq :+
    dec zaStage                                 ; may be the stage being sought but step once more

:
    lda zWorldPtr                               ; move the ptr backwards through the data
    sec                                         ; to the previous data triplet
    sbc #3
    sta zWorldPtr
    bcs :+
    dec zWorldPtr + 1

:
    jsr gameUpdateDangerBar                     ; update the danger meter
    lda zaStage                                 ; see if this is the stage being sought
    beq done                                    ; and exit if it is
    bne lloop 
    
done:
    clc 
    rts 

.endproc

;-----------------------------------------------------------------------------
; Use the 4 user font characters (at $80+) to draw a dotted line danger bar
.proc gameUpdateDangerBar

    dec dangerTickCount                         ; count down to adding a dot
    beq tick                                    ; time to add a dot?
    rts 

tick:
    lda #DANGER_TICKS                           ; reset the countdown
    sta dangerTickCount
    ldx dangerTickIdx                           ; get the index for how far into the count
    lda textDangerBar, x                        ; get the character
    cmp #' '                                    ; 1st time - this is a space so init
    beq dot
    cmp #$84                                    ; 4 dots last character so move to next column?
    beq :+
    inc textDangerBar, x                        ; add a dot by advancing the font
    bne hud                                     ; JuMP to hud update
:
    inx                                         ; move to next character in the display
    stx dangerTickIdx                           ; save the index
dot:
    lda #$80                                    ; 1 dot - load the single dot font character
    sta textDangerBar, x                        ; put it in the display

hud:
    lda #2                                      ; update the hud twice so both buffers
    sta updateHUD                               ; get updated
    clc 
    rts 

.endproc 

;-----------------------------------------------------------------------------
; Process the flags in the world data stream. carry must be clear when called
; radar is %01, missile is %10
.proc gameWorldFlag

    bit Bit2Mask                                ; radar?
    beq missile                                 ; no, see if it's a missile
    lda gameMode                                ; In edit mode, don't draw nuke's for radars
    bit Bit2Mask                                ; GAME_MODE_EDIT is $02 so if bit set then skip nuke check
    bne :+
    lda stage
    cmp #(NUM_STAGES - 1)                       ; stages are 0 based - this is the last stage
    clc 
    beq nuke                                    ; in stage 4 the radars become nuke's

:
    lda worldBBuffer, x                         ; load the height of the world below this enemy
    sbc #0                                      ; carry is clear so this is -1
    tay                                         ; save the height in y
    sta enemyHgtBuffer, x                       ; set the height for the 1st radar strip
    lda #SPR_RADAR0                             ; put the strips into the enemy buffer
    sta enemyBuffer, x

    inx                                         ; go to the next strip
    tya                                         ; get the height from y
    sta enemyHgtBuffer, x                       ; and set up this strip and the remaining strips
    lda #SPR_RADAR1
    sta enemyBuffer, x

    inx 
    tya 
    sta enemyHgtBuffer, x
    lda #SPR_RADAR2
    sta enemyBuffer, x

    inx 
    tya 
    sta enemyHgtBuffer, x
    lda #SPR_RADAR3
    sta enemyBuffer, x

    clc 
    rts 

nuke:
    lda worldBBuffer, x                         ; put the nuke strips in
    sbc #0
    tay 
    sta enemyHgtBuffer, x
    lda #SPR_NUKE0
    sta enemyBuffer, x

    inx 
    tya 
    sta enemyHgtBuffer, x
    lda #SPR_NUKE1
    sta enemyBuffer, x

    clc 
    rts 

missile:
    bit Bit1Mask                                ; is it a missile
    beq done                                    ; no, move on

    ldy stage                                   ; see if in this stage some missiles become monsters
    lda makeMonster, y
    beq :+                                      ; 0 means no, not in this stage
    dec monsterCnt
    beq monster                                 ; when this is 0, make a missile a monster

:
    lda worldBBuffer, x                         ; get the height
    sbc #0                                      ; -1 (carry clear) to put it above ground
    tay 
    sta enemyHgtBuffer, x                       ; mark as the height
    lda #SPR_MISSILE0
    sta enemyBuffer, x 

    inx 
    tya 
    sta enemyHgtBuffer, x 
    lda #SPR_MISSILE1
    sta enemyBuffer, x

    clc 
    rts 

monster:
    lda #NUM_SKIP_MONST
    sta monsterCnt

    lda worldBBuffer, x 
    sbc #0
    tay 
    sta enemyHgtBuffer, x                       ; set the height for the 4 radar strips
    lda #SPR_MONSTER0                           ; put the strips into the enemy buffer
    sta enemyBuffer, x

    inx 
    tya 
    sta enemyHgtBuffer, x
    lda #SPR_MONSTER1
    sta enemyBuffer, x

    inx 
    tya 
    sta enemyHgtBuffer, x
    lda #SPR_MONSTER2
    sta enemyBuffer, x

    clc 
    rts 

 done:
    rts 

.endproc

;-----------------------------------------------------------------------------
.proc gamePlayerCollision

    lda #0
    sta zCollision                              ; reset the collision flag

    lda playerDead                              ; already marked as dead?
    beq :+
    rts 

:
    lda #$20                                    ; mark the death and hold for
    sta playerDead                              ; some frames

    lda #2                                      ; force a HUD redraw to show
    sta updateHUD                               ; reduced lives

    lda #1                                      ; stop the world moving
    sta stopScrolling

    lda gameMode                                ; get the game mode
    bne done                                    ; and only take a life if in gameplay (= 0)

    sed                                         ; take away a life
    lda lives                                   ; lives are done in decimal
    sec                                         ; for easier printing
    sbc #1
    sta lives 
    cld 
    clc 

done:
    rts 

.endproc 

;-----------------------------------------------------------------------------
.proc gameNextPlayer 

    lda lives 
    bne stillOk

    print  textGameOver, #(0), #(8*05)          ; show the game over text
    jsr drawPresent

    lda #0
    bit KBDSTRB                                 ; flush the keyboard buffer
    sta lastInput
:
    jsr inputCheckForInput                      ; wait for user interaction
    beq :-

    jsr uiCheckHighScore                        ; Did this player make it onto the high score table

stillOk:
    lda numPlayers                              ; see if it's a 2-player game
    beq done

    ldy activePlayer                            ; save the outgoing players' stats
    jsr gameStatsToPlayer

    ldy #0                                      ; assume p1 was active
    lda activePlayer                            ; get the active player
    eor #1                                      ; swap 0->1 or 1->0
    tax                                         ; next player in x
    beq :+                                      ; if 0 is next, y is set up already
    ldy #(rsEnd-rsStart)                        ; 1 is next, so set y up to point at p1
:
    lda playerStats, y                          ; get the lives (1st element is "struct")
    beq done                                    ; if none, then don't switch players

    ldy activePlayer                            ; incoming player has lives - save current active in Y
    stx activePlayer                            ; switch to this (incoming) player

    ldy activePlayer                            ; install the players' stats
    jsr gamePlayerToStats

done:
    rts 

.endproc

;-----------------------------------------------------------------------------
; Copy the "live" stats to the backup location for a player
; y is the index of the player (0 or 1)
.proc gameStatsToPlayer

    ldx #(rsEnd-rsStart) - 1                    ; assume p0 destination
    dey 
    bne :+
    ldx #((rsEnd-rsStart) * 2) - 1              ; actually p1 so adjust copy dest location
:
    ldy #(rsEnd-rsStart) - 1                    ; set y to the end of the runtime stats buffer
:
    lda rsStart, y                              ; load runtime stat
    sta playerStats, x                          ; save as player stat
    dex                                         ; do for all bytes needing to be copied
    dey 
    bpl :-                                      ; copy all bytes

    rts 

.endproc 

;-----------------------------------------------------------------------------
; Copy the backup player stats to the "live" stats location
; y is the index of the player (0 or 1)
.proc gamePlayerToStats

    ldx #(rsEnd-rsStart) - 1                    ; assume p0 destination
    dey 
    bne :+
    ldx #((rsEnd-rsStart) * 2) - 1              ; actually p1 so adjust destination
:
    ldy #(rsEnd-rsStart) - 1                    ; set y to the end of the runtime stats
:
    lda playerStats, x                          ; load a player stat byte
    sta rsStart, y                              ; copy it to a runtime stat byte
    dex                                         ; next bytes
    dey 
    bpl :-                                      ; do for all bytes that need copying

    rts 

.endproc 

;-----------------------------------------------------------------------------
.proc gameWinSequence

    iterations   = tempBlock + 0                ; how many times to do this loop

    inc lives                                   ; give the player another life
    lda #1
    sta updateHUD
    jsr uiUpdateGameHud
    jsr drawPresent                             ; get the score and live visible

    lda #$07                                    ; flash the screen iteration times
    sta iterations

:
    jsr drawInvertVisibleScreen

    dec iterations
    bne :-

    lda direction                               ; flip the direction
    eor #1
    sta direction
    beq :+                                      ; if dir zero - going out again from stage 0
    lda #3                                      ; zero based - second last stage - don't play nuke stage again
:
    sta stage

    jsr uiWinScreen                             ; show the BONUS screen

    ldx #(NUM_STAGES - 2)                       ; start at the stage before the last (0 based)
:
    dex                                         ; step back - but the second last stage was already marked
    bmi :+                                      ; if at -1 then all stages checked
    lda makeMonster, x                          ; see if this stage has rockets turn into monsters yet
    bne :-                                      ; if yes, then keep looking for a stage without that
    lda #1                                      ; mark this stage
    sta makeMonster, x                          ; as now also having monster-rockets
:

    ldy activePlayer                            ; make sure the stats are backed up
    jmp gameStatsToPlayer

.endproc 

;-----------------------------------------------------------------------------
; Removes an enemy from screen that overlaps with zEnemyCol
; zaDrawSpriteWidth says how many columns to check - the bomb could
; kill in one of 2 columns, otherwise just one
.proc gameKillEnemy

    zaDrawSprWidth  = tempBlock + 3             ; Parameter
    zaSprColumn     = tempBlock + 2             ; Internal segment/column of enemy sprite
    zaEnemyColLocal = tempBlock + 4             ; Internal Backup copy of zEnemyCol but modified

    lda zEnemyCol
    sta zaEnemyColLocal                         ; make a backup that's safe to change

    lda #0                                      ; reset the collision flag
    sta zCollision

    ldy #$ff 
    sty enemyHitType

loop:                                           ; for the width to check
    ldx zaEnemyColLocal
    lda enemyBuffer, x                          ; see if there's an enemy
    beq next 
    sta enemyHit                                ; save the flags of the enemy that was struck

    lsr                                         ; see if it's a radar or missile
    bcc other                                   ; lsb clear means not a missile
    lsr                                         ; missile or nuke. 1 more shift to get column in lsb's
    bcc missile
    ldy #1                                      ; hit a nuke
    sty victory
    ldy #0                                      ; nuke is 0 in the score table
    beq continue

missile:
    ldy #1                                      ; missile and nuke 2 columns wide
    bne continue

other:
    lsr                                         ; check next bit
    bcc monster                                 ; if also clear then a monster
    ldy #3                                      ; radar is 4 columns wide
    bne continue

monster:
    ldy #2                                      ; monster is 3 columns wide

continue:
    sty enemyHitType                            ; 0 = nuke, 1 = missile, 2 = monster, 3 = radar, $ff - nothing
    bne :+                                      ; now Y is being used as column (width - 1) of the enemy
    iny                                         ; for the nuke, make y = 1 (it's still 0 here as the type)
:
    and #%00000011                              ; mask column (or strip of the sprite)
    sta zaSprColumn                             ; save it

    lda audioFrame
    ora #AUDIO_EXPLOSION                        ; Add explosion to audio frame
    sta audioFrame
    lda #0                                      ; start the explosion at frame 0
    sta audioExplFrame


    lda zaEnemyColLocal                         ; see where the sprite starts
    sec                                         ; by subtracting the strip number (index)
    sbc zaSprColumn                             ; from the current world column
    tax                                         ; and save that

    lda enemyHgtBuffer, x                       ; get the height of the enemy that's being killed
    and #%11111000                              ; mask off 3 bits for the explosion sprite frame cntr
    sta explosionBuffer, x                      ; save as the explosion

    lda #0                                      ; write 0 to the enemy strip columns
    sta enemyBuffer, x
    sta enemyHgtBuffer, x
    inx 
    sta enemyBuffer, x 
    sta enemyHgtBuffer, x 
    dey 
    beq next                                    ; early out on missiles and nukes
    inx 
    sta enemyBuffer, x 
    sta enemyHgtBuffer, x 
    dey 
    beq next                                    ; early out on monsters
    inx 
    sta enemyBuffer, x
    sta enemyHgtBuffer, x

next:
    inc zaEnemyColLocal
    dec zaDrawSprWidth 
    bne loop 

    clc 
    rts 

.endproc

;-----------------------------------------------------------------------------
; Add score for the enemy destroyed and mark the HUD as needing an update
; Acc is 0 = nuke, 1 = missile, 2 = monster, 3 = radar from enemyHitType
; The scoreTable score is "amended" in that flying objects get 4 added
; Scores appear on the hud at 10x the score variable value
.proc gameAddScore
    
    asl                                         ; score table is 2 bytes wide
    tax                                         ; save the type
    lda enemyHit                                ; see if this enemy was flying
    and #%10000000
    bne flying

    lda #0                                      ; not flying
    beq :+

flying:
    lda #4                                      ; was flying
:
    sed                                         ; use decimal mode
    adc scoreTable, x                           ; add across all 3 score bytes, as needed (3 bytes up to 999999 score)
    adc score
    sta score
    inx 
    lda scoreTable, x
    adc score + 1
    sta score + 1
    bcc :+
    lda #0                                      ; only adc and sbc work right in sed
    adc score + 2
    sta score + 2
    clc 
:
    cld                                         ; turn off decimal mode

    lda #2
    sta updateHUD

    rts 

.endproc 
