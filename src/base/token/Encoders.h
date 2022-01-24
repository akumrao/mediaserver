#ifndef _Encoders_hxx
#define _Encoders_hxx 1



//#include "cif_stdc.h"


                            /*### @+prefix ************************************/
#include <string>
                            /*### @-prefix ************************************/
using namespace std;

// Classes defined here
class Base64Encoder;
class HexEncoder;
class UrlEncoder;
                            /*### @class Base64Encoder {{{2 *******************/

/*!
** Base64 encoding routines
*/
class Base64Encoder
{
	public:

		/*! HTTP-safe base64 encoding chars. */
		static const string DIGITS_safe;

		/*! Standard base64 encoding chars. */
		static const string DIGITS_std;

		/*! HTTP-safe base64 padding char. */
		static const char PCHAR_safe;

		/*! Standard base64 padding char. */
		static const char PCHAR_std;

		/*! Append base64-decoded version of in[pos,+n] onto out */
		static string& decode(
			string& out,
			const string& in,
			string::size_type pos = 0,
			string::size_type n = string::npos,
			const string& digits = Base64Encoder::DIGITS_std,
			char pad = Base64Encoder::PCHAR_std);

		/*! Append base64-encoded version of in[pos,+n] onto out */
		static string& encode(
			string& out,
			const string& in,
			string::size_type pos = 0,
			string::size_type n = string::npos,
			const string& digits = Base64Encoder::DIGITS_std,
			char pad = Base64Encoder::PCHAR_std);

		/*! Append (url-safe) base64-decoded version of in[pos,+n] onto out */
		static string& decodeUrl(string& out, const string& in, string::size_type pos = 0, string::size_type n = string::npos) {
			return decode(out, in, pos, n, DIGITS_safe, PCHAR_safe);
		}

		/*! Append (url-safe) base64-encoded version of in[pos,+n] onto out */
		static string& encodeUrl(string& out, const string& in, string::size_type pos = 0, string::size_type n = string::npos) {
			return encode(out, in, pos, n, DIGITS_safe, PCHAR_safe);
		}
};
                            /*### @class Base64Encoder }}}2 *******************/
                            /*### @class HexEncoder {{{2 **********************/

/*!
** Hex encoding routines
*/
class HexEncoder
{
	public:

		/*! Append hex-decoded version of in[pos,+n] onto out */
		static string& decode(
			string& out,
			const string& in,
			string::size_type pos = 0,
			string::size_type n = string::npos);

		/*! Decode single hex digit */
		static uint8_t decodeChar(char c) {
			return DIGITS_decode[static_cast<uint8_t>(c)];
		}

		/*! Append hex-encoded version of in[pos,+n] onto out */
		static string& encode(
			string& out,
			const string& in,
			bool lower = true,
			string::size_type pos = 0,
			string::size_type n = string::npos);

	private:

		/*! Mapping of hex digits to nibble value */
		static const uint8_t DIGITS_decode[256];
};
                            /*### @class HexEncoder }}}2 **********************/
                            /*### @class UrlEncoder {{{2 **********************/

/*!
** URL encoding routines
*/
//class UrlEncoder
//{
//	public:
//
//		/*! Append url-decoded version of in[pos,+n] onto out */
//		static string& decode(
//			string& out,
//			const string& in,
//			string::size_type pos = 0,
//			string::size_type n = string::npos);
//
//		/*! Append url-encoded version of in[pos,+n] onto out */
//		static string& encode(
//			string& out,
//			const string& in,
//			string::size_type pos = 0,
//			string::size_type n = string::npos);
//};
                            /*### @class UrlEncoder }}}2 **********************/

#endif // !_Encoders_hxx


/* ==================== editors ====================== */

