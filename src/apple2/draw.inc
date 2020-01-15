;-----------------------------------------------------------------------------
; draw.inc
; Part of penetrator, the zx spectrum game, made for Apple II
;
; Stefan Wessels, 2019
; This is free and unencumbered software released into the public domain.

;-----------------------------------------------------------------------------
.segment "CODE"

;-----------------------------------------------------------------------------
; turns layers on and off so that the backLayer is hidden and the other
; layer is visible to the user
.proc drawPresent

    lda backLayer
    eor #1
    sta backLayer
    beq :+

    bit LOWSCR
    lda #>ram_layer1
    sta zVramH
    rts 

:
    bit HISCR                                   ; Page 2
    lda #>ram_layer0
    sta zVramH
    rts 

.endproc 

;-----------------------------------------------------------------------------
; Clears the world area
.proc drawClearRows

    lda #$00
    ldx #(XSIZE/2)

    ldy backLayer
    beq layer0
    jmp layer1 

layer0:
    .repeat $B0, Row
        sta XINSET + $100 * ($20 + (Row + 8) & $07 << 2 | (Row + 8) & $30 >> 4) | (Row + 8) & $08 << 4 | (Row + 8) & $C0 >> 1 | (Row + 8) & $C0 >> 3, x
    .endrepeat 
    dex 
    bmi done0
    jmp layer0

done0:
    rts 

layer1:
    .repeat $B0, Row
        sta XINSET + $100 * ($40 + (Row + 8) & $07 << 2 | (Row + 8) & $30 >> 4) | (Row + 8) & $08 << 4 | (Row + 8) & $C0 >> 1 | (Row + 8) & $C0 >> 3, x
    .endrepeat 
    dex 
    bmi done1
    jmp layer1

done1:
    rts 

.endproc

;-----------------------------------------------------------------------------
.proc drawClearScreen

    lda #0
    tax 
:
    .repeat $20, B
        sta ram_layer0+(B*256), x
        sta ram_layer1+(B*256), x
    .endrep
    dex 
    beq done
    jmp :-
done:
    rts 

.endproc 

;-----------------------------------------------------------------------------
; Invert the visible screen (the user facing layer)
.proc drawInvertVisibleScreen

    scrnPtrL          = tempBlock + 7           ; past uiWinScreen
    scrnPtrH          = tempBlock + 8

    lda #0
    sta scrnPtrL 
    lda backLayer
    eor #1
    tax 
    lda layersH, x 
    sta scrnPtrH
    ldy #0
    ldx #$20

loop:
    lda (scrnPtrL), y 
    eor #%01111111
    sta (scrnPtrL), y 
    dey 
    bne loop 
    inc scrnPtrH
    dex 
    bne loop 

    rts 

.endproc 

;-----------------------------------------------------------------------------
; Plot a square pixel
; Coords in x and y registers
; width and height in 1's but min "pixel" plot size is 4x4 pixels
.proc drawPlotXY

    width             = tempBlock + 14          ; Param (local variables come after uiWriteName locals)
    height            = tempBlock + 15          ; Param
    side              = tempBlock + 16          ; Param
    rows              = tempBlock + 17          ; internal
    x0                = tempBlock + 18          ; internal
    iWidth            = tempBlock + 19          ; internal

    txa                                         ; x pos in a
    lsr                                         ; go from 80 to 40, odd in carry
    sta x0                                      ; save the x
    lda #0 
    rol                                         ; get carry into a
    sta side                                    ; stor this as which "side" of the byte is active

    tya                                         ; y * 4 since min width is 4 pixels wide, height must be min 4 as well
    asl 
    asl 
    tay                                         ; put the draw height in Y

    lda height                                  ; Adjust the height up by 4x as well
    asl 
    asl 
    sta rows                                    ; save as count for how many rows

loop:
    clc 
    lda rowL, y 
    adc x0                                      ; get the x y address set up
    sta write + 1
    sta write + 4
    lda rowH, y 
    adc zVramH 
    sta write + 2
    sta write + 5

    ldx width                                   ; Write 0 to (width-1)
    dex 
    stx iWidth                                  ; save as the offset index
