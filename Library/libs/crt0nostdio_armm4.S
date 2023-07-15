/* Dummy a.out header - not quite right as the GNU linker is apparently
   not able to handle symbol differences as a relocation format  */
	.long 8192
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0
#undef WITH_STDIO
#include "crt0_template_armm0.def"


