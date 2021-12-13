	.file	"Sketch.cpp"
__SP_H__ = 0x3e
__SP_L__ = 0x3d
__SREG__ = 0x3f
__tmp_reg__ = 0
__zero_reg__ = 1
	.text
.Ltext0:
	.cfi_sections	.debug_frame
	.section	.text.setup,"ax",@progbits
.global	setup
	.type	setup, @function
setup:
.LFB124:
	.file 1 ".././Sketch.cpp"
	.loc 1 21 0
	.cfi_startproc
/* prologue: function */
/* frame size = 0 */
/* stack size = 0 */
.L__stack_usage = 0
	.loc 1 22 0
	in r24,0x34
	andi r24,lo8(-9)
	out 0x34,r24
.LBB7:
.LBB8:
	.file 2 "c:\\program files (x86)\\atmel\\studio\\7.0\\toolchain\\avr8\\avr8-gnu-toolchain\\avr\\include\\avr\\wdt.h"
	.loc 2 528 0
/* #APP */
 ;  528 "c:\program files (x86)\atmel\studio\7.0\toolchain\avr8\avr8-gnu-toolchain\avr\include\avr\wdt.h" 1
	in __tmp_reg__,__SREG__
	cli
	wdr
	lds r24,96
	ori r24,24
	sts 96,r24
	sts 96,__zero_reg__
	out __SREG__,__tmp_reg__
	
 ;  0 "" 2
/* #NOAPP */
.LBE8:
.LBE7:
	.loc 1 25 0
	sbi 0x10,0
	.loc 1 26 0
	cbi 0x11,0
	.loc 1 28 0
	sbi 0x10,1
	.loc 1 29 0
	sbi 0x11,1
	.loc 1 31 0
	sbi 0x4,4
	.loc 1 32 0
	cbi 0x5,4
	.loc 1 34 0
	cbi 0xa,6
	.loc 1 35 0
	sbi 0xb,6
	.loc 1 37 0
	ldi r24,lo8(Keyboard)
	ldi r25,hi8(Keyboard)
	jmp _ZN9Keyboard_5beginEv
.LVL0:
	.cfi_endproc
.LFE124:
	.size	setup, .-setup
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string	"hellow dreg"
	.section	.text.loop,"ax",@progbits
.global	loop
	.type	loop, @function
loop:
.LFB125:
	.loc 1 40 0
	.cfi_startproc
/* prologue: function */
/* frame size = 0 */
/* stack size = 0 */
.L__stack_usage = 0
	.loc 1 41 0
	sbis 0x9,6
	rjmp .L3
	.loc 1 43 0
	cbi 0x5,4
	ret
.L3:
	.loc 1 47 0
	ldi r22,lo8(.LC0)
	ldi r23,hi8(.LC0)
	ldi r24,lo8(Keyboard)
	ldi r25,hi8(Keyboard)
	call _ZN5Print5printEPKc
.LVL1:
	.loc 1 48 0
	sbi 0x5,4
.LVL2:
.LBB9:
.LBB10:
	.file 3 "c:\\program files (x86)\\atmel\\studio\\7.0\\toolchain\\avr8\\avr8-gnu-toolchain\\avr\\include\\util\\delay.h"
	.loc 3 187 0
	ldi r18,lo8(639999)
	ldi r24,hi8(639999)
	ldi r25,hlo8(639999)
1:	subi r18,1
	sbci r24,0
	sbci r25,0
	brne 1b
	rjmp .
	nop
	ret
.LBE10:
.LBE9:
	.cfi_endproc
.LFE125:
	.size	loop, .-loop
	.text
.Letext0:
	.file 4 "c:\\program files (x86)\\atmel\\studio\\7.0\\toolchain\\avr8\\avr8-gnu-toolchain\\avr\\include\\stdint.h"
	.file 5 "c:\\program files (x86)\\atmel\\studio\\7.0\\toolchain\\avr8\\avr8-gnu-toolchain\\lib\\gcc\\avr\\5.4.0\\include\\stddef.h"
	.file 6 "..\\\\..\\ArduinoCore\\include\\core/Print.h"
	.file 7 "..\\\\..\\ArduinoCore\\include\\variants\\leonardo/pins_arduino.h"
	.file 8 "..\\\\..\\ArduinoCore\\include\\libraries\\Keyboard/Keyboard.h"
	.file 9 "..\\\\..\\ArduinoCore\\include\\core/Stream.h"
	.section	.debug_info,"",@progbits
