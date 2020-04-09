   
#include "dump.h" 



FILE *fpDump = NULL;

int dump_vfprintf( const char *format, va_list arg )
{
    vfprintf(fpDump, format, arg);
    //vfprintf(stderr, format, arg);

    return 0;
}





char g_bigBuf[1024] = {0};

char* IntToBinary(int val, int bits)
{
	int i;
	int c = 0;
    for (i = 0; i < 32; i++) g_bigBuf[i] = '\0';
 
    for (i = bits - 1; i >= 0; i--)
    {
        int on = (val & (1 << i)) ? 1 : 0;
        g_bigBuf[c] = on ? '1' : '0';
        c++;
    }

    return &g_bigBuf[0];
}

/***************************************************************************/

/*
void DumpHufCodes(bitstring* m_blocks, int size)
{
	int i;
    myPrintf("HufCodes\n");
    myPrintf("Num: %d\n", size);

	for (i = 0; i < size; i++)
    {
        myPrintf("%03d\t [%s] [%d]\n", i, IntToBinary(m_blocks[i].value, m_blocks[i].length), m_blocks[i].value);
    }
    myPrintf("\n");
    
    fflush(stdout);

}
*/
/***************************************************************************/

void DumpDCTValues(short dct[64])
{

	int i;
    int c = 0;
	myPrintf("\n#Extracted DCT values from SOS#\n");
    for ( i = 0; i < 64; i++)
    {
        myPrintf("% 4d  ", dct[c++]);

        if ((c > 0) && (c % 8 == 0)) myPrintf("\n");
    }
    myPrintf("\n");
    
    fflush(stdout);
}

void dumpByte(BYTE *dct, int dx, int dy)
{
	int j;
	int i;
    for (j = 0; j < dy; j++)
    {
        for ( i = 0; i < dx; i++)
        {
            myPrintf("%03d  ", dct[j * dx + i]);

        }
        myPrintf("\n");
    }
    myPrintf("\n");
    myPrintf("*********************************************************************************************************\n");
    fflush(stdout);

}

void printColor(long *col, int size)
{
	int i;
    for (i = 0; i < size; i++)
        myPrintf("%d ", col[i]);

    myPrintf("\n");
    myPrintf("\n");
    fflush(stdout);

}

void printLong(long *dct, int dx,  int dy)
{
	int i;
	int j;
    for (j = 0; j < dy; j++)
    {
        for (i = 0; i < dx; i++)
        {
            myPrintf("%d  ", dct[j * dx + i]);

        }
        myPrintf("\n");
    }
    myPrintf("\n");
    
    fflush(stdout);

}

void printRangeLimit(BYTE* range_limit, int size)
{
	int i;
    for ( i = 0; i < size; i++)
        myPrintf("%d ", range_limit[i]);

    myPrintf("\n");
    myPrintf("\n");
    fflush(stdout);

}

void dump8x8(long dct[64])
{

    int c = 0;
    int i = 0;
    for (; i < 64; i++)
    {
        myPrintf("%4d  ", (int) dct[c++]);

        if ((c > 0) && (c % 8 == 0)) myPrintf("\n");
    }
    myPrintf("\n");
    fflush(stdout);
}




int log_set_file(const char* filename)
{
        FILE* fp = fopen(filename, "w");
        if (fp == NULL) return -1;
        fpDump = fp;
        return 0;
}


   
