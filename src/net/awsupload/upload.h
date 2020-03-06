#define UdpDataSize 32768
#define StorageCount 100*1024

char *backstorage[StorageCount];



struct Packet{
    uint8_t type;
    uint16_t sequence_number;
    uint16_t len;
    char data[UdpDataSize];
};
