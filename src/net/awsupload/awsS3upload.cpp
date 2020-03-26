#include "base/logger.h"
#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/model/BucketCannedACL.h>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include "awsS3upload.h"
#include "awsDynamodb.h"
#include "udpUpload.h"
using namespace base;

const Aws::String region =  "sa-east-1";  // "sa-east-1" ;   // "us-east-2"; // Optional
Aws::Client::ClientConfiguration clientConfig;

const Aws::String s3_bucket_name = "ubercloudproject" ; //"ubercloudproject"; //"uberproject" ;    //";

// Set up request
// snippet-start:[s3.cpp.put_object.code]
Aws::S3::S3Client *s3_client;





/********************************************************************/

/**
 * Check if file exists
 * 
 * Note: If using C++17, can use std::filesystem::exists()
 */
inline bool file_exists(const std::string& name) {
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

//std::mutex upload_mutex;
//std::condition_variable upload_variable;

void put_object_async_finished1(const Aws::S3::S3Client* client,
        const Aws::S3::Model::PutObjectRequest& request,
        const Aws::S3::Model::PutObjectOutcome& outcome,
        const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) {
    // Output operation status
    if (outcome.IsSuccess()) {
        SInfo << "put_object_async_finished: Finished uploading "
                << context->GetUUID();
        
        //system("SendMail"); 
        
    } else {
        auto error = outcome.GetError();
        SError << "ERROR: " << error.GetExceptionName() << ": "
                << error.GetMessage() ;
    }

    // Notify the thread that started the operation
    //    upload_variable.notify_one();
}

inline char *storage_row( char*p ,  unsigned int n)
{
    return p + (n * UdpDataSize);
}


bool put_s3_object_async(
        const Aws::String& s3_object_name, char *serverstorage, long curPtr, int lastPacketLen) {
    // Verify file_name exists
    //    if (!file_exists(file_name)) {
    //        std::cout << "ERROR: NoSuchFile: The specified file does not exist"
    //                << std::endl;
    //        return false;
    //    }

    //    std::ifstream infile;
    //    infile.open(file_name, std::ios::binary | std::ios::in);

    // Set up request
    Aws::S3::Model::PutObjectRequest object_request;

    object_request.SetBucket(s3_bucket_name);
    object_request.SetACL(Aws::S3::Model::ObjectCannedACL::public_read);
    
    object_request.SetContentType("video/mp4");
   // object_request.SetContentType("text/plain");
    object_request.SetKey(s3_object_name);
    //    const std::shared_ptr<Aws::IOStream> input_data =
    //        Aws::MakeShared<Aws::FStream>("SampleAllocationTag",
    //            file_name.c_str(),
    //            std::ios_base::in | std::ios_base::binary);

    auto data = Aws::MakeShared<Aws::StringStream>("PutObjectInputStream", std::stringstream::in | std::stringstream::out | std::stringstream::binary);

    // char* buffer = new char[32768];

    // if (infile.is_open()) {

    // while (infile.read(buffer, 32768)) {
    
    std::cout << "**********************************" << std::endl;
    
//    for (int i = 0; i < lastPacketNO; ++i) {
//        
//        std::cout << std::string(serverstorage[i], UdpDataSize );
//
//        data->write(reinterpret_cast<char*> ( storage_row(serverstorage, i)), UdpDataSize);
//    }
     
    data->write(reinterpret_cast<char*> (serverstorage), curPtr);
    //std::cout << std::string(serverstorage, curPtr );
    std::cout << "*************************************" << std::endl;

    //    infile.close();

    // }
    // delete[] buffer;

    object_request.SetBody(data);

    // Set up AsyncCallerContext. Pass the S3 object name to the callback.
    auto context =
            Aws::MakeShared<Aws::Client::AsyncCallerContext>("PutObjectAllocationTag");
    context->SetUUID(s3_object_name);

    // Put the object asynchronously
    s3_client->PutObjectAsync(object_request,
            put_object_async_finished1,
            context);
    return true;

}
Aws::SDKOptions options;

void awsInit() {

    Aws::InitAPI(options);

    if (!region.empty())
        clientConfig.region = region;

    // Set up request
    // snippet-start:[s3.cpp.put_object.code]
    s3_client = new Aws::S3::S3Client(clientConfig);
    dbInit();

}

void awsExit() {

    delete s3_client;
    dbExit();

    Aws::ShutdownAPI(options);

}

#if 0    

int main(int argc, char** argv) {
    awsInit();

    {
        // Assign these values before running the program

        const Aws::String object_name = "test.mp4";
        const std::string file_name = "test.mp4";


        // Put the file into the S3 bucket
        //        if (put_s3_object(bucket_name, object_name, file_name, region)) {
        //            std::cout << "Put file " << file_name 
        //                << " to S3 bucket " << bucket_name 
        //                << " as object " << object_name << std::endl;
        //        }





        std::unique_lock<std::mutex> lock(upload_mutex);
        if (put_s3_object_async(s3_client,
                bucket_name,
                object_name,
                file_name)) {
            // While the upload is in progress, we can perform other tasks.
            // For this example, we just wait for the upload to finish.
            std::cout << "main: Waiting for file upload to complete..."
                    << std::endl;
            upload_variable.wait(lock);

            // The upload has finished. The S3Client object can be cleaned up 
            // now. We can also terminate the program if we wish.
            std::cout << "main: File upload completed" << std::endl;


        }
    }


    awsExit()
}
#endif
