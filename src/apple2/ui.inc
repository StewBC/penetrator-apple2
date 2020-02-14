;-----------------------------------------------------------------------------
; ui.inc
; Part of penetrator, the zx spectrum game, made for Apple II
;
; Stefan Wessels, 2019
; This is free and unencumbered software released into the public domain.

;-----------------------------------------------------------------------------
.segment "CODE"

;-----------------------------------------------------------------------------
.proc uiTitleScreen 

    zaCounterL = tempBlock + 17                 ; timer for flipping text
    zaCounterH = tempBlock + 18
    zaState    = tempBlock + 19                 ; which text to show

    lda #0
    bit KBDSTRB                                 ; flush the keyboard buffer before uiWriteName
    sta lastInput

    jsr drawClearScreen
    jsr uiWriteName                             ; also looks for a key press

    jsr uiShowHighScores 
    jsr drawPresent                             ; present and
    jsr uiShowHighScores                        ; show again so it's on both layers

loopo:
    lda #0                                      ; set all keys to down for joystick
    bit KBDSTRB
    sta lastInput                               ; debounce the last key if it was a joy button
    sta zaCounterL
    sta zaState 

loop:
    lda #$08
    sta zaCounterH

    lda zaState 
    bne :+
    print textPhilip   , #(00), #(8*21)         ; the text shows in 3 stages
    print textCopyright, #(00), #(8*22)
    lda zaState 

:
    cmp #1
    bne :+
    print textApple2   , #(00), #(8*21)
    print textStefan   , #(00), #(8*22)
    lda zaState 

:
    cmp #2
    bne :+
    print textSupport   , #(00), #(8*21)
    print textOliver    , #(00), #(8*22)
    lda zaState 

:
    cmp #3
    bne :+
    print textSource   , #(00), #(8*21)
    print textGitHub   , #(00), #(8*22)

:
    jsr drawPresent                             ; show the sceen

:
    jsr inputCheckForInput                      ; wait for user interaction
    bne done
    dec zaCounterL
    bne :-
    dec zaCounterH
    bne :-

    inc zaState
    lda zaState 
    cmp #4
    bcc :+
    jmp loopo
:
    jmp loop

done:
    rts 

.endproc 

;-----------------------------------------------------------------------------
.proc uiShowHighScores

    zaStrL     = tempBlock + 1
    zaStrH     = tempBlock + 2
    zaForce    = tempBlock + 3
    textX      = tempBlock + 10
    textY      = tempBlock + 11
    scorePtrL  = tempBlock + 14                 ; 2 + 3 used by textOut
    scorePtrH  = tempBlock + 15
    scoreIdx   = tempBlock + 16
    scoreShown = tempBlock + 17

    lda #<highScores                            ; set up a pointer to the scores
    sta scorePtrL
    lda #>highScores
    sta scorePtrH
    lda #0                                      ; and init two counters
    sta scoreIdx                                ; which high-score is being processed
    sta scoreShown                              ; were any high-scores printed

loop: 
    ldy #3                                      ; 3 bytes to a score
    ldx #0                                      ; don't force a number
    lda scorePtrL                               ; get the score to convert
    sta zaStrL
    lda scorePtrH
    sta zaStrH
    jsr textBCDtoSZ                             ; convert it
    dex                                         ; step back from the null-terminator
    lda textNumber, x                           ; get the last character out
    cmp #' '                                    ; was it a space
    bne show                                    ; no - there was a score so show it
    beq heading                                 ; as soon as there's no score there won't be any more

next:
    inc scoreIdx                                ; go to next score
    lda scoreIdx
    cmp #5                                      ; all 5 scores shown?
    bcc cont                                    ; if not, carry on processing

heading: 
    lda scoreShown                              ; were any score printed
    beq done                                    ; if not, nothing more
    print textHighScores, #(11), #(8*0)         ; Show the scores' heading

done:
    rts 

cont:
    lda scorePtrL                               ; advance the score ptr to the next 3 bytes
    adc #3
    sta scorePtrL
    bcc loop 
    inc scorePtrH
    bne loop                                    ; JuMP

