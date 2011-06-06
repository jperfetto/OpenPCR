	.file	"MassStorage.c"
__SREG__ = 0x3f
__SP_H__ = 0x3e
__SP_L__ = 0x3d
__CCP__  = 0x34
__tmp_reg__ = 0
__zero_reg__ = 1
	.section	.debug_abbrev,"",@progbits
.Ldebug_abbrev0:
	.section	.debug_info,"",@progbits
.Ldebug_info0:
	.section	.debug_line,"",@progbits
.Ldebug_line0:
	.text
.Ltext0:
	.section	.text.EVENT_USB_Device_Connect,"ax",@progbits
.global	EVENT_USB_Device_Connect
	.type	EVENT_USB_Device_Connect, @function
EVENT_USB_Device_Connect:
.LFB90:
.LSM0:
/* prologue: function */
/* frame size = 0 */
/* epilogue start */
.LSM1:
	ret
.LFE90:
	.size	EVENT_USB_Device_Connect, .-EVENT_USB_Device_Connect
	.section	.text.EVENT_USB_Device_Disconnect,"ax",@progbits
.global	EVENT_USB_Device_Disconnect
	.type	EVENT_USB_Device_Disconnect, @function
EVENT_USB_Device_Disconnect:
.LFB91:
.LSM2:
/* prologue: function */
/* frame size = 0 */
/* epilogue start */
.LSM3:
	ret
.LFE91:
	.size	EVENT_USB_Device_Disconnect, .-EVENT_USB_Device_Disconnect
	.section	.text.CALLBACK_MS_Device_SCSICommandReceived,"ax",@progbits
.global	CALLBACK_MS_Device_SCSICommandReceived
	.type	CALLBACK_MS_Device_SCSICommandReceived, @function
CALLBACK_MS_Device_SCSICommandReceived:
.LFB94:
.LSM4:
.LVL0:
/* prologue: function */
/* frame size = 0 */
.LSM5:
	call SCSI_DecodeSCSICommand
.LVL1:
/* epilogue start */
.LSM6:
	ret
.LFE94:
	.size	CALLBACK_MS_Device_SCSICommandReceived, .-CALLBACK_MS_Device_SCSICommandReceived
	.section	.text.EVENT_USB_Device_ControlRequest,"ax",@progbits
.global	EVENT_USB_Device_ControlRequest
	.type	EVENT_USB_Device_ControlRequest, @function
EVENT_USB_Device_ControlRequest:
.LFB93:
.LSM7:
/* prologue: function */
/* frame size = 0 */
.LSM8:
	ldi r24,lo8(Disk_MS_Interface)
	ldi r25,hi8(Disk_MS_Interface)
	call MS_Device_ProcessControlRequest
/* epilogue start */
.LSM9:
	ret
.LFE93:
	.size	EVENT_USB_Device_ControlRequest, .-EVENT_USB_Device_ControlRequest
	.section	.text.EVENT_USB_Device_ConfigurationChanged,"ax",@progbits
.global	EVENT_USB_Device_ConfigurationChanged
	.type	EVENT_USB_Device_ConfigurationChanged, @function
EVENT_USB_Device_ConfigurationChanged:
.LFB92:
.LSM10:
/* prologue: function */
/* frame size = 0 */
.LSM11:
	ldi r24,lo8(Disk_MS_Interface)
	ldi r25,hi8(Disk_MS_Interface)
	call MS_Device_ConfigureEndpoints
/* epilogue start */
.LSM12:
	ret
.LFE92:
	.size	EVENT_USB_Device_ConfigurationChanged, .-EVENT_USB_Device_ConfigurationChanged
	.section	.text.SetupHardware,"ax",@progbits
.global	SetupHardware
	.type	SetupHardware, @function
SetupHardware:
.LFB89:
.LSM13:
/* prologue: function */
/* frame size = 0 */
.LSM14:
	in r24,84-32
	andi r24,lo8(-9)
	out 84-32,r24
.LSM15:
	ldi r24,lo8(24)
