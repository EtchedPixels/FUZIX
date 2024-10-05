#if MANGLED == 1
	#define panic fpanic
	#define _read f_read
	#define _write f_write
	#define _sbrk f_sbrk
#else
	#undef panic
	#undef _read
	#undef _write
	#undef _sbrk
#endif

#undef MANGLED

