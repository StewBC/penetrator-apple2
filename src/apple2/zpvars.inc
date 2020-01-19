;-----------------------------------------------------------------------------
; zpvars.inc
; Part of penetrator, the zx spectrum game, made for Apple II
;
; Stefan Wessels, 2019
; This is free and unencumbered software released into the public domain.

;-----------------------------------------------------------------------------
.segment "ZEROPAGE"

;-----------------------------------------------------------------------------
; zero page variables
zWorldPtr:       .res 2                         ; where data is next read from the world stream
zVramH:          .res 1                         ; for HRG writes, points at the Hi of the back bank RAM
zScreenCol:      .res 1                         ; used for drawing - the column to draw
zEnemyCol:       .res 1                         ; used for enemies - the column to process
zCollision:      .res 1                         ; drawSprite will put a 1 in when there's a write collision
tempBlock:       .res 22                        ; used by different routines that need local storage

;-----------------------------------------------------------------------------
; zero page constants
; Bit patterns for the bit instruction
Bit1Mask:        .res 1 
Bit2Mask:        .res 1
Bit3Mask:        .res 1
Bit4Mask:        .res 1
Bit5Mask:        .res 1
Bit6Mask:        .res 1
Bit7Mask:        .res 1
Bit8Mask:        .res 1
Bit12Mask:       .res 1
Bit34Mask:       .res 1
Bit76Mask:       .res 1
Bit8432Mask:     .res 1
Bit8765Mask:     .res 1

;-----------------------------------------------------------------------------
; Game variables
numPlayers:      .res 1                         ; [0|1] where 0 is a 1 player game, 1 = 2 player game
activePlayer:    .res 1                         ; 0 for player 1, 1 for player 2
gameMode:        .res 1                         ; 0 for gameplay, 1 for trainer, 2 for edit

; per "run" variables
playerShipX:     .res 1                         ; x position in columns
playerShipY:     .res 1                         ; y position in rows
terrainOrigin:   .res 1                         ; from 0 to XSize - where to start world draw on screen
stopScrolling:   .res 1                         ; 1 - world will not move (for death)
playerDead:      .res 1                         ; 0 - gamePlay flow, > 0 players dead
monsterCnt:      .res 1                         ; skip # missiles before turning one into a monster
inFlight:        .res 2                         ; count launched missiles for launch control
victory:         .res 1                         ; 0 = not done, 1 = hit the nuke so done with run
nextStage:       .res 1                         ; -1 nothing, 0 at next stage, >0 count down
bufferInsert:    .res 1                         ; index where data gets added to buffers (right hand of screen)
bufferDraw:      .res 1                         ; index that corresponds to draw start (usually col 0 of screen)
bulletIndex:     .res 1                         ; like bufferDraw but bullets "travel" in opposite direction
dangerTickCount: .res 1                         ; how often a dot is added to the danger line, when the world moves
dangerTickIdx:   .res 1                         ; the index into the text that holds the danger line of dots
moveHorz:        .res 1                         ; keeps track of horizontal movement - only moves every 2nd frame
moveVert:        .res 1                         ; keeps track of vertical movement - moves 4 rows but over 2 frames

; helper variables
updateHUD:       .res 1                         ; 0 - don't draw, 1+ - clean and redraw
flipFlop:        .res 1                         ; counter to advance explosion frames
backLayer:       .res 1                         ; [0|1] - Layer not shown to user *=(to be drawn to)
enemyHit:        .res 1                         ; the flags of the enemy destroyed in gameKillEnemy
enemyHitType:    .res 1                         ; 0 = nuke, 1 = missile, 2 = monster, 3 = radar, $ff - nothing
lastInput:       .res 1                         ; holds the value of the prev joystick frame
pause:           .res 1                         ; <> 0 when the game is paused
fireCoolDown:    .res 1                         ; decay to zero before fire allowed
pathPos:         .res 1                         ; index to start of file name in path


; audio variables
audioMask:       .res 1                         ; which audio "channels" are active (and with audioFrame)
audioFrame:      .res 1                         ; prioritized mask for sounds to make (msb to lsb)
audioExplFrame:  .res 1                         ; explosion frame index, for sound