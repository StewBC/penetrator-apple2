;-----------------------------------------------------------------------------
; variables.inc
; Part of penetrator, the zx spectrum game, made for Apple II
;
; Stefan Wessels, 2019
; This is free and unencumbered software released into the public domain.

;-----------------------------------------------------------------------------
.segment "DATA"

;-----------------------------------------------------------------------------
; Player specific variables
rsStart:                                        ; runtime stats "struct".  lives MUST be 1st
lives:              .res 1                      ; Chances the player gets (decimal mode), indexed by activePlayer
score:              .res 3                      ; activePlayer's score is copied here for gameplay
stage:              .res 1                      ; offset to stages in worldDataStart - needed for reset
direction:          .res 1                      ; 0 = right, 1 = left (fly reverse through level)
makeMonster:        .res NUM_STAGES             ; 1 in stage offset lookup turns missiles into monsters
rsEnd:

playerStats:        .res ((rsEnd-rsStart) * 2)  ; 2 copies of stats, for p1 and p2 is 2p game

;-----------------------------------------------------------------------------
; High scores table saved to disc
scoresTable:
highScores:
highScore1:         .res 3
highScore2:         .res 3
highScore3:         .res 3
highScore4:         .res 3
highScore5:         .res 3
highScoresEnd:

textHS:
textHighScore1:   .asciiz "123456"
textHighScore2:   .asciiz "123456"
textHighScore3:   .asciiz "123456"
textHighScore4:   .asciiz "123456"
textHighScore5:   .asciiz "123456"
textHSEnd:
scoresTableEnd:

;-----------------------------------------------------------------------------
; user facing text area
textStage:        .asciiz "1"
textPlayerNum:    .asciiz "1"
textNumber:       .asciiz "1234567"             ; score display 10x score - extra digit needed
textDangerBar:    .asciiz "123"
textEnd:

textFileName:     .asciiz "PENEWORLD01"         ; default file name
textFilePad:      .asciiz ""                    ; This makes the null terminator on textFileName viable and a 12 char filename
textFileNameEnd:
szHex:            .asciiz "  "

;-----------------------------------------------------------------------------
; per "run" buffers
bombX:              .res NUM_BOMBS              ; 2 bombs allowed, their x values
bombY:              .res NUM_BOMBS              ; Y values for 2 bombs (0=available)
bombDir:            .res NUM_BOMBS              ; direction of travel (and thus sprite)
worldTBuffer:       .res BUFLENGTH              ; holds the world bottom height
worldBBuffer:       .res BUFLENGTH              ; holds the world top height
enemyBuffer:        .res BUFLENGTH              ; holds enemy sprite draw data
enemyHgtBuffer:     .res BUFLENGTH              ; holds enemy sprite draw height data
bulletsBuffer:      .res BUFLENGTH              ; holds the bullet heights (0=no bullet)
explosionBuffer:    .res BUFLENGTH              ; holds the explosion heights (0=no explosion)