show:                                           ; a high-score was set, so show it
    inc scoreShown
    inx                                         ; back to the null terminator
    lda #'0'                                    ; Show score as 10x the actual score
    sta textNumber, x                           ; append the 0 to the buffer
    ldx scoreIdx                                ; get the index of which score
    lda #<textNumber                            ; print the converted score text
    sta zaStrL
    lda #>textNumber
    sta zaStrH 
    lda scorePosX, x                            ; get the X position of where
    sta textX
    lda scoreTextPosY, x                        ; and the Y position
    sta textY
    jsr textOut                                 ; now render the text

    ldx scoreIdx                                ; get the score index again
    lda scoreTextOffset, x                      ; and figure out the start of the name text for this score
    clc 
    adc #<textHS                                ; offset from where the names are stored
    sta zaStrL
    lda #0
    adc #>textHS
    sta zaStrH
    lda scoreTextPosX, x                        ; get the x position
    sta textX 
    lda scoreTextPosY, x                        ; and the y position
    sta textY 
    jsr textOut                                 ; and render the text

    jmp next                                    ; and go to the next score

.endproc

;-----------------------------------------------------------------------------
; Uses line drawing to spell out the Penetrator logo
.proc uiWriteName

    x0      = tempBlock + 0                     ; local variables
    x1      = tempBlock + 1
    y0      = tempBlock + 2
    y1      = tempBlock + 3
    dx      = tempBlock + 4
    dy      = tempBlock + 5
    sx      = tempBlock + 6
    sy      = tempBlock + 7
    dyNeg   = tempBlock + 8
    err     = tempBlock + 9
    err2    = tempBlock + 10
    index   = tempBlock + 11
    remain  = tempBlock + 12
    skip    = tempBlock + 13
    width   = tempBlock + 14
    height  = tempBlock + 15

    lda #0
    sta audioExplFrame                          ; use this as a delay counter for logo sound

    lda #((dataLogoLinesEnd - dataLogoLines) / 4)
    sta remain
    
    ldx #0
    stx index                                   ; current line
    stx skip                                    ; impatient user

    lda #1                                      ; set up the plot
    sta width

    lda #1
    sta height

loop:
    dec remain                                  ; iterate over all lines
    bpl :+
    lda #0
    sta audioExplFrame
    rts 
: 
    ldx index 
    
    lda dataLogoLines,x                         ; Load x0,y0 x1,y1 into variables
    sta x0
    inx 
    lda dataLogoLines,x
    sta y0
    inx 
    lda dataLogoLines,x
    sta x1
    inx 
    lda dataLogoLines,x
    sta y1
    inx 
    
    stx index                                   ; update the index to next line params

    ldy #2                                      ; dx = abs(x1-x0) - sx=2
    lda x1 
    sec 
    sbc x0 
    bcs :+                                      ; x1 >= x0
    eor #$ff                                    ; abs
    adc #1
    ldy #0                                      ; sx=0

:
    sta dx 
    dey                                         ; sx=-1|1
    sty sx 

    
    ldy #2                                      ; dy = abs(y1-y0) - sy=2
    lda y1 
    sec 
    sbc y0 
    bcs :+                                      ; y1 >= y0
    eor #$ff                                    ; abs
    adc #1
    ldy #0                                      ; sy=0
    sec 
:
    sta dy 
    lda #0
    sbc dy 
    sta dyNeg                                   ; = -dy
    dey                                         ; sy=-1|1
    sty sy 

    
    lda dx                                      ; err = dx-dy
    sec 
    sbc dy 
    sta err 

plot:
    ldx x0
    ldy y0 
    jsr drawPlotXY                              ; plot a "pixel" at x y

    jsr drawPresent                             ; present this plot

    ldx x0                                      ; do for the 2nd buffer the same
    ldy y0 
    jsr drawPlotXY 

    lda skip                                    ; if key/button pressed - fast draw logo
    bne :+
    lda audioMask
    beq nosnd
snd:    
    ldy audioExplFrame
    ldx #4
    jsr playNote                                ; play the note
    dec audioExplFrame                          ; alter the note
    dec audioExplFrame
    bmi nosnd                                   ; variable decay so that the duration
    dec audioExplFrame                          ; is a close match to when the logo ends
    dec audioExplFrame
