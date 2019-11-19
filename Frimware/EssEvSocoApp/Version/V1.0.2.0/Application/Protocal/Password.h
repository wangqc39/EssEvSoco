#ifndef __PASSWORD__
#define __PASSWORD__
#include "Rsaeuro.h"


#define LENGTH_PASSWORD         16
#define LENGTH_PASSWORD_RANDOM  16
extern uint8_t Password[LENGTH_PASSWORD];

#define PASSWORD_CHAR_MIN           0x21
#define PASSWORD_CHAR_MAX           0x7E

int32_t MakeRamdomRsaKey(void);
R_RSA_PUBLIC_KEY* GetPasswordPublicKey(void);
uint8_t * GetPasswordRandom(void);
int32_t AuthorizePassword(uint8_t *SecurePassword);
bool IsPasswordPassed(void);
int32_t SetPassword(uint8_t *SecurePassword);
int32_t ResetPassword(uint8_t *SecurePassword);
void SetPasswordInvalid(void);








#endif

