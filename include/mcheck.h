#ifndef __MCHECK_H__
#define __MCHECK_H__

/*
 * mcheck, communication verify algorithm(crc, checksum...; md5, sha serial defined in mhash.h)
 */
__BEGIN_DECLS

uint16_t mcrc16(const uint8_t *data, uint16_t len);
uint32_t mcrc32(const uint8_t *data, uint16_t len);
uint8_t  mchecksum(const uint8_t *data, size_t len);

__END_DECLS
#endif  /* __MCHECK_H__ */