store:
    txa                                         ; take the index
    clc 
    adc side                                    ; add the side
    and #1                                      ; and with 1 to see what byte to use
    tax                                         ; x now 0 ot 1
    lda plotPix, x                              ; and load the byte to use
    pha                                         ; save for later
    lda iWidth                                  ; index to write tp
    clc 
    adc side                                    ; add side to that as well
    lsr                                         ; turn into a column
    tax                                         ; and put the index offset into x
    pla                                         ; and get back the value to write to the column
write:
    ora PLACEHOLDER, x                          ; merge with the other half of the column
    sta PLACEHOLDER, x                          ; and write that into the column
    dec iWidth                                  ; step back 1 column
    ldx iWidth                                  ; and load that into x
    bpl store                                   ; and if x < 0 then 0 to (width-1) was written
    iny                                         ; fo to next row down
    dec rows                                    ; all rows were written
    bne loop 

    rts 

.endproc 

;-----------------------------------------------------------------------------
; Generic routine to draw a sprite rect to the screen.  Sets zCollision <> 0
; when the pixels from this sprite overlapped with pixels already drawn
; Parameters - see zaDraw* below
.proc drawSprite

    zaDrawWorldHeight = tempBlock + 2           ; parameter - where on screen
    zaDrawSprHeight   = tempBlock + 5           ; parameter - # of rows in sprite
    zaDrawSprWidth    = tempBlock + 3           ; parameter - # of columns in sprite
    zaScreenColLocal  = tempBlock + 4           ; not a parameter - internal for drawing
    zaDrawByte        = tempBlock + 6           ; not a parameter - internal for collisions
    zaDataWidth       = tempBlock + 8           ; not a parameter - internal for collisions

    lda zScreenCol                              ; make a copy since it needs to be modified
    lsr                                         ; 80 to 40
    sta zaScreenColLocal

    ldy zaDrawWorldHeight
    cpy #WORLD_START
    bcc done

rows:
    clc 
    ldx zaDrawSprWidth
    lda rowL, y                                 ; get the screen row start address, low byte
    adc zaScreenColLocal                        ; add the column to the address
    sta write + 1                               ; set eor low address
    sta write + 4                               ; set sta low address
    lda rowH, y                                 ; get the high byte of the screen row start
    adc zVramH                                  ; add the VRAM high byte (for which layer)
    sta write + 2                               ; set the eor hi
    sta write + 5                               ; set the sta hi
offset:
    lda PLACEHOLDER, x                          ; this address is modified by the caller
    sta zaDrawByte                              ; save the byte
write:
    eor PLACEHOLDER,x                           ; get what's on screen xor'd with what's being added
    sta PLACEHOLDER,x                           ; save that to the screen
    and zaDrawByte                              ; mask off any bits that the sprite isn't setting
    cmp zaDrawByte                              ; see if the sprite overlapped something there already
    clc                                         ; cmp can set carry.  keep it clear
    beq :+                                      ; if 0 then no overlap / collision
    inc zCollision                              ; there's a collision, count it
:
    dex 
    bpl offset
    dec zaDrawSprHeight
    beq done
    dey                                         ; go up a row on screen
    cpy #WORLD_START 
    bcc done
    clc 
    lda zaDataWidth
    adc offset + 1
    sta offset + 1
    bcc rows
    inc offset + 2
    bne rows

done:
    rts 

.endproc

;-----------------------------------------------------------------------------
; Draws the player's ship using drawSprite
; Takes no parameters
.proc drawPlayer


    zaDrawWorldHeight = tempBlock + 2
    zaDrawSprHeight   = tempBlock + 5
    zaDrawSprWidth    = tempBlock + 3
    zaDataWidth       = tempBlock + 8
    explFrame         = tempBlock + 9           ; Persistent across a death animation - explosion explFrame

    lda playerShipY
    sta zaDrawWorldHeight

    lda #SHIP_HEIGHT
    sta zaDrawSprHeight

    lda playerShipX                             ; mark where on screen to draw
    sta zScreenCol
    clc 
    adc bufferDraw                              ; make enemyBuffer relative to screen col 0
    sta zEnemyCol                               ; mark where enemies would start that intersect with the ship

    lda playerDead                              ; if playerDead <> 0 then the player
    bne explode                                 ; is dead and should explode

    lda zScreenCol 
    and #1 
    beq evenScreen

