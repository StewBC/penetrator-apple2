;-----------------------------------------------------------------------------
; text.inc
; Part of penetrator, the zx spectrum game, made for Apple II
;
; Stefan Wessels, 2019
; This is free and unencumbered software released into the public domain.

;-----------------------------------------------------------------------------
.segment "RODATA"

.feature        string_escapes

; In game text
textTopLine:      .asciiz "Stage  . Player - - Score      0"
textDanger:       .asciiz "Danger"
textShips:        .asciiz "Ships"
textTrainer:      .asciiz "Trainer"
textPlayer:       .asciiz "P L A Y E R  "

; title screen
textHighScores:   .asciiz "High  Scores"
textPhilip:       .asciiz " Written by - Philip Mitchell - "
textCopyright:    .asciiz "** Copyright  \"Beam Software\" **"
textApple2:       .asciiz "      Apple II version by       "
textStefan:       .asciiz "    Stefan Wessels, Dec 2019    "
textSupport:      .asciiz "     Lots of great Apple II     "
textOliver:       .asciiz "   Support from Oliver Schmidt  "
textSource:       .asciiz "        Source on GitHub:       "
textGitHub:       .asciiz "     StewBC/penetrator-apple2   "
textGameOver:     .asciiz "********** GAME  OVER **********"

; main menu
textInstructions: .asciiz "INSTRUCTIONS"
textPress:        .asciiz "PRESS"
textUnderline:    .asciiz "-----"
textOneTwo:       .asciiz "\"1\" OR \"2\" FOR NUMBER OF PLAYERS"
textTrain:        .asciiz "\"T\"  FOR TRAINING CONTROL CENTER"
textEdit:         .asciiz "\"E\" FOR THE LANDSCAPE EDITOR"
textLoad:         .asciiz "\"L\"  TO LOAD ANOTHER LANDSCAPE"
textSirens:       .asciiz "\"S\" SET AUDIO: "
textQuit:         .asciiz "\"Q\" QUIT"

; main menu audio options
textAudio1:       .asciiz "ENGINE, SFX"
textAudio2:       .asciiz "SFX        "
textAudio3:       .asciiz "NONE       "

; training mode menu
textTrainingMode: .asciiz "TRAINING MODE"
textPressStage:   .asciiz "PRESS NUMBER OF DESIRED STAGE"
textOneToFour:    .asciiz "(1 - 4) TO START."
textBackup:       .asciiz "ESC to back up or exit mode"

; edit mode screen
textEditStage:    .asciiz "STAGE"
textEditHelp:     .byte   "PRESS 'C' FOR  ",$86," COMMAND SUMMARY", $00
textEditBrush:    .asciiz "BRUSH: "
textEditTerrain:  .asciiz "TERRAIN"
textEditEnemies:  .asciiz "ENEMIES"
textEditDnArrow:  .byte   $85, $00

; edit help screen
textEdtHlp01:      .asciiz "LANDSCAPE EDITOR COMMANDS"
textEdtHlp02:      .asciiz "--------- ------ --------"
textEdtHlp03:      .asciiz "JOYSTICK- MOVE THE CURSOR"
textEdtHlp04:      .asciiz "KBD-B   - TOGGLE BETWEEN TERRAIN"
textEdtHlp05:      .asciiz "          AND ENEMY BRUSHES"
textEdtHlp06:      .asciiz "SPACE   - CONTINIOUS MOVEMENT"
textEdtHlp07:      .asciiz ""
textEdtHlp08:      .asciiz "JOYBTN1 - SET TERRAIN TOP"
textEdtHlp09:      .asciiz "          OR PLACE A MISSILE"
textEdtHlp10:      .asciiz "JOYBTN0 - SET TERRAIN BOTTOM OR"
textEdtHlp11:      .asciiz "          PLACE A RADAR"
textEdtHlp12:      .asciiz "1 - 5   - SKIP TO STAGE"
textEdtHlp13:      .asciiz "S       - SAVE THIS WORLD"
textEdtHlp14:      .asciiz "L       - LOAD A WORLD"
textEdtHlp15:      .asciiz "ERASE ENEMIES USING A/B"
textEdtHlp16:      .asciiz "WITH THE ENEMIES BRUSH"
textEdtHlp17:      .asciiz "ESC - END THE EDIT SESSION"

; win screen
textBonus:        .asciiz "BONUS"
textPoints:       .asciiz "POINTS"
text1000:         .asciiz "1000"
textWow:          .asciiz "WOW"
textHome:         .asciiz "HOME"

; high score name entry
textCongrats:     .asciiz "*****   CONGRATULATIONS    *****"
textHSPlayer:     .asciiz "PLAYER -- "
textTop5:         .asciiz "Your score is in the top 5"
textTypeName:     .asciiz "Please type in your name"

