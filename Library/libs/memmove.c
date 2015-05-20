/* From dLibs 1.20 but ANSIfied */

void *memmove(void *dest, const void *source, size_t len)
{
	uint8_t *dp = dest;
	uint8_t *sp = src;
	
	if (sp < dp) {
		dp += len;
		sp += len;
		while(len--)
			*--dp = *--sp;
	} else {
		while(len--)
			*dp++ = *sp++;
	}
	return dest;
}