nosnd:
    jsr inputCheckForInput                      ; check for user interaction
    sta skip 
    bne :+
    wait #$01

:
    lda x0                                      ; if(x0=x1 && y0=y1) done
    cmp x1 
    bne :+ 
    lda y0 
    cmp y1 
    bne :+ 
    jmp loop 

:
    lda err                                     ; e2 = err << 1
    asl 
    sta err2 

    
    lda dyNeg                                   ; if(e2 > -dy)
    sec 
    sbc err2 
    bpl :+ 

    
    lda err                                     ; err = err - dy
    sec 
    sbc dy 
    sta err 
    
    lda x0                                      ; x0 = x0 + sx
    clc 
    adc sx 
    sta x0 

:
    lda dx                                      ; if(e2 <  dx)
    sec 
    sbc err2 
    bpl :+ 
    jmp plot 

:
    lda err                                     ; err = err + dx
    clc 
    adc dx 
    sta err 
    
    lda y0                                      ; y0 = y0 + sy
    clc 
    adc sy 
    sta y0 
    jmp plot 

.endproc 

;-----------------------------------------------------------------------------
; Shows the main menu and waits for user choice
.proc uiMainMenu

    jsr drawClearScreen

redraw:
    print textInstructions, #(10), #(8*04)
    print textPress       , #(13), #(8*08)
    print textUnderline   , #(13), #(8*09)
    print textOneTwo      , #(00), #(8*11)
    print textTrain       , #(00), #(8*12)
    print textEdit        , #(02), #(8*13)
    print textLoad        , #(01), #(8*14)
    print textSirens      , #(05), #(8*15)
    print textQuit        , #(12), #(8*16)

    lda audioMask 
    beq none

    bit Bit8432Mask 
    bne :+
    print textAudio2      , #(20), #(8*15)
    jmp present

:
    print textAudio1      , #(20), #(8*15)
    jmp present

none:
    print textAudio3      , #(20), #(8*15)

present:
    jsr drawPresent

    lda #GAME_MODE_PLAY                         ; assume the player will play (play is 0)
    sta gameMode 
    sta stage                                   ; and set the stage to 0
    sta lastInput

loop:
    lda #(optionsEnd-options)                   ; how many options on this menu
    ldx #<options                               ; x - lo options "string"
    ldy #>options                               ; y - hi options "string"
    jsr uiHandleOptions                         ; on return, x is the option selected (0 based)

    cpx #(optionsEnd-options) + 1
    bne :+
    rts 

:
    cpx #2                                      ; 0 & 1 for 1/2 player game
    bcs train                                   ; greater means another option

    stx numPlayers                              ; x happens to be the value for numPlayers
    jmp gamePlay                                ; go gameplay and return to title screen

train:
    bne edit                                    ; still on the cpx #2 here
    jmp uiTrainerMode                           ; 2 means train, and then return to title screen

edit:
    cpx #3
    bne load

    jsr editLoop                                ; 3 means edit
    jmp uiMainMenu                              ; and go to main menu, not title

load:
    cpx #4
    bne sirens
    lda #1                                      ; 1 means load
    jsr uiFileLoadSave                          ; call the code to do file name and load
    jmp uiMainMenu

sirens:                                         ; For Apple II this is the "set sound type"
    cpx #5
    bne quit

    lda audioMask 
    beq audioSetAll
    bit Bit8432Mask 
    bne :+
    lda #0
    beq :++
:
    lda #(AUDIO_EXPLOSION | AUDIO_BOMB_DROP | AUDIO_FIRE | AUDIO_UI_TICK)
    bne :+

audioSetAll:
    lda #%01111111
:
    sta audioMask
    jmp redraw

quit:
    inc PWREDUP                                 ; Make sure this isn't a power vector

    jsr MLI                                     ; call the prodos mli ($bf00) - never returns here
    .byte QUIT_CALL                             ; call type = quit
    .addr quitParam                             ; pointer to parameter table

