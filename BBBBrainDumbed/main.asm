cli
ld0 ld0 ld0 ld8
mta
loop: ;6

;LD16M B[15:0]=(A)[15:0]
cli clj mfn mty
;loop1: 10
mfb ldr mtb mfa
mtx sec
ad4 ad4 ad4 ad4
mtx 
ld2 ldd ld0 ld0 ;end1 addr
mta mfj
bzz mfx mta
ldc ld3 ld0 ld0 ;loop1 addr
mtp
;end1: 35

mfx mtd
lde ld2 ld2 ld0 ;end addr
mta
ld0 lde ld0 ld7
mty
ad4 ad4 ad4 ad4
bzz
ld0 ld0 ld0 ldc
mta mfb 

;ST16A (A)[15:0]=Z[15:0]
cli clj mtb mfn
mty
;loop2: 63
mfb str mfa mtx
sec ad4 mtx cli 
lde ldf ld1 ld0 ;end2 addr
mta mfj
bzz mfx mta
lda ld7 ld1 ld0 ;loop2 addr
mtp
;end2: 85

mfd mta
ldi loop ;loop addr
mtp
;end: 93
nop