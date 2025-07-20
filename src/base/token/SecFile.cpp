/*
 * Copyright (c) 2011-2013 by Arvind Umrao.
 * 
 */

#include "SecFile.h"
#include "base/filesystem.h"
#include <sys/file.h>
#include "base/logger.h"
using namespace base;

bool SecFile::readFile(std::string &fileName,  std::string &content)
{

    FILE* fp_out{nullptr};
    if ((fp_out = fopen(fileName.c_str(), "rb")) == NULL) 
    {

        SError << "Could not open " <<  fileName;
        return false;
    }

    if (0 != flock(fileno(fp_out), LOCK_SH))
    {
         SError << "Could not flock " <<  fileName;
        return false;
    }


    char buffer[100];

    int bytes_read ;
    while(!feof(fp_out))
    {
        bytes_read = fread(buffer,1,sizeof(buffer), fp_out);
        content += std::string( buffer,bytes_read);
    }

    flock(fileno(fp_out), LOCK_UN);

    if(fp_out)
    fclose(fp_out);
    fp_out = nullptr;
}



bool SecFile::writeFile(std::string &fileName,  std::string &content)
{

    FILE* fp_out{nullptr};
    if ((fp_out = fopen(fileName.c_str(), "wb")) == NULL) 
    {

        SError << "Could not open " <<  fileName;
        return false;
    }
    if (0 != flock(fileno(fp_out), LOCK_EX))
    {
         SError << "Could not flock " <<  fileName;
        return false;
    }



   int ret = fwrite(content.data(), content.size(), 1, fp_out);
    flock(fileno(fp_out), LOCK_UN);
    if(fp_out)
    fclose(fp_out);
    fp_out = nullptr;
}