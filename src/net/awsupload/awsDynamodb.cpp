/*************************************
Create Table to Create table
PutItem to put item in table
GetItem to get Item in table
 *************************************/
#include "base/logger.h"
#include "awsDynamodb.h"
#include <aws/core/Aws.h>
#include <aws/core/utils/Outcome.h> 
#include <aws/dynamodb/DynamoDBClient.h>
#include <aws/dynamodb/model/AttributeDefinition.h>
#include <aws/dynamodb/model/CreateTableRequest.h>
#include <aws/dynamodb/model/KeySchemaElement.h>
#include <aws/dynamodb/model/ProvisionedThroughput.h>
#include <aws/dynamodb/model/ScalarAttributeType.h>
#include <iostream>

#include <aws/dynamodb/model/PutItemRequest.h>
#include <aws/dynamodb/model/PutItemResult.h>

#include <aws/dynamodb/model/GetItemRequest.h>

#include "json/json.h"

using namespace base;

Aws::DynamoDB::DynamoDBClient *dynamoClient;
const Aws::String region =  "sa-east-1";               ;//"sa-east-1" ; //"us-east-2"; // Optional
Aws::Client::ClientConfiguration clientConfig1;

#define PRIMERYKEY "driverid"
const Aws::String table = "UberDriverInfo";

void CreateTable() {

    std::cout << "Creating table " << table << " with a simple primary key: " << PRIMERYKEY << std::endl;

    Aws::DynamoDB::Model::CreateTableRequest req;

    Aws::DynamoDB::Model::AttributeDefinition haskKey;
    haskKey.SetAttributeName(PRIMERYKEY);
    haskKey.SetAttributeType(Aws::DynamoDB::Model::ScalarAttributeType::S);
    req.AddAttributeDefinitions(haskKey);

    Aws::DynamoDB::Model::KeySchemaElement keyscelt;
    keyscelt.WithAttributeName(PRIMERYKEY).WithKeyType(Aws::DynamoDB::Model::KeyType::HASH);
    req.AddKeySchema(keyscelt);

    Aws::DynamoDB::Model::ProvisionedThroughput thruput;
    thruput.WithReadCapacityUnits(5).WithWriteCapacityUnits(5);
    req.SetProvisionedThroughput(thruput);

    req.SetTableName(table);

    const Aws::DynamoDB::Model::CreateTableOutcome& result = dynamoClient->CreateTable(req);
    if (result.IsSuccess()) {
        STrace << "Table \"" << result.GetResult().GetTableDescription().GetTableName() <<    " created!";
    } else {
        SError << "Failed to create table: " << result.GetError().GetMessage();
    }

}

void putItemFinished(const Aws::DynamoDB::DynamoDBClient* client,
        const Aws::DynamoDB::Model::PutItemRequest& request,
        const Aws::DynamoDB::Model::PutItemOutcome& result,
        const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) {

    if (result.IsSuccess()) {
        SInfo << "added row item" << std::endl;
    } else {
        SError << "Failed to create table: " << result.GetError().GetMessage();
    }
    // Notify the thread that started the operation
    //    upload_variable.notify_one();

}

