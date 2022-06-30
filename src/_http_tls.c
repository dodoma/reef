#include "mbedtls/net_sockets.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/certs.h"

struct _tls {
    mbedtls_net_context      ssl_fd;
    mbedtls_entropy_context  entropy;
    mbedtls_ctr_drbg_context ctr_drbg;

    mbedtls_ssl_context      ssl;
    mbedtls_ssl_config       conf;
    mbedtls_x509_crt         cacert;
};

static void _debug(void *ctx, int level,
                   const char *file, int line, const char *str)
{
    mtc_dbg("%s:%04d: %s", file, line, str);
}

MERR* _tls_new(struct _tls **ssl, const char *hostname, int timeout)
{
    struct _tls *s = mos_calloc(1, sizeof(struct _tls));

    mbedtls_net_init(&s->ssl_fd);
    mbedtls_entropy_init(&s->entropy);
    mbedtls_ctr_drbg_init(&s->ctr_drbg);

    mbedtls_ssl_init(&s->ssl);
    mbedtls_ssl_config_init(&s->conf);
    mbedtls_x509_crt_init(&s->cacert);

    int ret = mbedtls_ctr_drbg_seed(&s->ctr_drbg, mbedtls_entropy_func, &s->entropy,
                                    (const unsigned char *)"reef", strlen("reef"));
    if (ret != 0) return merr_raise(MERR_ASSERT, "mbedtls_ctr_drbg_seed failure %d", ret);

    ret = mbedtls_x509_crt_parse(&s->cacert, (const unsigned char*)mbedtls_test_cas_pem, mbedtls_test_cas_pem_len);
    if (ret < 0) return merr_raise(MERR_ASSERT, "mbedtls_x509_crt_parse failure %d", ret);

    ret = mbedtls_ssl_config_defaults(&s->conf,
                                      MBEDTLS_SSL_IS_CLIENT,
                                      MBEDTLS_SSL_TRANSPORT_STREAM,
                                      MBEDTLS_SSL_PRESET_DEFAULT);
    if (ret != 0) return merr_raise(MERR_ASSERT, "mbedtls_ssl_config_defaults failure %d", ret);

    mbedtls_ssl_conf_authmode(&s->conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
    mbedtls_ssl_conf_ca_chain(&s->conf, &s->cacert, NULL);
    mbedtls_ssl_conf_rng(&s->conf, mbedtls_ctr_drbg_random, &s->ctr_drbg);
    mbedtls_ssl_conf_read_timeout(&s->conf, timeout);
    mbedtls_ssl_conf_dbg(&s->conf, _debug, NULL);

    ret = mbedtls_ssl_setup(&s->ssl, &s->conf);
    if (ret != 0) return merr_raise(MERR_ASSERT, "mbedtls_ssl_setup failure %d", ret);

    ret = mbedtls_ssl_set_hostname(&s->ssl, hostname);
    if (ret != 0) return merr_raise(MERR_ASSERT, "mbedtls_ssl_set_hostname %s %d", hostname, ret);

    *ssl = s;

    return MERR_OK;
}

void _tls_free(struct _tls *s)
{
    if (!s) return;

    mbedtls_x509_crt_free(&s->cacert);
    mbedtls_ssl_free(&s->ssl);
    mbedtls_ssl_config_free(&s->conf);
    mbedtls_ctr_drbg_free(&s->ctr_drbg);
    mbedtls_entropy_free(&s->entropy);

    mos_free(s);
}
