
struct acia {
	uint8_t ctrl;
	uint8_t pad;
	uint8_t data;
};

struct via6522 {
	uint8_t rb;
	uint8_t pad0;
	uint8_t ra;
	uint8_t pad1;
	uint8_t ddrb;
	uint8_t pad2;
	uint8_t ddra;
	uint8_t pad3;
	uint8_t t1c_l;
	uint8_t pad4;
	uint8_t t1c_h;
	uint8_t pad5;
	uint8_t t1l_l;
	uint8_t pad6;
	uint8_t t1l_h;
	uint8_t pad7;
	uint8_t t2c_l;
	uint8_t pad8;
	uint8_t t2c_h;
	uint8_t pad9;
	uint8_t sr;
	uint8_t pad10;
	uint8_t acr;
	uint8_t pad11;
	uint8_t pcr;
	uint8_t pad12;
	uint8_t ifr;
	uint8_t pad13;
	uint8_t ier;
	uint8_t pad14;
	uint8_t ra_nh;
	uint8_t pad15;
};

static volatile struct acia *const acia = (struct acia *) 0x30001;
static volatile struct via6522 *const via = (struct via6522 *) 0x30000;