oddScreen:
    lda #3                                      ; set the dimensions of the ship to draw
    sta zaDrawSprWidth
    lda #4
    sta zaDataWidth


    lda #<shipU                                 ; point at the ship sprite data
    sta drawSprite::offset + 1
    lda #>shipU
    sta drawSprite::offset + 2
    bne draw

evenScreen:
    lda #2                                      ; set the dimensions of the ship
    sta zaDrawSprWidth
    lda #3
    sta zaDataWidth

    lda #<shipA                                 ; point at the ship sprite data
    sta drawSprite::offset + 1
    lda #>shipA
    sta drawSprite::offset + 2

draw:
    jsr drawSprite                              ; draw the sprite
    lda zCollision                              ; if there's a collision, kill the player
    beq done

    ldx zEnemyCol                               ; player collided - see if it was with an enemy
    ldy #SHIP_WIDTH

colLoop:
    lda playerShipY                             ; if p ship top < enemy.bot then no collision
    sec 
    sbc #SHIP_HEIGHT
    cmp enemyHgtBuffer, x
    bcs notHit
    sec 
    lda enemyHgtBuffer, x                       ; if enemy top < p ship bot then no collision
    sbc #RADAR_HEIGHT
    cmp playerShipY
    bcs notHit

    lda #1                                      ; they intersect so kill the enemy (assume just 1 enemy hit)
    sta zaDrawSprWidth
    jsr gameKillEnemy                           ; kill the enemy that collided with the player
    jmp gamePlayerCollision                     ; and kill the player if it collided with an enemy

notHit:
    inc zEnemyCol                               ; advance the column where the enemy may be that collided with the player
    inx 
    dey                                         ; check all columns for the player ship
    bne colLoop
    jmp gamePlayerCollision                     ; and if no enemy collision, just kill the player

done:
    rts 

explode:
    inc explFrame                               ; advance the local explFrame counter
    lda explFrame 
    lsr                                         ; Slow it down
    tax                                         ; and see if it is .ge. 4
    cpx #4 
    bcc :+
    ldx #0                                      ; if so, wrap back to zero
    stx explFrame

:
    stx audioExplFrame
    lda audioFrame
    ora #AUDIO_EXPLOSION
    sta audioFrame

    lda #3                                      ; set a nonsense width to make the explosion
    sta zaDataWidth                             ; not so recognizable

    lda #3                                      ; set the dimensions of the explosion
    sta zaDrawSprWidth

    lda explosionAL,x                           ; point at the explosion sprite data
    sta drawSprite::offset + 1
    lda explosionAH,x
    sta drawSprite::offset + 2

    jmp drawSprite                              ; draw the explosion sprite instead of the ship

.endproc

;-----------------------------------------------------------------------------
; Draws a single column of a radar sprite (because of clipping)
; Takes the enemy flags as a parameter
; Radar is either 2 byte AB (A being col 0 & 1 and B being 2 & 3) or
; 3 byte CDE where C is blank & 0, D is 1 & 2 and E is 3 and blank)
; On odd, the column can only be 00 because odd 1 is drawn by even A,
; odd 2 is drawn by even 1 (D) and odd 3 is drawn by even B.
; Even can be 0 (A), 1(D), 2(B) or 3(E)
.proc drawRadar

    zaFrame           = tempBlock + 6           ; internal
    zaDrawWorldHeight = tempBlock + 2           ; setup for drawSprite
    zaDrawSprHeight   = tempBlock + 5
    zaDrawSprWidth    = tempBlock + 3
    zaDataWidth       = tempBlock + 8
    zaSkipLength      = tempBlock + 7

    and #%01111100                              ; frame and column
    lsr                                         ; column to 1s
    lsr 
    sta zaFrame                                 ; save

    lda enemyHgtBuffer, x                       ; get the height - 1 pixel above the terrain
    sta zaDrawWorldHeight

    lda #RADAR_HEIGHT
    sta zaDrawSprHeight

    lda zScreenCol                              ; see if it's an even or odd sceeen sub-column
    and #$01
    beq evenScreen

