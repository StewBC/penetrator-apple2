;-----------------------------------------------------------------------------
; file.inc
; Part of penetrator, the zx spectrum game, made for Apple II
;
; Stefan Wessels, 2019
; This is free and unencumbered software released into the public domain.

;-----------------------------------------------------------------------------
.segment "DATA"

createParam:
    .byte $07                                   ; param_count
createName:
    .addr PATHNAME                              ; pathname
    .byte $C3                                   ; access
    .byte $06                                   ; file_type (6 is binay)
    .word $0000                                 ; aux_type
    .byte $01                                   ; storage_type
    .word $0000                                 ; create_date
    .word $0000                                 ; create_time

openParam:
    .byte $03                                   ; param_count
openName:
    .addr PATHNAME                              ; pathname
    .addr $2000-$400                            ; io_buffer
openRef:
    .byte $00                                   ; ref_num

readParam:
    .byte $04                                   ; param_count
readRef:
    .byte $00                                   ; ref_num
readAddress:
    .addr worldDataStart                        ; data_buffer
readLength:
    .word $FFFF                                 ; request_count
    .word $0000                                 ; trans_count

writeParam:
    .byte $04                                   ; param_count
writeRef:
    .byte $00                                   ; ref_num
writeAddress:
    .addr worldDataStart                        ; data_buffer
writeLength:
    .word worldDataEnd-worldDataStart           ; request_count
    .word $0000                                 ; trans_count

closeParam:
    .byte   $01                                 ; param_count
closeRef:
    .byte   $00                                 ; ref_num
    
;-----------------------------------------------------------------------------
.segment "CODE"

;-----------------------------------------------------------------------------
.proc saveWorld

    lda #<worldDataStart                        ; set the location
    sta writeAddress
    lda #>worldDataStart
    sta writeAddress + 1

    lda #<(worldDataEnd-worldDataStart)         ; set the size
    sta writeLength
    lda #>(worldDataEnd-worldDataStart)
    sta writeLength + 1

    jmp saveFile                                ; save

.endproc

;-----------------------------------------------------------------------------
.proc loadWorld

    lda #<worldDataStart                        ; set the location
    sta readAddress
    lda #>worldDataStart
    sta readAddress + 1

    lda #<(worldDataEnd-worldDataStart)         ; set the length (size)
    sta readLength
    lda #>(worldDataEnd-worldDataStart)
    sta readLength + 1

    jmp loadFile                                ; load

.endproc

;-----------------------------------------------------------------------------
.proc saveHighScores

    jsr setHighScoreFileNames                   ; set the name

    lda #<scoresTable                           ; set the location
    sta writeAddress
    lda #>scoresTable
    sta writeAddress + 1

    lda #<(scoresTableEnd-scoresTable)          ; set the size
    sta writeLength
    lda #>(scoresTableEnd-scoresTable)
    sta writeLength + 1

    jmp saveFile                                ; save

.endproc

;-----------------------------------------------------------------------------
.proc loadHighScores

    jsr setHighScoreFileNames                   ; set the name

    lda #<scoresTable                           ; set the location
    sta readAddress
    lda #>scoresTable
    sta readAddress + 1

    lda #<(scoresTableEnd-scoresTable)          ; set the size
    sta readLength
    lda #>(scoresTableEnd-scoresTable)
    sta readLength + 1

    jmp loadFile                                ; load

.endproc

;-----------------------------------------------------------------------------
.proc setHighScoreFileNames

    ldx pathPos                                 ; append to the end of the path
    ldy #0                                      ; from the 1st char pf the name
:
    lda hihgScoreFileName, y                    ; copy name to path
    sta PATHNAME, x
    beq :+
    iny 
    inx 
    bne :-

:
    stx PATHNAME
    dec PATHNAME                                ; don't count the trailing null

    rts 

.endproc 

;-----------------------------------------------------------------------------
.proc setWorldFileName

    zaEntryL = zWorldPtr                        ; internal - ptr to a string being entered (abusing world ptr)

    tya                                         ; add the file length to the path length
    clc 
    adc pathPos 
    tax 
    stx PATHNAME 
    dec PATHNAME                                ; don't count the trailing null

:
    lda (zaEntryL), y                           ; copy the file name to the end of the path
    sta PATHNAME, x
    dex 
    dey 
    bpl :-

    rts 

.endproc 

;-----------------------------------------------------------------------------
.proc saveFile

    jsr MLI                                     ; create the file, ignoring errors
    .byte CREATE_CALL
    .word createParam

    jsr MLI                                     ; open the (now hopefully existing) file
    .byte OPEN_CALL
    .word openParam
    bcc :+
    jmp error

: 
    lda openRef
    sta writeRef
    sta closeRef

    jsr MLI
    .byte WRITE_CALL
    .word writeParam
    bcs error

    jsr MLI
    .byte CLOSE_CALL
    .word closeParam
    bcs error

error:
    rts 

.endproc

;-----------------------------------------------------------------------------
.proc loadFile

    jsr MLI
    .byte OPEN_CALL
    .word openParam
    bcc :+
    jmp error

: 
    lda openRef
    sta readRef
    sta closeRef

    jsr MLI
    .byte READ_CALL
    .word readParam
    bcs error

    jsr MLI
    .byte CLOSE_CALL
    .word closeParam
    bcs error

error:
    rts 

.endproc
