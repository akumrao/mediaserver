#ifndef _SecurityToken_hxx_
#define _SecurityToken_hxx_

/*
 * Copyright (c) 2011-2013 byArvind Umrao.
 * 
 */


#include "Digest.h"

class SecurityToken
{
	public:

		static string createServerToken(string& value);
		static bool isValidServerToken(string& token, string& value, uint32_t timeDiff = 300);
};




#endif
