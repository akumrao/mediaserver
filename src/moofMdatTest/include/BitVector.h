#ifndef _BIT_VECTOR_HH
#define _BIT_VECTOR_HH

#ifndef _BOOLEAN_HH
#include "Boolean.h"
#endif

class BitVector {
public:
  BitVector(unsigned char* baseBytePtr,
	    unsigned baseBitOffset,
	    unsigned totNumBits);

  void setup(unsigned char* baseBytePtr,
	     unsigned baseBitOffset,
	     unsigned totNumBits);

  void putBits(unsigned from, unsigned numBits); // "numBits" <= 32
  void put1Bit(unsigned bit);

  unsigned getBits(unsigned numBits); // "numBits" <= 32
  unsigned get1Bit();
  Boolean get1BitBoolean() { return get1Bit() != 0; }

  void skipBits(unsigned numBits);

  unsigned curBitIndex() const { return fCurBitIndex; }
  unsigned totNumBits() const { return fTotNumBits; }
  unsigned numBitsRemaining() const { return fTotNumBits - fCurBitIndex; }

  unsigned get_expGolomb();
      // Returns the value of the next bits, assuming that they were encoded using an exponential-Golomb code of order 0
  int get_expGolombSigned(); // signed version of the above

private:
  unsigned char* fBaseBytePtr;
  unsigned fBaseBitOffset;
  unsigned fTotNumBits;
  unsigned fCurBitIndex;
};

// A general bit copy operation:
void shiftBits(unsigned char* toBasePtr, unsigned toBitOffset,
	       unsigned char const* fromBasePtr, unsigned fromBitOffset,
	       unsigned numBits);

#endif
