#include <ultra64.h>
#include "constants.h"
#include "game/data/data_000000.h"
#include "game/data/data_0083d0.h"
#include "game/data/data_00e460.h"
#include "game/data/data_0160b0.h"
#include "game/data/data_01a3a0.h"
#include "game/data/data_020df0.h"
#include "game/data/data_02da90.h"
#include "gvars/gvars.h"
#include "types.h"

#define GETBYTE()   (inbuf[inptr++])
#define NEXTBYTE()  (u8)GETBYTE()
#define NEEDBITS(n) {while(k<(n)){b|=((u32)NEXTBYTE())<<k;k+=8;}}
#define DUMPBITS(n) {b>>=(n);k-=(n);}

struct huft {
	u8 e;                /* number of extra bits or operation */
	u8 b;                /* number of bits in this code or subcode */
	union {
		u16 n;            /* literal, length base, or distance base */
		struct huft *t;   /* pointer to next level of table */
	} v;
};

u8 *inbuf = 0;
u8 *outbuf = 0;

u32 inptr = 0;
u32 wp = 0;
u8 *var702012e0 = 0;

u8 border[] = { /* Order of the bit length code lengths */
        16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

/* Tables for deflate from PKZIP's appnote.txt. */
u16 cplens[] = { /* Copy lengths for literal codes 257..285 */
        3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
        35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258, 0, 0};

        /* actually lengths - 2; also see note #13 above about 258 */
u8 cplext[] = { /* Extra bits for literal codes 257..285 */
        0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
        3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0, 99, 99}; /* 128==invalid */

u16 cpdist[] = { /* Copy offsets for distance codes 0..29 */
        1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
        257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
        8193, 12289, 16385, 24577};

u8 cpdext[] = { /* Extra bits for distance codes */
        0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
        7, 7, 8, 8, 9, 9, 10, 10, 11, 11,
        12, 12, 13, 13};

u32 bb = 0;
u32 bk = 0;

/* And'ing with mask[n] masks the lower n bits */
u16 mask_bits[] = {
    0x0000,
    0x0001, 0x0003, 0x0007, 0x000f, 0x001f, 0x003f, 0x007f, 0x00ff,
    0x01ff, 0x03ff, 0x07ff, 0x0fff, 0x1fff, 0x3fff, 0x7fff, 0xffff
};

u32 lbits = 9;
u32 dbits = 6;
u32 hufts = 0;

GLOBAL_ASM(
glabel huft_build
.L70200000:
/* 70200000:	27bdfa18 */ 	addiu	$sp,$sp,-1512
/* 70200004:	afb20010 */ 	sw	$s2,0x10($sp)
/* 70200008:	afb00008 */ 	sw	$s0,0x8($sp)
/* 7020000c:	00808025 */ 	move	$s0,$a0
/* 70200010:	00a09025 */ 	move	$s2,$a1
/* 70200014:	afbf002c */ 	sw	$ra,0x2c($sp)
/* 70200018:	afbe0028 */ 	sw	$s8,0x28($sp)
/* 7020001c:	afb70024 */ 	sw	$s7,0x24($sp)
/* 70200020:	afb60020 */ 	sw	$s6,0x20($sp)
/* 70200024:	afb5001c */ 	sw	$s5,0x1c($sp)
/* 70200028:	afb40018 */ 	sw	$s4,0x18($sp)
/* 7020002c:	afb30014 */ 	sw	$s3,0x14($sp)
/* 70200030:	afb1000c */ 	sw	$s1,0xc($sp)
/* 70200034:	afa605f0 */ 	sw	$a2,0x5f0($sp)
/* 70200038:	afa705f4 */ 	sw	$a3,0x5f4($sp)
/* 7020003c:	27a305a0 */ 	addiu	$v1,$sp,0x5a0
/* 70200040:	27a205e4 */ 	addiu	$v0,$sp,0x5e4
.L70200044:
/* 70200044:	24630004 */ 	addiu	$v1,$v1,0x4
/* 70200048:	1462fffe */ 	bne	$v1,$v0,.L70200044
/* 7020004c:	ac60fffc */ 	sw	$zero,-0x4($v1)
/* 70200050:	0200f825 */ 	move	$ra,$s0
/* 70200054:	02406825 */ 	move	$t5,$s2
/* 70200058:	27a405a0 */ 	addiu	$a0,$sp,0x5a0
.L7020005c:
/* 7020005c:	8fee0000 */ 	lw	$t6,0x0($ra)
/* 70200060:	25adffff */ 	addiu	$t5,$t5,-1
/* 70200064:	27ff0004 */ 	addiu	$ra,$ra,0x4
/* 70200068:	000e7880 */ 	sll	$t7,$t6,0x2
/* 7020006c:	008f1021 */ 	addu	$v0,$a0,$t7
/* 70200070:	8c580000 */ 	lw	$t8,0x0($v0)
/* 70200074:	27190001 */ 	addiu	$t9,$t8,0x1
/* 70200078:	15a0fff8 */ 	bnez	$t5,.L7020005c
/* 7020007c:	ac590000 */ 	sw	$t9,0x0($v0)
/* 70200080:	8fae05a0 */ 	lw	$t6,0x5a0($sp)
/* 70200084:	8fa50600 */ 	lw	$a1,0x600($sp)
/* 70200088:	24070001 */ 	li	$a3,0x1
/* 7020008c:	164e0007 */ 	bne	$s2,$t6,.L702000ac
/* 70200090:	27a605a4 */ 	addiu	$a2,$sp,0x5a4
/* 70200094:	8fb605fc */ 	lw	$s6,0x5fc($sp)
/* 70200098:	8fa50600 */ 	lw	$a1,0x600($sp)
/* 7020009c:	00001025 */ 	move	$v0,$zero
/* 702000a0:	aec00000 */ 	sw	$zero,0x0($s6)
/* 702000a4:	10000121 */ 	b	.L7020052c
/* 702000a8:	aca00000 */ 	sw	$zero,0x0($a1)
.L702000ac:
/* 702000ac:	8cb10000 */ 	lw	$s1,0x0($a1)
/* 702000b0:	24020011 */ 	li	$v0,0x11
.L702000b4:
/* 702000b4:	8ccf0000 */ 	lw	$t7,0x0($a2)
/* 702000b8:	55e00005 */ 	bnezl	$t7,.L702000d0
/* 702000bc:	0227082b */ 	sltu	$at,$s1,$a3
/* 702000c0:	24e70001 */ 	addiu	$a3,$a3,0x1
/* 702000c4:	14e2fffb */ 	bne	$a3,$v0,.L702000b4
/* 702000c8:	24c60004 */ 	addiu	$a2,$a2,0x4
/* 702000cc:	0227082b */ 	sltu	$at,$s1,$a3
.L702000d0:
/* 702000d0:	10200002 */ 	beqz	$at,.L702000dc
/* 702000d4:	00e09825 */ 	move	$s3,$a3
/* 702000d8:	00e08825 */ 	move	$s1,$a3
.L702000dc:
/* 702000dc:	240d0010 */ 	li	$t5,0x10
/* 702000e0:	27a205e0 */ 	addiu	$v0,$sp,0x5e0
.L702000e4:
/* 702000e4:	8c580000 */ 	lw	$t8,0x0($v0)
/* 702000e8:	57000005 */ 	bnezl	$t8,.L70200100
/* 702000ec:	01b1082b */ 	sltu	$at,$t5,$s1
/* 702000f0:	25adffff */ 	addiu	$t5,$t5,-1
/* 702000f4:	15a0fffb */ 	bnez	$t5,.L702000e4
/* 702000f8:	2442fffc */ 	addiu	$v0,$v0,-4
/* 702000fc:	01b1082b */ 	sltu	$at,$t5,$s1
.L70200100:
/* 70200100:	10200002 */ 	beqz	$at,.L7020010c
/* 70200104:	afad0598 */ 	sw	$t5,0x598($sp)
/* 70200108:	01a08825 */ 	move	$s1,$t5
.L7020010c:
/* 7020010c:	24190001 */ 	li	$t9,0x1
/* 70200110:	00ed082b */ 	sltu	$at,$a3,$t5
/* 70200114:	acb10000 */ 	sw	$s1,0x0($a1)
/* 70200118:	1020000b */ 	beqz	$at,.L70200148
/* 7020011c:	00f91804 */ 	sllv	$v1,$t9,$a3
/* 70200120:	000d7080 */ 	sll	$t6,$t5,0x2
/* 70200124:	27af05a0 */ 	addiu	$t7,$sp,0x5a0
/* 70200128:	01cf2821 */ 	addu	$a1,$t6,$t7
.L7020012c:
/* 7020012c:	8cd80000 */ 	lw	$t8,0x0($a2)
/* 70200130:	24c60004 */ 	addiu	$a2,$a2,0x4
/* 70200134:	00c5082b */ 	sltu	$at,$a2,$a1
/* 70200138:	00781823 */ 	subu	$v1,$v1,$t8
/* 7020013c:	0003c840 */ 	sll	$t9,$v1,0x1
/* 70200140:	1420fffa */ 	bnez	$at,.L7020012c
/* 70200144:	03201825 */ 	move	$v1,$t9
.L70200148:
/* 70200148:	8c460000 */ 	lw	$a2,0x0($v0)
/* 7020014c:	2442fffc */ 	addiu	$v0,$v0,-4
/* 70200150:	27af05a0 */ 	addiu	$t7,$sp,0x5a0
/* 70200154:	00661823 */ 	subu	$v1,$v1,$a2
/* 70200158:	00c37021 */ 	addu	$t6,$a2,$v1
/* 7020015c:	ac4e0004 */ 	sw	$t6,0x4($v0)
/* 70200160:	afa00070 */ 	sw	$zero,0x70($sp)
/* 70200164:	00003825 */ 	move	$a3,$zero
/* 70200168:	27bf05a4 */ 	addiu	$ra,$sp,0x5a4
/* 7020016c:	104f0009 */ 	beq	$v0,$t7,.L70200194
/* 70200170:	27a50074 */ 	addiu	$a1,$sp,0x74
/* 70200174:	27a605a0 */ 	addiu	$a2,$sp,0x5a0
.L70200178:
/* 70200178:	8ff80000 */ 	lw	$t8,0x0($ra)
/* 7020017c:	2442fffc */ 	addiu	$v0,$v0,-4
/* 70200180:	24a50004 */ 	addiu	$a1,$a1,0x4
/* 70200184:	00f83821 */ 	addu	$a3,$a3,$t8
/* 70200188:	aca7fffc */ 	sw	$a3,-0x4($a1)
/* 7020018c:	1446fffa */ 	bne	$v0,$a2,.L70200178
/* 70200190:	27ff0004 */ 	addiu	$ra,$ra,0x4
.L70200194:
/* 70200194:	0200f825 */ 	move	$ra,$s0
/* 70200198:	00006825 */ 	move	$t5,$zero
/* 7020019c:	afb205ec */ 	sw	$s2,0x5ec($sp)
/* 702001a0:	02404825 */ 	move	$t1,$s2
/* 702001a4:	27a8006c */ 	addiu	$t0,$sp,0x6c
/* 702001a8:	27a600b4 */ 	addiu	$a2,$sp,0xb4
/* 702001ac:	8fe70000 */ 	lw	$a3,0x0($ra)
.L702001b0:
/* 702001b0:	27ff0004 */ 	addiu	$ra,$ra,0x4
/* 702001b4:	10e00008 */ 	beqz	$a3,.L702001d8
/* 702001b8:	0007c880 */ 	sll	$t9,$a3,0x2
/* 702001bc:	01191021 */ 	addu	$v0,$t0,$t9
/* 702001c0:	8c450000 */ 	lw	$a1,0x0($v0)
/* 702001c4:	00057080 */ 	sll	$t6,$a1,0x2
/* 702001c8:	00ce7821 */ 	addu	$t7,$a2,$t6
/* 702001cc:	aded0000 */ 	sw	$t5,0x0($t7)
/* 702001d0:	24b80001 */ 	addiu	$t8,$a1,0x1
/* 702001d4:	ac580000 */ 	sw	$t8,0x0($v0)
.L702001d8:
/* 702001d8:	25ad0001 */ 	addiu	$t5,$t5,0x1
/* 702001dc:	01a9082b */ 	sltu	$at,$t5,$t1
/* 702001e0:	5420fff3 */ 	bnezl	$at,.L702001b0
/* 702001e4:	8fe70000 */ 	lw	$a3,0x0($ra)
/* 702001e8:	8fb90598 */ 	lw	$t9,0x598($sp)
/* 702001ec:	afa30064 */ 	sw	$v1,0x64($sp)
/* 702001f0:	00006825 */ 	move	$t5,$zero
/* 702001f4:	0333082a */ 	slt	$at,$t9,$s3
/* 702001f8:	afa0006c */ 	sw	$zero,0x6c($sp)
/* 702001fc:	00c0f825 */ 	move	$ra,$a2
/* 70200200:	240cffff */ 	li	$t4,-1
/* 70200204:	00115023 */ 	negu	$t2,$s1
/* 70200208:	afa00534 */ 	sw	$zero,0x534($sp)
/* 7020020c:	00002825 */ 	move	$a1,$zero
/* 70200210:	142000bf */ 	bnez	$at,.L70200510
/* 70200214:	00004025 */ 	move	$t0,$zero
/* 70200218:	00137080 */ 	sll	$t6,$s3,0x2
/* 7020021c:	27af05a0 */ 	addiu	$t7,$sp,0x5a0
/* 70200220:	01cfc021 */ 	addu	$t8,$t6,$t7
/* 70200224:	3c1e7020 */ 	lui	$s8,%hi(var702012e0)
/* 70200228:	3c147020 */ 	lui	$s4,%hi(hufts)
/* 7020022c:	269413e8 */ 	addiu	$s4,$s4,%lo(hufts)
/* 70200230:	27de12e0 */ 	addiu	$s8,$s8,%lo(var702012e0)
/* 70200234:	afb80050 */ 	sw	$t8,0x50($sp)
/* 70200238:	8fb605fc */ 	lw	$s6,0x5fc($sp)
/* 7020023c:	27b50574 */ 	addiu	$s5,$sp,0x574
.L70200240:
/* 70200240:	8fb90050 */ 	lw	$t9,0x50($sp)
/* 70200244:	8faf05ec */ 	lw	$t7,0x5ec($sp)
/* 70200248:	000c8080 */ 	sll	$s0,$t4,0x2
/* 7020024c:	8f370000 */ 	lw	$s7,0x0($t9)
/* 70200250:	27ae006c */ 	addiu	$t6,$sp,0x6c
/* 70200254:	000fc080 */ 	sll	$t8,$t7,0x2
/* 70200258:	02e03025 */ 	move	$a2,$s7
/* 7020025c:	12e000a4 */ 	beqz	$s7,.L702004f0
/* 70200260:	26f7ffff */ 	addiu	$s7,$s7,-1
/* 70200264:	020e5821 */ 	addu	$t3,$s0,$t6
/* 70200268:	27b900b4 */ 	addiu	$t9,$sp,0xb4
/* 7020026c:	03197021 */ 	addu	$t6,$t8,$t9
/* 70200270:	24180001 */ 	li	$t8,0x1
/* 70200274:	266f001f */ 	addiu	$t7,$s3,0x1f
/* 70200278:	01f8c804 */ 	sllv	$t9,$t8,$t7
/* 7020027c:	afb90034 */ 	sw	$t9,0x34($sp)
/* 70200280:	afae0038 */ 	sw	$t6,0x38($sp)
.L70200284:
/* 70200284:	01511821 */ 	addu	$v1,$t2,$s1
/* 70200288:	0073082a */ 	slt	$at,$v1,$s3
/* 7020028c:	10200047 */ 	beqz	$at,.L702003ac
/* 70200290:	26f20001 */ 	addiu	$s2,$s7,0x1
/* 70200294:	27ae0534 */ 	addiu	$t6,$sp,0x534
/* 70200298:	020e4821 */ 	addu	$t1,$s0,$t6
/* 7020029c:	8fb80598 */ 	lw	$t8,0x598($sp)
.L702002a0:
/* 702002a0:	258c0001 */ 	addiu	$t4,$t4,0x1
/* 702002a4:	26100004 */ 	addiu	$s0,$s0,0x4
/* 702002a8:	03034023 */ 	subu	$t0,$t8,$v1
/* 702002ac:	0228082b */ 	sltu	$at,$s1,$t0
/* 702002b0:	25290004 */ 	addiu	$t1,$t1,0x4
/* 702002b4:	256b0004 */ 	addiu	$t3,$t3,0x4
/* 702002b8:	10200002 */ 	beqz	$at,.L702002c4
/* 702002bc:	00605025 */ 	move	$t2,$v1
/* 702002c0:	02204025 */ 	move	$t0,$s1
.L702002c4:
/* 702002c4:	026a1023 */ 	subu	$v0,$s3,$t2
/* 702002c8:	240f0001 */ 	li	$t7,0x1
/* 702002cc:	004f2004 */ 	sllv	$a0,$t7,$v0
/* 702002d0:	0244082b */ 	sltu	$at,$s2,$a0
/* 702002d4:	10200013 */ 	beqz	$at,.L70200324
/* 702002d8:	00403825 */ 	move	$a3,$v0
/* 702002dc:	24470001 */ 	addiu	$a3,$v0,0x1
/* 702002e0:	00971823 */ 	subu	$v1,$a0,$s7
/* 702002e4:	0013c880 */ 	sll	$t9,$s3,0x2
/* 702002e8:	27ae05a0 */ 	addiu	$t6,$sp,0x5a0
/* 702002ec:	00e8082b */ 	sltu	$at,$a3,$t0
/* 702002f0:	2463ffff */ 	addiu	$v1,$v1,-1
/* 702002f4:	1020000b */ 	beqz	$at,.L70200324
/* 702002f8:	032e2821 */ 	addu	$a1,$t9,$t6
.L702002fc:
/* 702002fc:	8ca40004 */ 	lw	$a0,0x4($a1)
/* 70200300:	00031040 */ 	sll	$v0,$v1,0x1
/* 70200304:	24a50004 */ 	addiu	$a1,$a1,0x4
/* 70200308:	0082082b */ 	sltu	$at,$a0,$v0
/* 7020030c:	50200006 */ 	beqzl	$at,.L70200328
/* 70200310:	8e830000 */ 	lw	$v1,0x0($s4)
/* 70200314:	24e70001 */ 	addiu	$a3,$a3,0x1
/* 70200318:	00e8082b */ 	sltu	$at,$a3,$t0
/* 7020031c:	1420fff7 */ 	bnez	$at,.L702002fc
/* 70200320:	00441823 */ 	subu	$v1,$v0,$a0
.L70200324:
/* 70200324:	8e830000 */ 	lw	$v1,0x0($s4)
.L70200328:
/* 70200328:	8fd90000 */ 	lw	$t9,0x0($s8)
/* 7020032c:	24180001 */ 	li	$t8,0x1
/* 70200330:	00f84004 */ 	sllv	$t0,$t8,$a3
/* 70200334:	000378c0 */ 	sll	$t7,$v1,0x3
/* 70200338:	00687021 */ 	addu	$t6,$v1,$t0
/* 7020033c:	25d80001 */ 	addiu	$t8,$t6,0x1
/* 70200340:	01f92821 */ 	addu	$a1,$t7,$t9
/* 70200344:	ae980000 */ 	sw	$t8,0x0($s4)
/* 70200348:	24a40008 */ 	addiu	$a0,$a1,0x8
/* 7020034c:	aec40000 */ 	sw	$a0,0x0($s6)
/* 70200350:	aca00004 */ 	sw	$zero,0x4($a1)
/* 70200354:	24b60004 */ 	addiu	$s6,$a1,0x4
/* 70200358:	00802825 */ 	move	$a1,$a0
/* 7020035c:	1180000f */ 	beqz	$t4,.L7020039c
/* 70200360:	ad240000 */ 	sw	$a0,0x0($t1)
/* 70200364:	ad6d0000 */ 	sw	$t5,0x0($t3)
/* 70200368:	24ef0010 */ 	addiu	$t7,$a3,0x10
/* 7020036c:	a3b10575 */ 	sb	$s1,0x575($sp)
/* 70200370:	a3af0574 */ 	sb	$t7,0x574($sp)
/* 70200374:	afa40578 */ 	sw	$a0,0x578($sp)
/* 70200378:	8d39fffc */ 	lw	$t9,-0x4($t1)
/* 7020037c:	01517023 */ 	subu	$t6,$t2,$s1
/* 70200380:	8ea10000 */ 	lw	$at,0x0($s5)
/* 70200384:	01cdc006 */ 	srlv	$t8,$t5,$t6
/* 70200388:	001878c0 */ 	sll	$t7,$t8,0x3
/* 7020038c:	032f7021 */ 	addu	$t6,$t9,$t7
/* 70200390:	adc10000 */ 	sw	$at,0x0($t6)
/* 70200394:	8eaf0004 */ 	lw	$t7,0x4($s5)
/* 70200398:	adcf0004 */ 	sw	$t7,0x4($t6)
.L7020039c:
/* 7020039c:	01511821 */ 	addu	$v1,$t2,$s1
/* 702003a0:	0073082a */ 	slt	$at,$v1,$s3
/* 702003a4:	5420ffbe */ 	bnezl	$at,.L702002a0
/* 702003a8:	8fb80598 */ 	lw	$t8,0x598($sp)
.L702003ac:
/* 702003ac:	8fb80038 */ 	lw	$t8,0x38($sp)
/* 702003b0:	24190001 */ 	li	$t9,0x1
/* 702003b4:	026a1823 */ 	subu	$v1,$s3,$t2
/* 702003b8:	01593004 */ 	sllv	$a2,$t9,$t2
/* 702003bc:	03f8082b */ 	sltu	$at,$ra,$t8
/* 702003c0:	a3a30575 */ 	sb	$v1,0x575($sp)
/* 702003c4:	14200004 */ 	bnez	$at,.L702003d8
/* 702003c8:	24c6ffff */ 	addiu	$a2,$a2,-1
/* 702003cc:	240e0063 */ 	li	$t6,0x63
/* 702003d0:	10000020 */ 	b	.L70200454
/* 702003d4:	a3ae0574 */ 	sb	$t6,0x574($sp)
.L702003d8:
/* 702003d8:	8fe20000 */ 	lw	$v0,0x0($ra)
/* 702003dc:	8faf05f0 */ 	lw	$t7,0x5f0($sp)
/* 702003e0:	8fa405f0 */ 	lw	$a0,0x5f0($sp)
/* 702003e4:	8fb905f8 */ 	lw	$t9,0x5f8($sp)
/* 702003e8:	004f082b */ 	sltu	$at,$v0,$t7
/* 702003ec:	1020000b */ 	beqz	$at,.L7020041c
/* 702003f0:	2c410100 */ 	sltiu	$at,$v0,0x100
/* 702003f4:	10200004 */ 	beqz	$at,.L70200408
/* 702003f8:	2418000f */ 	li	$t8,0xf
/* 702003fc:	24190010 */ 	li	$t9,0x10
/* 70200400:	10000002 */ 	b	.L7020040c
/* 70200404:	a3b90574 */ 	sb	$t9,0x574($sp)
.L70200408:
/* 70200408:	a3b80574 */ 	sb	$t8,0x574($sp)
.L7020040c:
/* 7020040c:	8fee0000 */ 	lw	$t6,0x0($ra)
/* 70200410:	27ff0004 */ 	addiu	$ra,$ra,0x4
/* 70200414:	1000000f */ 	b	.L70200454
/* 70200418:	a7ae0578 */ 	sh	$t6,0x578($sp)
.L7020041c:
/* 7020041c:	00447823 */ 	subu	$t7,$v0,$a0
/* 70200420:	01f9c021 */ 	addu	$t8,$t7,$t9
/* 70200424:	930e0000 */ 	lbu	$t6,0x0($t8)
/* 70200428:	8faf05f4 */ 	lw	$t7,0x5f4($sp)
/* 7020042c:	27ff0004 */ 	addiu	$ra,$ra,0x4
/* 70200430:	a3ae0574 */ 	sb	$t6,0x574($sp)
/* 70200434:	8ff9fffc */ 	lw	$t9,-0x4($ra)
/* 70200438:	0019c040 */ 	sll	$t8,$t9,0x1
/* 7020043c:	0004c840 */ 	sll	$t9,$a0,0x1
/* 70200440:	01f87021 */ 	addu	$t6,$t7,$t8
/* 70200444:	00197823 */ 	negu	$t7,$t9
/* 70200448:	01cfc021 */ 	addu	$t8,$t6,$t7
/* 7020044c:	97190000 */ 	lhu	$t9,0x0($t8)
/* 70200450:	a7b90578 */ 	sh	$t9,0x578($sp)
.L70200454:
/* 70200454:	014d3806 */ 	srlv	$a3,$t5,$t2
/* 70200458:	00e8082b */ 	sltu	$at,$a3,$t0
/* 7020045c:	1020000b */ 	beqz	$at,.L7020048c
/* 70200460:	240e0001 */ 	li	$t6,0x1
/* 70200464:	006e1004 */ 	sllv	$v0,$t6,$v1
.L70200468:
/* 70200468:	8ea10000 */ 	lw	$at,0x0($s5)
/* 7020046c:	000778c0 */ 	sll	$t7,$a3,0x3
/* 70200470:	00afc021 */ 	addu	$t8,$a1,$t7
/* 70200474:	af010000 */ 	sw	$at,0x0($t8)
/* 70200478:	8eae0004 */ 	lw	$t6,0x4($s5)
/* 7020047c:	00e23821 */ 	addu	$a3,$a3,$v0
/* 70200480:	00e8082b */ 	sltu	$at,$a3,$t0
/* 70200484:	1420fff8 */ 	bnez	$at,.L70200468
/* 70200488:	af0e0004 */ 	sw	$t6,0x4($t8)
.L7020048c:
/* 7020048c:	8fa70034 */ 	lw	$a3,0x34($sp)
/* 70200490:	8d620000 */ 	lw	$v0,0x0($t3)
/* 70200494:	01a77824 */ 	and	$t7,$t5,$a3
/* 70200498:	11e00005 */ 	beqz	$t7,.L702004b0
.L7020049c:
/* 7020049c:	0007c842 */ 	srl	$t9,$a3,0x1
/* 702004a0:	01a76826 */ 	xor	$t5,$t5,$a3
/* 702004a4:	01b9c024 */ 	and	$t8,$t5,$t9
/* 702004a8:	1700fffc */ 	bnez	$t8,.L7020049c
/* 702004ac:	03203825 */ 	move	$a3,$t9
.L702004b0:
/* 702004b0:	01a76826 */ 	xor	$t5,$t5,$a3
/* 702004b4:	01a67024 */ 	and	$t6,$t5,$a2
/* 702004b8:	11c2000b */ 	beq	$t6,$v0,.L702004e8
/* 702004bc:	02e03025 */ 	move	$a2,$s7
.L702004c0:
/* 702004c0:	01515023 */ 	subu	$t2,$t2,$s1
/* 702004c4:	240f0001 */ 	li	$t7,0x1
/* 702004c8:	014fc804 */ 	sllv	$t9,$t7,$t2
/* 702004cc:	8d6ffffc */ 	lw	$t7,-0x4($t3)
/* 702004d0:	2738ffff */ 	addiu	$t8,$t9,-1
/* 702004d4:	01b87024 */ 	and	$t6,$t5,$t8
/* 702004d8:	258cffff */ 	addiu	$t4,$t4,-1
/* 702004dc:	2610fffc */ 	addiu	$s0,$s0,-4
/* 702004e0:	15cffff7 */ 	bne	$t6,$t7,.L702004c0
/* 702004e4:	256bfffc */ 	addiu	$t3,$t3,-4
.L702004e8:
/* 702004e8:	16e0ff66 */ 	bnez	$s7,.L70200284
/* 702004ec:	26f7ffff */ 	addiu	$s7,$s7,-1
.L702004f0:
/* 702004f0:	8fb90050 */ 	lw	$t9,0x50($sp)
/* 702004f4:	8fae0598 */ 	lw	$t6,0x598($sp)
/* 702004f8:	26730001 */ 	addiu	$s3,$s3,0x1
/* 702004fc:	27380004 */ 	addiu	$t8,$t9,0x4
/* 70200500:	01d3082a */ 	slt	$at,$t6,$s3
/* 70200504:	1020ff4e */ 	beqz	$at,.L70200240
/* 70200508:	afb80050 */ 	sw	$t8,0x50($sp)
/* 7020050c:	afb605fc */ 	sw	$s6,0x5fc($sp)
.L70200510:
/* 70200510:	8fa20064 */ 	lw	$v0,0x64($sp)
/* 70200514:	0002782b */ 	sltu	$t7,$zero,$v0
/* 70200518:	11e00004 */ 	beqz	$t7,.L7020052c
/* 7020051c:	01e01025 */ 	move	$v0,$t7
/* 70200520:	8fa20598 */ 	lw	$v0,0x598($sp)
/* 70200524:	38590001 */ 	xori	$t9,$v0,0x1
/* 70200528:	0019102b */ 	sltu	$v0,$zero,$t9
.L7020052c:
/* 7020052c:	8fbf002c */ 	lw	$ra,0x2c($sp)
/* 70200530:	8fb00008 */ 	lw	$s0,0x8($sp)
/* 70200534:	8fb1000c */ 	lw	$s1,0xc($sp)
/* 70200538:	8fb20010 */ 	lw	$s2,0x10($sp)
/* 7020053c:	8fb30014 */ 	lw	$s3,0x14($sp)
/* 70200540:	8fb40018 */ 	lw	$s4,0x18($sp)
/* 70200544:	8fb5001c */ 	lw	$s5,0x1c($sp)
/* 70200548:	8fb60020 */ 	lw	$s6,0x20($sp)
/* 7020054c:	8fb70024 */ 	lw	$s7,0x24($sp)
/* 70200550:	8fbe0028 */ 	lw	$s8,0x28($sp)
/* 70200554:	03e00008 */ 	jr	$ra
/* 70200558:	27bd05e8 */ 	addiu	$sp,$sp,0x5e8
);

u32 inflate_codes(struct huft *tl, struct huft *td, int bl, int bd)
{
	register u32 e;  /* table entry flag/number of extra bits */
	u32 n, d;        /* length and index for copy */
	u32 w;           /* current window position */
	struct huft *t;       /* pointer to table entry */
	u32 ml, md;      /* masks for bl and bd bits */
	register u32 b;       /* bit buffer */
	register u32 k;  /* number of bits in bit buffer */

	/* make local copies of globals */
	b = bb;                       /* initialize bit buffer */
	k = bk;
	w = wp;                       /* initialize window position */

	/* inflate the coded data */
	ml = mask_bits[bl];           /* precompute masks for speed */
	md = mask_bits[bd];

	for (;;) {                    /* do until end of block */
		NEEDBITS(bl)
		if ((e = (t = tl + (b & ml))->e) > 16) {
			do {
				DUMPBITS(t->b)
				e -= 16;
				NEEDBITS(e)
			} while ((e = (t = t->v.t + (b & mask_bits[e]))->e) > 16);
		}
		DUMPBITS(t->b)

		if (e == 16) {              /* then it's a literal */
			outbuf[w++] = (u8)t->v.n;
		} else {                      /* it's an EOB or a length */
			/* exit if end of block */
			if (e == 15) {
				break;
			}

			/* get length of block to copy */
			NEEDBITS(e)
			n = t->v.n + (b & mask_bits[e]);
			DUMPBITS(e);

			/* decode distance of block to copy */
			NEEDBITS(bd)
			if ((e = (t = td + (b & md))->e) > 16) {
				do {
					DUMPBITS(t->b)
					e -= 16;
					NEEDBITS(e)
				} while ((e = (t = t->v.t + (b & mask_bits[e]))->e) > 16);
			}
			DUMPBITS(t->b)
			NEEDBITS(e)
			d = w - t->v.n - (b & mask_bits[e]);
			DUMPBITS(e)

			/* do the copy */
			do {
				e = n;
				n = 0;

				do {
					outbuf[w++] = outbuf[d++];
				} while (--e);
			} while (n);
		}
	}

	/* restore the globals from the locals */
	wp = w;                       /* restore global window pointer */
	bb = b;                       /* restore global bit buffer */
	bk = k;

	/* done */
	return 0;
}

u32 inflate_stored(void)
{
	s32 n;           /* number of bytes in block */
	s32 w;           /* current window position */
	register u32 b; /* bit buffer */
	register u32 k;  /* number of bits in bit buffer */

	/* make local copies of globals */
	b = bb;                       /* initialize bit buffer */
	k = bk;
	w = wp;                       /* initialize window position */

	/* go to byte boundary */
	n = k & 7;
	DUMPBITS(n);

	/* get the length and its complement */
	NEEDBITS(16)
	n = (b & 0xffff);
	DUMPBITS(16)

	NEEDBITS(16)
	DUMPBITS(16)

	/* read and output the compressed data */
	while (n--) {
		NEEDBITS(8)
		outbuf[w++] = (u8)b;

		DUMPBITS(8)
	}

	/* restore the globals from the locals */
	wp = w;                       /* restore global window pointer */
	bb = b;                       /* restore global bit buffer */
	bk = k;
	return 0;
}

u32 inflate_fixed(void)
{
	s32 i;                /* temporary variable */
	struct huft *tl;      /* literal/length code table */
	struct huft *td;      /* distance code table */
	u32 bl;               /* lookup bits for tl */
	u32 bd;               /* lookup bits for td */
	u32 l[288];           /* length list for huft_build */

	/* set up literal table */
	for (i = 0; i < 144; i++) {
		l[i] = 8;
	}
	for (; i < 256; i++) {
		l[i] = 9;
	}
	for (; i < 280; i++) {
		l[i] = 7;
	}
	for (; i < 288; i++) {
		l[i] = 8;
	}

	bl = 7;

	huft_build(l, 288, 257, cplens, cplext, &tl, &bl);

	/* set up distance table */
	for (i = 0; i < 30; i++) {
		l[i] = 5;
	}

	bd = 5;

	huft_build(l, 30, 0, cpdist, cpdext, &td, &bd);

	inflate_codes(tl, td, bl, bd);

	return 0;
}

GLOBAL_ASM(
glabel inflate_dynamic
/* 70200b54:	3c087020 */ 	lui	$t0,%hi(bk)
/* 70200b58:	8d0813b8 */ 	lw	$t0,%lo(bk)($t0)
/* 70200b5c:	27bdfa98 */ 	addiu	$sp,$sp,-1384
/* 70200b60:	3c097020 */ 	lui	$t1,%hi(bb)
/* 70200b64:	2d010005 */ 	sltiu	$at,$t0,0x5
/* 70200b68:	afbf002c */ 	sw	$ra,0x2c($sp)
/* 70200b6c:	afb00028 */ 	sw	$s0,0x28($sp)
/* 70200b70:	1020000f */ 	beqz	$at,.L70200bb0
/* 70200b74:	8d2913b4 */ 	lw	$t1,%lo(bb)($t1)
/* 70200b78:	3c037020 */ 	lui	$v1,%hi(inbuf)
/* 70200b7c:	3c0a7020 */ 	lui	$t2,%hi(inptr)
/* 70200b80:	254a12d8 */ 	addiu	$t2,$t2,%lo(inptr)
/* 70200b84:	8c6312d0 */ 	lw	$v1,%lo(inbuf)($v1)
.L70200b88:
/* 70200b88:	8d420000 */ 	lw	$v0,0x0($t2)
/* 70200b8c:	00627021 */ 	addu	$t6,$v1,$v0
/* 70200b90:	91cf0000 */ 	lbu	$t7,0x0($t6)
/* 70200b94:	24590001 */ 	addiu	$t9,$v0,0x1
/* 70200b98:	ad590000 */ 	sw	$t9,0x0($t2)
/* 70200b9c:	010fc004 */ 	sllv	$t8,$t7,$t0
/* 70200ba0:	25080008 */ 	addiu	$t0,$t0,0x8
/* 70200ba4:	2d010005 */ 	sltiu	$at,$t0,0x5
/* 70200ba8:	1420fff7 */ 	bnez	$at,.L70200b88
/* 70200bac:	01384825 */ 	or	$t1,$t1,$t8
.L70200bb0:
/* 70200bb0:	312e001f */ 	andi	$t6,$t1,0x1f
/* 70200bb4:	2508fffb */ 	addiu	$t0,$t0,-5
/* 70200bb8:	3c0a7020 */ 	lui	$t2,%hi(inptr)
/* 70200bbc:	25cf0101 */ 	addiu	$t7,$t6,0x101
/* 70200bc0:	0009c142 */ 	srl	$t8,$t1,0x5
/* 70200bc4:	2d010005 */ 	sltiu	$at,$t0,0x5
/* 70200bc8:	254a12d8 */ 	addiu	$t2,$t2,%lo(inptr)
/* 70200bcc:	afaf053c */ 	sw	$t7,0x53c($sp)
/* 70200bd0:	1020000d */ 	beqz	$at,.L70200c08
/* 70200bd4:	03004825 */ 	move	$t1,$t8
/* 70200bd8:	3c037020 */ 	lui	$v1,%hi(inbuf)
/* 70200bdc:	8c6312d0 */ 	lw	$v1,%lo(inbuf)($v1)
.L70200be0:
/* 70200be0:	8d420000 */ 	lw	$v0,0x0($t2)
/* 70200be4:	0062c821 */ 	addu	$t9,$v1,$v0
/* 70200be8:	932e0000 */ 	lbu	$t6,0x0($t9)
/* 70200bec:	24580001 */ 	addiu	$t8,$v0,0x1
/* 70200bf0:	ad580000 */ 	sw	$t8,0x0($t2)
/* 70200bf4:	010e7804 */ 	sllv	$t7,$t6,$t0
/* 70200bf8:	25080008 */ 	addiu	$t0,$t0,0x8
/* 70200bfc:	2d010005 */ 	sltiu	$at,$t0,0x5
/* 70200c00:	1420fff7 */ 	bnez	$at,.L70200be0
/* 70200c04:	012f4825 */ 	or	$t1,$t1,$t7
.L70200c08:
/* 70200c08:	3139001f */ 	andi	$t9,$t1,0x1f
/* 70200c0c:	2508fffb */ 	addiu	$t0,$t0,-5
/* 70200c10:	272e0001 */ 	addiu	$t6,$t9,0x1
/* 70200c14:	00097942 */ 	srl	$t7,$t1,0x5
/* 70200c18:	2d010004 */ 	sltiu	$at,$t0,0x4
/* 70200c1c:	afae0538 */ 	sw	$t6,0x538($sp)
/* 70200c20:	1020000d */ 	beqz	$at,.L70200c58
/* 70200c24:	01e04825 */ 	move	$t1,$t7
/* 70200c28:	3c037020 */ 	lui	$v1,%hi(inbuf)
/* 70200c2c:	8c6312d0 */ 	lw	$v1,%lo(inbuf)($v1)
.L70200c30:
/* 70200c30:	8d420000 */ 	lw	$v0,0x0($t2)
/* 70200c34:	0062c021 */ 	addu	$t8,$v1,$v0
/* 70200c38:	93190000 */ 	lbu	$t9,0x0($t8)
/* 70200c3c:	244f0001 */ 	addiu	$t7,$v0,0x1
/* 70200c40:	ad4f0000 */ 	sw	$t7,0x0($t2)
/* 70200c44:	01197004 */ 	sllv	$t6,$t9,$t0
/* 70200c48:	25080008 */ 	addiu	$t0,$t0,0x8
/* 70200c4c:	2d010004 */ 	sltiu	$at,$t0,0x4
/* 70200c50:	1420fff7 */ 	bnez	$at,.L70200c30
/* 70200c54:	012e4825 */ 	or	$t1,$t1,$t6
.L70200c58:
/* 70200c58:	3126000f */ 	andi	$a2,$t1,0xf
/* 70200c5c:	24c60004 */ 	addiu	$a2,$a2,0x4
/* 70200c60:	0009c102 */ 	srl	$t8,$t1,0x4
/* 70200c64:	03004825 */ 	move	$t1,$t8
/* 70200c68:	2508fffc */ 	addiu	$t0,$t0,-4
/* 70200c6c:	10c0001d */ 	beqz	$a2,.L70200ce4
/* 70200c70:	00002025 */ 	move	$a0,$zero
/* 70200c74:	3c057020 */ 	lui	$a1,%hi(border)
/* 70200c78:	24a512e4 */ 	addiu	$a1,$a1,%lo(border)
/* 70200c7c:	27b00040 */ 	addiu	$s0,$sp,0x40
.L70200c80:
/* 70200c80:	2d010003 */ 	sltiu	$at,$t0,0x3
/* 70200c84:	1020000d */ 	beqz	$at,.L70200cbc
/* 70200c88:	24840001 */ 	addiu	$a0,$a0,0x1
/* 70200c8c:	3c037020 */ 	lui	$v1,%hi(inbuf)
/* 70200c90:	8c6312d0 */ 	lw	$v1,%lo(inbuf)($v1)
.L70200c94:
/* 70200c94:	8d420000 */ 	lw	$v0,0x0($t2)
/* 70200c98:	0062c821 */ 	addu	$t9,$v1,$v0
/* 70200c9c:	932e0000 */ 	lbu	$t6,0x0($t9)
/* 70200ca0:	24580001 */ 	addiu	$t8,$v0,0x1
/* 70200ca4:	ad580000 */ 	sw	$t8,0x0($t2)
/* 70200ca8:	010e7804 */ 	sllv	$t7,$t6,$t0
/* 70200cac:	25080008 */ 	addiu	$t0,$t0,0x8
/* 70200cb0:	2d010003 */ 	sltiu	$at,$t0,0x3
/* 70200cb4:	1420fff7 */ 	bnez	$at,.L70200c94
/* 70200cb8:	012f4825 */ 	or	$t1,$t1,$t7
.L70200cbc:
/* 70200cbc:	90ae0000 */ 	lbu	$t6,0x0($a1)
/* 70200cc0:	31390007 */ 	andi	$t9,$t1,0x7
/* 70200cc4:	24a50001 */ 	addiu	$a1,$a1,0x1
/* 70200cc8:	000e7880 */ 	sll	$t7,$t6,0x2
/* 70200ccc:	020fc021 */ 	addu	$t8,$s0,$t7
/* 70200cd0:	000970c2 */ 	srl	$t6,$t1,0x3
/* 70200cd4:	af190000 */ 	sw	$t9,0x0($t8)
/* 70200cd8:	01c04825 */ 	move	$t1,$t6
/* 70200cdc:	1486ffe8 */ 	bne	$a0,$a2,.L70200c80
/* 70200ce0:	2508fffd */ 	addiu	$t0,$t0,-3
.L70200ce4:
/* 70200ce4:	2c810013 */ 	sltiu	$at,$a0,0x13
/* 70200ce8:	1020000d */ 	beqz	$at,.L70200d20
/* 70200cec:	27b00040 */ 	addiu	$s0,$sp,0x40
/* 70200cf0:	3c0f7020 */ 	lui	$t7,%hi(border)
/* 70200cf4:	25ef12e4 */ 	addiu	$t7,$t7,%lo(border)
/* 70200cf8:	3c027020 */ 	lui	$v0,0x7020
/* 70200cfc:	244212f7 */ 	addiu	$v0,$v0,0x12f7
/* 70200d00:	008f2821 */ 	addu	$a1,$a0,$t7
.L70200d04:
/* 70200d04:	90b90000 */ 	lbu	$t9,0x0($a1)
/* 70200d08:	24a50001 */ 	addiu	$a1,$a1,0x1
/* 70200d0c:	00a2082b */ 	sltu	$at,$a1,$v0
/* 70200d10:	0019c080 */ 	sll	$t8,$t9,0x2
/* 70200d14:	02187021 */ 	addu	$t6,$s0,$t8
/* 70200d18:	1420fffa */ 	bnez	$at,.L70200d04
/* 70200d1c:	adc00000 */ 	sw	$zero,0x0($t6)
.L70200d20:
/* 70200d20:	240f0007 */ 	li	$t7,0x7
/* 70200d24:	27b90550 */ 	addiu	$t9,$sp,0x550
/* 70200d28:	27b80548 */ 	addiu	$t8,$sp,0x548
/* 70200d2c:	afaf0548 */ 	sw	$t7,0x548($sp)
/* 70200d30:	afb80018 */ 	sw	$t8,0x18($sp)
/* 70200d34:	afb90014 */ 	sw	$t9,0x14($sp)
/* 70200d38:	02002025 */ 	move	$a0,$s0
/* 70200d3c:	24050013 */ 	li	$a1,0x13
/* 70200d40:	24060013 */ 	li	$a2,0x13
/* 70200d44:	00003825 */ 	move	$a3,$zero
/* 70200d48:	afa00010 */ 	sw	$zero,0x10($sp)
/* 70200d4c:	afa80534 */ 	sw	$t0,0x534($sp)
/* 70200d50:	0c080000 */ 	jal	.L70200000
/* 70200d54:	afa90530 */ 	sw	$t1,0x530($sp)
/* 70200d58:	8fb90548 */ 	lw	$t9,0x548($sp)
/* 70200d5c:	8fae053c */ 	lw	$t6,0x53c($sp)
/* 70200d60:	8faf0538 */ 	lw	$t7,0x538($sp)
/* 70200d64:	3c0b7020 */ 	lui	$t3,%hi(mask_bits)
/* 70200d68:	0019c040 */ 	sll	$t8,$t9,0x1
/* 70200d6c:	3c0a7020 */ 	lui	$t2,%hi(inptr)
/* 70200d70:	01785821 */ 	addu	$t3,$t3,$t8
/* 70200d74:	01cf3821 */ 	addu	$a3,$t6,$t7
/* 70200d78:	254a12d8 */ 	addiu	$t2,$t2,%lo(inptr)
/* 70200d7c:	8fa80534 */ 	lw	$t0,0x534($sp)
/* 70200d80:	8fa90530 */ 	lw	$t1,0x530($sp)
/* 70200d84:	956b13bc */ 	lhu	$t3,%lo(mask_bits)($t3)
/* 70200d88:	00003025 */ 	move	$a2,$zero
/* 70200d8c:	10e0008a */ 	beqz	$a3,.L70200fb8
/* 70200d90:	00002825 */ 	move	$a1,$zero
/* 70200d94:	240d0011 */ 	li	$t5,0x11
/* 70200d98:	240c0010 */ 	li	$t4,0x10
/* 70200d9c:	8fae0548 */ 	lw	$t6,0x548($sp)
.L70200da0:
/* 70200da0:	3c037020 */ 	lui	$v1,%hi(inbuf)
/* 70200da4:	010e082b */ 	sltu	$at,$t0,$t6
/* 70200da8:	5020000e */ 	beqzl	$at,.L70200de4
/* 70200dac:	8fae0550 */ 	lw	$t6,0x550($sp)
/* 70200db0:	8c6312d0 */ 	lw	$v1,%lo(inbuf)($v1)
.L70200db4:
/* 70200db4:	8d420000 */ 	lw	$v0,0x0($t2)
/* 70200db8:	00627821 */ 	addu	$t7,$v1,$v0
/* 70200dbc:	91f90000 */ 	lbu	$t9,0x0($t7)
/* 70200dc0:	8faf0548 */ 	lw	$t7,0x548($sp)
/* 70200dc4:	244e0001 */ 	addiu	$t6,$v0,0x1
/* 70200dc8:	0119c004 */ 	sllv	$t8,$t9,$t0
/* 70200dcc:	25080008 */ 	addiu	$t0,$t0,0x8
/* 70200dd0:	010f082b */ 	sltu	$at,$t0,$t7
/* 70200dd4:	01384825 */ 	or	$t1,$t1,$t8
/* 70200dd8:	1420fff6 */ 	bnez	$at,.L70200db4
/* 70200ddc:	ad4e0000 */ 	sw	$t6,0x0($t2)
/* 70200de0:	8fae0550 */ 	lw	$t6,0x550($sp)
.L70200de4:
/* 70200de4:	012bc824 */ 	and	$t9,$t1,$t3
/* 70200de8:	0019c0c0 */ 	sll	$t8,$t9,0x3
/* 70200dec:	030e7821 */ 	addu	$t7,$t8,$t6
/* 70200df0:	afaf054c */ 	sw	$t7,0x54c($sp)
/* 70200df4:	91e40001 */ 	lbu	$a0,0x1($t7)
/* 70200df8:	0005c880 */ 	sll	$t9,$a1,0x2
/* 70200dfc:	0219c021 */ 	addu	$t8,$s0,$t9
/* 70200e00:	00894806 */ 	srlv	$t1,$t1,$a0
/* 70200e04:	01044023 */ 	subu	$t0,$t0,$a0
/* 70200e08:	95e40004 */ 	lhu	$a0,0x4($t7)
/* 70200e0c:	2c810010 */ 	sltiu	$at,$a0,0x10
/* 70200e10:	10200005 */ 	beqz	$at,.L70200e28
/* 70200e14:	00000000 */ 	nop
/* 70200e18:	00803025 */ 	move	$a2,$a0
/* 70200e1c:	af040000 */ 	sw	$a0,0x0($t8)
/* 70200e20:	10000062 */ 	b	.L70200fac
/* 70200e24:	24a50001 */ 	addiu	$a1,$a1,0x1
.L70200e28:
/* 70200e28:	148c0020 */ 	bne	$a0,$t4,.L70200eac
/* 70200e2c:	2d010002 */ 	sltiu	$at,$t0,0x2
/* 70200e30:	1020000c */ 	beqz	$at,.L70200e64
/* 70200e34:	3c037020 */ 	lui	$v1,%hi(inbuf)
/* 70200e38:	8c6312d0 */ 	lw	$v1,%lo(inbuf)($v1)
.L70200e3c:
/* 70200e3c:	8d420000 */ 	lw	$v0,0x0($t2)
/* 70200e40:	00627021 */ 	addu	$t6,$v1,$v0
/* 70200e44:	91cf0000 */ 	lbu	$t7,0x0($t6)
/* 70200e48:	24580001 */ 	addiu	$t8,$v0,0x1
/* 70200e4c:	ad580000 */ 	sw	$t8,0x0($t2)
/* 70200e50:	010fc804 */ 	sllv	$t9,$t7,$t0
/* 70200e54:	25080008 */ 	addiu	$t0,$t0,0x8
/* 70200e58:	2d010002 */ 	sltiu	$at,$t0,0x2
/* 70200e5c:	1420fff7 */ 	bnez	$at,.L70200e3c
/* 70200e60:	01394825 */ 	or	$t1,$t1,$t9
.L70200e64:
/* 70200e64:	31240003 */ 	andi	$a0,$t1,0x3
/* 70200e68:	24840003 */ 	addiu	$a0,$a0,0x3
/* 70200e6c:	00801825 */ 	move	$v1,$a0
/* 70200e70:	00097082 */ 	srl	$t6,$t1,0x2
/* 70200e74:	01c04825 */ 	move	$t1,$t6
/* 70200e78:	2508fffe */ 	addiu	$t0,$t0,-2
/* 70200e7c:	1080004b */ 	beqz	$a0,.L70200fac
/* 70200e80:	2484ffff */ 	addiu	$a0,$a0,-1
/* 70200e84:	00057880 */ 	sll	$t7,$a1,0x2
/* 70200e88:	020f1021 */ 	addu	$v0,$s0,$t7
.L70200e8c:
/* 70200e8c:	00801825 */ 	move	$v1,$a0
/* 70200e90:	ac460000 */ 	sw	$a2,0x0($v0)
/* 70200e94:	24a50001 */ 	addiu	$a1,$a1,0x1
/* 70200e98:	24420004 */ 	addiu	$v0,$v0,0x4
/* 70200e9c:	1480fffb */ 	bnez	$a0,.L70200e8c
/* 70200ea0:	2484ffff */ 	addiu	$a0,$a0,-1
/* 70200ea4:	10000042 */ 	b	.L70200fb0
/* 70200ea8:	00a7082b */ 	sltu	$at,$a1,$a3
.L70200eac:
/* 70200eac:	148d0021 */ 	bne	$a0,$t5,.L70200f34
/* 70200eb0:	00003025 */ 	move	$a2,$zero
/* 70200eb4:	2d010003 */ 	sltiu	$at,$t0,0x3
/* 70200eb8:	1020000c */ 	beqz	$at,.L70200eec
/* 70200ebc:	3c037020 */ 	lui	$v1,%hi(inbuf)
/* 70200ec0:	8c6312d0 */ 	lw	$v1,%lo(inbuf)($v1)
.L70200ec4:
/* 70200ec4:	8d420000 */ 	lw	$v0,0x0($t2)
/* 70200ec8:	0062c821 */ 	addu	$t9,$v1,$v0
/* 70200ecc:	93380000 */ 	lbu	$t8,0x0($t9)
/* 70200ed0:	244f0001 */ 	addiu	$t7,$v0,0x1
/* 70200ed4:	ad4f0000 */ 	sw	$t7,0x0($t2)
/* 70200ed8:	01187004 */ 	sllv	$t6,$t8,$t0
/* 70200edc:	25080008 */ 	addiu	$t0,$t0,0x8
/* 70200ee0:	2d010003 */ 	sltiu	$at,$t0,0x3
/* 70200ee4:	1420fff7 */ 	bnez	$at,.L70200ec4
/* 70200ee8:	012e4825 */ 	or	$t1,$t1,$t6
.L70200eec:
/* 70200eec:	31240007 */ 	andi	$a0,$t1,0x7
/* 70200ef0:	24840003 */ 	addiu	$a0,$a0,0x3
/* 70200ef4:	00801825 */ 	move	$v1,$a0
/* 70200ef8:	0009c8c2 */ 	srl	$t9,$t1,0x3
/* 70200efc:	03204825 */ 	move	$t1,$t9
/* 70200f00:	2508fffd */ 	addiu	$t0,$t0,-3
/* 70200f04:	10800029 */ 	beqz	$a0,.L70200fac
/* 70200f08:	2484ffff */ 	addiu	$a0,$a0,-1
/* 70200f0c:	0005c080 */ 	sll	$t8,$a1,0x2
/* 70200f10:	02181021 */ 	addu	$v0,$s0,$t8
.L70200f14:
/* 70200f14:	00801825 */ 	move	$v1,$a0
/* 70200f18:	ac400000 */ 	sw	$zero,0x0($v0)
/* 70200f1c:	24a50001 */ 	addiu	$a1,$a1,0x1
/* 70200f20:	24420004 */ 	addiu	$v0,$v0,0x4
/* 70200f24:	1480fffb */ 	bnez	$a0,.L70200f14
/* 70200f28:	2484ffff */ 	addiu	$a0,$a0,-1
/* 70200f2c:	10000020 */ 	b	.L70200fb0
/* 70200f30:	00a7082b */ 	sltu	$at,$a1,$a3
.L70200f34:
/* 70200f34:	2d010007 */ 	sltiu	$at,$t0,0x7
/* 70200f38:	1020000c */ 	beqz	$at,.L70200f6c
/* 70200f3c:	3c037020 */ 	lui	$v1,%hi(inbuf)
/* 70200f40:	8c6312d0 */ 	lw	$v1,%lo(inbuf)($v1)
.L70200f44:
/* 70200f44:	8d420000 */ 	lw	$v0,0x0($t2)
/* 70200f48:	00627021 */ 	addu	$t6,$v1,$v0
/* 70200f4c:	91cf0000 */ 	lbu	$t7,0x0($t6)
/* 70200f50:	24580001 */ 	addiu	$t8,$v0,0x1
/* 70200f54:	ad580000 */ 	sw	$t8,0x0($t2)
/* 70200f58:	010fc804 */ 	sllv	$t9,$t7,$t0
/* 70200f5c:	25080008 */ 	addiu	$t0,$t0,0x8
/* 70200f60:	2d010007 */ 	sltiu	$at,$t0,0x7
/* 70200f64:	1420fff7 */ 	bnez	$at,.L70200f44
/* 70200f68:	01394825 */ 	or	$t1,$t1,$t9
.L70200f6c:
/* 70200f6c:	3124007f */ 	andi	$a0,$t1,0x7f
/* 70200f70:	2484000b */ 	addiu	$a0,$a0,0xb
/* 70200f74:	00801825 */ 	move	$v1,$a0
/* 70200f78:	000971c2 */ 	srl	$t6,$t1,0x7
/* 70200f7c:	01c04825 */ 	move	$t1,$t6
/* 70200f80:	2508fff9 */ 	addiu	$t0,$t0,-7
/* 70200f84:	10800009 */ 	beqz	$a0,.L70200fac
/* 70200f88:	2484ffff */ 	addiu	$a0,$a0,-1
/* 70200f8c:	00057880 */ 	sll	$t7,$a1,0x2
/* 70200f90:	020f1021 */ 	addu	$v0,$s0,$t7
.L70200f94:
/* 70200f94:	00801825 */ 	move	$v1,$a0
/* 70200f98:	ac400000 */ 	sw	$zero,0x0($v0)
/* 70200f9c:	24a50001 */ 	addiu	$a1,$a1,0x1
/* 70200fa0:	24420004 */ 	addiu	$v0,$v0,0x4
/* 70200fa4:	1480fffb */ 	bnez	$a0,.L70200f94
/* 70200fa8:	2484ffff */ 	addiu	$a0,$a0,-1
.L70200fac:
/* 70200fac:	00a7082b */ 	sltu	$at,$a1,$a3
.L70200fb0:
/* 70200fb0:	5420ff7b */ 	bnezl	$at,.L70200da0
/* 70200fb4:	8fae0548 */ 	lw	$t6,0x548($sp)
.L70200fb8:
/* 70200fb8:	3c017020 */ 	lui	$at,0x7020
/* 70200fbc:	3c197020 */ 	lui	$t9,%hi(lbits)
/* 70200fc0:	8f3913e0 */ 	lw	$t9,%lo(lbits)($t9)
/* 70200fc4:	ac2913b4 */ 	sw	$t1,0x13b4($at)
/* 70200fc8:	3c187020 */ 	lui	$t8,%hi(cplext)
/* 70200fcc:	3c017020 */ 	lui	$at,0x7020
/* 70200fd0:	27181338 */ 	addiu	$t8,$t8,%lo(cplext)
/* 70200fd4:	3c077020 */ 	lui	$a3,%hi(cplens)
/* 70200fd8:	27ae0550 */ 	addiu	$t6,$sp,0x550
/* 70200fdc:	27af0548 */ 	addiu	$t7,$sp,0x548
/* 70200fe0:	ac2813b8 */ 	sw	$t0,0x13b8($at)
/* 70200fe4:	afaf0018 */ 	sw	$t7,0x18($sp)
/* 70200fe8:	afae0014 */ 	sw	$t6,0x14($sp)
/* 70200fec:	24e712f8 */ 	addiu	$a3,$a3,%lo(cplens)
/* 70200ff0:	afb80010 */ 	sw	$t8,0x10($sp)
/* 70200ff4:	02002025 */ 	move	$a0,$s0
/* 70200ff8:	8fa5053c */ 	lw	$a1,0x53c($sp)
/* 70200ffc:	24060101 */ 	li	$a2,0x101
/* 70201000:	0c080000 */ 	jal	.L70200000
/* 70201004:	afb90548 */ 	sw	$t9,0x548($sp)
/* 70201008:	3c197020 */ 	lui	$t9,%hi(dbits)
/* 7020100c:	8f3913e4 */ 	lw	$t9,%lo(dbits)($t9)
/* 70201010:	8fb8053c */ 	lw	$t8,0x53c($sp)
/* 70201014:	3c0f7020 */ 	lui	$t7,%hi(cpdext)
/* 70201018:	afb90544 */ 	sw	$t9,0x544($sp)
/* 7020101c:	00187080 */ 	sll	$t6,$t8,0x2
/* 70201020:	27b80544 */ 	addiu	$t8,$sp,0x544
/* 70201024:	27b9054c */ 	addiu	$t9,$sp,0x54c
/* 70201028:	25ef1394 */ 	addiu	$t7,$t7,%lo(cpdext)
/* 7020102c:	3c077020 */ 	lui	$a3,%hi(cpdist)
/* 70201030:	24e71358 */ 	addiu	$a3,$a3,%lo(cpdist)
/* 70201034:	afaf0010 */ 	sw	$t7,0x10($sp)
/* 70201038:	afb90014 */ 	sw	$t9,0x14($sp)
/* 7020103c:	afb80018 */ 	sw	$t8,0x18($sp)
/* 70201040:	020e2021 */ 	addu	$a0,$s0,$t6
/* 70201044:	8fa50538 */ 	lw	$a1,0x538($sp)
/* 70201048:	0c080000 */ 	jal	.L70200000
/* 7020104c:	00003025 */ 	move	$a2,$zero
/* 70201050:	8fa40550 */ 	lw	$a0,0x550($sp)
/* 70201054:	8fa5054c */ 	lw	$a1,0x54c($sp)
/* 70201058:	8fa60548 */ 	lw	$a2,0x548($sp)
/* 7020105c:	0c080157 */ 	jal	inflate_codes
/* 70201060:	8fa70544 */ 	lw	$a3,0x544($sp)
/* 70201064:	8fbf002c */ 	lw	$ra,0x2c($sp)
/* 70201068:	8fb00028 */ 	lw	$s0,0x28($sp)
/* 7020106c:	27bd0568 */ 	addiu	$sp,$sp,0x568
/* 70201070:	03e00008 */ 	jr	$ra
/* 70201074:	00001025 */ 	move	$v0,$zero
);

u32 inflate_block(u32 *e)
{
	u32 t;
	u32 b = bb;
	u32 k = bk;

	NEEDBITS(1)
	*e = b & 1;
	DUMPBITS(1)

	NEEDBITS(2)
	t = b & 3;
	DUMPBITS(2)

	bb = b;
	bk = k;

	if (t == 2) {
		return inflate_dynamic();
	}

	if (t == 0) {
		return inflate_stored();
	}

	if (t == 1) {
		return inflate_fixed();
	}

	return 2;
}

u32 inflate(void)
{
	u32 sp52;
	u32 r;
	u32 s1;

	wp = 0;
	bk = 0;
	bb = 0;
	s1 = 0;

	do {
		hufts = 0;
		r = inflate_block(&sp52);

		if (r != 0) {
			return r;
		}

		if (s1 < hufts) {
			s1 = hufts;
		}
	} while (sp52 == 0);

	while (bk >= 8) {
		bk -= 8;
		inptr--;
	}

	return 0;
}

u32 inflate1173(void *src, void *dst, void *buffer)
{
	inbuf = src;
	outbuf = dst;
	var702012e0 = buffer;
	inbuf += 2;
	inbuf += 3;
	wp = 0;
	inptr = 0;

	inflate();

	return wp;
}
