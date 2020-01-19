;-----------------------------------------------------------------------------
; edit.inc
; Part of penetrator, the zx spectrum game, made for Apple II
;
; Stefan Wessels, 2019
; This is free and unencumbered software released into the public domain.

;-----------------------------------------------------------------------------
.segment "CODE"

;-----------------------------------------------------------------------------
.proc editLoop

    jsr drawClearScreen                         ; make sure the screen is clear before adjusting the

    jsr editInit
    jsr editLoadStage
    lda #2
    sta updateHUD

loop:
    jsr inputEdit
    beq :+
    jsr editHandleKeys
    lda pause                                   ; pause is quit flag in endit
    beq :+
    rts 
:
    clc 
    jsr terrainShow                             ; show terrain 1st because it collides with nothing
    jsr drawEnemies                             ; enemies collide with nothing in edit mode
    jsr editDrawCursor
    lda updateHUD 
    beq :+
    jsr uiShowEditLabels
:
    jsr drawPresent                             ; flip layers 0/1 (bring backLayer to visible)

    jmp loop

.endproc

;-----------------------------------------------------------------------------
.proc editHandleKeys

    cmp #'6'                                    ; 6 or bigger - not a stage command
    bcs notnum 
    cmp #'1'                                    ; less than 1 - not a stage command
    bcc notnum
    sbc #'1'                                    ; stages are 0 based
    sta stage 

    lda #AUDIO_UI_TICK                          ; make a sound
    sta audioFrame
    jsr serviceAudio

    lda #0                                      ; reset where the buffers will start
    sta bufferInsert
    jsr drawClearScreen
    jsr editLoadStage                           ; get the right stage at the right place
    jmp uiShowEditLabels                        ; update the HUD

notnum:
    cmp #$1b                                    ; ESC key
    bne notQuit

    lda #AUDIO_UI_TICK                          ; make a sound
    sta audioFrame
    jsr serviceAudio

    lda #1
    sta pause                                   ; abuse pause to signal quit
    rts 

notQuit:
    cmp #'c'                                    ; c or C for help
    beq help
    cmp #'C'
    bne notHelp 

help: 
    lda #AUDIO_UI_TICK                          ; make a sound
    sta audioFrame
    jsr serviceAudio

    jmp uiShowEditHelp

notHelp:
    cmp #'s'                                    ; s or S for save
    beq save 
    cmp #'S'
    bne notSave 

save:
    lda #AUDIO_UI_TICK                          ; make a sound
    sta audioFrame
    jsr serviceAudio

    lda #0                                      ; a 0 is save
    jsr uiFileLoadSave
    jmp editLoop 
    
notSave:
    cmp #'l'                                    ; l or L for load
    beq load 
    cmp #'L'
    bne done 

load:
    lda #AUDIO_UI_TICK                          ; make a sound
    sta audioFrame
    jsr serviceAudio

    lda #1                                      ; a 1 is load
    jsr uiFileLoadSave
    jmp editLoop 

done:
    clc 
    rts 

.endproc 

;-----------------------------------------------------------------------------
; Move the world left (feels like YOU are moving right)
; will update the stage when a stage boundary is crossed and will then
; update the HUD
.proc editGameWorldRight

    zaWorldPtr = tempBlock + 14                 ; 2 bytes for a ptr

    lda stage                                   ; only scroll right while the stage
    cmp #4                                      ; isn't 5 (0 based so 4).  Stage 5
    bcc :+                                      ; cannot be edited
    rts 
:
    jsr gameWorldMove                           ; use the game move code

    lda zWorldPtr + 1                           ; but then calculate a point where the stage marker might be
    sta zaWorldPtr + 1
    lda zWorldPtr
    sec 
    sbc #((XSIZE+3)*3)
    sta zaWorldPtr
    bcs :+
    dec zaWorldPtr + 1
   
:
    ldy #2                                      ; and read the enemy byte
    lda (zaWorldPtr), y
    bit Bit7Mask                                ; and see if this is a stage change
    beq :+
    inc stage                                   ; bit was set so change stage
    lda #2                                      ; and update the HUD
    sta updateHUD
:
    inc bufferDraw                              ; the bufferDraw (col 0) is also one more to the right
    rts 

.endproc 

;-----------------------------------------------------------------------------
; Scroll to the left, updating the stage boundary and adding enemies as needed.
.proc editGameWorldLeft

    leftWorldPtr = tempBlock + 14

    lda zWorldPtr + 1                           ; get the left edge insert point by subtracting
    sta leftWorldPtr + 1                        ; XSIZE+3 from the right edge. XSIZE-1 is the current
    lda zWorldPtr                               ; left screen col.  +3 is 4 past, which is the width
    sec                                         ; of a radar that could now scroll on-screen
    sbc #((XSIZE+3)*3)                          ; the buffer will have proper height data for at least
    sta leftWorldPtr                            ; 2 of these 3 cols to the left - see editLoadStage
    bcs :+
    dec leftWorldPtr + 1