quitParam: 
    .byte 4                                     ; number of parameters is 4
    .byte 0                                     ; quit type
    .word 0000                                  ; reserved
    .byte 0                                     ; reserved
    .word 0000                                  ; reserved

options: .byte "12TELSQ"
optionsEnd:

.endproc

;-----------------------------------------------------------------------------
; Shows the trainer menu asking for a stage to train (1-4).  Allows
; backspace to go back to main menu
.proc uiTrainerMode

    jsr drawClearScreen

    print textTrainingMode, #(11), #(8*04)
    print textPressStage  , #(02), #(8*12)
    print textOneToFour   , #(08), #(8*13)
    print textBackup      , #(03), #(8*23)

    jsr drawPresent

    lda #GAME_MODE_TRAIN
    sta gameMode

    lda #(optionsEnd-options)
    ldx #<options 
    ldy #>options 
    jsr uiHandleOptions

    cpx #4                                      ; backspace option
    bcc stageSel
    cpx #(optionsEnd-options)+1                 ; time out goes to title, not main menu like backup
    bne :+
    rts 
:
    jmp uiMainMenu                              ; on backspace go back to main menu

stageSel:
    stx stage
    jmp gamePlay
    rts 

options: .byte  "1234", $1b                     ; $1b is ESC
optionsEnd:

.endproc 

;-----------------------------------------------------------------------------
; Take the number of options in accumulator
; Take a pointer to a number of options in x (lo) y (hi)
; returns when an option was selected, the index of the selected option in x
.proc uiHandleOptions

    sta len + 1                                 ; set the length for the number of options to look at
    stx option + 1                              ; set address of the options "string"
    sty option + 2

    lda #0
    bit KBDSTRB                                 ; flush the keyboard buffer
    sta lastInput
    
loop:
    jsr inputCheckForInput                      ; get the input
    beq loop                                    ; no input keep looping
    bit KBDSTRB                                 ; reset the keyboard buffer
    cmp len + 1                                 ; if a >= num options
    bcs len                                     ; then it's a keyboard menu
    tax                                         ; it's a joystick selection
    dex                                         ; so 0 based, x-1 is the chosen option
    rts                                         ; done

len:
    ldx #0                                      ; placeholder 0

option:
    cmp PLACEHOLDER,x                           ; placeholder address
    beq done                                    ; compare the keyboard to the options
    dex 
    bpl option                                  ; check all options
    bmi loop                                    ; when x is -1 it was an invalid entry so loop back

done:
    rts                                         ; x contains the number of the chosen option

.endproc 

;-----------------------------------------------------------------------------
.proc uiShowEditHelp

    lda #0                                      ; clear the keyboard buffer
    bit KBDSTRB
    sta lastInput

    jsr drawClearScreen                         ; clear the screen

    print textEdtHlp01, #(04), #(8*00)          ; show all the help text
    print textEdtHlp02, #(04), #(8*01)
    print textEdtHlp03, #(00), #(8*03)
    print textEdtHlp04, #(00), #(8*05)
    print textEdtHlp05, #(00), #(8*06)
    print textEdtHlp06, #(00), #(8*08)
    print textEdtHlp07, #(00), #(8*09)
    print textEdtHlp08, #(00), #(8*11)
    print textEdtHlp09, #(00), #(8*12)
    print textEdtHlp10, #(00), #(8*14)
    print textEdtHlp11, #(00), #(8*15)
    print textEdtHlp12, #(00), #(8*17)
    print textEdtHlp13, #(00), #(8*18)
    print textEdtHlp14, #(00), #(8*19)
    print textEdtHlp15, #(04), #(8*21)
    print textEdtHlp16, #(04), #(8*22)
    print textEdtHlp17, #(03), #(8*23)
    
    jsr drawPresent 

:
    jsr inputCheckForInput                      ; wait for the user to dismiss the help
    beq :-

    jsr drawClearScreen                         ; clear the screen

    lda #2                                      ; and force a
    sta updateHUD                               ; HUD update

    rts 

.endproc 