/* #APP */
 ;  82 "MassStorage.c" 1
	in __tmp_reg__, __SREG__
	cli
	sts 96, r24
	sts 96, __zero_reg__
	out __SREG__,__tmp_reg__
	
 ;  0 "" 2
.LSM16:
/* #NOAPP */
	call USB_Init
/* epilogue start */
.LSM17:
	ret
.LFE89:
	.size	SetupHardware, .-SetupHardware
	.section	.text.main,"ax",@progbits
.global	main
	.type	main, @function
main:
.LFB88:
.LSM18:
/* prologue: function */
/* frame size = 0 */
.LSM19:
	call SetupHardware
.LSM20:
/* #APP */
 ;  68 "MassStorage.c" 1
	sei
 ;  0 "" 2
/* #NOAPP */
.L14:
.LSM21:
	ldi r24,lo8(Disk_MS_Interface)
	ldi r25,hi8(Disk_MS_Interface)
	call MS_Device_USBTask
.LSM22:
	call USB_USBTask
	rjmp .L14
.LFE88:
	.size	main, .-main
.global	Disk_MS_Interface
	.data
	.type	Disk_MS_Interface, @object
	.size	Disk_MS_Interface, 55
Disk_MS_Interface:
	.byte	0
	.byte	3
	.word	64
	.byte	0
	.byte	4
	.word	64
	.byte	0
	.byte	1
	.skip 45,0
	.section	.debug_frame,"",@progbits
.Lframe0:
	.long	.LECIE0-.LSCIE0
.LSCIE0:
	.long	0xffffffff
	.byte	0x1
	.string	""
	.uleb128 0x1
	.sleb128 -1
	.byte	0x24
	.byte	0xc
	.uleb128 0x20
	.uleb128 0x0
	.p2align	2
.LECIE0:
.LSFDE0:
	.long	.LEFDE0-.LASFDE0
.LASFDE0:
	.long	.Lframe0
	.long	.LFB90
	.long	.LFE90-.LFB90
	.p2align	2
.LEFDE0:
.LSFDE2:
	.long	.LEFDE2-.LASFDE2
.LASFDE2:
	.long	.Lframe0
	.long	.LFB91
	.long	.LFE91-.LFB91
	.p2align	2
.LEFDE2:
.LSFDE4:
	.long	.LEFDE4-.LASFDE4
.LASFDE4:
	.long	.Lframe0
	.long	.LFB94
	.long	.LFE94-.LFB94
	.p2align	2
.LEFDE4:
.LSFDE6:
	.long	.LEFDE6-.LASFDE6
.LASFDE6:
	.long	.Lframe0
	.long	.LFB93
	.long	.LFE93-.LFB93
	.p2align	2
.LEFDE6:
.LSFDE8:
	.long	.LEFDE8-.LASFDE8
.LASFDE8:
	.long	.Lframe0
	.long	.LFB92
	.long	.LFE92-.LFB92
	.p2align	2
.LEFDE8:
.LSFDE10:
	.long	.LEFDE10-.LASFDE10
.LASFDE10:
	.long	.Lframe0
	.long	.LFB89
	.long	.LFE89-.LFB89
	.p2align	2
.LEFDE10:
.LSFDE12:
	.long	.LEFDE12-.LASFDE12
.LASFDE12:
	.long	.Lframe0
	.long	.LFB88
	.long	.LFE88-.LFB88
	.p2align	2
.LEFDE12:
	.text
.Letext0:
	.section	.debug_loc,"",@progbits
