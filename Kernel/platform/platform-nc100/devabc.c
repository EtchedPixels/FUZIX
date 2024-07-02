
static uint16_t hz_to_reg(uint16_t hz)
{
	/* The given equation is freq = 1000000/(rate * 2 * 1.6276) */
	/* ie rate = 307201/freq */
	return 307201UL/hz;
}

void plt_cht_op(uint8_t c)
{
        uint8_t chan = c & 0x01;
        c & = 0xF0;
        if (c == AUDB_FREQ) {
            out(0x50 + 2 * c, *cht_ptr++);
            out(0x51 + 2 * c, *cht_ptr++);
            return;
        } else if (c == AUDB_ENVELOPE) {
            cht->envelope = *cht_ptr++;
            return;
        } else if (c == AUDB_NOISEFREQ || c == AUDB_PULSE)
            cht_ptr++;
        cht_ptr++;
    }
}

void plt_do_envelope(uint8_t c, struct channel *ch)
{
    uint16_t f = hz_to_reg(ch->freq);
    if (ch->vol >= 0x10) {
        out(0x50 + 2 * c, f >> 8);
        out(0x51 + 2 * c, f);
    } else
        out(0x51 + 2 * c, 0xFF);
}
