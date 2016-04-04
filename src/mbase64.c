/* PUBLIC DOMAIN - Jon Mayo - November 13, 2003 */
#include "reef.h"

static const char cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int mb64_encode(const unsigned char *in, size_t in_len, char *out, size_t out_len)
{
	unsigned ii, io;
	uint_least32_t v;
	unsigned rem;

	for (io=0, ii=0, v=0, rem=0; ii < in_len; ii++) {
		unsigned char ch;
		ch = in[ii];
		v = (v<<8) | ch;
		rem += 8;
		while (rem >= 6) {
			rem -= 6;
			if ( io >= out_len) return -1; /* truncation is failure */
			out[io++] = cb64[(v>>rem)&63];
		}
	}

	if (rem) {
		v <<= (6-rem);
		if (io >= out_len) return -1; /* truncation is failure */
		out[io++] = cb64[v&63];
	}

	while (io & 3) {
		if (io >= out_len) return -1; /* truncation is failure */
		out[io++] = '=';
	}

	if (io >= out_len) return -1; /* no room for null terminator */

	out[io] = 0;

	return io;
}



static const uint8_t cd64[256]= {
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255, 62,255,255,255, 63,
	 52, 53, 54, 55, 56, 57, 58, 59, 60, 61,255,255,255,  0,255,255,
	255,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
	 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,255,255,255,255,255,
	255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
	 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
};

int mb64_decode(const char *in, size_t in_len, unsigned char *out, size_t out_len)
{
	unsigned int ii, io;
	uint32_t v;
	unsigned int rem;

	for (io=0, ii=0, v=0, rem=0; ii<in_len; ii++) {
		unsigned char ch;
		if (isspace((int)in[ii])) continue;
		if (in[ii]=='=') break; /* stop at = */

		ch = cd64[(unsigned int)in[ii]];

		if (ch == 255) break; /* stop at a parse error */

		v = (v<<6)|ch;
		rem += 6;

		if (rem >= 8) {
			rem -= 8;
			if (io >= out_len) return -1; /* truncation is failure */
			out[io++] = (v>>rem)&255;
		}
	}

	if (rem >= 8) {
		rem -= 8;
		if (io >= out_len) return -1; /* truncation is failure */
		out[io++] = (v>>rem)&255;
	}

	return io;
}
