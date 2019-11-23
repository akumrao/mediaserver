#include "ping.h"

struct timeval CPing::m_begin_tvsend;
struct timeval CPing::m_end_tvrecv;
int CPing::m_nSocketfd = -1;
int CPing::m_nSend = 0;
int CPing::m_nRecv = 0;

CPing::CPing()
{}

CPing::CPing(const char *ip, const int timeout) :
    m_strIp(ip),
    //m_nSend(0),
    //m_nRecv(0),
    m_nCnt(0),
    m_nMaxTimeWait(timeout),
    //m_nSocketfd(-1),
    m_nPacketNoLimit(0)
{;
    if(timeout > MAX_WAIT_TIME)
        m_nMaxTimeWait = MAX_WAIT_TIME;
    else
        m_nMaxTimeWait = timeout;
}

CPing::~CPing()
{
    if(!CloseSocket())
    {
        std::cout << "CloseSocket error...\n";
    } 
}


bool CPing::ping()
{

uid_t uid = getuid();
//setuid(uid);

    if(!CreateSocket())
    {
        ALOG("This message comes from C at line %d.", __LINE__);
        pingStr+="Connect error...\n";
//        LOGD("sdff");
        std::cout << "\n*****************************************************************\n";
        std::cout << pingStr;
        std::cout << "\n*****************************************************************\n";
         pingStr="";
        std::cout << "Create Socket error...\n";
        return false;
    }
    
    //inet_ntoa : convert ip_address network format  to string format
    //argument type : struct in_addr not struct in_addr::s_addr(unsigned long)
    printf("PING %s(%s): %d bytes data in ICMP packets.\n", m_strIp.c_str(), inet_ntoa(m_dest_addr.sin_addr), SEND_DATA_LEN);
    char tmp[256];
    sprintf( tmp,"PING %s(%s): %d bytes data in ICMP packets.\n", m_strIp.c_str(), inet_ntoa(m_dest_addr.sin_addr), SEND_DATA_LEN);
    
    pingStr+=tmp;
    
    Init_PacketNoArray();
    signal(SIGINT, statistics);
    for(int i=0 ; i < 3 ; ++i)
    {
        Send_Packet();
        Receive_Packet();
//        LOGE(pingStr);
        std::cout << "\n*****************************************************************\n";
        std::cout << pingStr;
        std::cout << "\n*****************************************************************\n";
        
        pingStr="";
        sleep(1);
        
        
        
    }

    //statistics(SIGINT);

    return true;
}


bool CPing::CreateSocket()
{
    //return a protoent struct that match the protocol name
    //member p_proto : the protocol number
    struct protoent *protocol = getprotobyname("icmp");
    if(protocol == NULL)
    {
        perror("CreateSocket: getprotobyname error...\n");
        return false;
    }
    
    //SOCK_RAW : different from SOCK_INET, SOCK_DGRAM
    //when treat ICMP protocol, 
    //when define IP_header, TCP_header etc, 
    //when receive then packet send to machine but TCP/IP stack can't treat then packet
    //use SOCK_RAW
    //
    //AF_INET : internet IP protocol
    //
    //return -1 when error
    m_nSocketfd = socket(AF_INET, SOCK_RAW, protocol->p_proto);
    if(m_nSocketfd < 0)
    {
        perror( "CreateSocket: socket error...\n");
        return false;
    }



    //file authority contains real_uid, effective_uid and saved_set_uid
    //real_uid is the id that run this project
    //effective_uid is the id that run this process
    //saved_set_uid is the copy of effective_uid
    //
    //set min authority : set effective uid to real_uid
    setuid(getuid());

    //set zero such as memset(char *, '\0', len)
    bzero(&m_dest_addr, sizeof(m_dest_addr));

    //destination address
    m_dest_addr.sin_family = AF_INET;
        
    int h_errnop;   //use to gethostbyname_r
    char buf[1024]; //like up
    struct hostent hostinfo, *dest_phost;   //host struct, use to get ip_address etc.

    //inet_addr : convert string to ip_address in network, 
    //when the string is not ip_address, is the website like www.baidu.com, return INADDR_NONE
    //and need to use gethostbyname/gethostbyname_r(thread safety) to get ip_address
    unsigned long inaddr = inet_addr(m_strIp.c_str());  
    if(inaddr == INADDR_NONE)
    {
        /*
        if((dest_phost = gethostbyname(m_strIp.c_str())) == NULL)
        {
            return false;
        }
        */
        
        //get ip_address according to website
        int ret = gethostbyname_r(m_strIp.c_str(), &hostinfo, buf, sizeof(buf), &dest_phost, &h_errnop);
        if( ret ||  !dest_phost)
        {
            pingStr+="Host resolve error...\n";
            perror("CreateSocket: gethostbyname_r error...\n");
            return false;
        }
        //h_addr : the first ip_address, char*
        //need to copy it to struct in_addr.s_addr
        
       // memcpy((char*)&m_dest_addr.sin_addr, dest_phost->h_addr, dest_phost->h_length); 
        m_dest_addr.sin_addr.s_addr = *(unsigned long*)(dest_phost->h_addr);
    }
    else
    {
        m_dest_addr.sin_addr.s_addr = inaddr;
    }
    
    return true;
}