oddScreen:
    lda #3                                      ; odd radar is 3 bytes
    sta zaDataWidth                             ; that's how many bytes between sprite rows

    lda #(XSIZE-1)                              ; check the right edge
    sec                                         ; for clipping
    sbc zScreenCol
    lsr 
    cmp #3
    bcc :+
    lda #2                                      ; maximum 3 (0-2) bytes to write
:
    sta zaDrawSprWidth

    lda zaFrame                                 ; ignore column - will/must be 00
    lsr 
    lsr 
    tax 
    lda radarUL, x                              ; get the frame
    sta drawSprite::offset + 1
    lda radarUH, x
    sta drawSprite::offset + 2
    lda #4                                      ; snd set the step size
    sta zaSkipLength
    bne draw

evenScreen:
    ldx zaFrame                                 ; frame + col is offset in table
    lda radarAL, x                              ; point at data in table index x
    sta drawSprite::offset + 1
    lda radarAH, x
    sta drawSprite::offset + 2
    txa 
    and #%00000011                              ; extract just the column
    tax 
    lda radarAS, x                              ; and see how many bytes to skip based on column
    sta zaSkipLength
    lda radarAD, x                              ; see how wide this sprite is
    sta zaDataWidth
    lda radarAR, x                              ; see how many bytes to render
    sta zaDrawSprWidth

    lda zScreenCol
    cmp #(XSIZE - 2)                            ; last column only draw 1 byte
    bcc draw
    lda #0
    sta zaDrawSprWidth                          ; save as draw width

draw:
    jmp drawSprite

.endproc 

;-----------------------------------------------------------------------------
; Draws a missile on-screen
; Takes the flags as a parameter
.proc drawMissile

    zaFrame           = tempBlock + 6           ; internal
    zaDrawWorldHeight = tempBlock + 2           ; setup for drawSprite
    zaDrawSprHeight   = tempBlock + 5
    zaDrawSprWidth    = tempBlock + 3
    zaDataWidth       = tempBlock + 8
    zaSkipLength      = tempBlock + 7

    tay 

    lda enemyHgtBuffer, x                       ; get the height - 1 pixel above the terrain
    sta zaDrawWorldHeight                       ; while x is still good
    
    tya 
    and #%11000100                              ; launch mask, frame and column
    lsr                                         ; column to 2s
    bit Bit76Mask                               ; check if in the air
    beq :+
    ora #%00000001                              ; yes - force frame 1
:
    and #%00000011
    tax 

    lda #MISSILE_HEIGHT
    sta zaDrawSprHeight

    lda zScreenCol                              ; see if it's an even or odd sceeen sub-column
    and #$01
    beq evenScreen

oddScreen:
    lda #2                                      ; odd missile is 2 bytes
    sta zaDataWidth                             ; that's how many bytes between sprite rows

    lda zScreenCol
    cmp #(XSIZE-1) 
    bne :+
    lda #0
    beq :++
:
    lda #1
:
    sta zaDrawSprWidth

    lda missileUL, x                            ; point at data in table index x
    sta drawSprite::offset + 1
    lda missileUH, x
    sta drawSprite::offset + 2
    lda #2
    sta zaSkipLength
    bne draw

evenScreen:
    lda missileAL, x                            ; point at data in table index x
    sta drawSprite::offset + 1
    lda missileAH, x
    sta drawSprite::offset + 2

    lda #1                                      ; and see how many bytes to skip based on column
    sta zaSkipLength
    lda missileAD, x                            ; see how wide this sprite is
    sta zaDataWidth
    lda #0
    sta zaDrawSprWidth                          ; save as draw width

draw:
    jmp drawSprite

.endproc 

;-----------------------------------------------------------------------------
; Draws a nuke on-screen
; Takes the flags as a parameter
.proc drawNuke

    zaFrame           = tempBlock + 6           ; internal
    zaDrawWorldHeight = tempBlock + 2           ; setup for drawSprite
    zaDrawSprHeight   = tempBlock + 5
    zaDrawSprWidth    = tempBlock + 3
    zaDataWidth       = tempBlock + 8
    zaSkipLength      = tempBlock + 7

    tay 

    lda enemyHgtBuffer, x                       ; get the height - 1 pixel above the terrain
    sta zaDrawWorldHeight                       ; while x is still good
    
    tya 
    ldx #0 
    bit Bit3Mask
    beq :+
    inx 