.Ldebug_loc0:
.LLST3:
	.long	.LVL0
	.long	.LVL1
	.word	0x6
	.byte	0x68
	.byte	0x93
	.uleb128 0x1
	.byte	0x69
	.byte	0x93
	.uleb128 0x1
	.long	0x0
	.long	0x0
	.section	.debug_info
	.long	0x33a
	.word	0x2
	.long	.Ldebug_abbrev0
	.byte	0x4
	.uleb128 0x1
	.long	.LASF41
	.byte	0x1
	.long	.LASF42
	.long	.LASF43
	.long	0x0
	.long	0x0
	.long	.Ldebug_ranges0+0x0
	.long	.Ldebug_line0
	.uleb128 0x2
	.byte	0x1
	.byte	0x6
	.long	.LASF0
	.uleb128 0x3
	.long	.LASF2
	.byte	0x2
	.byte	0x7a
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
	.byte	0x2
	.byte	0x7c
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
	.byte	0x2
	.byte	0x7e
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
	.uleb128 0x5
	.byte	0x1
	.byte	0x8
	.uleb128 0x5
	.byte	0x2
	.byte	0x7
	.uleb128 0x2
	.byte	0x1
	.byte	0x2
	.long	.LASF10
	.uleb128 0x2
	.byte	0x1
	.byte	0x8
	.long	.LASF11
	.uleb128 0x6
	.byte	0x1f
	.byte	0x3
	.word	0x107
	.long	0x109
	.uleb128 0x7
	.long	.LASF12
	.byte	0x3
	.word	0x108
	.long	0x62
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x8
	.string	"Tag"
	.byte	0x3
	.word	0x109
	.long	0x62
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.uleb128 0x7
	.long	.LASF13
	.byte	0x3
	.word	0x10a
	.long	0x62
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x7
	.long	.LASF14
	.byte	0x3
	.word	0x10b
	.long	0x30
	.byte	0x2
	.byte	0x23
	.uleb128 0xc
	.uleb128 0x8
	.string	"LUN"
	.byte	0x3
	.word	0x10c
	.long	0x30
	.byte	0x2
	.byte	0x23
	.uleb128 0xd
	.uleb128 0x7
	.long	.LASF15
	.byte	0x3
	.word	0x10d
	.long	0x30
	.byte	0x2
	.byte	0x23
	.uleb128 0xe
	.uleb128 0x7
	.long	.LASF16
	.byte	0x3
	.word	0x10e
	.long	0x109
	.byte	0x2
	.byte	0x23
	.uleb128 0xf
	.byte	0x0
	.uleb128 0x9
	.long	0x30
	.long	0x119
	.uleb128 0xa
	.long	0x85
	.byte	0xf
	.byte	0x0
	.uleb128 0xb
	.long	.LASF17
	.byte	0x3
	.word	0x10f
	.long	0x96
	.uleb128 0x6
	.byte	0xd
	.byte	0x3
	.word	0x116
	.long	0x16b
	.uleb128 0x7
	.long	.LASF12
	.byte	0x3
	.word	0x117
	.long	0x62
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x8
	.string	"Tag"
	.byte	0x3
	.word	0x118
	.long	0x62
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.uleb128 0x7
	.long	.LASF18
	.byte	0x3
	.word	0x119
	.long	0x62
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x7
	.long	.LASF19
	.byte	0x3
	.word	0x11a
	.long	0x30
	.byte	0x2
	.byte	0x23
	.uleb128 0xc
	.byte	0x0
	.uleb128 0xb
	.long	.LASF20
	.byte	0x3
	.word	0x11b
	.long	0x125
	.uleb128 0xc
	.byte	0xa
	.byte	0x4
	.byte	0x57
	.long	0x1f0
	.uleb128 0xd
	.long	.LASF21
	.byte	0x4
	.byte	0x58
	.long	0x30
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0xd
	.long	.LASF22
	.byte	0x4
	.byte	0x5a
	.long	0x30
	.byte	0x2
	.byte	0x23
	.uleb128 0x1
	.uleb128 0xd
	.long	.LASF23
	.byte	0x4
	.byte	0x5b
	.long	0x49
	.byte	0x2
	.byte	0x23
	.uleb128 0x2
	.uleb128 0xd
	.long	.LASF24
	.byte	0x4
	.byte	0x5c
	.long	0x88
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.uleb128 0xd
	.long	.LASF25
	.byte	0x4
	.byte	0x5e
	.long	0x30
	.byte	0x2
	.byte	0x23
	.uleb128 0x5
	.uleb128 0xd
	.long	.LASF26
	.byte	0x4
	.byte	0x5f
	.long	0x49
	.byte	0x2
	.byte	0x23
	.uleb128 0x6
	.uleb128 0xd
	.long	.LASF27
	.byte	0x4
	.byte	0x60
	.long	0x88
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0xd
	.long	.LASF28
	.byte	0x4
	.byte	0x62
	.long	0x30
	.byte	0x2
	.byte	0x23
	.uleb128 0x9
	.byte	0x0
	.uleb128 0xc
	.byte	0x2d
	.byte	0x4
	.byte	0x67
	.long	0x223
	.uleb128 0xd
	.long	.LASF29
	.byte	0x4
	.byte	0x68
	.long	0x119
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0xd
	.long	.LASF30
	.byte	0x4
	.byte	0x6b
	.long	0x16b
	.byte	0x2
	.byte	0x23
	.uleb128 0x1f
	.uleb128 0xd
	.long	.LASF31
	.byte	0x4
	.byte	0x6e
	.long	0x223
	.byte	0x2
	.byte	0x23
	.uleb128 0x2c
	.byte	0x0
	.uleb128 0xe
	.long	0x88
	.uleb128 0xc
	.byte	0x37
	.byte	0x4
	.byte	0x55
	.long	0x24d
	.uleb128 0xd
	.long	.LASF32
	.byte	0x4
	.byte	0x63
	.long	0x24d
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0xd
	.long	.LASF33
	.byte	0x4
	.byte	0x71
	.long	0x1f0
	.byte	0x2
	.byte	0x23
	.uleb128 0xa
	.byte	0x0
	.uleb128 0xf
	.long	0x177
	.uleb128 0x3
	.long	.LASF34
	.byte	0x4
	.byte	0x74
	.long	0x228
	.uleb128 0x10
	.byte	0x1
	.long	.LASF35
	.byte	0x1
	.byte	0x5f
	.byte	0x1
	.long	.LFB90
	.long	.LFE90
	.byte	0x2
	.byte	0x90
	.uleb128 0x20
	.uleb128 0x10
	.byte	0x1
	.long	.LASF36
	.byte	0x1
	.byte	0x65
	.byte	0x1
	.long	.LFB91
	.long	.LFE91
	.byte	0x2
	.byte	0x90
	.uleb128 0x20
	.uleb128 0x11
	.byte	0x1
	.long	.LASF44
	.byte	0x1
	.byte	0x7c
	.byte	0x1
	.long	0x88
	.long	.LFB94
	.long	.LFE94
	.byte	0x2
	.byte	0x90
	.uleb128 0x20
	.long	0x2bc
	.uleb128 0x12
	.long	.LASF45
	.byte	0x1
	.byte	0x7b
	.long	0x2bc
	.long	.LLST3
	.uleb128 0x13
	.long	.LASF38
	.byte	0x1
	.byte	0x7d
	.long	0x88
	.byte	0x0
	.uleb128 0xf
	.long	0x2c1
	.uleb128 0x14
	.byte	0x2
	.long	0x252
	.uleb128 0x10
	.byte	0x1
	.long	.LASF37
	.byte	0x1
	.byte	0x73
	.byte	0x1
	.long	.LFB93
	.long	.LFE93
	.byte	0x2
	.byte	0x90
	.uleb128 0x20
	.uleb128 0x15
	.byte	0x1
	.long	.LASF46
	.byte	0x1
	.byte	0x6b
	.byte	0x1
	.long	.LFB92
	.long	.LFE92
	.byte	0x2
	.byte	0x90
	.uleb128 0x20
	.long	0x2ff
	.uleb128 0x13
	.long	.LASF39
	.byte	0x1
	.byte	0x6c
	.long	0x88
	.byte	0x0
	.uleb128 0x10
	.byte	0x1
	.long	.LASF40
	.byte	0x1
	.byte	0x4f
	.byte	0x1
	.long	.LFB89
	.long	.LFE89
	.byte	0x2
	.byte	0x90
	.uleb128 0x20
	.uleb128 0x16
	.byte	0x1
	.long	.LASF47
	.byte	0x1
	.byte	0x41
	.byte	0x1
	.long	0x42
	.long	.LFB88
	.long	.LFE88
	.byte	0x2
	.byte	0x90
	.uleb128 0x20
	.uleb128 0x17
	.long	.LASF48
	.byte	0x1
	.byte	0x2b
	.long	0x252
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.long	Disk_MS_Interface
	.byte	0x0
	.section	.debug_abbrev
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
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x52
	.uleb128 0x1
	.uleb128 0x55
	.uleb128 0x6
	.uleb128 0x10
	.uleb128 0x6
	.byte	0x0
	.byte	0x0
	.uleb128 0x2
	.uleb128 0x24
	.byte	0x0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.byte	0x0
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x16
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x4
	.uleb128 0x24
	.byte	0x0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0x8
	.byte	0x0
	.byte	0x0
	.uleb128 0x5
	.uleb128 0x24
	.byte	0x0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.byte	0x0
	.byte	0x0
	.uleb128 0x6
	.uleb128 0x13
	.byte	0x1
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x1
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x7
	.uleb128 0xd
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x8
	.uleb128 0xd
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x9
	.uleb128 0x1
	.byte	0x1
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x1
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0xa
	.uleb128 0x21
	.byte	0x0
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2f
	.uleb128 0xb
	.byte	0x0
	.byte	0x0
	.uleb128 0xb
	.uleb128 0x16
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0xc
	.uleb128 0x13
	.byte	0x1
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0xd
	.uleb128 0xd
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0xe
	.uleb128 0x35
	.byte	0x0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0xf
	.uleb128 0x26
	.byte	0x0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x10
	.uleb128 0x2e
	.byte	0x0
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x11
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
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0xa
	.uleb128 0x1
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x12
	.uleb128 0x5
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x6
	.byte	0x0
	.byte	0x0
	.uleb128 0x13
	.uleb128 0x34
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x14
	.uleb128 0xf
	.byte	0x0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
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
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0xa
	.uleb128 0x1
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x16
	.uleb128 0x2e
	.byte	0x0
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x17
	.uleb128 0x34
	.byte	0x0
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
	.uleb128 0x2
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.byte	0x0
	.section	.debug_pubnames,"",@progbits
	.long	0xf5
	.word	0x2
	.long	.Ldebug_info0
	.long	0x33e
	.long	0x25d
	.string	"EVENT_USB_Device_Connect"
	.long	0x271
	.string	"EVENT_USB_Device_Disconnect"
	.long	0x285
	.string	"CALLBACK_MS_Device_SCSICommandReceived"
	.long	0x2c7
	.string	"EVENT_USB_Device_ControlRequest"
	.long	0x2db
	.string	"EVENT_USB_Device_ConfigurationChanged"
	.long	0x2ff
	.string	"SetupHardware"
	.long	0x313
	.string	"main"
	.long	0x32b
	.string	"Disk_MS_Interface"
	.long	0x0
	.section	.debug_aranges,"",@progbits
	.long	0x4c
	.word	0x2
	.long	.Ldebug_info0
	.byte	0x4
	.byte	0x0
	.word	0x0
	.word	0x0
	.long	.LFB90
	.long	.LFE90-.LFB90
	.long	.LFB91
	.long	.LFE91-.LFB91
	.long	.LFB94
	.long	.LFE94-.LFB94
	.long	.LFB93
	.long	.LFE93-.LFB93
	.long	.LFB92
	.long	.LFE92-.LFB92
	.long	.LFB89
	.long	.LFE89-.LFB89
	.long	.LFB88
	.long	.LFE88-.LFB88
	.long	0x0
	.long	0x0
	.section	.debug_ranges,"",@progbits
