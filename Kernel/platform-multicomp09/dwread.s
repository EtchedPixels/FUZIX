*******************************************************
*
* Derived from HDB-DOS from toolshed.sf.net
* The original code is public domain
*
* DWRead
*    Receive a response from the DriveWire server.
*    Times out if serial port goes idle for more than 1.4 (0.7) seconds.
*    Serial data format:  1-8-N-1
*    28Jul2106 by Neal Crook for Multicomp UART
*
* Entry:
*    X  = starting address where data is to be stored
*    Y  = number of bytes expected
*
* Exit:
*    CC = Carry set on framing error, Z set if all bytes received
*    X  = starting address of data received
*    Y  = checksum
*    U is preserved.  All accumulators are clobbered
*
* [NAC HACK 2016Jul29] assume: timeout indicated by C=0, Z=0
*
          include "platform.def"        ; makes ports available to dwread, dwwrite

*******************************************************
* 57600 (115200) bps using 6809 code and hw UART
*******************************************************

DWRead    clra                          ; clear Carry (no framing error)
          deca                          ; clear Z flag, A = timeout msb ($ff)
          tfr       cc,b
          pshs      u,x,b,a     	; preserve registers, push timeout msb

* stack now looks like this:
* PCL PCH UL UH XL XH B A
* at exit, A will be discarded. B is a "clean" version of CC
* (!Z and !C) and will be popped into CC.

	  IFEQ      NOINTMASK
          orcc      #IntMasks           ; mask interrupts
          ENDC
          leau      ,x                  ; U = storage ptr
          ldx       #0                  ; initialize checksum

* initialise timeout
rxNext    ldb       #$ff                ; init timeout LSB
	  stb	    ,s			; init timeout MSB - don't need
					; this 1st time around.

* character available?
rxAvail	  lda	    UARTSTA2
	  bita	    #1
	  bne	    rxGet

* no. Decrement timeout
          subb      #1                  ; decrement timeout lsb
          bcc       rxAvail             ; loop until timeout lsb rolls under
          addb      ,s                  ; B = timeout msb - 1
					; leaves CC.C=0 on timeout
          stb       ,s                  ; store decremented timeout msb
	  bcc	    rxTimout		; oops!
          ldb       #$ff                ; reload timeout LSB
	  bra	    rxAvail		; test again..

* yes. Get it and move on
rxGet	  ldb	    UARTDAT2
	  abx				; accummulate checksum
	  stb	    ,u+			; store byte
	  leay	    ,-y			; decrement count
	  bne	    rxNext
	  lda	    #4			; represents CC.Z=1
					; to indicate all bytes rx'ed

* clean up, set status and return
rxExit	  leas      1,s                 ; remove timeout MSB from stack
          ora       ,s                  ; place status information into the..
          sta       ,s                  ; ..C and Z bits of the preserved CC
          leay      ,x                  ; return checksum in Y
          puls      cc,x,u,pc		; restore registers and return

rxTimout  clra				; represents CC.C=0, CC.Z=0
	  bra	    rxExit