.Ldebug_info0:
	.long	0x352
	.word	0x2
	.long	.Ldebug_abbrev0
	.byte	0x4
	.uleb128 0x1
	.long	.LASF37
	.byte	0x4
	.long	.LASF38
	.long	.LASF39
	.long	.Ldebug_ranges0+0
	.long	0
	.long	0
	.long	.Ldebug_line0
	.uleb128 0x2
	.byte	0x1
	.byte	0x6
	.long	.LASF0
	.uleb128 0x3
	.long	.LASF2
	.byte	0x4
	.byte	0x7e
	.long	0x3b
	.uleb128 0x2
	.byte	0x1
	.byte	0x8
	.long	.LASF1
	.uleb128 0x4
	.byte	0x2
	.byte	0x5
	.string	"int"
	.uleb128 0x3
	.long	.LASF3
	.byte	0x4
	.byte	0x80
	.long	0x54
	.uleb128 0x2
	.byte	0x2
	.byte	0x7
	.long	.LASF4
	.uleb128 0x2
	.byte	0x4
	.byte	0x5
	.long	.LASF5
	.uleb128 0x3
	.long	.LASF6
	.byte	0x4
	.byte	0x82
	.long	0x6d
	.uleb128 0x2
	.byte	0x4
	.byte	0x7
	.long	.LASF7
	.uleb128 0x2
	.byte	0x8
	.byte	0x5
	.long	.LASF8
	.uleb128 0x2
	.byte	0x8
	.byte	0x7
	.long	.LASF9
	.uleb128 0x3
	.long	.LASF10
	.byte	0x5
	.byte	0xd8
	.long	0x54
	.uleb128 0x2
	.byte	0x1
	.byte	0x2
	.long	.LASF11
	.uleb128 0x2
	.byte	0x1
	.byte	0x8
	.long	.LASF12
	.uleb128 0x5
	.byte	0x2
	.long	0xa1
	.uleb128 0x6
	.long	0x94
	.uleb128 0x2
	.byte	0x4
	.byte	0x4
	.long	.LASF13
	.uleb128 0x2
	.byte	0x4
	.byte	0x4
	.long	.LASF14
	.uleb128 0x7
	.long	.LASF40
	.byte	0x1
	.long	0x3b
	.byte	0x9
	.byte	0x29
	.long	0xd7
	.uleb128 0x8
	.long	.LASF15
	.byte	0
	.uleb128 0x8
	.long	.LASF16
	.byte	0x1
	.uleb128 0x8
	.long	.LASF17
	.byte	0x2
	.byte	0
	.uleb128 0x9
	.long	.LASF31
	.byte	0x1
	.long	0x11d
	.uleb128 0xa
	.byte	0x1
	.long	.LASF26
	.byte	0x6
	.byte	0x39
	.long	.LASF41
	.long	0x82
	.byte	0x1
	.long	0xfa
	.long	0x10b
	.uleb128 0xb
	.long	0x11d
	.byte	0x1
	.uleb128 0xc
	.long	0x9b
	.uleb128 0xc
	.long	0x82
	.byte	0
	.uleb128 0xd
	.byte	0x1
	.byte	0x1
	.long	.LASF33
	.long	.LASF35
	.byte	0x6
	.byte	0x43
	.long	.LASF33
	.byte	0
	.uleb128 0x5
	.byte	0x2
	.long	0xd7
	.uleb128 0x2
	.byte	0x4
	.byte	0x4
	.long	.LASF18
	.uleb128 0xe
	.long	.LASF42
	.uleb128 0x2
	.byte	0x2
	.byte	0x7
	.long	.LASF19
	.uleb128 0x2
	.byte	0x2
	.byte	0x7
	.long	.LASF20
	.uleb128 0x6
	.long	0x30
	.uleb128 0xf
	.long	.LASF21
	.byte	0x2
	.word	0x1ec
	.byte	0x3
	.long	0x15e
	.uleb128 0x10
	.uleb128 0x11
	.long	.LASF23
	.byte	0x2
	.word	0x202
	.long	0x30
	.byte	0
	.byte	0
	.uleb128 0x12
	.long	.LASF22
	.byte	0x3
	.byte	0xa6
	.byte	0x3
	.long	0x19b
	.uleb128 0x13
	.long	.LASF43
	.byte	0x3
	.byte	0xa6
	.long	0xad
	.uleb128 0x14
	.long	.LASF24
	.byte	0x3
	.byte	0xa8
	.long	0xad
	.uleb128 0x14
	.long	.LASF25
	.byte	0x3
	.byte	0xac
	.long	0x62
	.uleb128 0x15
	.byte	0x1
	.long	.LASF44
	.byte	0x3
	.byte	0xad
	.byte	0x1
	.uleb128 0xc
	.long	0x6d
	.byte	0
	.byte	0
	.uleb128 0x16
	.byte	0x1
	.long	.LASF27
	.byte	0x1
	.byte	0x15
	.long	.LFB124
	.long	.LFE124
	.byte	0x3
	.byte	0x92
	.uleb128 0x20
	.sleb128 2
	.byte	0x1
	.long	0x1e2
	.uleb128 0x17
	.long	0x142
	.long	.LBB7
	.long	.LBE7
	.byte	0x1
	.byte	0x17
	.long	0x1d7
	.uleb128 0x18
	.long	.LBB8
	.long	.LBE8
	.uleb128 0x19
	.long	0x150
	.byte	0
	.byte	0
	.uleb128 0x1a
	.long	.LVL0
	.byte	0x1
	.long	0x336
	.byte	0
	.uleb128 0x16
	.byte	0x1
	.long	.LASF28
	.byte	0x1
	.byte	0x28
	.long	.LFB125
	.long	.LFE125
	.byte	0x3
	.byte	0x92
	.uleb128 0x20
	.sleb128 2
	.byte	0x1
	.long	0x24f
	.uleb128 0x17
	.long	0x15e
	.long	.LBB9
	.long	.LBE9
	.byte	0x1
	.byte	0x31
	.long	0x236
	.uleb128 0x1b
	.long	0x16a
	.byte	0x4
	.long	0x43480000
	.uleb128 0x18
	.long	.LBB10
	.long	.LBE10
	.uleb128 0x1c
	.long	0x175
	.byte	0x4
	.long	0x4a435000
	.uleb128 0x1d
	.long	0x180
	.long	0x30d400
	.byte	0
	.byte	0
	.uleb128 0x1e
	.long	.LVL1
	.long	0x10b
	.uleb128 0x1f
	.byte	0x6
	.byte	0x66
	.byte	0x93
	.uleb128 0x1
	.byte	0x67
	.byte	0x93
	.uleb128 0x1
	.byte	0x5
	.byte	0x3
	.long	.LC0
	.byte	0
	.byte	0
	.uleb128 0x20
	.long	0x18b
	.byte	0
	.long	0x25f
	.uleb128 0xc
	.long	0x6d
	.byte	0
	.uleb128 0x21
	.string	"SDA"
	.byte	0x7
	.byte	0x67
	.long	0x13d
	.byte	0x2
	.uleb128 0x21
	.string	"SCL"
	.byte	0x7
	.byte	0x68
	.long	0x13d
	.byte	0x3
	.uleb128 0x21
	.string	"SS"
	.byte	0x7
	.byte	0x74
	.long	0x13d
	.byte	0x11
	.uleb128 0x22
	.long	.LASF29
	.byte	0x7
	.byte	0x75
	.long	0x13d
	.byte	0x10
	.uleb128 0x22
	.long	.LASF30
	.byte	0x7
	.byte	0x76
	.long	0x13d
	.byte	0xe
	.uleb128 0x21
	.string	"SCK"
	.byte	0x7
	.byte	0x77
	.long	0x13d
	.byte	0xf
	.uleb128 0x21
	.string	"A0"
	.byte	0x7
	.byte	0x88
	.long	0x13d
	.byte	0x12
	.uleb128 0x21
	.string	"A1"
	.byte	0x7
	.byte	0x89
	.long	0x13d
	.byte	0x13
	.uleb128 0x21
	.string	"A2"
	.byte	0x7
	.byte	0x8a
	.long	0x13d
	.byte	0x14
	.uleb128 0x21
	.string	"A3"
	.byte	0x7
	.byte	0x8b
	.long	0x13d
	.byte	0x15
	.uleb128 0x21
	.string	"A4"
	.byte	0x7
	.byte	0x8c
	.long	0x13d
	.byte	0x16
	.uleb128 0x21
	.string	"A5"
	.byte	0x7
	.byte	0x8d
	.long	0x13d
	.byte	0x17
	.uleb128 0x21
	.string	"A6"
	.byte	0x7
	.byte	0x8e
	.long	0x13d
	.byte	0x18
	.uleb128 0x21
	.string	"A7"
	.byte	0x7
	.byte	0x8f
	.long	0x13d
	.byte	0x19
	.uleb128 0x21
	.string	"A8"
	.byte	0x7
	.byte	0x90
	.long	0x13d
	.byte	0x1a
	.uleb128 0x21
	.string	"A9"
	.byte	0x7
	.byte	0x91
	.long	0x13d
	.byte	0x1b
	.uleb128 0x21
	.string	"A10"
	.byte	0x7
	.byte	0x92
	.long	0x13d
	.byte	0x1c
	.uleb128 0x21
	.string	"A11"
	.byte	0x7
	.byte	0x93
	.long	0x13d
	.byte	0x1d
	.uleb128 0x9
	.long	.LASF32
	.byte	0x1
	.long	0x348
	.uleb128 0xd
	.byte	0x1
	.byte	0x1
	.long	.LASF34
	.long	.LASF36
	.byte	0x8
	.byte	0x66
	.long	.LASF34
	.byte	0
	.uleb128 0x23
	.long	.LASF45
	.byte	0x8
	.byte	0x6e
	.long	0x32c
	.byte	0x1
	.byte	0x1
	.byte	0
	.section	.debug_abbrev,"",@progbits
