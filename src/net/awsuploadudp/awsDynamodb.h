

#ifndef AWS_DB_SAVE_H
#define AWS_DB_SAVE_H

#include <aws/core/Aws.h>

void CreateTable() ;
int PutItem( std::string driverName, std::string s3file, std::string jsonArray  ) ;
int GetItem( std::string driverName  );

void dbInit();
void dbExit();


#endif  //AWS_DB_SAVE_H

