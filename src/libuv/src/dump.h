#include <stdio.h>
#include <stdarg.h>    


#define BYTE unsigned char




#define LOG_CALL   // myPrintf("%s:%s\n", __FILE__, __func__)

int dump_vfprintf( const char *format, va_list arg );

static void myPrintf(const char *fmt, ...)
{
    va_list parms;
    char buf[256];

    // Try to print in the allocated space.
    va_start(parms, fmt);
    vsprintf(buf, fmt, parms);
    va_end(parms);

    // Write the information out to a txt file
  /*  if(fpDump)
    {    
      fprintf(fpDump, "%s", buf);
      fflush( fpDump);
    }
   */
    printf("%s", buf);
   
}// End myPrintf(..)





char* IntToBinary(int val, int bits);


/***************************************************************************/

/*
void DumpHufCodes(bitstring* m_blocks, int size);

*/
/***************************************************************************/

void DumpDCTValues(short dct[64]);


void dumpByte(BYTE *dct, int dx, int dy);


void printColor(long *col, int size);


void printLong(long *dct, int dx,  int dy);


void printRangeLimit(BYTE* range_limit, int size);

void dump8x8(long dct[64]);





int log_set_file(const char* filename);


/*

#define INFO    1
#define ERR 2
#define STD_OUT stdout
#define STD_ERR stderr
#define LOG_MESSAGE(prio, stream, msg, ...)  do {\
                                              char *str;\
                                                if (prio == INFO)\
                                                    str = "INFO";\
                                                else if (prio == ERR)\
                                                    str = "ERR";\
                                                  if(fpDump)\
                                                    {fprintf(fpDump, "[%s] : %s : %d : "msg" \n", str, __FILE__, __LINE__, ##__VA_ARGS__);fflush(fpDump);}\
                                                  else\
                                                    fprintf(stream, "[%s] : %s : %d : "msg" \n", str, __FILE__, __LINE__, ##__VA_ARGS__);\
                                            } while (0)


#define LOG_RGB( msg)                    do {\
                                                  if(fpDump)\
                                                    {fprintf(fpDump, "%03d ", msg);fflush(fpDump);}\
                                                  else\
                                                    fprintf(stdout, "%03d ", msg);\
                                            } while (0)

#define LOG_DELIMETER( msg)                    do {\
                                                  if(fpDump)\
                                                    {fprintf(fpDump, msg);fflush(fpDump);}\
                                                  else\
                                                    fprintf(stdout, msg);\
                                            } while (0)
*/
/*
{
    char *s = "Hello";

       
    LOG_MESSAGE(ERR, STD_ERR, "Failed to open file");

    
    LOG_MESSAGE(INFO, STD_OUT, "%s Geeks for Geeks", s);

   
    LOG_MESSAGE(INFO, STD_OUT, "%d + %d = %d", 10, 20, (10 + 20));

    return 0;
}
*/
   