.Ldebug_abbrev0:
	.uleb128 0x1
	.uleb128 0x11
	.byte	0x1
	.uleb128 0x25
	.uleb128 0xe
	.uleb128 0x13
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x1b
	.uleb128 0xe
	.uleb128 0x55
	.uleb128 0x6
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x52
	.uleb128 0x1
	.uleb128 0x10
	.uleb128 0x6
	.byte	0
	.byte	0
	.uleb128 0x2
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.byte	0
	.byte	0
	.uleb128 0x3
	.uleb128 0x16
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x4
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0x8
	.byte	0
	.byte	0
	.uleb128 0x5
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x6
	.uleb128 0x26
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x7
	.uleb128 0x4
	.byte	0x1
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x8
	.uleb128 0x28
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x1c
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x9
	.uleb128 0x2
	.byte	0x1
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3c
	.uleb128 0xc
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xa
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x2007
	.uleb128 0xe
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3c
	.uleb128 0xc
	.uleb128 0x64
	.uleb128 0x13
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xb
	.uleb128 0x5
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x34
	.uleb128 0xc
	.byte	0
	.byte	0
	.uleb128 0xc
	.uleb128 0x5
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xd
	.uleb128 0x2e
	.byte	0
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3c
	.uleb128 0xc
	.uleb128 0x2007
	.uleb128 0xe
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x2007
	.uleb128 0xe
	.byte	0
	.byte	0
	.uleb128 0xe
	.uleb128 0x3b
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.byte	0
	.byte	0
	.uleb128 0xf
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x20
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x10
	.uleb128 0xb
	.byte	0x1
	.byte	0
	.byte	0
	.uleb128 0x11
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x12
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x20
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x13
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x14
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x15
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x3c
	.uleb128 0xc
	.byte	0
	.byte	0
	.uleb128 0x16
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0xa
	.uleb128 0x2117
	.uleb128 0xc
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x17
	.uleb128 0x1d
	.byte	0x1
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x58
	.uleb128 0xb
	.uleb128 0x59
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x18
	.uleb128 0xb
	.byte	0x1
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.byte	0
	.byte	0
	.uleb128 0x19
	.uleb128 0x34
	.byte	0
	.uleb128 0x31
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x1a
	.uleb128 0x4109
	.byte	0
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x2115
	.uleb128 0xc
	.uleb128 0x31
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x1b
	.uleb128 0x5
	.byte	0
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x1c
	.uleb128 0xa
	.byte	0
	.byte	0
	.uleb128 0x1c
	.uleb128 0x34
	.byte	0
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x1c
	.uleb128 0xa
	.byte	0
	.byte	0
	.uleb128 0x1d
	.uleb128 0x34
	.byte	0
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x1c
	.uleb128 0x6
	.byte	0
	.byte	0
	.uleb128 0x1e
	.uleb128 0x4109
	.byte	0x1
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x31
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x1f
	.uleb128 0x410a
	.byte	0
	.uleb128 0x2
	.uleb128 0xa
	.uleb128 0x2111
	.uleb128 0xa
	.byte	0
	.byte	0
	.uleb128 0x20
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x47
	.uleb128 0x13
	.uleb128 0x20
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x21
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x1c
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x22
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x1c
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x23
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3c
	.uleb128 0xc
	.byte	0
	.byte	0
	.byte	0
	.section	.debug_aranges,"",@progbits
	.long	0x24
	.word	0x2
	.long	.Ldebug_info0
	.byte	0x4
	.byte	0
	.word	0
	.word	0
	.long	.LFB124
	.long	.LFE124-.LFB124
	.long	.LFB125
	.long	.LFE125-.LFB125
	.long	0
	.long	0
	.section	.debug_ranges,"",@progbits