:
    lda leftWorldPtr + 1                        ; see if this new dest is <= the world start.
    cmp #>worldDataStart                        ; if it is then don't scroll any further left, just exit
    beq :+                                      ; hi equal to worldStart hi, so test low
    bcs leftOk                                  ; hi > worldStart hi so good to go
done:
    rts 
:
    lda leftWorldPtr
    cmp #<worldDataStart
    bcc done                                    ; lo < lo - too small, no go
    beq done                                    ; lo equal so also no go

leftOk:
    lda zWorldPtr                               ; there's room to go left so move the right edge left
    sec                                         ; by 1 col, or one triplet of data, so 3 bytes
    sbc #3
    sta zWorldPtr
    bcs :+
    dec zWorldPtr + 1
:
    dec bufferDraw                              ; adjust the draw and insert buffer "counters"
    dec bufferInsert                            ; now points at the "new" screen locations

    clc 
    ldx bufferDraw                              ; the bufferDraw is the left edge
    dex                                         ; back up 4 bytes (so 1 past where a radar
    dex                                         ; could be added) and write a 0 in there
    dex                                         ; to stomp any old circular buffer data
    dex 
    lda #0
    sta enemyBuffer, x
    inx                                         ; now move to the col where the heights will be added

    ldy #0                                      ; get the world bottom
    lda (leftWorldPtr), y 
    sta worldBBuffer, x                         ; and put it in the buffer
    iny                                         ; and get the world top
    lda (leftWorldPtr), y
    sta worldTBuffer, x                         ; and put it in the buffer

    ldy #2                                      ; see if there's a radar to go with this height data
    lda (leftWorldPtr), y                       ; get the enemy flag
    bit Bit7Mask                                ; see if this is a stage change
    beq radar                                   ; no - check for radar
    pha                                         ; stage is changing so save a
    dec stage                                   ; dec the stage
    lda #2                                      ; mark the HUD for update
    sta updateHUD
    pla                                         ; restore a - could also hold a missile or radar

radar:
    bit Bit2Mask                                ; is it a radar
    beq missile                                 ; no - go check for a missile
    jmp gameWorldFlag                           ; add the radar - can't also be a missile so all done

missile:
    inx                                         ; the missile will start 2 cols later
    inx 
    ldy #8                                      ; and is also 2 (past height)+(2*3) bytes later in the world
    lda (leftWorldPtr), y                       ; get the enemy flag
    bit Bit1Mask                                ; see if it's a missile
    beq done                                    ; if not a missile then done
    jmp gameWorldFlag                           ; add the missile and finish

.endproc

;-----------------------------------------------------------------------------
; Prepare for editing by initializing the needed variables
.proc editInit

    brushType = tempBlock + 16

    lda #GAME_MODE_EDIT
    sta gameMode

    lda #0
    sta brushType                               ; BRUSH_TERRAIN is also 0
    sta direction                               ; go to the right
    sta terrainOrigin                           ; don't scroll in, just be all there
    sta zCollision
    sta stopScrolling
    sta bufferInsert
    sta dangerTickIdx
    sta pause

    tax                                         ; start at 0 for the buffers
:
    sta worldBBuffer, x                         ; and init all 256 bytes of each buffer to 0
    sta worldTBuffer, x 
    sta enemyBuffer, x 
    sta enemyHgtBuffer, x 
    sta explosionBuffer, x
    sta bulletsBuffer, x 
    dex 
    bne :-

    ldx #NUM_STAGES-1                           ; mark all stages as all rockets, no monsters
:
    sta makeMonster, x
    dex 
    bpl :-

    lda #$ff
    sta lastInput                               ; set no keys down - all 1's
    sta nextStage

    lda #DANGER_TICKS
    sta dangerTickCount

    lda #(XSIZE/2)                              ; middle of the screen in X
    sta playerShipX                             ; is where the cursor is

    lda #(WORLD_START + (WORLD_END - WORLD_START) / 2)
    sta playerShipY                             ; and in Y also in the middle
 
    rts 

.endproc

