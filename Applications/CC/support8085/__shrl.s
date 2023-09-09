		.export __shrl
		.export __shrul

		.setcpu 8085
		.code

__shrl:
		mov	a,l		; shift amount
		pop	h
		shld	__retaddr
		pop	d
		pop	h		; shifting HLDE by A
		ani	31		; nothing to do ?
		jz	done

		push	b
		mov	b,a

shrlp:
		arhl
shrde:
		mov	a,d
		rar
		mov	d,a
		mov	a,e
		rar
		mov	e,a
		dcr	b
		jnz	shrlp
		pop	b
done:
		shld	__hireg
		xchg
		jmp	__ret


__shrul:
		mov	a,l
		pop	h
		shld	__retaddr
		pop	d
		pop	h
		ani	31
		jz	done
		push	b
		mov	b,a

		; Do the first half of the initial loop to force the top
		; new bit to 0. After that we can use the same loop and
		; any future optimizations
		mov	a,h
		ora	a
		rar
		mov	h,a
		mov	a,l
		rar
		mov	l,a
		jp	shrde
