		;in: edi=4k buffer
		;out: buffer filled with font
		;clear even/odd mode
global video_load_font
extern _fontbuffer
video_load_font:
		mov 			edi, _fontbuffer
		mov			dx, 03ceh
		mov			ax, 5
		out			dx, ax
		;map VGA memory to 0A0000h
		mov			ax, 0406h
		out			dx, ax
		;set bitplane 2
		mov			dx, 03c4h
		mov			ax, 0402h
		out			dx, ax
		;clear even/odd mode (the other way, don't ask why)
		mov			ax, 0604h
		out			dx, ax
		;copy charmap
		mov			esi, 0A0000h
		mov			ecx, 256
		;copy 16 bytes to bitmap
.loopie:		
		movsd
		movsd
		movsd
		movsd
		;skip another 16 bytes
		add			esi, 16
		loop			.loopie
		;restore VGA state to normal operation
		mov			ax, 0302h
		out			dx, ax
		mov			ax, 0204h
		out			dx, ax
		mov			dx, 03ceh
		mov			ax, 1005h
		out			dx, ax
		mov			ax, 0E06h
		out			dx, ax
		ret