bool CPing::CloseSocket()
{
    close(m_nSocketfd);
    return true;
}

void CPing::SetNonBlocking(const bool flag)
{
    int fl = fcntl(m_nSocketfd, F_GETFL);
    if(flag)
        fcntl(m_nSocketfd, F_SETFL, fl | O_NONBLOCK);
    else
        fcntl(m_nSocketfd, F_SETFL, fl & (~O_NONBLOCK));
}

void CPing::Init_PacketNoArray()
{
    for(int i = 0; i < MAX_PACKET_NO; ++i)
        m_bPacketNo[i] = false;
}

void CPing::Send_Packet()
{
    int packSize = Pack(m_nSend);
        
    //because not connect and bind, need to use sendto to send packet with destination address
    if(sendto(m_nSocketfd, m_sendpack, packSize, 0, (struct sockaddr*)&m_dest_addr, sizeof(m_dest_addr)) < 0)
    {
        pingStr+="Send_Packet: sendto error...\n";
        perror("Send_Packet: sendto error...\n");
        return;
    }
    
    ++m_nSend;
    return;
}


//create a ICMP data packet, type : struct icmp
int CPing::Pack(int pack_no)
{
    //8 bit ICMP_header + ICMP_data
    int packSize = 8 + SEND_DATA_LEN;
    
    //create ICMP
    bzero(m_sendpack, PACKET_SIZE);
    struct icmp *pIcmp =  (struct icmp*)m_sendpack;
    //ICMP_ECHO : request reply
    //ICMP_ECHOREPLY : reply
    pIcmp->icmp_type = ICMP_ECHO;
    pIcmp->icmp_code = 0;
    //!!!  check sum : need to calulate and need to set 0 when init!!!
    //when the packet reach the destination, the destination also calculate the icmp_cksum to judge 
    //whether occur error on the way to destination
    pIcmp->icmp_cksum = 0;
    //ICMP_id : custom, can be the getpid(), process_id
    pIcmp->icmp_id = getpid();
    //sequence : custom , need unique , use to judge Reply ICMP packet
    //can be the number of the packets sent
    pIcmp->icmp_seq = pack_no;
    
    //icmp_data : copy to time the packet creation(the time of send)
    struct timeval *tvsend = (struct timeval*)pIcmp->icmp_data;
    gettimeofday(tvsend, NULL);
    
    //record the first packet to send, use to calulate the total time
    if(pack_no == 0)
    {
        m_begin_tvsend.tv_sec = m_tvsend.tv_sec;
        m_begin_tvsend.tv_usec = m_tvsend.tv_usec;
    }
    
    //calulate check_sum
    pIcmp->icmp_cksum = Cal_ChkSum((unsigned short*)pIcmp, packSize);

    return packSize;
}