;-----------------------------------------------------------------------------
.proc uiWinScreen

    cntrl   = tempBlock + 14                    ; some delay counters, past the text* temporary vars
    cntrh   = tempBlock + 15

    jsr drawClearScreen

    lda #$0                                     ; init the counters - value
    sta cntrl
    lda #2                                      ; init the counters - value
    sta cntrh                                   ; not important what these are init with

    jsr drawPresent

    print textBonus    , #(13), #(8*00)         ; show the win screen text

    printBig textPoints, #$08, #$04, #$01, #$01
    printBig text1000  , #$03, #$0D, #$02, #$02

    lda stage                                   ; last word changes for home vs victory
    beq :+
    printBig textWow   , #$02, #$1D, #$03, #$03
    jmp present
:
    printBig textHome  , #$04, #$1D, #$02, #$03

present:
    jsr drawPresent 

    lda #0
    bit KBDSTRB                                 ; flush the keyboard buffer
    sta lastInput

loop:
    dec cntrl
    bne :+
    dec cntrh 
    bne :+
    ldy #$0
    sty cntrl 
    ldy #2
    sty cntrh 
    jsr drawInvertVisibleScreen
:
    jsr inputCheckForInput 
    beq loop                                    ; exit on a key / button

done:
    rts

.endproc 

;-----------------------------------------------------------------------------
; Show the portions of the HUD that don't change between runs
.proc uiShowGameLabels

    lda #2                                      ; mark the HUD as needing update
    sta updateHUD

loop:

    print textTopLine  , #(00), #(8*00)
    print textPlayerNum, #(17), #(8*00)
    print textDanger   , #(00), #(8*23)
    lda gameMode
    bne :+

    print textShips    , #(23), #(8*23)
    beq show                                    ; JuMP to show - print always leaves with Z flag set

:
    print textTrainer  , #(24), #(8*23)

show:
    jsr uiUpdateGameHud
    jsr drawPresent

    lda updateHUD
    beq done 
    jmp loop

done:
    rts 

.endproc 

;-----------------------------------------------------------------------------
; show the HUD variables that change over time
.proc uiUpdateGameHud

    clc                                         ; prep the HUD text for the stage
    lda stage 
    adc #'1'                                    ; zero based but HUD display 1-5 not 0-4
    sta textStage

    print textStage    , #(06), #(8*00)
    print textDangerBar, #(06), #(8*23)

    printBCD score     , #3, #0, #(25), #(8*00)
 
    lda gameMode
    bne :+

    printBCD lives     , #1, #1, #(30), #(8*23)

:
    dec updateHUD                               ; dec "show" counter

    rts 

.endproc

;-----------------------------------------------------------------------------
; Show the portions of the HUD that don't change between runs
.proc uiShowEditLabels

    brushType = tempBlock + 16

    lda #2                                      ; mark the HUD as needing update
    sta updateHUD

    clc                                         ; prep the HUD text for the stage
    lda stage 
    adc #'1'                                    ; zero based but HUD display 1-5 not 0-4
    sta textStage

loop:

    print textEditStage  , #(05), #(8*00)
    print textStage      , #(12), #(8*00)
    print textEditHelp   , #(01), #(8*23)
    print textEditBrush  , #(19), #(8*00)
    lda brushType 
    bne modeEnemy
    
    print textEditTerrain, #(26), #(8*00)
    jmp :+

modeEnemy:
    print textEditEnemies, #(26), #(8*00)

:
    jsr drawPresent
    dec updateHUD                               ; dec "show" counter
    beq done 
    jmp loop

done:
    rts 

.endproc 

;-----------------------------------------------------------------------------
.proc uiCheckHighScore

    scoreIdx    = tempBlock + 16

    clc 
    lda score                                   ; make sure one of the score components is non-zero
    adc score + 1                               ; because a zero score is not high-score eligible
    adc score + 2
    bne didScore
    rts 

didScore:
    lda #((highScoresEnd-highScores) / 3)       ; score insert point (1 past end of 0 based table)
    sta scoreIdx                                ; save the index
    ldx #((highScoresEnd-highScores) - 1)       ; point x at the high byte of the last score

nextScore:
    ldy #2                                      ; point y at the high byte of the current score
nextByte:
    lda score, y                                ; get the score byte
    cmp highScores, x                           ; compare to the high score byte
    bcc lt                                      ; if it's smaller, then this high-score is bigger than the score
    beq eq