.Ldebug_ranges0:
	.long	.LFB124
	.long	.LFE124
	.long	.LFB125
	.long	.LFE125
	.long	0
	.long	0
	.section	.debug_line,"",@progbits
.Ldebug_line0:
	.section	.debug_str,"MS",@progbits,1
.LASF33:
	.string	"_ZN5Print5printEPKc"
.LASF10:
	.string	"size_t"
.LASF23:
	.string	"temp_reg"
.LASF32:
	.string	"Keyboard_"
.LASF27:
	.string	"setup"
.LASF21:
	.string	"wdt_disable"
.LASF37:
	.ascii	"GN"
	.string	"U C++11 5.4.0 -mn-flash=1 -mmcu=avr5 -g2 -Os -std=gnu++11 -funsigned-char -funsigned-bitfields -fno-threadsafe-statics -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -mn-flash=1 -mno-skip-bug -fno-rtti -fno-enforce-eh-specs -fno-exceptions"
.LASF11:
	.string	"bool"
.LASF14:
	.string	"double"
.LASF36:
	.string	"begin"
.LASF44:
	.string	"__builtin_avr_delay_cycles"
.LASF13:
	.string	"float"
.LASF28:
	.string	"loop"
.LASF7:
	.string	"long unsigned int"
.LASF22:
	.string	"_delay_ms"
