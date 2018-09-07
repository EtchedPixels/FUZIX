
all: mk_scale mk_font mk_ascii_key_h

mk_font: mk_font.c
	$(CC) -o mk_font mk_font.c

mk_scale: mk_scale.c
	$(CC) -lm -o mk_scale mk_scale.c

mk_ascii_key_h: mk_ascii_key_h.c
	$(CC) -o mk_ascii_key_h mk_ascii_key_h.c

clean:
	rm -f *~ mk_scale mk_font mk_ascii_key_h
