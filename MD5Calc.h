#ifndef MD5_CALC_H
#define MD5_CALC_H

/*
MD5 test suite:
MD5 ("") = d41d8cd98f00b204e9800998ecf8427e
MD5 ("a") = 0cc175b9c0f1b6a831c399e269772661
MD5 ("abc") = 900150983cd24fb0d6963f7d28e17f72
MD5 ("message digest") = f96b697d7cb7938d525a2f31aaf161d0
MD5 ("abcdefghijklmnopqrstuvwxyz") = c3fcd3d76192e4007dfb496cca67e13b
MD5 ("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789") =
 d174ab98d277d9f5a5611c2c9f419d9f
MD5 ("12345678901234567890123456789012345678901234567890123456789012345678901234567890") =
 57edf4a22be3c955ac49da2e2107b67a
*/

namespace beam_me_up {

/** Ovaj tip koristis kad racunas MD5 u vise koraka, jer u svakom koraku dobavljas iduci ulazni blok
 npr. kad citas iz fajla.
*/
class MD5Calc {
    static unsigned int const state_init[4];
    unsigned int state[4];/* state (ABCD) */
    unsigned int count[2];/* number of bits, modulo 2^64 (lsb first) */
    unsigned char buffer[64];/* input buffer */
public:
    /** parcijalna vrijednost MD5 za tekuci blok uzimajuci u obzir dosadasnje stanje */
    void Update(unsigned char const* input, unsigned int inputLen);

    void Update(char const* input, unsigned int inputLen)
    {
        return Update(reinterpret_cast<unsigned char const*>(input), inputLen);
    }

    /** konacni rezultat, kad su funkciji Update isporuceni svi blokovi */
    void Finish(unsigned char digest[16]);
    MD5Calc(void);
    ~MD5Calc(void);
};

/** Ovo koristis kad racunas MD5 u jednom koraku, jer imas raspoloziv citav ulaz. */
void MD5Calculate (unsigned char digest[16], unsigned char const* input, unsigned int inputLen);

/** Isto kao MD5Calculate ali kao rezultat daje ex-ili 4 32-bitska bloka od kojih je sastavljen MD5 digest */
unsigned int MD5_32(unsigned char* input, unsigned int inputLen);

}

#endif //MD5_CALC_H
