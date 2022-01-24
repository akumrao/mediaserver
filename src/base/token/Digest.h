#ifndef _Digest_hxx
#define _Digest_hxx 1

#include <stdint.h>

#include <string>

using namespace std;

                            /*### @+prefix ************************************/
#ifndef MD5_DIGESTSIZE
# define MD5_DIGESTSIZE 16
#endif

#ifndef SHA1_DIGESTSIZE
# define SHA1_DIGESTSIZE 20
#endif
                            /*### @-prefix ************************************/

// Classes defined here
class DigestMD5;
class DigestSHA1;
                            /*### @class DigestMD5 {{{2 ***********************/

/*!
** For generating MD5 digests of input data.
*/
class DigestMD5
{
	public:

		/*!
		** Initialization
		*/
		DigestMD5();

		/*!
		** Empty dtor.
		*/
		~DigestMD5();

		/*!
		** Finalize
		*/
		void final(uint8_t digest[MD5_DIGESTSIZE]);

		/*!
		** Reset our parameters
		*/
		DigestMD5& init();

		/*!
		** Add more data.
		*/
		DigestMD5& update(
			const void* inbuf,
			uint32_t len);

		/*!
		** Add data from file
		*/
		bool update(
			string& emsg,
			const string& fname,
			off_t offset = 0);

	private:
		/*! No copying */
		DigestMD5(const DigestMD5&);
		DigestMD5& operator=(const DigestMD5&);

		/*!
		** Convert from u32 array to u8
		*/
		void u32to8(
			uint8_t* out,
			const uint32_t* in,
			uint32_t nbytes);

		/*!
		** Convert from u32 array to u8
		*/
		void u8to32(
			uint32_t* out,
			const uint8_t* in,
			uint32_t nbytes);

		/*!
		** The meat of the engine
		*/
		void xform(const uint8_t buf[64]);

		/*! For padding data out. */
		static const uint8_t PADDING[64];

		/*! Input buffer. */
		uint8_t mBuffer[64];

		/*! Number of bits mod 2^64. */
		uint32_t mCount[2];

		/*! State (ABCD). */
		uint32_t mState[4];
};
                            /*### @class DigestMD5 }}}2 ***********************/
                            /*### @class DigestSHA1 {{{2 **********************/

/*!
** For generating SHA1 digests of input data.
*/
class DigestSHA1
{
	public:

		/*!
		** Initialization
		*/
		DigestSHA1();

		/*!
		** Empty dtor.
		*/
		~DigestSHA1();

		/*!
		** Finalize
		*/
		void final(uint8_t digest[SHA1_DIGESTSIZE]);

		/*!
		** Reset our parameters
		*/
		DigestSHA1& init();

		/*!
		** Add more data.
		*/
		DigestSHA1& update(
			const void* ibuf,
			uint32_t len);

		/*!
		** Add data from file
		*/
		bool update(
			string& emsg,
			const string& fname,
			off_t offset = 0);

	private:
		/*! No copying */
		DigestSHA1(const DigestSHA1&);
		DigestSHA1& operator=(const DigestSHA1&);

		/*!
		** The meat of the engine
		*/
		void xform(const uint8_t buf[64]);

		/*! Working buffer. */
		uint8_t mBuffer[64];

		/*! Byte counts */
		uint32_t mCount[2];

		/*! Digest state (H0 - H4) */
		uint32_t mState[5];
};
                            /*### @class DigestSHA1 }}}2 **********************/

#endif // !_Digest_hxx


/* ==================== editors ====================== */

