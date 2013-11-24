#ifndef CRYPTOGRAPH_H
#define CRYPTOGRAPH_H

#include "cryptograph_global.h"

int CRYPTOGRAPHSHARED_EXPORT calc_sha256_hash(char *pbsource, int cbsource, char *pboutput, int *pcboutput);
int CRYPTOGRAPHSHARED_EXPORT encrypt_aes256_ecb(char *pbsource, int cbsource, char *pbkey, char *pboutput, int *pcboutput);
int CRYPTOGRAPHSHARED_EXPORT decrypt_aes256_ecb(char *pbsource, int cbsource, char *pbkey, char *pboutput, int *pcboutput);
int CRYPTOGRAPHSHARED_EXPORT encrypt_aes256_cbc(char *pbsource, int cbsource, char *pbkey, char *pboutput, int *pcboutput, char *iv, int iv_len);
int CRYPTOGRAPHSHARED_EXPORT decrypt_aes256_cbc(char *pbsource, int cbsource, char *pbkey, char *pboutput, int *pcboutput, char *iv, int iv_len);
int CRYPTOGRAPHSHARED_EXPORT drbg_generate(char *pboutput, int pcboutput);
int CRYPTOGRAPHSHARED_EXPORT derive_key(char *pbsource, int pcbsource, char *pboutput, int pcboutput, int pAmt = 4, char *otherInfo = 0, int otherLen = 0);
//void CRYPTOGRAPHSHARED_EXPORT drbg_save_state(byte *out, dword *len);
//void CRYPTOGRAPHSHARED_EXPORT drbg_load_state(byte *in, dword len);

#endif // CRYPTOGRAPH_H
