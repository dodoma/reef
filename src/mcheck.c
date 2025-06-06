#include "reef.h"

// crc16 table
const uint16_t crc_table_mcrf4xx[1024] = {
	0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
	0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
	0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
	0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
	0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
	0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
	0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
	0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
	0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
	0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
	0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
	0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
	0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
	0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
	0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
	0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
	0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
	0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
	0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
	0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
	0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
	0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
	0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
	0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
	0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
	0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
	0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
	0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
	0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
	0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
	0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
	0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78,
	0x0000, 0x19d8, 0x33b0, 0x2a68, 0x6760, 0x7eb8, 0x54d0, 0x4d08,
	0xcec0, 0xd718, 0xfd70, 0xe4a8, 0xa9a0, 0xb078, 0x9a10, 0x83c8,
	0x9591, 0x8c49, 0xa621, 0xbff9, 0xf2f1, 0xeb29, 0xc141, 0xd899,
	0x5b51, 0x4289, 0x68e1, 0x7139, 0x3c31, 0x25e9, 0x0f81, 0x1659,
	0x2333, 0x3aeb, 0x1083, 0x095b, 0x4453, 0x5d8b, 0x77e3, 0x6e3b,
	0xedf3, 0xf42b, 0xde43, 0xc79b, 0x8a93, 0x934b, 0xb923, 0xa0fb,
	0xb6a2, 0xaf7a, 0x8512, 0x9cca, 0xd1c2, 0xc81a, 0xe272, 0xfbaa,
	0x7862, 0x61ba, 0x4bd2, 0x520a, 0x1f02, 0x06da, 0x2cb2, 0x356a,
	0x4666, 0x5fbe, 0x75d6, 0x6c0e, 0x2106, 0x38de, 0x12b6, 0x0b6e,
	0x88a6, 0x917e, 0xbb16, 0xa2ce, 0xefc6, 0xf61e, 0xdc76, 0xc5ae,
	0xd3f7, 0xca2f, 0xe047, 0xf99f, 0xb497, 0xad4f, 0x8727, 0x9eff,
	0x1d37, 0x04ef, 0x2e87, 0x375f, 0x7a57, 0x638f, 0x49e7, 0x503f,
	0x6555, 0x7c8d, 0x56e5, 0x4f3d, 0x0235, 0x1bed, 0x3185, 0x285d,
	0xab95, 0xb24d, 0x9825, 0x81fd, 0xccf5, 0xd52d, 0xff45, 0xe69d,
	0xf0c4, 0xe91c, 0xc374, 0xdaac, 0x97a4, 0x8e7c, 0xa414, 0xbdcc,
	0x3e04, 0x27dc, 0x0db4, 0x146c, 0x5964, 0x40bc, 0x6ad4, 0x730c,
	0x8ccc, 0x9514, 0xbf7c, 0xa6a4, 0xebac, 0xf274, 0xd81c, 0xc1c4,
	0x420c, 0x5bd4, 0x71bc, 0x6864, 0x256c, 0x3cb4, 0x16dc, 0x0f04,
	0x195d, 0x0085, 0x2aed, 0x3335, 0x7e3d, 0x67e5, 0x4d8d, 0x5455,
	0xd79d, 0xce45, 0xe42d, 0xfdf5, 0xb0fd, 0xa925, 0x834d, 0x9a95,
	0xafff, 0xb627, 0x9c4f, 0x8597, 0xc89f, 0xd147, 0xfb2f, 0xe2f7,
	0x613f, 0x78e7, 0x528f, 0x4b57, 0x065f, 0x1f87, 0x35ef, 0x2c37,
	0x3a6e, 0x23b6, 0x09de, 0x1006, 0x5d0e, 0x44d6, 0x6ebe, 0x7766,
	0xf4ae, 0xed76, 0xc71e, 0xdec6, 0x93ce, 0x8a16, 0xa07e, 0xb9a6,
	0xcaaa, 0xd372, 0xf91a, 0xe0c2, 0xadca, 0xb412, 0x9e7a, 0x87a2,
	0x046a, 0x1db2, 0x37da, 0x2e02, 0x630a, 0x7ad2, 0x50ba, 0x4962,
	0x5f3b, 0x46e3, 0x6c8b, 0x7553, 0x385b, 0x2183, 0x0beb, 0x1233,
	0x91fb, 0x8823, 0xa24b, 0xbb93, 0xf69b, 0xef43, 0xc52b, 0xdcf3,
	0xe999, 0xf041, 0xda29, 0xc3f1, 0x8ef9, 0x9721, 0xbd49, 0xa491,
	0x2759, 0x3e81, 0x14e9, 0x0d31, 0x4039, 0x59e1, 0x7389, 0x6a51,
	0x7c08, 0x65d0, 0x4fb8, 0x5660, 0x1b68, 0x02b0, 0x28d8, 0x3100,
	0xb2c8, 0xab10, 0x8178, 0x98a0, 0xd5a8, 0xcc70, 0xe618, 0xffc0,
	0x0000, 0x5adc, 0xb5b8, 0xef64, 0x6361, 0x39bd, 0xd6d9, 0x8c05,
	0xc6c2, 0x9c1e, 0x737a, 0x29a6, 0xa5a3, 0xff7f, 0x101b, 0x4ac7,
	0x8595, 0xdf49, 0x302d, 0x6af1, 0xe6f4, 0xbc28, 0x534c, 0x0990,
	0x4357, 0x198b, 0xf6ef, 0xac33, 0x2036, 0x7aea, 0x958e, 0xcf52,
	0x033b, 0x59e7, 0xb683, 0xec5f, 0x605a, 0x3a86, 0xd5e2, 0x8f3e,
	0xc5f9, 0x9f25, 0x7041, 0x2a9d, 0xa698, 0xfc44, 0x1320, 0x49fc,
	0x86ae, 0xdc72, 0x3316, 0x69ca, 0xe5cf, 0xbf13, 0x5077, 0x0aab,
	0x406c, 0x1ab0, 0xf5d4, 0xaf08, 0x230d, 0x79d1, 0x96b5, 0xcc69,
	0x0676, 0x5caa, 0xb3ce, 0xe912, 0x6517, 0x3fcb, 0xd0af, 0x8a73,
	0xc0b4, 0x9a68, 0x750c, 0x2fd0, 0xa3d5, 0xf909, 0x166d, 0x4cb1,
	0x83e3, 0xd93f, 0x365b, 0x6c87, 0xe082, 0xba5e, 0x553a, 0x0fe6,
	0x4521, 0x1ffd, 0xf099, 0xaa45, 0x2640, 0x7c9c, 0x93f8, 0xc924,
	0x054d, 0x5f91, 0xb0f5, 0xea29, 0x662c, 0x3cf0, 0xd394, 0x8948,
	0xc38f, 0x9953, 0x7637, 0x2ceb, 0xa0ee, 0xfa32, 0x1556, 0x4f8a,
	0x80d8, 0xda04, 0x3560, 0x6fbc, 0xe3b9, 0xb965, 0x5601, 0x0cdd,
	0x461a, 0x1cc6, 0xf3a2, 0xa97e, 0x257b, 0x7fa7, 0x90c3, 0xca1f,
	0x0cec, 0x5630, 0xb954, 0xe388, 0x6f8d, 0x3551, 0xda35, 0x80e9,
	0xca2e, 0x90f2, 0x7f96, 0x254a, 0xa94f, 0xf393, 0x1cf7, 0x462b,
	0x8979, 0xd3a5, 0x3cc1, 0x661d, 0xea18, 0xb0c4, 0x5fa0, 0x057c,
	0x4fbb, 0x1567, 0xfa03, 0xa0df, 0x2cda, 0x7606, 0x9962, 0xc3be,
	0x0fd7, 0x550b, 0xba6f, 0xe0b3, 0x6cb6, 0x366a, 0xd90e, 0x83d2,
	0xc915, 0x93c9, 0x7cad, 0x2671, 0xaa74, 0xf0a8, 0x1fcc, 0x4510,
	0x8a42, 0xd09e, 0x3ffa, 0x6526, 0xe923, 0xb3ff, 0x5c9b, 0x0647,
	0x4c80, 0x165c, 0xf938, 0xa3e4, 0x2fe1, 0x753d, 0x9a59, 0xc085,
	0x0a9a, 0x5046, 0xbf22, 0xe5fe, 0x69fb, 0x3327, 0xdc43, 0x869f,
	0xcc58, 0x9684, 0x79e0, 0x233c, 0xaf39, 0xf5e5, 0x1a81, 0x405d,
	0x8f0f, 0xd5d3, 0x3ab7, 0x606b, 0xec6e, 0xb6b2, 0x59d6, 0x030a,
	0x49cd, 0x1311, 0xfc75, 0xa6a9, 0x2aac, 0x7070, 0x9f14, 0xc5c8,
	0x09a1, 0x537d, 0xbc19, 0xe6c5, 0x6ac0, 0x301c, 0xdf78, 0x85a4,
	0xcf63, 0x95bf, 0x7adb, 0x2007, 0xac02, 0xf6de, 0x19ba, 0x4366,
	0x8c34, 0xd6e8, 0x398c, 0x6350, 0xef55, 0xb589, 0x5aed, 0x0031,
	0x4af6, 0x102a, 0xff4e, 0xa592, 0x2997, 0x734b, 0x9c2f, 0xc6f3,
	0x0000, 0x1cbb, 0x3976, 0x25cd, 0x72ec, 0x6e57, 0x4b9a, 0x5721,
	0xe5d8, 0xf963, 0xdcae, 0xc015, 0x9734, 0x8b8f, 0xae42, 0xb2f9,
	0xc3a1, 0xdf1a, 0xfad7, 0xe66c, 0xb14d, 0xadf6, 0x883b, 0x9480,
	0x2679, 0x3ac2, 0x1f0f, 0x03b4, 0x5495, 0x482e, 0x6de3, 0x7158,
	0x8f53, 0x93e8, 0xb625, 0xaa9e, 0xfdbf, 0xe104, 0xc4c9, 0xd872,
	0x6a8b, 0x7630, 0x53fd, 0x4f46, 0x1867, 0x04dc, 0x2111, 0x3daa,
	0x4cf2, 0x5049, 0x7584, 0x693f, 0x3e1e, 0x22a5, 0x0768, 0x1bd3,
	0xa92a, 0xb591, 0x905c, 0x8ce7, 0xdbc6, 0xc77d, 0xe2b0, 0xfe0b,
	0x16b7, 0x0a0c, 0x2fc1, 0x337a, 0x645b, 0x78e0, 0x5d2d, 0x4196,
	0xf36f, 0xefd4, 0xca19, 0xd6a2, 0x8183, 0x9d38, 0xb8f5, 0xa44e,
	0xd516, 0xc9ad, 0xec60, 0xf0db, 0xa7fa, 0xbb41, 0x9e8c, 0x8237,
	0x30ce, 0x2c75, 0x09b8, 0x1503, 0x4222, 0x5e99, 0x7b54, 0x67ef,
	0x99e4, 0x855f, 0xa092, 0xbc29, 0xeb08, 0xf7b3, 0xd27e, 0xcec5,
	0x7c3c, 0x6087, 0x454a, 0x59f1, 0x0ed0, 0x126b, 0x37a6, 0x2b1d,
	0x5a45, 0x46fe, 0x6333, 0x7f88, 0x28a9, 0x3412, 0x11df, 0x0d64,
	0xbf9d, 0xa326, 0x86eb, 0x9a50, 0xcd71, 0xd1ca, 0xf407, 0xe8bc,
	0x2d6e, 0x31d5, 0x1418, 0x08a3, 0x5f82, 0x4339, 0x66f4, 0x7a4f,
	0xc8b6, 0xd40d, 0xf1c0, 0xed7b, 0xba5a, 0xa6e1, 0x832c, 0x9f97,
	0xeecf, 0xf274, 0xd7b9, 0xcb02, 0x9c23, 0x8098, 0xa555, 0xb9ee,
	0x0b17, 0x17ac, 0x3261, 0x2eda, 0x79fb, 0x6540, 0x408d, 0x5c36,
	0xa23d, 0xbe86, 0x9b4b, 0x87f0, 0xd0d1, 0xcc6a, 0xe9a7, 0xf51c,
	0x47e5, 0x5b5e, 0x7e93, 0x6228, 0x3509, 0x29b2, 0x0c7f, 0x10c4,
	0x619c, 0x7d27, 0x58ea, 0x4451, 0x1370, 0x0fcb, 0x2a06, 0x36bd,
	0x8444, 0x98ff, 0xbd32, 0xa189, 0xf6a8, 0xea13, 0xcfde, 0xd365,
	0x3bd9, 0x2762, 0x02af, 0x1e14, 0x4935, 0x558e, 0x7043, 0x6cf8,
	0xde01, 0xc2ba, 0xe777, 0xfbcc, 0xaced, 0xb056, 0x959b, 0x8920,
	0xf878, 0xe4c3, 0xc10e, 0xddb5, 0x8a94, 0x962f, 0xb3e2, 0xaf59,
	0x1da0, 0x011b, 0x24d6, 0x386d, 0x6f4c, 0x73f7, 0x563a, 0x4a81,
	0xb48a, 0xa831, 0x8dfc, 0x9147, 0xc666, 0xdadd, 0xff10, 0xe3ab,
	0x5152, 0x4de9, 0x6824, 0x749f, 0x23be, 0x3f05, 0x1ac8, 0x0673,
	0x772b, 0x6b90, 0x4e5d, 0x52e6, 0x05c7, 0x197c, 0x3cb1, 0x200a,
	0x92f3, 0x8e48, 0xab85, 0xb73e, 0xe01f, 0xfca4, 0xd969, 0xc5d2
};

