 
//snippet-sourcedescription:[put_object.cpp demonstrates how to put a file into an Amazon S3 bucket.]
//snippet-service:[s3]
//snippet-keyword:[Amazon S3]
//snippet-keyword:[C++]
//snippet-sourcesyntax:[cpp]
//snippet-keyword:[Code Sample]
//snippet-sourcetype:[full-example]
//snippet-sourceauthor:[AWS]

/*
   Copyright 2010-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.

   This file is licensed under the Apache License, Version 2.0 (the "License").
   You may not use this file except in compliance with the License. A copy of
   the License is located at

    http://aws.amazon.com/apache2.0/

   This file is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied. See the License for the
   specific language governing permissions and limitations under the License.
*/

//snippet-start:[s3.cpp.put_object.inc]
#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/model/BucketCannedACL.h>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
//snippet-end:[s3.cpp.put_object.inc]
/********************************************************************/
/**
 * Check if file exists
 * 
 * Note: If using C++17, can use std::filesystem::exists()
 */
inline bool file_exists(const std::string& name)
{
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

 std::mutex upload_mutex;
std::condition_variable upload_variable;
void put_object_async_finished(const Aws::S3::S3Client* client, 
    const Aws::S3::Model::PutObjectRequest& request, 
    const Aws::S3::Model::PutObjectOutcome& outcome,
    const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context)
{
    // Output operation status
    if (outcome.IsSuccess()) {
        std::cout << "put_object_async_finished: Finished uploading " 
            << context->GetUUID() << std::endl;
    }
    else {
        auto error = outcome.GetError();
        std::cout << "ERROR: " << error.GetExceptionName() << ": "
            << error.GetMessage() << std::endl;
    }

    // Notify the thread that started the operation
    upload_variable.notify_one();
}


bool put_s3_object_async(const Aws::S3::S3Client& s3_client,
    const Aws::String& s3_bucket_name,
    const Aws::String& s3_object_name,
    const std::string& file_name)
{
    // Verify file_name exists
    if (!file_exists(file_name)) {
        std::cout << "ERROR: NoSuchFile: The specified file does not exist"
            << std::endl;
        return false;
    }

    // Set up request
    Aws::S3::Model::PutObjectRequest object_request;

    object_request.SetBucket(s3_bucket_name);
    object_request.SetACL(Aws::S3::Model::ObjectCannedACL::public_read);
     object_request.SetContentType("video/mp4");
    object_request.SetKey(s3_object_name);
    const std::shared_ptr<Aws::IOStream> input_data =
        Aws::MakeShared<Aws::FStream>("SampleAllocationTag",
            file_name.c_str(),
            std::ios_base::in | std::ios_base::binary);
    object_request.SetBody(input_data);

    // Set up AsyncCallerContext. Pass the S3 object name to the callback.
    auto context =
        Aws::MakeShared<Aws::Client::AsyncCallerContext>("PutObjectAllocationTag");
    context->SetUUID(s3_object_name);

    // Put the object asynchronously
    s3_client.PutObjectAsync(object_request, 
                             put_object_async_finished,
                             context);
    return true;
    
}


    

/**********************************************************************/

/**
 * Put an object into an Amazon S3 bucket
 */
bool put_s3_object(const Aws::String& s3_bucket_name, 
    const Aws::String& s3_object_name, 
    const std::string& file_name, 
    const Aws::String& region = "us-east-2")
{
    // Verify file_name exists
    if (!file_exists(file_name)) {
        std::cout << "ERROR: NoSuchFile: The specified file does not exist" 
            << std::endl;
        return false;
    }

    // If region is specified, use it
    Aws::Client::ClientConfiguration clientConfig;
    if (!region.empty())
        clientConfig.region = region;

    // Set up request
    // snippet-start:[s3.cpp.put_object.code]
    Aws::S3::S3Client s3_client(clientConfig);
    Aws::S3::Model::PutObjectRequest object_request;

//    object_request.SetBucket(s3_bucket_name);
//    object_request.SetKey(s3_object_name);
//    const std::shared_ptr<Aws::IOStream> input_data = 
//        Aws::MakeShared<Aws::FStream>("SampleAllocationTag", 
//                                      file_name.c_str(), 
//                                      std::ios_base::in | std::ios_base::binary);
//    object_request.SetBody(input_data);
    
    
    ///////////////////////////////////////////////////////
    
     char buffer[]="arvind1 is working";
    object_request.SetBucket(s3_bucket_name);
    object_request.SetKey(s3_object_name);
    
    object_request.SetACL(Aws::S3::Model::ObjectCannedACL::public_read);
    
    auto data = Aws::MakeShared<Aws::StringStream>("PutObjectInputStream", std::stringstream::in | std::stringstream::out | std::stringstream::binary);
    data->write(reinterpret_cast<char*>(buffer), strlen(buffer));
     data->write(reinterpret_cast<char*>(buffer), strlen(buffer));
      data->write(reinterpret_cast<char*>(buffer), strlen(buffer));
    object_request.SetBody(data);

    ///////////////////////////////////////////////////////

    // Put the object
    auto put_object_outcome = s3_client.PutObject(object_request);
    if (!put_object_outcome.IsSuccess()) {
        auto error = put_object_outcome.GetError();
        std::cout << "ERROR: " << error.GetExceptionName() << ": " 
            << error.GetMessage() << std::endl;
        return false;
    }
    return true;
    // snippet-end:[s3.cpp.put_object.code]
}




/**
 * Exercise put_s3_object()
 */
int main(int argc, char** argv)
{

    Aws::SDKOptions options;
    Aws::InitAPI(options);
    {
        // Assign these values before running the program
        const Aws::String bucket_name = "ubercloudproject";
        const Aws::String object_name = "test.mp4";
        const std::string file_name = "test.mp4";
        const Aws::String region = "us-east-2";      // Optional

        // Put the file into the S3 bucket
//        if (put_s3_object(bucket_name, object_name, file_name, region)) {
//            std::cout << "Put file " << file_name 
//                << " to S3 bucket " << bucket_name 
//                << " as object " << object_name << std::endl;
//        }
        
        
     Aws::Client::ClientConfiguration clientConfig;
    if (!region.empty())
        clientConfig.region = region;

    // Set up request
    // snippet-start:[s3.cpp.put_object.code]
    Aws::S3::S3Client s3_client(clientConfig);
    


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
    Aws::ShutdownAPI(options);
}

