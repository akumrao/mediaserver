#ifndef _SecurityFile_hxx_
#define _SecurityFile_hxx_

/*
 * Copyright (c) 2011-2013 byArvind Umrao.
 * 
 */


#include "Digest.h"

class SecFile
{
    public:
        static bool readFile(std::string &fileName,  std::string &content);
        static bool writeFile(std::string &fileName,  std::string &content);
};




#endif
