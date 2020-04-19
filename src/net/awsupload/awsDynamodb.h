/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */



#ifndef AWS_DB_SAVE_H
#define AWS_DB_SAVE_H

#include <aws/core/Aws.h>

void CreateTable() ;
int PutItem( std::string driverName, std::string s3file, std::string jsonArray  ) ;
int GetItem( std::string driverName  );

void dbInit();
void dbExit();


#endif  //AWS_DB_SAVE_H