.Ldebug_ranges0:
	.long	.Ltext0
	.long	.Letext0
	.long	.LFB90
	.long	.LFE90
	.long	.LFB91
	.long	.LFE91
	.long	.LFB94
	.long	.LFE94
	.long	.LFB93
	.long	.LFE93
	.long	.LFB92
	.long	.LFE92
	.long	.LFB89
	.long	.LFE89
	.long	.LFB88
	.long	.LFE88
	.long	0x0
	.long	0x0
	.section	.debug_line
	.long	.LELT0-.LSLT0
.LSLT0:
	.word	0x2
	.long	.LELTP0-.LASLTP0
.LASLTP0:
	.byte	0x1
	.byte	0x1
	.byte	0xf6
	.byte	0xf5
	.byte	0xa
	.byte	0x0
	.byte	0x1
	.byte	0x1
	.byte	0x1
	.byte	0x1
	.byte	0x0
	.byte	0x0
	.byte	0x0
	.byte	0x1
	.ascii	"../../LUFA101122/LUFA/Drivers/USB/Class/Device"
	.byte	0
	.ascii	"../../LUFA101122/LUFA/Drivers/USB/Class/Device/../Common"
	.byte	0
	.ascii	"c:/winavr-20100110/lib/gcc/../../avr/include"
	.byte	0
	.byte	0x0
	.string	"MassStorage.c"
	.uleb128 0x0
	.uleb128 0x0
	.uleb128 0x0
	.string	"stdint.h"
	.uleb128 0x3
	.uleb128 0x0
	.uleb128 0x0
	.string	"../Common/MassStorage.h"
	.uleb128 0x1
	.uleb128 0x0
	.uleb128 0x0
	.string	"MassStorage.h"
	.uleb128 0x1
	.uleb128 0x0
	.uleb128 0x0
	.byte	0x0