; file name entry
textFileLoad:     .asciiz "FILE LOAD"
textFileSave:     .asciiz "FILE SAVE"
textFileLines:    .asciiz "---------"
textFileInfoL:    .asciiz "Load"
textFileInfoS:    .asciiz "Save"
textFileInfo:     .asciiz "world data"
textFileEnter:    .asciiz "Please enter a file name"
textFileSuccess:  .asciiz "SUCCESS."
textFileThe:      .asciiz "The"
textFileFailed:   .asciiz "Failed.  Error Code"

hihgScoreFileName:.asciiz "PENSCORES"

;-----------------------------------------------------------------------------
.segment "CODE"

;-----------------------------------------------------------------------------
; Use the print macro to call this code
; Writes a null-terminated string to the back buffer
; temp1/2 Ptr to the string
; textX - the string draw X position (in col coords)
; textY - the string draw Y position
.proc textOut

    textX = tempBlock + 10
    textY = tempBlock + 11
    xPos  = tempBlock + 12
    yPos  = tempBlock + 13

    zaStrL = tempBlock + 1                      ; parameter - string lo
    zaStrH = tempBlock + 2                      ; parameter - string hi
    zaFontL = tempBlock + 4                     ; internal - point at the character memory Lo
    zaFontH = tempBlock + 5                     ; internal - point at the character memory Hi
    zaFontOffset = tempBlock + 6                ; 0-15 - index from zaFont
 
loop:
    ldy #0
    lda (zaStrL), y                             ; get the character in the string
    bne :+                                      ; if non-null process it
    rts                                         ; exit on 0 terminator

:
    jsr setFont
    lda textY                                   ; get the screen Y for text
    sta yPos                                    ; init the row working buffer

plotLoop:
    ldy yPos                                    ; at the working row offset
    lda rowL, y                                 ; get the memory address
    adc textX                                   ; add in column (x pos of text)
    sta write + 1                               ; point lo at memory
    lda rowH, y
    adc zVramH
    sta write + 2

    ldy zaFontOffset                            ; get the offset into the font
    lda (zaFontL), y                            ; get the actual left character byte
write:
    sta PLACEHOLDER                             ; plot the left hand side
    cpy #7
    bcs nextChar                                ; 0-7 then done with this character (bottom always blank)
    inc zaFontOffset                            ; move a byte for last row plotted
    inc yPos                                    ; move down a row on screen
    
    bne plotLoop                                ; always take this branch

nextChar:
    inc textX                                   ; character column
    inc zaStrL                                  ; next character in string
    bne loop 
    inc zaStrH 
    bne loop                                    ; always branch back

.endproc

;-----------------------------------------------------------------------------
; Like textOut but uses drawPoltXY to plot each pixel in the text
; so you can write big strings using width and height.
; a setting of width and height both 1 results in a character that's
; 32x32 on-screen
.proc textBigPrint

    zaStrL       = tempBlock + 1                ; parameter - string lo
    zaStrH       = tempBlock + 2                ; parameter - string hi
    zaBitCnt     = tempBlock + 3                ; internal - how many bits to process for a byte to write
    zaFontL      = tempBlock + 4                ; internal - point at the character memory Lo
    zaFontH      = tempBlock + 5                ; internal - point at the character memory Hi
    zaFontOffset = tempBlock + 6                ; internal - 0-15 - index from zaFont
 
    width        = tempBlock + 14               ; shared with drawPlotXY
    height       = tempBlock + 15

    textX        = tempBlock + 12
    textY        = tempBlock + 13
    xPos         = tempBlock + 7
    yPos         = tempBlock + 8

loop:
    ldy #0
    lda (zaStrL), y                             ; get the character in the string
    bne :+                                      ; if non-null process it
    rts                                         ; exit on 0 terminator

:
    jsr setFont 

    lda textY                                   ; get the screen Y for text
    sta yPos 

    inc zaFontOffset

plotLoop:

    lda textX
    sta xPos 

    ldx #6                                      ; each font character byte uses 7 pixels
    stx zaBitCnt

    ldy zaFontOffset                            ; get the offset into the font
    lda (zaFontL), y                            ; get the actual left character byte
    lsr 
    jsr drawBits 

    inc zaFontOffset                            ; go to the next line
    ldy zaFontOffset

    cpy #7                                      ; characters are 8 bytes long, care about 7
    bcs nextChar                                ; if 16 then done with this character
    lda yPos                                    ; adjust the "plot" down a line
    clc 
    adc height
    sta yPos

    bne plotLoop                                ; always take this branch

nextChar:
    lda width                                   ; move along in X to the next character
    asl                                         ; 6x the width + 2
    asl 
    adc width 
    adc width
    adc #2 
    adc textX                                   ; add to current "plot" position
    sta textX 
    inc zaStrL                                  ; next character in string
    bne loop 
    inc zaStrH 
    bne loop                                    ; always branch back

