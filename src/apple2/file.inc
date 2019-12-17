;-----------------------------------------------------------------------------
; file.inc
; Part of penetrator, the zx spectrum game, made for Apple II
;
; Stefan Wessels, 2019
; This is free and unencumbered software released into the public domain.

;-----------------------------------------------------------------------------
PATHNAME    = $0280
MLI         = $BF00

QUIT_CALL   = $65
OPEN_CALL   = $C8
READ_CALL   = $CA
WRITE_CALL  = $CB
CLOSE_CALL  = $CC
CREATE_CALL = $C0
;-----------------------------------------------------------------------------
.segment "DATA"
createParam:
    .byte $07                                   ;PARAM_COUNT
    .addr PATHNAME                              ;PATHNAME
    .byte $C3                                   ;ACCESS
    .byte $06                                   ;FILE_TYPE (6 is binay)
    .word $0000                                 ;AUX_TYPE
    .byte $01                                   ;STORAGE_TYPE
    .word $0000                                 ;CREATE_DATE
    .word $0000                                 ;CREATE_TIME

openParam:
    .byte $03                                   ;PARAM_COUNT
    .addr PATHNAME                              ;PATHNAME
    .addr $2000-$400                            ;IO_BUFFER
openRef:
    .byte $00                                   ;REF_NUM

readParam:
    .byte $04                                   ;PARAM_COUNT
readRef:
    .byte $00                                   ;REF_NUM
    .addr worldDataStart                        ;DATA_BUFFER
    .word $FFFF                                 ;REQUEST_COUNT
    .word $0000                                 ;TRANS_COUNT

writeParam:
    .byte $04                                   ;PARAM_COUNT
writeRef:
    .byte $00                                   ;REF_NUM
    .addr worldDataStart                        ;DATA_BUFFER
    .word worldDataEnd-worldDataStart           ;REQUEST_COUNT
    .word $0000                                 ;TRANS_COUNT

closeParam:
    .byte   $01                                 ;PARAM_COUNT
closeRef:
    .byte   $00                                 ;REF_NUM
    

;-----------------------------------------------------------------------------
.segment "CODE"

;-----------------------------------------------------------------------------
.proc fileSave

    jsr MLI                                     ; create the file
    .byte CREATE_CALL
    .word createParam
    bcc :+
    jmp error

:
    jsr MLI
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
.proc fileLoad

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