:                                               ; score > highscore so ignore remaining bytes in this comparison
    dex 
    dey 
    bpl:-
    bmi gt                                      ; step over this smaller highscore
eq:
    dex                                         ; go to the next bytes
    dey 
    bpl nextByte                                ; look at the whole score
gt:
    dec scoreIdx                                ; the score is bigger than this high score so move "up" one
    bne nextScore                               ; compare against all high-scores in the table

lt:
    ldx #((highScoresEnd-highScores) / 3)       ; get the index past end
    cpx scoreIdx                                ; see if the scoreIdx still matches
    bne newHS                                   ; if not, then a new table entry needs to be made
    rts 

newHS:
    dex                                         ; x now points at last table entry (zero based)
    cpx scoreIdx                                ; see if the score overwrites the last table entry
    beq copyScore                               ; if yes, just copy score over last table entry

    jsr uiInsertHighScoreRow                    ; otherwise scores need to be moved so the score inserts

copyScore:
    lda scoreIdx                                ; calc the offset of the last byte of the score being replaced
    asl                                         ; start with index * 2 (and clears carry)
    adc scoreIdx                                ; + index (* 3 now sice 3 bytes / score)
    adc #2                                      ; and add 2 to point at the last byte
    tax                                         ; save this offset in x
    ldy #2                                      ; last byte of the score
: 
    lda score, y                                ; copy the score
    sta highScores, x                           ; over the high-score
    dex                                         ; for all 3 bytes
    dey 
    bpl :-

    jsr uiGetHighScoreName                      ; now get the new name
    jmp saveHighScores

.endproc

;-----------------------------------------------------------------------------
; move the score and the name in the highscore table "down" a row
; so the new entry can go into the "space"
; scoreIdx is the number of the row that will move down
.proc uiInsertHighScoreRow

    bytes     = tempBlock + 15
    scoreIdx  = tempBlock + 16
    rows      = tempBlock + 17

    lda #(((highScoresEnd-highScores) / 3) - 1) ; Get the length of the table 0 based
    sec 
    sbc scoreIdx                                ; subtract the insert row
    sta rows                                    ; number of rows to move
    asl                                         ; mult by 3
    adc rows
    sta bytes                                   ; that's how many bytes to move
:beq:-
    ldy #((highScoresEnd-highScores)-4)         ; point at the second last row, last byte
    ldx #((highScoresEnd-highScores)-1)         ; point at the last row, last byte

scoreLoop:
    lda highScores, y                           ; copy the bytes
    sta highScores, x 
    dex 
    dey 
    dec bytes 
    bne scoreLoop                               ; till all bytes copied

    lda rows                                    ; prep to copy name rows
    asl                                         ; * 8 - 1 is * 7 (7 bytes/name - incl null terminator)
    asl 
    asl 
    sec 
    sbc rows 
    sta bytes                                   ; set the number of bytes to copy

    ldy #((textHSEnd-textHS)-8)                 ; point at the second last row, last byte
    ldx #((textHSEnd-textHS)-1)                 ; point at the last row, last byte

nameLoop:
    lda textHS, y                               ; copy the bytes
    sta textHS, x 
    dex 
    dey 
    dec bytes 
    bne nameLoop                                ; till all bytes copied

    rts 

.endproc 

;-----------------------------------------------------------------------------
; Let the player enter a name for the high score table
; the table entry (0 based) is in scoreIdx when called
.proc uiGetHighScoreName

    zaStrL   = tempBlock + 1                    ; ptr to a string to print
    zaStrH   = tempBlock + 2
    zaEntryL = zWorldPtr                        ; ptr to a string being entered (abusing world ptr)
    zaEntryH = zWorldPtr + 1
    scoreIdx = tempBlock + 16                   ; 0 based index into score table
    entryLen = tempBlock + 17                   ; how many characters the player has entered - shared with uiGetUserText
    textLen  = tempBlock + 18                   ; how many characters the player may enter with uiGetUserText

    jsr drawClearScreen

    clc 
    ldx scoreIdx                                ; get the index into the score table
    lda #<textHS                                ; and zaEntry with the start of the
    adc scoreTextOffset, x                      ; 1st byte of where the text must go
    sta zaEntryL
    lda #0
    adc #>textHS
    sta zaEntryH

    lda #95                                     ; the "_" character so stomp the current entry with underscores
    ldx #((textHighScore2-textHighScore1)-1)    ; all scores have the same length
    stx textLen                                 ; save how long the name may be
    ldy #0                                      ; start at character 0