.LELTP0:
	.byte	0x0
	.uleb128 0x5
	.byte	0x2
	.long	.Letext0
	.byte	0x0
	.uleb128 0x1
	.byte	0x1
	.byte	0x0
	.uleb128 0x5
	.byte	0x2
	.long	.LSM0
	.byte	0x72
	.byte	0x0
	.uleb128 0x5
	.byte	0x2
	.long	.LSM1
	.byte	0x16
	.byte	0x0
	.uleb128 0x5
	.byte	0x2
	.long	.LFE90
	.byte	0x0
	.uleb128 0x1
	.byte	0x1
	.byte	0x0
	.uleb128 0x5
	.byte	0x2
	.long	.LSM2
	.byte	0x78
	.byte	0x0
	.uleb128 0x5
	.byte	0x2
	.long	.LSM3
	.byte	0x16
	.byte	0x0
	.uleb128 0x5
	.byte	0x2
	.long	.LFE91
	.byte	0x0
	.uleb128 0x1
	.byte	0x1
	.byte	0x0
	.uleb128 0x5
	.byte	0x2
	.long	.LSM4
	.byte	0x8f
	.byte	0x0
	.uleb128 0x5
	.byte	0x2
	.long	.LSM5
	.byte	0x17
	.byte	0x0
	.uleb128 0x5
	.byte	0x2
	.long	.LSM6
	.byte	0x17
	.byte	0x0
	.uleb128 0x5
	.byte	0x2
	.long	.LFE94
	.byte	0x0
	.uleb128 0x1
	.byte	0x1
	.byte	0x0
	.uleb128 0x5
	.byte	0x2
	.long	.LSM7
	.byte	0x86
	.byte	0x0
	.uleb128 0x5
	.byte	0x2
	.long	.LSM8
	.byte	0x15
	.byte	0x0
	.uleb128 0x5
	.byte	0x2
	.long	.LSM9
	.byte	0x15
	.byte	0x0
	.uleb128 0x5
	.byte	0x2
	.long	.LFE93
	.byte	0x0
	.uleb128 0x1
	.byte	0x1
	.byte	0x0
	.uleb128 0x5
	.byte	0x2
	.long	.LSM10
	.byte	0x7e
	.byte	0x0
	.uleb128 0x5
	.byte	0x2
	.long	.LSM11
	.byte	0x17
	.byte	0x0
	.uleb128 0x5
	.byte	0x2
	.long	.LSM12
	.byte	0x15
	.byte	0x0
	.uleb128 0x5
	.byte	0x2
	.long	.LFE92
	.byte	0x0
	.uleb128 0x1
	.byte	0x1
	.byte	0x0
	.uleb128 0x5
	.byte	0x2
	.long	.LSM13
	.byte	0x62
	.byte	0x0
	.uleb128 0x5
	.byte	0x2
	.long	.LSM14
	.byte	0x16
	.byte	0x0
	.uleb128 0x5
	.byte	0x2
	.long	.LSM15
	.byte	0x15
	.byte	0x0
	.uleb128 0x5
	.byte	0x2
	.long	.LSM16
	.byte	0x1c
	.byte	0x0
	.uleb128 0x5
	.byte	0x2
	.long	.LSM17
	.byte	0x15
	.byte	0x0
	.uleb128 0x5
	.byte	0x2
	.long	.LFE89
	.byte	0x0
	.uleb128 0x1
	.byte	0x1
	.byte	0x0
	.uleb128 0x5
	.byte	0x2
	.long	.LSM18
	.byte	0x54
	.byte	0x0
	.uleb128 0x5
	.byte	0x2
	.long	.LSM19
	.byte	0x15
	.byte	0x0
	.uleb128 0x5
	.byte	0x2
	.long	.LSM20
	.byte	0x16
	.byte	0x0
	.uleb128 0x5
	.byte	0x2
	.long	.LSM21
	.byte	0x18
	.byte	0x0
	.uleb128 0x5
	.byte	0x2
	.long	.LSM22
	.byte	0x15
	.byte	0x0
	.uleb128 0x5
	.byte	0x2
	.long	.LFE88
	.byte	0x0
	.uleb128 0x1
	.byte	0x1