;-----------------------------------------------------------------------------
; Show a custom character as a cursor.  Also redraw the down arrow
; since the cursor draws into the top 8 likes that don't get cleaned/updated
; with every frame.  Drawing the arrow fixes up the out of bounds areas
.proc editDrawCursor

    yPos         = tempBlock + 14
    xPos         = tempBlock + 15

    zaStrL       = tempBlock + 1                ; parameter - string lo
    zaStrH       = tempBlock + 2                ; parameter - string hi
    zaFontL      = tempBlock + 4                ; internal - point at the character memory Lo
    zaFontH      = tempBlock + 5                ; internal - point at the character memory Hi
    zaFontOffset = tempBlock + 6                ; 0-15 - index from zaFont

    lda playerShipX
    lsr 
    sta xPos

    print textEditDnArrow, xPos, 0              ; redraw arrow to fix out of top cursor overwrite

    lda #$87                                    ; cursor symbol
    ldy #0
    jsr setFont

    lda playerShipY                             ; get the screen Y for cursor
    sec                                         ; but raise it so the horz bar is at the "ship" level
    sbc #2
    sta yPos                                    ; init the row working buffer
    ldx backLayer                               ; write to back layer

plotLoop:
    ldy yPos                                    ; at the working row offset
    lda rowL, y                                 ; get the memory address
    clc 
    adc xPos                                    ; add in column
    sta write + 1                               ; point lo at memory
    lda rowH, y
    adc layersH, x
    sta write + 2

    ldy zaFontOffset                            ; get the offset into the character
    lda (zaFontL), y

write:                                          ; get the actual left character byte
    sta PLACEHOLDER                             ; plot the left hand side
    cpy #$05                                    ; cursor character is 5 rows high
    bcs done                                    ; if 10 then done with the cursor
    inc zaFontOffset                            ; move 2 bytes for last row plotted
    inc zaFontOffset
    inc yPos                                    ; move down a row on screen
    
    bne plotLoop                                ; always take this branch

done:
    rts 

.endproc 

;-----------------------------------------------------------------------------
; Will set the terrain to the cursor, and will move enemies to be back
; on the terrain when needed.  Will set both the draw buffers and the
; actual world data stream (the triplets)
.proc editSetTerrainBottom

    zaColumn   = tempBlock + 3
    zaMidScreen = tempBlock + 4

    lda zWorldPtr + 1                           ; make a copy of the world pointer
    sta zaMidScreen + 1                         ; that corresponds with the position of the cursor
    lda zWorldPtr 
    sec 
    sbc #((XSIZE/2)*3)                          ; 1/2 way in screen, 3 bytes per column
    sta zaMidScreen
    bcs :+
    dec zaMidScreen + 1

:
    ldy #3                                      ; index from ptr
    lda playerShipY                             ; cursor height in playerShipY
    sta (zaMidScreen), y                        ; store in the terrain bottom

    lda bufferDraw                              ; now update the draw buffer
    clc 
    adc playerShipX                             ; get to the correct column offset
    tax                                         ; save in x
    lda playerShipY                             ; get the height
    sta worldBBuffer, x                         ; update the buffer
    lda enemyBuffer, x                          ; get the enemy flag
    beq done                                    ; no enemy - all done
    bit Bit12Mask                               ; see if it's a radar or missile
    beq done                                    ; if bits 12 not set then not an enemy
    ldy #2                                      ; assume a missile - 2 columns
    and #%00001110                              ; column and way to id enemy (10 is radar)
    lsr                                         ; move col to 1's position
    lsr 
    bcc :+                                      ; if carry clear then this is a missile
    ldy #4                                      ; a radar is 4 columns
:
    sta zaColumn                                ; save the enemy column
    txa                                         ; get the screen column under the cursor (in the buffer)
    sec 
    sbc zaColumn                                ; subtract the enemy column, giving the buffer column of the 1st enemy column
    tax                                         ; get the index into a
    lda worldBBuffer, x                         ; get the world height
    sec 
    sbc #1                                      ; and go 1 row higher (on top of ground)
:
    sta enemyHgtBuffer, x                       ; write the world height to all
    inx                                         ; the columns of the enemy
    dey 
    bne :-

done:
    rts 

.endproc 

;-----------------------------------------------------------------------------
; Set the terrain top in the world data stream as well as the draw buffers
.proc editSetTerrainTop

    zaMidScreen = tempBlock + 4

    lda zWorldPtr + 1                           ; make a ptr to the cursor
    sta zaMidScreen + 1
    lda zWorldPtr 
    sec 
    sbc #((XSIZE/2)*3)
    sta zaMidScreen
    bcs :+
    dec zaMidScreen + 1

:
    ldy #4                                      ; index from ptr
    lda playerShipY 
    sta (zaMidScreen), y                        ; save the top in the world data

    lda bufferDraw                              ; also update the draw buffer
    clc 
    adc playerShipX
    tax 
    lda playerShipY 
    sta worldTBuffer, x

    rts 

.endproc 