:
    sta (zaEntryL), y                           ; write the underscores (also restores the original length)
    iny                                         ; by overwriting terminators of old entered text
    dex 
    bne :-
    stx entryLen                                ; save 0 to the length of the new name

showName:
    print textCongrats , #(00), #(8*05)         ; show the screen
    print textHSPlayer , #(10), #(8*06)
    print textPlayerNum, #(20), #(8*06)
    print textHSPlayer , #(10), #(8*06)
    print textTop5     , #(03), #(8*08)
    print textTypeName , #(04), #(8*09)

    lda zaEntryL                                ; prep the print string
    sta zaStrL                                  ; by using the entry string location
    lda zaEntryH                                ; zaStrL is modified through printing
    sta zaStrH                                  ; but the entry copy is stable

    printZAStr #(13), #(8*11)                   ; print the name
    jsr drawPresent

    jsr uiGetUserText                           ; Get a character
    bcc enter                                   ; on carry clear enter was pressed
    jmp showName                                ; show the updated name

enter:
    ldy entryLen                                ; get how many characters entered
    lda #0                                      ; load a null terminator
    sta (zaEntryL), y                           ; terminate the string

done:
    jsr drawClearScreen

    rts 

.endproc

;-----------------------------------------------------------------------------
; Show a UI screen asking for a file name to load or save
; Parameter in acc. 0 is save and 1 is load
.proc uiFileLoadSave

    zaStrL   = tempBlock + 1                    ; internal - ptr to a string to print
    zaStrH   = tempBlock + 2                    ; internal
    zaEntryL = zWorldPtr                        ; internal - ptr to a string being entered (abusing world ptr)
    zaEntryH = zWorldPtr + 1                    ; internal
    entryLen = tempBlock + 17                   ; internal - how many characters the player has entered
    textLen  = tempBlock + 18                   ; internal - how many characters the player may enter
    forLoad  = tempBlock + 15                   ; Parameter - 0 means save, 1 means load
    allOver  = tempBlock + 16                   ; internal - on enter, redraw the screen 1 more time

    sta forLoad
    jsr drawClearScreen

    lda #<textFileName                          ; point at the file name
    sta zaEntryL
    lda #>textFileName
    sta zaEntryH
    lda #((textFileNameEnd - textFileName) - 1) ; get how long the name may be
    sta textLen

    ldy #0 
    sty KBDSTRB                                 ; clear the keyboard buffer
    sty lastInput                               ; and the last key
    sty allOver                                 ; 1 when the name has been entered - for redraw
:                                               ; get the length of the file name there already
    lda (zaEntryL), y                           ; get the first letter
    beq :+                                      ; if trailing zero then name is known
    iny 
    bne :-
:
    sty entryLen                                ; save the length of the name

    lda #95                                     ; the "_" character so stomp the remaining name with underscores
:
    cpy textLen 
    bcs showName
    sta (zaEntryL), y                           ; write the underscores (also restores the original length)
    iny                                         ; by overwriting terminators of old entered text
    bne :-

showName:
    lda forLoad
    beq :+
    print textFileLoad   , #(12), #(8*02)
    print textFileInfoL  , #(09), #(8*08)
    jmp cont
:
    print textFileSave   , #(12), #(8*02)
    print textFileInfoS  , #(09), #(8*08)