drawBits:
    lsr                                         ; shift a bit into carry
    pha                                         ; save the shifted byte
    bcc :+                                      ; if carry is set, need to plot a pixel
    clc 
    ldx xPos                                    ; get the x and y of where to plot
    ldy yPos
    jsr drawPlotXY                              ; draw the pixel
:
    lda xPos                                    ; move along one "pixel"
    clc 
    adc width
    sta xPos
    pla                                         ; restore the shifted character
    dec zaBitCnt                                ; see if all bytes were handled
    ldx zaBitCnt
    bne drawBits
    rts 

.endproc 

;-----------------------------------------------------------------------------
; Point zaFont(H|L) at the character definition for the character in the accumulator
; y register must be 0 when calling this routine
.proc setFont

    zaFontL      = tempBlock + 4                ; internal - point at the character memory Lo
    zaFontH      = tempBlock + 5                ; internal - point at the character memory Hi
    zaFontOffset = tempBlock + 6                ; internal - 0-15 - index from zaFont

    sty zaFontOffset                            ; start at 0th byte in font
    sty zaFontH 
    sty zaFontL 

    sec                                         ; subtract 32 as font starts at char 32 (space)
    sbc #$20
    asl                                         ; mult 8 as that's how many bytes per char
    rol zaFontH 
    asl 
    rol zaFontH 
    asl 
    rol zaFontH 
    adc #<font                                  ; add in the memory location
    sta zaFontL 
    lda #>font 
    adc zaFontH 
    sta zaFontH                                 ; now font points at the character data

    rts 

.endproc 

;-----------------------------------------------------------------------------
; Turns a BCD number into a text string at textNumber.
; zaStr is a ptr to the number in BCD
; y reg is the number of bytes to convert
; x reg <> 0 means print at least one digit, even if 0
.proc textBCDtoSZ 

    zaStrL     = tempBlock + 1
    zaStrH     = tempBlock + 2
    zaForce    = tempBlock + 3

    stx zaForce 
    dey                                         ; zero based so -1
    ldx #0

leading:                                        ; skip leading 0's with spaces
    lda #' '                                    ; prep buffer with two spaces
    sta textNumber, x                           ; if the byte to output is $00
    sta textNumber + 1, x                       ; so leading 0's are skipped
    lda (zaStrL), y                             ; get the high byte
    bit Bit8765Mask                             ; check 1st digit
    bne loop1                                   ; if not zero - start printing
    inx                                         ; skip the zero by accepting the space
    and #$0f                                    ; check the 2nd digit
    beq :+                                      ; go skip if zero
    jsr digit                                   ; print the digit
    bne loop2                                   ; and join the main print loop
:
    inx                                         ; skip the next char in the output string
    dey                                         ; go to next byte in the BCD number
    bpl leading                                 ; keep going till all bytes done
    ldy zaForce                                 ; nothing printed - see if at least 1 digit
    beq done                                    ; needs to be printed and if no - then done
    dex                                         ; step back one digit in the string
    lda #'0'                                    ; get a zero
    sta textNumber, x                           ; put it in the string
    inx 
    bne done                                    ; JuMP to done

loop:                                           ; main loop once a digit is found
    lda (zaStrL), y                             ; get the 2 decimal digits
loop1:
    jsr both                                    ; print them both starting with left
loop2:
    dey                                         ; next score part
    bpl loop                                    ; keep going while there are score parts

done:
    lda #0                                      ; load a null-terminator
    sta textNumber, x                           ; store it in the output string
    rts 

both:
    pha                                         ; save the 2 digits
    lsr                                         ; shift right down
    lsr 
    lsr 
    lsr 
    jsr digit                                   ; go print it
    pla                                         ; restore both
    and #$0f                                    ; mask off the right leaving left
digit:                                          ; print the digit
    ora #$30                                    ; add "0" to the decimal
    sta textNumber, x                           ; put it in the string
    inx                                         ; advance the string pointer
    rts 

.endproc

;-----------------------------------------------------------------------------
; Prints the accumulator to the szHez text as 2 hex bytes
.proc toHex

    ldx #0                                      ; offset into the szHex buffer
    pha                                         ; save a for lower 4 bits
    lsr                                         ; move upper 4 bits to lo
    lsr 
    lsr 
    lsr 
    jsr :+                                      ; process upper 4 bits
    pla                                         ; restore a to process lower 4 bits
:
    and #$0f                                    ; mask to only the 4 bits now in lo
    cmp #10                                     ; see if it's > 9
    bcc :+
    adc #6                                      ; move from 10+ to a+
   :
    adc #$30                                    ; and move to '0' and beyond
    sta szHex, x                                ; save this in the buffer
    inx                                         ; go to next buffer location
    rts                                         ; and return to self or caller

.endproc