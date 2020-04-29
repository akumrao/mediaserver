/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

/* UUID 1 to 5 possible. Guid is also uuid from Microsoft
Type 3 and Type 5 UUIDs are just a technique of stuffing a hash into a UUID.

Type 1: stuffs MAC address, datetime into 128 bits
Type 3: stuffs an MD5 hash into 128 bits
Type 4: stuffs random data into 128 bits
Type 5: stuffs an SHA1 hash into 128 bits
An SHA1 hash outputs 160 bits (20 bytes). The result of the hash is converted into a UUID. From the 20-bytes from SHA1:

SHA1 Digest:   74738ff5 5367 e958 9aee 98fffdcd1876 94028007
UUID (v5):     74738ff5-5367-5958-9aee-98fffdcd1876
                             ^_low nibble is set to 5 to indicate type 5
                                  ^_first two bits set to 1 and 0, respectively
 */ 
/*  
A Universally Unique IDentifier (UUID) - also called a Global Unique IDentifier (GUID) - is a 128-bit value formatted into blocks of hexadecimal digits separated by a hyphen ("-", U+002D). A typical UUID is AA97B177-9383-4934-8543-0F91A7A02836. It doesn't matter whether the letters A-F are upper or lower case.

A version 4 UUID is defined in RFC 4122: 128 randomly-generated bits with six bits at certain positions set to particular values. For example,

AA97B177-9383-4934-8543-0F91A7A02836
              ^    ^
              1    2
The digit at position 1 above is always "4" and the digit at position 2 is always one of "8", "9", "A" or "B".

Procedure
The procedure to generate a version 4 UUID is as follows:

Generate 16 random bytes (=128 bits)
Adjust certain bits according to RFC 4122 section 4.4 as follows:
set the four most significant bits of the 7th byte to 0100'B, so the high nibble is "4"
set the two most significant bits of the 9th byte to 10'B, so the high nibble will be one of "8", "9", "A", or "B" (see Note 1).
Encode the adjusted bytes as 32 hexadecimal digits
Add four hyphen "-" characters to obtain blocks of 8, 4, 4, 4 and 12 hex digits
Output the resulting 36-character string "XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX"
 * 
 * 
 */ 

#ifndef __UUID_H
#define __UUID_H

#include <string>
#include "base/util.h"

namespace base {
namespace uuid4 {

#define NBYTES 16
#define maxchars 37
    
constexpr char hexmap[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                           '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

static std::string byteTohex(unsigned char *data, int len)
{
  std::string s(len * 2, ' ');
  for (int i = 0; i < len; ++i) {
    s[2 * i]     = hexmap[(data[i] & 0xF0) >> 4];
    s[2 * i + 1] = hexmap[data[i] & 0x0F];
  }
  return s;
}

static std::string uuid()
{
    char uuid[maxchars];

    //                                           12345678 9012 3456 7890 123456789012
    // Returns a 36-character string in the form XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX
    // where "X" is an "upper-case" hexadecimal digit [0-9A-F].
    // Use the LCase function if you want lower-case letters.

    //unsigned char abData[NBYTES];
   // char strHex[2*NBYTES+1];

    // 1. Generate 16 random bytes = 128 bits
   // RNG_Bytes(abData, NBYTES, "", 0);
    
    std::string rnd = util::randomString(NBYTES);
    
    
    unsigned char *abData =  (unsigned char *)rnd.c_str();

    // 2. Adjust certain bits according to RFC 4122 section 4.4.
    // This just means do the following
    // (a) set the high nibble of the 7th byte equal to 4 and
    // (b) set the two most significant bits of the 9th byte to 10'B,
    //     so the high nibble will be one of {8,9,A,B}.
    abData[6] = 0x40 | (abData[6] & 0xf);
    abData[8] = 0x80 | (abData[8] & 0x3f);

    // 3. Convert the adjusted bytes to hex values
   // CNV_HexStrFromBytes(strHex, sizeof(strHex)-1, abData, NBYTES);
    
    std::string hex =  byteTohex(abData, NBYTES);
            
    const char* strHex  = hex.c_str();

    // 4. Add four hyphen '-' characters
    memset(uuid, '\0', maxchars+1);
    strncpy(uuid, strHex, 8);
    strcat(uuid, "-");
    strncat(uuid, &strHex[8], 4);
    strcat(uuid, "-");
    strncat(uuid, &strHex[12], 4);
    strcat(uuid, "-");
    strncat(uuid, &strHex[16], 4);
    strcat(uuid, "-");
    strncat(uuid, &strHex[20], 12);

    // Return the UUID string
    return uuid;
}

} // namespace base64

namespace uuid5{
    
   // TBD //check the source code of websocket 
    
//            return base64::encode(util::randomString(16));
//
//
//        std::string computeAccept(const std::string& key) {
//            std::string accept(key + ProtocolGuid);
//            
//           // crypto::Hash engine("SHA1");
//           // engine.update(key + ProtocolGuid);
//           //  return base64::encode(engine.digest());
//            
//            sha1::SHA1_CTX context;
//            sha1::reid_SHA1_Init(&context);
//            sha1::reid_SHA1_Update(&context, (uint8_t*)accept.c_str(), accept.length());
//            std::vector<uint8_t> digest(SHA1_DIGEST_SIZE);
//            sha1::reid_SHA1_Final(&context, &digest[0]);
//            return base64::encode(digest);
    
}//uuid5

} // namespace base


#endif /* __SHA1_H */