cont:
    print textFileLines  , #(12), #(8*03)
    print textFileInfo   , #(14), #(8*08)
    print textFileEnter  , #(05), #(8*10)

    lda zaEntryL                                ; prep the print string
    sta zaStrL                                  ; by using the entry string location
    lda zaEntryH                                ; zaStrL is modified through printing
    sta zaStrH                                  ; but the entry copy is stable

    printZAStr             #(11), #(8*14)

    lda allOver                                 ; is this the last redraw before going to disc
    bne disc                                    ; if <> 0 then time to go to disc

    jsr drawPresent                             ; show the screen with changes to file name

    jsr uiGetUserText                           ; Get a character
    bcc enter                                   ; on carry clear enter was pressed
    jmp showName                                ; show the updated name

enter:
    ldy entryLen                                ; get how many characters entered
    lda #0                                      ; and null terminate the string
    sta (zaEntryL), y

    jsr setWorldFileName

    lda #1                                      ; enter has been pressed - it's all over
    sta allOver                                 ; time to attempt a disc action, but just redraw once more
    jmp showName 

disc:
    lda forLoad                                 ; load or save
    beq save

    jsr loadWorld                               ; load the file by the name
    bcs fail                                    ; if carry set then there's an error
    jmp success                                 ; no error - all good

save:
    jsr saveWorld                               ; save the file
    bcs fail                                    ; if carry set then there's an error

success:
    print textFileSuccess, #(13), #(8*18)
    jmp show

fail:                                           ; there was an error saving the file
    pha                                         ; error code in the accumulator, so save it
    print textFileThe    , #(01), #(8*18)
    print textFileFailed , #(10), #(8*18)

    lda forLoad                                 ; message for load/save differ
    beq :+

    print textFileInfoL  , #(05), #(8*18)
    jmp errcde

:
    print textFileInfoS  , #(05), #(8*18)

errcde:
    pla                                         ; get the saved error code
    jsr toHex                                   ; print it as human readable hex and show it to the user
    print szHex          , #(30), #(8*18)

show:
    jsr drawPresent                             ; finally show the whole screen

getkey:
    jsr inputCheckForInput                      ; wait for the user to acknowledge
    beq getkey

    jsr drawClearScreen                         ;

    rts 

.endproc

;-----------------------------------------------------------------------------
; Called with zaEntry pointing at a location to receive some text
; and entryLen should be 0 and is used internally to track where the current
; edit location in the string is.
; returns with partial string when carry set
; User pressed enter and accepted the string when carry clear on return
.proc uiGetUserText

    zaEntryL = zWorldPtr                        ; ptr to a string being entered (abusing world ptr)
    entryLen = tempBlock + 17                   ; Parameter/Internal - how many characters the player has entered
    textLen  = tempBlock + 18                   ; how many characters the player may enter

    lda #0
    bit KBDSTRB                                 ; flush the keyboard buffer
    
loop:
    lda KBD                                     ; wait for a key
    bit Bit8Mask
    beq loop
    bit KBDSTRB

    pha 
    lda #AUDIO_UI_TICK
    sta audioFrame
    jsr serviceAudio
    pla 

    and #$7F
    cmp #$0D                                    ; enter key then done
    beq enter 
    cmp #$08                                    ; backspace key is handled
    beq erase 
    cmp #$20
    beq accept                                  ; space is okay
    cmp #'0'
    bcc loop                                    ; ignore below '0'
    cmp #('9'+1)
    bcc accept                                  ; 0-9 okay
    cmp #'A'                                    ; < 'A' ignore
    bcc loop
    cmp #('Z'+1)
    bcc accept 
    cmp #'a'
    bcc loop 
    cmp #('z'+1)
    bcs loop                                    ; ignore past 'Z'

accept:
    ; lda theChar
    ldy entryLen                                ; how many characters already entered
    cpy textLen                                 ; vs how many can be entered
    bcs loop                                    ; already full, don't accept
    sta (zaEntryL), y                           ; and write this character there
    inc entryLen                                ; and increase the length entered
goShow:
    sec                                         ; on carry set, not done
    rts 

erase:
    ldy entryLen                                ; get how many characters have been entered
    beq loop                                    ; if none then ignore the backspace key
    dec entryLen                                ; 1 less character entered
    dey 
    lda #95                                     ; load the underscore
    sta (zaEntryL), y                           ; stomp the last character with the underscore
    bne goShow                                  ; redraw the screen

enter:
    clc 
    rts 

.endproc 