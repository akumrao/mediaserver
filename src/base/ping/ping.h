#ifndef _PING_H__
#define _PING_H__

#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <sys/types.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <signal.h>
#include <pthread.h>

#include <string>
#include <iostream>     // std::cout, std::ios
#include <sstream>      // std::ostringstream

#define PACKET_SIZE 4096
#define SEND_DATA_LEN 56
#define ERROR -1
#define SUCCESS 1
#define MAX_WAIT_TIME 3
#define MAX_NO_PACKETS 4
#define MAX_PACKET_NO 50

#include <android/log.h>

#include <android/log.h>
#define  LOG_TAG    "testjni"

#define  ALOG(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)



#define LOG    "so"
#define LOGD (...)  __android_log_print (android_log_debug,log,__va_args__)//define LOGD type
#define Logi (...)  __android_log_print (android_log_info,log,__va_args__)//define Logi type
#define LOGW (...)  __android_log_print (android_log_warn,log,__va_args__)//define LOGW type
#define LOGE (...)  __android_log_print (android_log_error,log,__va_args__)//define Loge Type
#define LOGF (...)  __android_log_print (android_log_fatal,log,__va_args__)//define LOGF type


//LOGD ("string1");
//LOGD ("string2%s%d", "Hello", 10);

class CPing
{
    public:
        CPing();
        explicit CPing(const char *ip, const int timeout);
       // CPing(const CPing& orig);
        ~CPing();

        bool ping();

    private:
        bool CreateSocket();
        bool CloseSocket();
        
        void SetNonBlocking(const bool flag);

        void Send_Packet();
        void Receive_Packet();

        int Pack(int pack_no);
        int Unpack(int packSize);

        static void tv_sub(struct timeval *in, struct timeval *out);

        static void statistics(int sig);
        unsigned short Cal_ChkSum(unsigned short *addr, int len);
        
        void Init_PacketNoArray(); 

    private:
        std::string m_strIp;
        std::string m_copyIp;

        static int m_nSend;
        static int m_nRecv;
        int m_nCnt;

        static struct timeval m_begin_tvsend;
        static struct timeval m_end_tvrecv;
        struct timeval m_tvsend;

        char m_sendpack[PACKET_SIZE];
        char m_recvpack[PACKET_SIZE];

        struct sockaddr_in m_dest_addr;
        struct sockaddr_in m_from_addr;
        
        int m_nMaxTimeWait;
        static int m_nSocketfd;
        
        int m_nPacketNoLimit;
        bool m_bPacketNo[MAX_PACKET_NO]; 
        
public:
    std::string  pingStr;
};
#endif