:
    lda #NUKE_HEIGHT
    sta zaDrawSprHeight

    lda zScreenCol                              ; see if it's an even or odd sceeen sub-column
    and #$01
    beq evenScreen

oddScreen:
    lda #2                                      ; odd nuke is 2 bytes
    sta zaDataWidth                             ; that's how many bytes between sprite rows

    lda zScreenCol                              ; see if clipping to right hand side is needed
    cmp #(XSIZE-1) 
    bne :+
    lda #0
    beq :++
:
    lda #1
:
    sta zaDrawSprWidth                          ; save how many bytes to render

    lda nukeUL, x                               ; point at data in table index x
    sta drawSprite::offset + 1
    lda nukeUH, x
    sta drawSprite::offset + 2

    lda #2                                      ; odd screen nuke is 2 bytes (col can't be 1)
    sta zaSkipLength
    bne draw

evenScreen:
    lda nukeAD, x                               ; see how wide this sprite is
    sta zaDataWidth

    lda #0
    sta zaDrawSprWidth                          ; save as draw width

    lda nukeAL, x                               ; point at data in table index x
    sta drawSprite::offset + 1
    lda nukeAH, x
    sta drawSprite::offset + 2

    lda #1                                      ; even screen nuke is 1 byte (col 0 or 1)
    sta zaSkipLength

draw:
    jmp drawSprite

.endproc 

;-----------------------------------------------------------------------------
; Takes the flags as a parameter
.proc drawMonster

    zaFrame           = tempBlock + 6           ; internal
    zaDrawWorldHeight = tempBlock + 2           ; setup for drawSprite
    zaDrawSprHeight   = tempBlock + 5
    zaDrawSprWidth    = tempBlock + 3
    zaDataWidth       = tempBlock + 8
    zaSkipLength      = tempBlock + 7

    tay 

    lda enemyHgtBuffer, x                       ; get the height - 1 pixel above the terrain
    sta zaDrawWorldHeight                       ; while x is still good
    
    lda #2                                      ; monster is always 2 bytes
    sta zaDataWidth                             ; that's how many bytes between sprite rows

    tya 
    lsr 
    lsr 
    and #%00000011
    tax 

    lda #MONSTER_HEIGHT
    sta zaDrawSprHeight

    lda zScreenCol                              ; see if it's an even or odd sceeen sub-column
    and #$01
    beq evenScreen

oddScreen:
    lda zScreenCol                              ; see if clipping to right hand side is needed
    cmp #(XSIZE-1) 
    bne :+
    lda #0
    beq :++
:
    lda #1
:
    sta zaDrawSprWidth                          ; save how many bytes to render

    lda #<monsterU                              ; point at data in table index x
    sta drawSprite::offset + 1
    lda #>monsterU
    sta drawSprite::offset + 2

    lda #3                                      ; odd screen monster is 2 bytes (col can't be 1)
    sta zaSkipLength
    bne draw

evenScreen:
    lda monsterAR, x
    sta zaDrawSprWidth                          ; save as draw width

    lda monsterAL, x                            ; point at data in table index x
    sta drawSprite::offset + 1
    lda monsterAH, x
    sta drawSprite::offset + 2

    lda monsterAS, x                            ; even screen monster is 1 byte (col 0 or 1)
    sta zaSkipLength

draw:
    jmp drawSprite

.endproc 

;-----------------------------------------------------------------------------
; Draws all the bombs on-screen
; Takes no parameters
.proc drawBombs 

    zaBombIndex       = tempBlock + 1           ; internal - which of the bombs
    zaDrawWorldHeight = tempBlock + 2           ; for drawSprite
    zaDrawSprHeight   = tempBlock + 5
    zaDrawSprWidth    = tempBlock + 3
    zaDataWidth       = tempBlock + 8

    ldx #NUM_BOMBS - 1                          ; 0 based index

loop:
    lda bombY, x                                ; get the height of the bomb
    bne doBomb                                  ; 0 is not active, otherwise height

nextBomb:
    dex 
    bpl loop
    clc 
    rts 