.LELT0:
	.section	.debug_str,"MS",@progbits,1
.LASF18:
	.string	"DataTransferResidue"
.LASF41:
	.string	"GNU C 4.3.3"
.LASF30:
	.string	"CommandStatus"
.LASF31:
	.string	"IsMassStoreReset"
.LASF38:
	.string	"CommandSuccess"
.LASF19:
	.string	"Status"
.LASF40:
	.string	"SetupHardware"
.LASF48:
	.string	"Disk_MS_Interface"
.LASF46:
	.string	"EVENT_USB_Device_ConfigurationChanged"
.LASF21:
	.string	"InterfaceNumber"
.LASF28:
	.string	"TotalLUNs"
.LASF39:
	.string	"ConfigSuccess"
.LASF36:
	.string	"EVENT_USB_Device_Disconnect"
.LASF32:
	.string	"Config"
.LASF29:
	.string	"CommandBlock"
.LASF33:
	.string	"State"
.LASF24:
	.string	"DataINEndpointDoubleBank"
.LASF1:
	.string	"unsigned char"
.LASF7:
	.string	"long unsigned int"
.LASF45:
	.string	"MSInterfaceInfo"
.LASF23:
	.string	"DataINEndpointSize"
.LASF16:
	.string	"SCSICommandData"
