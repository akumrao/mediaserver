/* public api for steve reid's public domain SHA-1 implementation */
/* this file is in the public domain */

#ifndef __SHA1_H
#define __SHA1_H

namespace base {
namespace sha1 {

struct SHA1_CTX {
    uint32_t state[5];
    uint32_t count[2];
    uint8_t  buffer[64];
};

#define SHA1_DIGEST_SIZE 20

void reid_SHA1_Init(SHA1_CTX* context);
void reid_SHA1_Update(SHA1_CTX* context, const uint8_t* data, const size_t len);
void reid_SHA1_Final(SHA1_CTX* context, uint8_t digest[SHA1_DIGEST_SIZE]);

} // namespace base64
} // namespace base


#endif /* __SHA1_H */
