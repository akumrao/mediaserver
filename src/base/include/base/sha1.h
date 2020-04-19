/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */


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