.LASF16:
	.string	"SKIP_NONE"
.LASF26:
	.string	"write"
.LASF40:
	.string	"LookaheadMode"
.LASF1:
	.string	"unsigned char"
.LASF45:
	.string	"Keyboard"
.LASF41:
	.string	"_ZN5Print5writeEPKcj"
.LASF24:
	.string	"__tmp"
.LASF39:
	.string	"C:\\\\Users\\\\xia\\\\Desktop\\\\rpk2\\\\firmwares\\\\leohellowkeyboard\\\\leohellowkeyboard\\\\helloworldleo\\\\Debug"
.LASF15:
	.string	"SKIP_ALL"
.LASF30:
	.string	"MISO"
.LASF34:
	.string	"_ZN9Keyboard_5beginEv"
.LASF29:
	.string	"MOSI"
.LASF4:
	.string	"unsigned int"
.LASF17:
	.string	"SKIP_WHITESPACE"
.LASF9:
	.string	"long long unsigned int"
.LASF2:
	.string	"uint8_t"
.LASF20:
	.string	"sizetype"
.LASF38:
	.string	".././Sketch.cpp"
.LASF8:
	.string	"long long int"
.LASF12:
	.string	"char"
.LASF35:
	.string	"print"
.LASF25:
	.string	"__ticks_dc"
.LASF42:
	.string	"decltype(nullptr)"
.LASF3:
	.string	"uint16_t"
.LASF6:
	.string	"uint32_t"
.LASF5:
	.string	"long int"
.LASF18:
	.string	"long double"
.LASF0:
	.string	"signed char"
.LASF19:
	.string	"short unsigned int"
.LASF31:
	.string	"Print"
.LASF43:
	.string	"__ms"
	.ident	"GCC: (AVR_8_bit_GNU_Toolchain_3.6.2_1778) 5.4.0"
.global __do_copy_data