;-----------------------------------------------------------------------------
; This routine will "kill" enemies that overlap with the cursor column.
; the zaDrawSprWidth is 4 (radar) or 2 (missile) for the type of enemy
; for which room needs to be created
; Enemies are removed from the world stream and the draw buffers
.proc editPreSetEnemy

    zaDrawSprWidth = tempBlock + 3              ; Parameter
    zaMidScreenL = tempBlock + 5
    zaMidScreenH = tempBlock + 6

    lda zWorldPtr + 1                           ; calculate an offset for the cursor
    sta zaMidScreenH
    lda zWorldPtr 
    sec 
    sbc #(((XSIZE/2)+2)*3)
    sta zaMidScreenL
    bcs :+
    dec zaMidScreenH

:
    lda #3                                      ; check 3 columns before the cursor as a 4 col
    clc                                         ; radar could start 3 col earlier and still overlap
    adc zaDrawSprWidth                          ; and add the length of the enemy (4 for radar, 2 for missile)
    tax                                         ; put the number of cols to check in x
    ldy #2                                      ; index off of ptr to point at enemy bytes
krl:
    lda (zaMidScreenL), y                       ; get the enemy byte
    bit Bit2Mask                                ; see if it's a radar
    beq :+
    and #%11111100                              ; mask off the enemy bits (making it blank but keeping stage data)
    sta (zaMidScreenL), y                       ; and save the "deleted" enemy
:
    iny                                         ; advance the offset by 3 to the next enemy byte
    iny 
    iny 
    dex                                         ; and one less column to do
    bne krl                                     ; repeat till all cols checked

    lda #1                                      ; now do the exact same but for missiles
    clc                                         ; but start only 1 column before the cursor as
    adc zaDrawSprWidth                          ; a missile that overlaps can only overlap from 1 col earlier
    tax 
    ldy #8
kml:
    lda (zaMidScreenL), y 
    bit Bit1Mask
    beq :+
    and #%11111100
    sta (zaMidScreenL), y 
:
    iny 
    iny 
    iny 
    dex 
    bne kml

    lda #(XSIZE/2)                              ; get the offset of the cursor into the buffers
    clc 
    adc bufferDraw
    sta zEnemyCol                               ; and put it in the "enemy column"
    jsr gameKillEnemy                           ; and call the game kill to see if there's an enemy to destroy

    rts 

.endproc 

;-----------------------------------------------------------------------------
.proc editSetRadar

    zaDrawSprWidth = tempBlock + 3              ; Parameter
    zaMidScreenL = tempBlock + 5

    lda #4                                      ; need room for a radar so 4 cols
    sta zaDrawSprWidth
    jsr editPreSetEnemy                         ; delete any overlapping enemies
    lda enemyHitType                            ; if set, then an enemy was deleted.  Just exit
    bmi :+
    rts 

:
    ldy #11                                     ; no overlapping enemies.  The offset in the world stream from the ptr
    lda #2                                      ; and the radar type
    ora (zaMidScreenL), y                       ; add it to whatever is there (stage marker)
    sta (zaMidScreenL), y                       ; and save it

    lda bufferDraw                              ; and also insert it into the draw buffers
    clc 
    adc #(XSIZE/2)
    tax 
    clc 
    lda #2
    jsr gameWorldFlag

    rts 
.endproc 

;-----------------------------------------------------------------------------
.proc editSetMissile

    zaDrawSprWidth = tempBlock + 3              ; Parameter
    zaMidScreenL = tempBlock + 5

    lda #2                                      ; need room for a missile (so 2 cols)
    sta zaDrawSprWidth
    jsr editPreSetEnemy                         ; delete overlapping enemies from the world and draw buffers
    lda enemyHitType                            ; if set, enemies were deleted so exit
    bmi :+
    rts 

:
    ldy #11                                     ; no overlapping enemies.  The offset in the world stream from the ptr
    lda #1                                      ; and the missile type
    ora (zaMidScreenL), y                       ; add it to whatever is there (stage marker)
    sta (zaMidScreenL), y                       ; and save it

    lda bufferDraw                              ; also add the missile to the draw buffers
    clc 
    adc #(XSIZE/2)
    tax 
    clc 
    lda #1
    jsr gameWorldFlag

    rts 
.endproc 

;-----------------------------------------------------------------------------
.proc editLoadStage

    jsr gameFindStage                           ; find the stage

    lda #(XSIZE+2)                              ; scroll it on-screen but make sure there are
    sta tempBlock + 1                           ; extra cols of data in the buffers
:
    jsr gameWorldMove                           ; populate the buffers
    dec tempBlock + 1 
    bpl :-

    lda #4                                      ; start the screen draw 2 cols more to the right than
    sta bufferDraw                              ; there's buffer data for so a left scroll will work

    rts 

.endproc 
