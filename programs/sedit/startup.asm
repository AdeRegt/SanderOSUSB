
	format	ELF
	public	_start
	extrn main
	
section '.text' executable
	
_start:
	call main
	ret