#define crc_n4(crc, data, table) crc ^= data;       \
	crc = (table[(crc & 0xff) + 0x300]) ^           \
        (table[((crc >> 8) & 0xff) + 0x200]) ^      \
        (table[((data >> 16) & 0xff) + 0x100]) ^	\
        (table[data >> 24]);

uint16_t mcrc16(const uint8_t *data, uint16_t len)
{
    uint16_t crc = 0x4c49;

	while (((uintptr_t)data & 3) && len) {
		crc = (crc >> 8) ^ crc_table_mcrf4xx[(crc & 0xff) ^ *data++];
		len--;
	}

	while (len >= 16) {
		len -= 16;
		crc_n4(crc, ((uint32_t *)data)[0], crc_table_mcrf4xx);
		crc_n4(crc, ((uint32_t *)data)[1], crc_table_mcrf4xx);
		crc_n4(crc, ((uint32_t *)data)[2], crc_table_mcrf4xx);
		crc_n4(crc, ((uint32_t *)data)[3], crc_table_mcrf4xx);
		data += 16;
	}

	while (len--) {
		crc = (crc >> 8) ^ crc_table_mcrf4xx[(crc & 0xff) ^ *data++];
	}

	return crc;
}

// crc32 table
const uint32_t crc_table_crc32[256] = {
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba,
	0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
	0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
	0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,
	0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
	0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940,
	0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116,
	0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
	0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
	0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a,
	0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818,
	0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
	0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
	0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c,
	0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
	0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
	0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
	0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086,
	0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4,
	0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
	0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
	0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
	0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe,
	0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
	0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
	0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252,
	0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60,
	0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
	0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
	0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04,
	0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a,
	0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
	0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
	0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e,
	0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
	0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
	0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
	0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0,
	0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6,
	0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
	0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

#define crcsm_n4d(crc, data, table) crc ^= data;    \
	crc = (crc >> 8) ^ (table[crc & 0xff]);         \
	crc = (crc >> 8) ^ (table[crc & 0xff]);         \
	crc = (crc >> 8) ^ (table[crc & 0xff]);         \
	crc = (crc >> 8) ^ (table[crc & 0xff]);

uint32_t mcrc32(const uint8_t *data, uint16_t len)
{
	uint32_t crc = 0x564f580a ^ 0xffffffff;

	while (((uintptr_t)data & 3) && len) {
		crc = (crc >> 8) ^ crc_table_crc32[(crc & 0xff) ^ *data++];
		len--;
	}

	while (len >= 16) {
		len -= 16;
		crcsm_n4d(crc, ((uint32_t *)data)[0], crc_table_crc32);
		crcsm_n4d(crc, ((uint32_t *)data)[1], crc_table_crc32);
		crcsm_n4d(crc, ((uint32_t *)data)[2], crc_table_crc32);
		crcsm_n4d(crc, ((uint32_t *)data)[3], crc_table_crc32);
		data += 16;
	}

	while (len--) {
		crc = (crc >> 8) ^ crc_table_crc32[(crc & 0xff) ^ *data++];
	}

	crc ^= 0xffffffff;

	return crc;
}

uint8_t mchecksum(const uint8_t *data, size_t len)
{
    uint8_t checksum = 0;

    while (len-- != 0) checksum += *data++;

    return checksum;
}