//when send a packet to destination, project needs to try to receive the packet sendback
void CPing::Receive_Packet()
{

    // !!!recvfrom the last argument fromlen need to init to sizeof(struct sockaddr_in) !!
    // if not, recvfrom maybe return error : Invalid argument
    int fromlen = sizeof(struct sockaddr_in);
    
    //select  timeout
    struct timeval timeout;
    timeout.tv_sec = m_nMaxTimeWait;
    timeout.tv_usec = 0;

     //max fd is the max fd in read_fd_set + 1
     int maxfd = m_nSocketfd + 1;

    //select fd_set
    fd_set recvfd;

    //the purpose to set a variable : m_nCnt is to solve loop on the way to destination
    //when there is loop path on the way, there maybe more than one same ICMP_PACKET send to destination
    //and the destination also will send these same ICMP_PACKET to project
    //which will cause the m_nRecv more than m_nSend and the while loop isn't valid
    //and cause can't receive the ICMP_PACKET new send
    //
    //m_nCnt increase has some conditions following:
    //one : select timeout, show that there isn't ICMP_PACKET sendback, set m_nCnt to m_nSend
    //two : receive a unique ICMP_PACKET, ++m_nCnt
    while(m_nCnt < m_nSend)
    {
        //use select to judge whether the request is timeout or not
        //
        //define a read_fd_set

        //clear the read_fd_set
        FD_ZERO(&recvfd);
        
        //add m_nSocketfd into read_fd_set
        FD_SET(m_nSocketfd, &recvfd);
 
        //return the number of changed fd in fd_set
        //there three conditions that select will return
        //one : error n == -1
        //two : timeout n == 0
        //three : there is one fd in read_fd, write_fd, error_fd has ready, return n = the number of fd ready
        int n = select(maxfd, &recvfd, NULL, NULL, &timeout);
        
        //if error, select again
        if(n < 0)
        {
            pingStr+="Receive_Packet: select error...\n";
            perror("Receive_Packet: select error...\n");
            continue;
        }
        //if timeout, then needn't receive packet again, so set m_nCnt to m_nSend and break
        else if(n == 0) 
        {
            std::cout << "Request Timeout..." <<  std::endl;
            pingStr+="Request Timeout...\n";
            
            m_nCnt = m_nSend;
            break;
        }

        //recvfrom : like recv
        //this function can get the address of send, put into m_from_addr
        //the packet message in m_recvpack
        //!!!!note the last argument!!!! look up the loop
        int packSize = recvfrom(m_nSocketfd, m_recvpack, sizeof(m_recvpack), 
                            0, (struct sockaddr*)&m_from_addr,(socklen_t*) &fromlen);
        
        //if error, receive again;
        if(packSize < 0)
        {
            pingStr+="recvfrom error...\n";
            perror( "recvfrom error...\n");
            continue;
        }
        
        //unpack the packet
        if(Unpack(packSize) > 0)
        {
            ++m_nRecv;
            ++m_nCnt;
        }
    }
 
}


