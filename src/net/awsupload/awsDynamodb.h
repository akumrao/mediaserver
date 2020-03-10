

#ifndef AWS_DB_SAVE_H
#define AWS_DB_SAVE_H

#include <aws/core/Aws.h>
#include <aws/dynamodb/DynamoDBClient.h>

Aws::DynamoDB::DynamoDBClient *dynamoClient;

void CreateTable() ;
int PutItem( std::string driverName, std::string jsonArray  ) ;
int GetItem( std::string driverName  );

#endif  //AWS_DB_SAVE_H

