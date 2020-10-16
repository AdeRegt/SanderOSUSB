
	format	ELF
	public	_start
	extrn probeer
	
section '.text' executable
	
_start:
	call probeer
	ret