int PutItem(std::string driverLoginid, std::string s3file, std::string jsonArray) {

    ////////////////////////////////////////////////////////////////////////////////////


    const Aws::String id(s3file.c_str());

    Aws::DynamoDB::Model::PutItemRequest pir;
    pir.SetTableName(table);

    Aws::DynamoDB::Model::AttributeValue av;
    av.SetS(id);
    pir.AddItem(PRIMERYKEY, av);

    const Aws::String sharedfile((s3file + ".mp4").c_str());
    av.SetS(sharedfile);
    pir.AddItem("s3file", av);

    const Aws::String driverId(driverLoginid.c_str());
    av.SetS(driverId);
    pir.AddItem("driverloginid", av);


    
    try {

        json optr = json::parse(jsonArray.c_str());
        for (json::iterator it = optr.begin(); it != optr.end(); ++it) {
           // std::cout << it.key() << " : " << it.value() << "\n";
            std::string key = it.key();
            std::string value;
            if (it.value().is_array()) {
                json fileNames = it.value();

                std::string files = "[";
                for (int x = 0; x < it.value().size(); ++x) {
                    if (x) {
                        files = files + ", ";
                    }
                    files = files + std::string(fileNames[x].get<std::string>());
                } //end for
                files += "]";
                value = files;

                // Aws::DynamoDB::Model::AttributeValue ival;

                //std::shared_ptr<Aws::DynamoDB::Model::AttributeValue> ival;
                // emptyList.push_back("arvind");
                // listValueAttribute.SetL(emptyList);
                // pir.AddItem(key.c_str(), emptyList); 
                // ival.SetS("arvind");
                //emptyList.push_back(ival);
                //Aws::DynamoDB::Model::AttributeValue val;
                //val.SetL(emptyList);
                //pir.AddItem(key.c_str(), val);
            } else {
                value = it.value();
            }



            Aws::DynamoDB::Model::AttributeValue val;
            val.SetS(value.c_str());
            pir.AddItem(key.c_str(), val);


        }
      }
      catch (...) 
      {
            std::cout << "wrong josn " << jsonArray;
            return 0;
      }
   



    auto context = Aws::MakeShared<Aws::Client::AsyncCallerContext>("PutObjectAllocationTag");
    context->SetUUID("UniqueRequestKey");

    dynamoClient->PutItemAsync(pir, &putItemFinished, context);
    //    if (!result.IsSuccess()) {
    //        std::cout << result.GetError().GetMessage() << std::endl;
    //        return 1;
    //    }

    return 0;
    //  std::cout << "Done!" << std::endl;
    ///////////////////////////////////////////////////////////////////////////////////
}


int GetItem(std::string driverName) {

    Aws::DynamoDB::Model::GetItemRequest req;

    const Aws::String projection("");
    /*
     projection expression (a quote-delimited,\n"
            "comma-separated list of attributes to retrieve) to limit the\n"
            "fields returned from the table.\n\n"
            "Example:\n"
            "    get_item HelloTable World\n"
            "    get_item SiteColors text \"default, bold\"\n";
     */

    // Set up the request
    req.SetTableName(table);
    Aws::DynamoDB::Model::AttributeValue hashKey;
    hashKey.SetS(driverName.c_str());
    req.AddKey(PRIMERYKEY, hashKey);
    if (!projection.empty())
        req.SetProjectionExpression(projection);

    // Retrieve the item's fields and values
    const Aws::DynamoDB::Model::GetItemOutcome& result = dynamoClient->GetItem(req);
    if (result.IsSuccess()) {
        // Reference the retrieved fields/values
        const Aws::Map<Aws::String, Aws::DynamoDB::Model::AttributeValue>& item = result.GetResult().GetItem();
        if (item.size() > 0) {
            // Output each retrieved field and its value
            for (const auto& i : item)
                std::cout << i.first << ": " << i.second.GetS() << std::endl;
        } else {
            std::cout << "No item found with the key " << driverName << std::endl;
        }

    } else {
        std::cout << "Failed to get item: " << result.GetError().GetMessage();
    }
}

void dbInit() {

    if (!region.empty())
    clientConfig1.region = region;

    dynamoClient = new Aws::DynamoDB::DynamoDBClient(clientConfig1);

}

void dbExit() {

    delete dynamoClient;

}


#if 0

int main(int argc, char** argv) {

    Aws::SDKOptions options;
    Aws::InitAPI(options);
    {
        // Assign these values before running the program
        const Aws::String region = "us-east-2"; // Optional

        Aws::Client::ClientConfiguration clientConfig;
        if (!region.empty())
            clientConfig.region = region;


        Aws::DynamoDB::DynamoDBClient dynamoClient(clientConfig);

       // CreateTable(dynamoClient);

       // std::string metadata = "{filename:driver-1234-1232323.mp4, gps-latitude:28.674109, gps-longitude:77.438009, timestamp:20200309194530, uploadmode:normal}";
       // std::string  metadata ="{\"filename\":\"1.mp4\",\"gps-latitude\":\"28.674109\",\"gps-longitude\":\"77.438009\",\"timestamp\":\"20200309194530\",\"uploadmode\":\"normal\"}";
        std::string  metadata ="{\"filename\":[\"1.mp4\",\"2.mp4\"],\"gps-latitude\":\"28.674109\",\"gps-longitude\":\"77.438009\",\"timestamp\":\"20200309194530\",\"uploadmode\":\"normal\"}";

                        
        std::cout << metadata << std::endl << std::flush;
        PutItem(dynamoClient, "driver12345", metadata);

        GetItem(dynamoClient, "driver12345");

    }
    Aws::ShutdownAPI(options);
}
#endif