doBomb:
    stx zaBombIndex
    sta zaDrawWorldHeight                       ; save the height

    lda bombX, x
    sta zScreenCol
    and #$01
    beq evenScreen

oddScreen:
    lda #1
    sta zaDrawSprWidth                          ; save as draw width
    lda #2 
    sta zaDataWidth

    lda bombDir, x                              ; select the sprite based on the state
    tax 
    beq :+                                      ; if .gt. 0 then it's bomb 1
    ldx #1
:
    lda bombUL, x                               ; forward travelling bomb
    sta drawSprite::offset + 1
    lda bombUH, x
    sta drawSprite::offset + 2
    lda bombH, x 
    sta zaDrawSprHeight
    bne draw

evenScreen:
    lda #0
    sta zaDrawSprWidth                          ; save as draw width
    lda #1 
    sta zaDataWidth

    lda bombDir, x                              ; select the sprite based on the state
    tax 
    beq :+
    ldx #1
:
    lda bombAL, x                               ; forward travelling bomb
    sta drawSprite::offset + 1
    lda bombAH, x
    sta drawSprite::offset + 2
    lda bombH, x 
    sta zaDrawSprHeight
    
draw:
    jsr drawSprite

    lda zCollision
    beq nextBombX

    lda zScreenCol 
    clc 
    adc bufferDraw                              ; make enemyBuffer relative to screen col 0
    tax 
    ldy #2

tryEnemy:
    stx zEnemyCol                               ; may be a collision with enemy
    lda enemyHgtBuffer, x                       ; is there an enemy in this row?
    beq noBombHit                               ; no - maybe next column?

checkCol:
    sec 
    sbc #RADAR_HEIGHT                           ; assume tallest enemy (favors the player)
    cmp zaDrawWorldHeight                       ; compare to bombY
    bcs noBombHit                               ; E top .le. b bot (carry clear) is maybe collision
    lda zaDrawWorldHeight                       ; get the bombY
    sec 
    sbc #BOMB_HEIGHT                            ; calc bomb top
    cmp enemyHgtBuffer, x                       ; B top .le. E bot (carry clear) is a collision
    bcc bombHit

noBombHit:
    inx                                         ; maybe bomb spans 2 cols - check next column
    dey                                         ; see if bomb is over 2 cols
    beq bombDies                                ; if 0 done all columns
    bne tryEnemy                                ; JuMP

bombHit:
    lda #1                                      ; enemy is in this 1 column
    sta zaDrawSprWidth

    jsr gameKillEnemy                           ; see if the bomb killed an enemy
    lda enemyHitType                            ; figure out if there's score
    bmi bombDies 
    jsr gameAddScore

bombDies:
    ldx zaBombIndex 
    lda #0                                      ; now kill the bomb itself
    sta bombY, x                                ; by setting its height to 0
    sta zCollision

nextBombX:
    ldx zaBombIndex
    jmp nextBomb

.endproc


;-----------------------------------------------------------------------------
.proc drawBullets

    zaOffset          = tempBlock + 1
    zaDrawSprWidth    = tempBlock + 3
    zaBulletCol       = tempBlock + 5
    zaMask            = tempBlock + 6

    lda #0                                      ; work by screen columns
    sta zScreenCol                              ; starting at 0
    
    ldx bulletIndex                             ; x is the index for the bullet

loop:
    lda bulletsBuffer, x                        ; get the column
    bne bullet                                  ; and if non-zero, there's a bullet in this column

next:
    inx                                         ; next x
    inc zScreenCol                              ; and next screen column
    lda zScreenCol                              ; see if the screen column is
    cmp #XSIZE                                  ; past the end of the screen
    bcc loop                                    ; if not, keep going
    lda #0 
    sta bulletsBuffer, x                        ; Kill any overflow bullets
    inx 
    sta bulletsBuffer, x                        ; Kill any overflow bullets
    clc                                         ; done - clear carry and end
    rts 

