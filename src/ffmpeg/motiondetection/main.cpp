#include "MVExtractor.h"

int main(int argc, char **argv)
{
    //int ret = 0, got_frame;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <video>\n", argv[0]);
        exit(1);
    }
  
    const char *src_filename = argv[1];
    
    av_register_all();
    
    MVExtractor  mve(src_filename );
    
    mve.extract();
    
    return 0;
}