.LASF26:
	.string	"DataOUTEndpointSize"
.LASF47:
	.string	"main"
.LASF4:
	.string	"unsigned int"
.LASF25:
	.string	"DataOUTEndpointNumber"
.LASF15:
	.string	"SCSICommandLength"
.LASF9:
	.string	"long long unsigned int"
.LASF2:
	.string	"uint8_t"
.LASF22:
	.string	"DataINEndpointNumber"
.LASF35:
	.string	"EVENT_USB_Device_Connect"
.LASF17:
	.string	"MS_CommandBlockWrapper_t"
.LASF8:
	.string	"long long int"
.LASF12:
	.string	"Signature"
.LASF11:
	.string	"char"
.LASF37:
	.string	"EVENT_USB_Device_ControlRequest"
.LASF14:
	.string	"Flags"
.LASF27:
	.string	"DataOUTEndpointDoubleBank"
.LASF43:
	.string	"Z:\\josh_on_mac\\dev\\OpenPCR\\usb"
.LASF44:
	.string	"CALLBACK_MS_Device_SCSICommandReceived"
.LASF3:
	.string	"uint16_t"
.LASF13:
	.string	"DataTransferLength"
.LASF6:
	.string	"uint32_t"
.LASF5:
	.string	"long int"
.LASF0:
	.string	"signed char"
.LASF34:
	.string	"USB_ClassInfo_MS_Device_t"
.LASF10:
	.string	"_Bool"
.LASF20:
	.string	"MS_CommandStatusWrapper_t"
.LASF42:
	.string	"MassStorage.c"
.global __do_copy_data
