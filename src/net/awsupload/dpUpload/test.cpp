/*************************************
Create Table to Create table
PutItem to put item in table
GetItem to get Item in table
 *************************************/
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
#include <aws/core/client/AsyncCallerContext.h>

#define PRIMERYKEY "driverid"
const Aws::String table = "UberDriverInfo";

void CreateTable(Aws::DynamoDB::DynamoDBClient & dynamoClient) {

    std::cout << "Creating table " << table <<
            " with a simple primary key: " << PRIMERYKEY << std::endl;

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

    const Aws::DynamoDB::Model::CreateTableOutcome& result = dynamoClient.CreateTable(req);
    if (result.IsSuccess()) {
        std::cout << "Table \"" << result.GetResult().GetTableDescription().GetTableName() <<
                " created!" << std::endl;
    } else {
        std::cout << "Failed to create table: " << result.GetError().GetMessage();
    }

}

void putItemFinished(const Aws::DynamoDB::DynamoDBClient* client,
        const Aws::DynamoDB::Model::PutItemRequest& request,
        const Aws::DynamoDB::Model::PutItemOutcome& result,
        const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) {


    if (result.IsSuccess()) {
        std::cout << "added row item" << std::endl;
    } else {
        std::cout << "Failed to create table: " << result.GetError().GetMessage();
    }
    // Notify the thread that started the operation
    //    upload_variable.notify_one();

}


//ws::StringStream ssKey;
//                    Aws::String relativePath = entry.relativePath;
//                    char delimiter[] = { Aws::FileSystem::PATH_DELIM, 0 };
//                    Aws::Utils::StringUtils::Replace(relativePath, delimiter, "/");

                    
int PutItem(Aws::DynamoDB::DynamoDBClient & dynamoClient, std::string driverName, std::string jsonArray) {

    ////////////////////////////////////////////////////////////////////////////////////

    
    const Aws::String name(driverName.c_str());

    Aws::DynamoDB::Model::PutItemRequest pir;
    pir.SetTableName(table);

    Aws::DynamoDB::Model::AttributeValue av;
    av.SetS(name);
    pir.AddItem(PRIMERYKEY, av);
    Aws::String args(jsonArray.c_str());
    
    char delimiter1[] = { '{', 0 };
    char delimiter2[] = { '}', 0 };
    char delimiter3[] = { ' ', 0 };
   
    
    
    Aws::Utils::StringUtils::Replace(args, delimiter1, "");
    Aws::Utils::StringUtils::Replace(args, delimiter2, "");
    Aws::Utils::StringUtils::Replace(args, delimiter3, "");
        
   args = args +",sharedfile:" +  driverName.c_str(); 
    const Aws::Vector<Aws::String>& fldspace = Aws::Utils::StringUtils::Split(args, ',');

    for (int x = 0; x < fldspace.size(); x++) {
        const Aws::String arg(fldspace[x]);
        const Aws::Vector<Aws::String>& flds = Aws::Utils::StringUtils::Split(arg, ':');
        if (flds.size() == 2) {
            Aws::DynamoDB::Model::AttributeValue val;
            val.SetS(flds[1]);
            pir.AddItem(flds[0], val);
        } else {
            std::cout << "Invalid argument: " << arg << std::endl;
            return 1;
        }
    }

    auto context = Aws::MakeShared<Aws::Client::AsyncCallerContext>("PutObjectAllocationTag");
    context->SetUUID("UniqueRequestKey");

    dynamoClient.PutItemAsync(pir, &putItemFinished, context);
    //    if (!result.IsSuccess()) {
    //        std::cout << result.GetError().GetMessage() << std::endl;
    //        return 1;
    //    }

    return 0;
    //  std::cout << "Done!" << std::endl;
    ///////////////////////////////////////////////////////////////////////////////////
}

int GetItem(Aws::DynamoDB::DynamoDBClient & dynamoClient, std::string driverName) {

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
    const Aws::DynamoDB::Model::GetItemOutcome& result = dynamoClient.GetItem(req);
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

        std::string jsonArray = "{filename:driver-1234-1232323.mp4, gps-latitude:28.674109, gps-longitude:77.438009, timestamp:20200309194530, uploadmode:normal}";
        std::cout << jsonArray << std::endl << std::flush;
        PutItem(dynamoClient, "driver12345", jsonArray);

        GetItem(dynamoClient, "driver12345");

    }
    Aws::ShutdownAPI(options);
}
