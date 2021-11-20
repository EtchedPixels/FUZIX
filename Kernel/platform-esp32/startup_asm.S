/* Must be the first instruction in the image; jumps over any header or literal
 * pool to the startup code. */

	.text
	.globl __startup
__startup:
	j startup

