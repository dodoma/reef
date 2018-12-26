static Rune _canon(Rune c)
{
	Rune u = toupperrune(c);
	if (c >= 128 && u < 128)
		return c;
	return u;
}

static bool _isnewline(Rune c)
{
	return c == 0xA || c == 0xD || c == 0x2028 || c == 0x2029;
}

static bool _iswordchar(Rune c)
{
	return c == '_' ||	(c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
}

static bool _accept(MRE *reo, char c)
{
    if (*reo->pos == c) {
        _tok_next(reo, false);
        return true;
    } else return false;
}

static uint32_t _instruct_count(MRE *reo)
{
    return reo->bcode.len / INSTRUCT_LEN;
}

static Instruct* _pc_relative(MRE *reo, Instruct *pc, int32_t pos)
{
    Instruct *istart = (Instruct *)reo->bcode.buf;
    Instruct *iend = (Instruct *)(reo->bcode.buf + reo->bcode.len);

    pc = pc + pos;

    if (pc < istart || pc >= iend) DIE(reo, "instruct overflow!");

    return pc;
}

static Instruct* _pc_absolute(MRE *reo, int32_t pos)
{
    Instruct *istart = (Instruct *)reo->bcode.buf;
    Instruct *iend = (Instruct *)(reo->bcode.buf + reo->bcode.len);

    if ((pos >= 0 && pos > reo->bcode.len / INSTRUCT_LEN) ||
        (pos < 0 && -pos > reo->bcode.len / INSTRUCT_LEN))
        DIE(reo, "instruct overflow");

    return pos >= 0 ? istart + pos : iend + pos;
}

static bool _inrange(MLIST *rlist, Rune c, bool igcase)
{
    Rune *a, *b;
    Rune ra, rb;

    for (int i = 0; i < mlist_length(rlist); i += 2) {
        mlist_get(rlist, i, (void**)&a);
        mlist_get(rlist, i+1, (void**)&b);
	ra = (Rune)MOS_MEM_2_OFFSET(a);
	rb = (Rune)MOS_MEM_2_OFFSET(b);
        //ra = (Rune)((void*)a - (void*)NULL);
        //rb = (Rune)((void*)b - (void*)NULL);

        if (igcase) {
            for (Rune p = ra; p <= rb; ++p) {
                if (_canon(c) == _canon(p)) return true;
            }
        } else if (ra <= c && c <= rb) return true;
    }

    return false;
}

static int _strcompare(const char *a, const char *b, int i, bool igcase)
{
    Rune ra, rb;
    int c;

    if (igcase) {
        while (i--) {
            if (!*a) return -1;
            if (!*b) return 1;
            a += chartorune(&ra, a);
            b += chartorune(&rb, b);
            c = _canon(ra) - _canon(rb);
            if (c) return c;
        }
        return 0;
    } else return strncmp(a, b, i);
}

static void _addrange(MRE *reo, MLIST *rlist, Rune a, Rune b)
{
    if (a > b) {
        DIE(reo, "invalid character class range");
    }

    mlist_append(rlist, MOS_OFFSET_2_MEM(a));
    mlist_append(rlist, MOS_OFFSET_2_MEM(b));
}

static void _add_ranges_d(MRE *reo, MLIST *rlist)
{
    _addrange(reo, rlist, '0', '9');
}

static void _add_ranges_D(MRE *reo, MLIST *rlist)
{
	_addrange(reo, rlist, 0, '0'-1);
	_addrange(reo, rlist, '9'+1, 0xFFFF);
}

static void _add_ranges_s(MRE *reo, MLIST *rlist)
{
	_addrange(reo, rlist, 0x9, 0xD);
	_addrange(reo, rlist, 0x20, 0x20);
	_addrange(reo, rlist, 0xA0, 0xA0);
	_addrange(reo, rlist, 0x2028, 0x2029);
	_addrange(reo, rlist, 0xFEFF, 0xFEFF);
}

static void _add_ranges_S(MRE *reo, MLIST *rlist)
{
	_addrange(reo, rlist, 0, 0x9-1);
	_addrange(reo, rlist, 0xD+1, 0x20-1);
	_addrange(reo, rlist, 0x20+1, 0xA0-1);
	_addrange(reo, rlist, 0xA0+1, 0x2028-1);
	_addrange(reo, rlist, 0x2029+1, 0xFEFF-1);
	_addrange(reo, rlist, 0xFEFF+1, 0xFFFF);
}

static void _add_ranges_w(MRE *reo, MLIST *rlist)
{
	_addrange(reo, rlist, '0', '9');
	_addrange(reo, rlist, 'A', 'Z');
	_addrange(reo, rlist, '_', '_');
	_addrange(reo, rlist, 'a', 'z');
}

static void _add_ranges_W(MRE *reo, MLIST *rlist)
{
	_addrange(reo, rlist, 0, '0'-1);
	_addrange(reo, rlist, '9'+1, 'A'-1);
	_addrange(reo, rlist, 'Z'+1, '_'-1);
	_addrange(reo, rlist, '_'+1, 'a'-1);
	_addrange(reo, rlist, 'z'+1, 0xFFFF);
}
