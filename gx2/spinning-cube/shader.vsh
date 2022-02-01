; $MODE = "UniformRegister"

; numOutputs - 1
; $SPI_VS_OUT_CONFIG.VS_EXPORT_COUNT = 0

; $NUM_SPI_VS_OUT_ID = 1

; color output
; $SPI_VS_OUT_ID[0].SEMANTIC_0 = 0

; C0
; $UNIFORM_VARS[0].name = "uModelViewProj"
; $UNIFORM_VARS[0].type = "mat4"
; $UNIFORM_VARS[0].count = 1
; $UNIFORM_VARS[0].block = -1
; $UNIFORM_VARS[0].offset = 0

; R1
; $ATTRIB_VARS[0].name = "aPosition"
; $ATTRIB_VARS[0].type = "vec4"
; $ATTRIB_VARS[0].location = 0
; R2
; $ATTRIB_VARS[1].name = "aColor"
; $ATTRIB_VARS[1].type = "vec4"
; $ATTRIB_VARS[1].location = 1

; Multiply uModelViewProj and aPosition
00 CALL_FS NO_BARRIER
01 ALU: ADDR(32) CNT(16)
    0  x: MUL    R0.x, R1.w, C3.x
       y: MUL    R0.y, R1.w, C3.y
    1  x: MUL    R0.z, R1.w, C3.z
       y: MUL    R0.w, R1.w, C3.w
    2  x: MULADD R0.x, R1.z, C2.x, R0.x
       y: MULADD R0.y, R1.z, C2.y, R0.y
    3  x: MULADD R0.z, R1.z, C2.z, R0.z
       y: MULADD R0.w, R1.z, C2.w, R0.w
    4  x: MULADD R0.x, R1.y, C1.x, R0.x
       y: MULADD R0.y, R1.y, C1.y, R0.y
    5  x: MULADD R0.z, R1.y, C1.z, R0.z
       y: MULADD R0.w, R1.y, C1.w, R0.w
    6  x: MULADD R0.x, R1.x, C0.x, R0.x
       y: MULADD R0.y, R1.x, C0.y, R0.y
    7  x: MULADD R0.z, R1.x, C0.z, R0.z
       y: MULADD R0.w, R1.x, C0.w, R0.w
02 EXP_DONE: POS0, R0
03 EXP_DONE: PARAM0, R2 NO_BARRIER
END_OF_PROGRAM
