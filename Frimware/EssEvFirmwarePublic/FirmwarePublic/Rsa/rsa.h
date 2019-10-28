/*
	RSA.H - header file for RSA.C

    Copyright (c) J.S.A.Kapp 1994 - 1996.

	RSAEURO - RSA Library compatible with RSAREF 2.0.

	All functions prototypes are the Same as for RSAREF.
	To aid compatiblity the source and the files follow the
	same naming comventions that RSAREF uses.  This should aid
        direct importing to your applications.

	This library is legal everywhere outside the US.  And should
	NOT be imported to the US and used there.

	RSA Routines Header File.

	Revision 1.00 - JSAK.
*/
#include "RsaEuro.h"
int RSAPublicEncrypt(unsigned char *output, unsigned int *outputLen, unsigned char *input, unsigned int inputLen, R_RSA_PUBLIC_KEY *publicKe, R_RANDOM_STRUCT *randomStruct);
int RSAPrivateEncrypt(unsigned char *output, unsigned int *outputLen, unsigned char *input, unsigned int inputLen, R_RSA_PRIVATE_KEY *privateKey);
int RSAPublicDecrypt(unsigned char *output, unsigned int *outputLen, unsigned char *input, unsigned int inputLen, R_RSA_PUBLIC_KEY *publicKey);
int RSAPrivateDecrypt(unsigned char *output, unsigned int *outputLen, unsigned char *input, unsigned int inputLen, R_RSA_PRIVATE_KEY *privateKey);
