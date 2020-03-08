
#include <aws/core/Aws.h>
#include <iostream>
#include <fstream>


bool put_s3_object_async( const Aws::String& s3_object_name, std::string &file_name);
void awsInit();
void awsExit();


#if 0    

int main(int argc, char** argv)
{
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