bullet:
    stx zaBulletCol                             ; where the bullet starts
    tay                                         ; height now in Y
    lda zScreenCol                              ; game column in a
    tax                                         ; save in x
    lsr                                         ; divide by 2 for byte position
    clc                                         ; lsr could set carry
    adc rowL, y                                 ; get the write locations set up
    sta write + 1
    sta write + 4
    lda rowH, y
    adc zVramH
    sta write + 2
    sta write + 5

    txa                                         ; restore game col
    and #1                                      ; test for odd/even
    beq evenScreen                              ; and go appropriate

oddScreen:
    txa                                         ; restore game column
    cmp #(XSIZE - 1)                            ; see if it's at the right edge
    bne :+
    ldx #1 
    stx zaOffset                                ; set how many columns to check
    ldx #0                                      ; will only write 1 byte, the last right col
    lda #%01111000
    bne :++
:
    ldx #3
    stx zaOffset
    ldx #1                                      ; write 2 bytes
    lda #%01111111                              ; the right col + the whole next byte (start there)
:
    sta zaMask 
    bne write

evenScreen:
    txa                                         ; restore game column
    cmp #(XSIZE - 2)                            ; see if it's at the right edge even screen
    bne :+
    ldx #2
    stx zaOffset
    ldx #0                                      ; will only write 1 byte
    lda #%01111111                              ; the whole byte is a bullet
    bne :++
:
    ldx #3
    stx zaOffset
    ldx #1                                      ; write 2 bytes
    lda #%00001111                              ; the right col + the whole next byte (start there)
:
    sta zaMask 

write:                                          ; bullet is a solid line
    eor PLACEHOLDER, x
    sta PLACEHOLDER, x
    and zaMask
    cmp zaMask
    bne collision
    dex 
    bmi step
    lda zaMask 
    cmp #%01111111
    beq :+
    lda #%01111111
    bne :++
:
    lda #%01111000
:
    sta zaMask
    bne write

step:                                           ; setp past the bullet
    inc zScreenCol                              ; and next screen column
    ldx zaBulletCol
    inx 
    jmp next                                    ; this will step past second part of the bullet

collision:
    lda zScreenCol                              ; index into screem
    clc 
    adc bufferDraw                              ; and make relative to enemy buffer
    tay 
:
    lda enemyHgtBuffer, y 
    bne hasEnemy 
    iny 
    dec zaOffset
    bpl :-
    bmi endBullet
    
hasEnemy:
    ldx zaBulletCol                             ; load the bullet index in x
    sec 
    sbc bulletsBuffer, x                        ; see where it hit relative to enemy
    bcc endBullet                               ; bullet hit lower
    cmp #RADAR_HEIGHT                           ; since it hit anyway, use tallest enemy
    bcs endBullet                               ; it hit higher

    lda #1
    sta zaDrawSprWidth                          ; gameKillEnemy looks at this to see how wide to look for a kill
    sty zEnemyCol
    jsr gameKillEnemy
    lda enemyHitType                            ; figure out if there's score
    bmi endBullet
    jsr gameAddScore

endBullet:
    ldx zaBulletCol                             ; load the bullet index in x
    lda #0
    sta bulletsBuffer, x
    beq step                                    ; JuMP

.endproc 

;-----------------------------------------------------------------------------
.proc drawExplosions

    zaFrame           = tempBlock + 6           ; internal
    zaDrawWorldHeight = tempBlock + 2           ; setup for drawSprite
    zaDrawSprHeight   = tempBlock + 5
    zaDrawSprWidth    = tempBlock + 3
    zaDataWidth       = tempBlock + 8
    zaExplodeCol      = tempBlock + 10

    lda #0                                      ; work by screen columns
    sta zScreenCol                              ; starting at 0
    
    ldx bufferDraw                              ; x is the index for the bullet

loop:
    lda explosionBuffer, x                      ; get the column
    bne explode                                 ; and if non-zero, there's a bullet in this column

next:
    inx                                         ; next x
    inc zScreenCol                              ; and next screen column
    lda zScreenCol                              ; see if the screen column is
    cmp #XSIZE                                  ; past the end of the screen
    bcc loop                                    ; if not, keep going
    lda #0
    sta zCollision
    clc                                         ; done - clear carry and end
    rts 

