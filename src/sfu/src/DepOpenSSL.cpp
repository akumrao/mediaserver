#define MS_CLASS "DepOpenSSL"
// #define MS_LOG_DEV_LEVEL 3

#include "DepOpenSSL.h"
#include "LoggerTag.h"
#include <openssl/crypto.h>
#include <openssl/rand.h>

/* Static methods. */

void DepOpenSSL::ClassInit()
{

	LTrace( "openssl version: ", OpenSSL_version(OPENSSL_VERSION));

	// Initialize some crypto stuff.
	RAND_poll();
}