int CPing::Unpack(int packSize)
{
   
    //the packet received is IP_packet
    struct ip* recv_ip = (struct ip*)m_recvpack;
    //IP_header : ip_hl is 4 bit , need to multi 4  //why?
    int iphdrlen = recv_ip->ip_hl << 2;
    //last len is ICMP_packet_len
    packSize -= iphdrlen;
    
    //remove the IP_Header then get ICMP_PACKET
    struct icmp *recv_icmp = (struct icmp*)(m_recvpack + iphdrlen);
    
    //ICMP_header len is 8, if the packet len less than 8, then the packet is error
    if(packSize < 8)
        return -1;
    
    //the packet is not REPLY_packet
    if(recv_icmp->icmp_type != ICMP_ECHOREPLY)
        return -1;
    
    //the packet_id is not the id which is set when send
    if(recv_icmp->icmp_id != getpid())
        return -1;
    

    struct timeval *sendtime, recvtime;
    //ICMP_data is the send_time which is set when send
    sendtime = (struct timeval*)recv_icmp->icmp_data;
    gettimeofday(&recvtime, NULL);
    tv_sub(&recvtime, sendtime);
    
    double send_to_recv_time = recvtime.tv_sec * 1000 + (double)recvtime.tv_usec / 1000;

    //the packet maybe the previous timeout packet, need to throw
    if(send_to_recv_time > m_nMaxTimeWait * 1000)
        return -1;

    printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.1lf ms", 
           packSize,
           inet_ntoa(m_dest_addr.sin_addr),
           recv_icmp->icmp_seq,
           recv_ip->ip_ttl,
           send_to_recv_time);

    char tmp[256];
    sprintf(tmp,"%d bytes from %s: icmp_seq=%d ttl=%d time=%.1lf ms", 
           packSize,
           inet_ntoa(m_dest_addr.sin_addr),
           recv_icmp->icmp_seq,
           recv_ip->ip_ttl,
           send_to_recv_time);
    
    pingStr+=tmp;
    //m_bPacketNo used to judge this ICMP_PACKET whether is repetive or not
    //if icmp_seq more than the size of m_bPacketNo, icmp_seq reduce m_nPacketNoLimit
    if(recv_icmp->icmp_seq - m_nPacketNoLimit >= MAX_PACKET_NO)
    {
        Init_PacketNoArray();
        m_nPacketNoLimit += MAX_PACKET_NO;
    }
    //judge whether there exist loop on the way to destination
    if(m_bPacketNo[recv_icmp->icmp_seq - m_nPacketNoLimit] == true)
    {
        std::cout << " DUP!\n";
        pingStr+=" DUP!\n";
        return -1;
    }
    else
    {   pingStr+= "\n";
        std::cout << "\n";
        m_bPacketNo[recv_icmp->icmp_seq - m_nPacketNoLimit] = true;
        return 1;
    }
}
           

void CPing::tv_sub(struct timeval *in, struct timeval *out)
{
    int sec = in->tv_sec - out->tv_sec;
    int usec = in->tv_usec - out->tv_usec;
    if(usec < 0)
    {
        in->tv_sec = sec - 1;
        in->tv_usec = 1000000 + usec;
    }
    else
    {
        in->tv_sec = sec;
        in->tv_usec = usec;
    }
}
    
//all data is 16bit
//sum all data, because sizeof(unsigned short) == 2, and len is calculated according to sizeof(unsigned char)
//so nleft -= 2
unsigned short CPing::Cal_ChkSum(unsigned short *addr, int len)
{
    int nleft = len;
    int sum = 0;
    unsigned short *w = addr;
    unsigned short answer = 0;

    while(nleft > 1)
    {
        sum += *w++;
        nleft -= 2;
    }
    
    if(nleft  == 1)
    {
        *(unsigned char *)(&answer) = *(unsigned char *)w;
        sum += answer;
    }
    
    //add high 16bit to low 16bit
    //treat carry
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    //negative
    answer = ~sum;

    return answer;
}


void CPing::statistics(int)
{
    gettimeofday(&m_end_tvrecv, NULL);
    tv_sub(&m_end_tvrecv, &m_begin_tvsend);
    double m_dTotalResponseTimes = m_end_tvrecv.tv_sec * 1000 + (double)m_end_tvrecv.tv_usec / 1000;
    std::cout << "-------statistics-------\n";
    printf("%d packets transmitted, %d receive, %lf%% lost, time %.3lfms\n",
           m_nSend,
           m_nRecv,
           ((double)(m_nSend - m_nRecv)) / m_nSend * 100,
            m_dTotalResponseTimes);
    close(m_nSocketfd);
    exit(0);
}