explode:
    stx zaExplodeCol

    pha 
    and #%00000011                              ; frame
    sta zaFrame                                 ; save

    pla 
    and #%11111000                              ; height
    sta zaDrawWorldHeight

    lda #EXPLOSION_HEIGHT
    sta zaDrawSprHeight

    lda zScreenCol                              ; see if it's an even or odd sceeen sub-column
    and #$01
    beq evenScreen

oddScreen:
    lda #3                                      ; odd explosion is 3 bytes
    sta zaDataWidth                             ; that's how many bytes between sprite rows

    lda #(XSIZE-1)                              ; check the right edge
    sec                                         ; for clipping
    sbc zScreenCol
    lsr 
    cmp #3
    bcc :+
    lda #2                                      ; maximum 3 (0-2) bytes to write
:
    sta zaDrawSprWidth

    ldx zaFrame                                 ; frame + col is offset in table
    lda explosionUL, x                          ; get the frame
    sta drawSprite::offset + 1
    lda explosionUH, x
    sta drawSprite::offset + 2
    bne draw

evenScreen:
    lda #2                                      ; odd explosion is 3 bytes
    sta zaDataWidth                             ; that's how many bytes between sprite rows

    ldx zaFrame                                 ; frame + col is offset in table
    lda explosionAL, x                          ; point at data in table index x
    sta drawSprite::offset + 1
    lda explosionAH, x
    sta drawSprite::offset + 2

    lda zScreenCol
    cmp #(XSIZE - 2)                            ; last column only draw 1 byte
    bcc :+
    lda #0
    beq :++
:
    lda #1
:
    sta zaDrawSprWidth                          ; save as draw width

draw:
    jsr drawSprite
    ldx zaExplodeCol
    jmp next

.endproc

;-----------------------------------------------------------------------------
; Iterate over the enemies on screen and get them drawn to the back buffer
; Parameter - zCollision must be 0 when called
.proc drawEnemies

    zaEnemyType       = tempBlock + 1           ; parameter to the specific enemy draw call
    zaDrawSprWidth    = tempBlock + 3
    zaSkipLength      = tempBlock + 7

    lda terrainOrigin                           ; will always be 0 since this isn't called till screen fully scrolled in
    sta zScreenCol                              ; start drawing here on-screen

    ldx bufferDraw                              ; make enemyBuffer relative to screen col 0
    stx zEnemyCol                               ; start here processing enemies

loop:
    lda enemyBuffer, x                          ; check this column for a (partial) enemy (just a strip)
    beq continue                                ; skip non-enemy columns

    sta zaEnemyType                             ; save the flags

    bit Bit34Mask                               ; check the column
    beq evalType                                ; col0 always evaluated
    lda zScreenCol                              ; Get the column
    bit Bit1Mask                                ; see if odd or even
    bne continue                                ; if odd, ignore
    lda zaEnemyType                             ; if even, need to draw type

evalType:
    bit Bit1Mask                                ; see if this is a missile (lsb set)
    bne missileOrNuke                           ; bits match so it's a missile

    bit Bit2Mask                                ; see if it's a radar
    bne doRadar

    jsr drawMonster                             ; lower 2 bits clear is a monster
    bcc colchk                                  ; drawSprite always exits with carry clear so this is a jmp

doRadar:
    jsr drawRadar
    bcc colchk                                  ; will always be clear, so in effect jmp

missileOrNuke:
    bit Bit2Mask
    beq missile
    jsr drawNuke
    bcc colchk                                  ; JuMP

missile:
    jsr drawMissile

colchk:
    lda gameMode                                ; in the edit mode, collision detection doesn't do anything
    cmp #GAME_MODE_EDIT
    clc 
    beq step
    lda zCollision                              ; see if this enemy is in collision (for missiles with terrain)
    beq step
    inc zaDrawSprWidth
    jsr gameKillEnemy

step: 
    lda zaSkipLength 
    adc zScreenCol
    cmp #XSIZE
    bcs done
    sta zScreenCol
    clc 
    lda zaSkipLength 
    adc zEnemyCol
    sta zEnemyCol
    tax 
    jmp loop

continue:
    inc zScreenCol
    ldx zScreenCol
    cpx #XSIZE
    bcs done 
    inc zEnemyCol
    ldx zEnemyCol
    jmp loop 

done:
    rts 

.endproc
