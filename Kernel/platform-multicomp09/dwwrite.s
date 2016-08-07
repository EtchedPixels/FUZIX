*******************************************************
*
* Derived from HDB-DOS from toolshed.sf.net
* The original code is public domain
*
* DWWrite
*    Send a packet to the DriveWire server.
*    Serial data format:  1-8-N-1
*    28Jul2106 by Neal Crook for Multicomp UART
*
* Entry:
*    X  = starting address of data to send
*    Y  = number of bytes to send
*
* Exit:
*    X  = address of last byte sent + 1
*    Y  = 0
*    All others preserved
*

*******************************************************
* 57600 (115200) bps using 6809 code and hw UART
*******************************************************

DWWrite   pshs      cc,a				; preserve registers
          IFEQ      NOINTMASK
          orcc      #IntMasks				; mask interrupts
          ENDC

WrBiz     lda		UARTSTA2
          bita		#2
          beq		WrBiz				; busy

          lda		,x+				; get byte to transmit
          sta		UARTDAT2			; send byte

          leay		,-y				; decrement byte counter
          bne		WrBiz				; loop if more to send

          puls		cc,a,pc				; restore registers and return
