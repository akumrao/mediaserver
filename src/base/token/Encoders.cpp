

#include "Encoders.h"



                            /*### @+prefix ************************************/
                            /*### @-prefix ************************************/
const string Base64Encoder::DIGITS_safe("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+!");
const string Base64Encoder::DIGITS_std("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");
const char Base64Encoder::PCHAR_safe = '_';
const char Base64Encoder::PCHAR_std = '=';
                            /*### @+Base64Encoder::decode {{{2 ****************/
string& Base64Encoder::decode(
	string& out,
	const string& in,
	string::size_type pos,
	string::size_type n,
	const string& digits,
	char pad)
{
	if ( pos >= in.size() )
		return out;

	string::size_type epos = in.size();
	if ( n != string::npos ) {
		if ( (epos = (pos + n)) >= in.size() )
			epos = in.size();
	}

	uint32_t outval = 0;
	uint32_t decodeChars = 0;
	uint32_t bits;
	string::size_type value;
	int outcount = 3;

	for ( ; pos < epos; ++pos ) {
		if ( in[pos] == pad ) {
			outcount--;
			value = 0;
		}
		else if ( (value = digits.find(in[pos])) == string::npos )
			continue;

		outval = (outval << 6) | value;
		if ( ++decodeChars == 4 ) {
			for ( bits = 16; outcount > 0; outcount--, bits -= 8 )
				out.append(1, static_cast<char>(outval >> bits));

			decodeChars = 0;
			outcount = 3;
			outval = 0;
		}
	}

	return out;
}
                            /*### @-Base64Encoder::decode }}}2 ****************/
                            /*### @+Base64Encoder::encode {{{2 ****************/
string& Base64Encoder::encode(
	string& out,
	const string& in,
	string::size_type pos,
	string::size_type n,
	const string& digits,
	char pad)
{
	if ( pos >= in.size() )
		return out;

	if ( n == string::npos || (pos + n) >= in.size() )
		n = in.size() - pos;

	const uint8_t* ip = reinterpret_cast<const uint8_t*>(in.data() + pos);
	char obuf[4];
	char* op;

	// go until there are less than 3 octets left for us to convert
	for ( op = obuf; n > 2; ip += 3, n -= 3, op = obuf ) {
		*op++ = digits[ ip[0] >> 2 ];
		*op++ = digits[ ((ip[0] & 0x03) << 4) | (ip[1] >> 4) ];
		*op++ = digits[ ((ip[1] & 0x0f) << 2) | (ip[2] >> 6) ];
		*op++ = digits[ ip[2] & 0x3f ];
		out.append(obuf, 4);
	}

	/* encode remaining octets and pad out multiple of 4 */
	if ( n == 2 ) {
		*op++ = digits[ ip[0] >> 2 ];
		*op++ = digits[ ((ip[0] & 0x03) << 4) | (ip[1] >> 4) ];
		*op++ = digits[ ((ip[1] & 0x0f) << 2) ];
		*op++ = pad;
		out.append(obuf, 4);
	}
	else if ( n == 1 ) {
		*op++ = digits[ ip[0] >> 2 ];
		*op++ = digits[ ((ip[0] & 0x03) << 4) ];
		*op++ = pad;
		*op++ = pad;
		out.append(obuf, 4);
	}

	return out;
}
                            /*### @-Base64Encoder::encode }}}2 ****************/
                            /*### @+HexEncoder::DIGITS_decode *****************/
const uint8_t HexEncoder::DIGITS_decode[256] =
{
	0xff,	// '^@'
	0xff,	// '^A'
	0xff,	// '^B'
	0xff,	// '^C'
	0xff,	// '^D'
	0xff,	// '^E'
	0xff,	// '^F'
	0xff,	// '^G'
	0xff,	// '^H'
	0xff,	// '^I'
	0xff,	// '^J'
	0xff,	// '^K'
	0xff,	// '^L'
	0xff,	// '^M'
	0xff,	// '^N'
	0xff,	// '^O'
	0xff,	// '^P'
	0xff,	// '^Q'
	0xff,	// '^R'
	0xff,	// '^S'
	0xff,	// '^T'
	0xff,	// '^U'
	0xff,	// '^V'
	0xff,	// '^W'
	0xff,	// '^X'
	0xff,	// '^Y'
	0xff,	// '^Z'
	0xff,	// '^['
	0xff,	// '^\'
	0xff,	// '^]'
	0xff,	// '^^'
	0xff,	// '^_'
	0xff,	// ' '
	0xff,	// '!'
	0xff,	// '"'
	0xff,	// '#'
	0xff,	// '$'
	0xff,	// '%'
	0xff,	// '&'
	0xff,	// '\''
	0xff,	// '('
	0xff,	// ')'
	0xff,	// '*'
	0xff,	// '+'
	0xff,	// ','
	0xff,	// '-'
	0xff,	// '.'
	0xff,	// '/'
	0x00,	// '0'
	0x01,	// '1'
	0x02,	// '2'
	0x03,	// '3'
	0x04,	// '4'
	0x05,	// '5'
	0x06,	// '6'
	0x07,	// '7'
	0x08,	// '8'
	0x09,	// '9'
	0xff,	// ':'
	0xff,	// ';'
	0xff,	// '<'
	0xff,	// '='
	0xff,	// '>'
	0xff,	// '?'
	0xff,	// '@'
	0x0a,	// 'A'
	0x0b,	// 'B'
	0x0c,	// 'C'
	0x0d,	// 'D'
	0x0e,	// 'E'
	0x0f,	// 'F'
	0xff,	// 'G'
	0xff,	// 'H'
	0xff,	// 'I'
	0xff,	// 'J'
	0xff,	// 'K'
	0xff,	// 'L'
	0xff,	// 'M'
	0xff,	// 'N'
	0xff,	// 'O'
	0xff,	// 'P'
	0xff,	// 'Q'
	0xff,	// 'R'
	0xff,	// 'S'
	0xff,	// 'T'
	0xff,	// 'U'
	0xff,	// 'V'
	0xff,	// 'W'
	0xff,	// 'X'
	0xff,	// 'Y'
	0xff,	// 'Z'
	0xff,	// '['
	0xff,	// '\\'
	0xff,	// ']'
	0xff,	// '^'
	0xff,	// '_'
	0xff,	// '`'
	0x0a,	// 'a'
	0x0b,	// 'b'
	0x0c,	// 'c'
	0x0d,	// 'd'
	0x0e,	// 'e'
	0x0f,	// 'f'
	0xff,	// 'g'
	0xff,	// 'h'
	0xff,	// 'i'
	0xff,	// 'j'
	0xff,	// 'k'
	0xff,	// 'l'
	0xff,	// 'm'
	0xff,	// 'n'
	0xff,	// 'o'
	0xff,	// 'p'
	0xff,	// 'q'
	0xff,	// 'r'
	0xff,	// 's'
	0xff,	// 't'
	0xff,	// 'u'
	0xff,	// 'v'
	0xff,	// 'w'
	0xff,	// 'x'
	0xff,	// 'y'
	0xff,	// 'z'
	0xff,	// '{'
	0xff,	// '|'
	0xff,	// '}'
	0xff,	// '~'
	0xff,	// '^?'
	0xff,	// '\x80'
	0xff,	// '\x81'
	0xff,	// '\x82'
	0xff,	// '\x83'
	0xff,	// '\x84'
	0xff,	// '\x85'
	0xff,	// '\x86'
	0xff,	// '\x87'
	0xff,	// '\x88'
	0xff,	// '\x89'
	0xff,	// '\x8a'
	0xff,	// '\x8b'
	0xff,	// '\x8c'
	0xff,	// '\x8d'
	0xff,	// '\x8e'
	0xff,	// '\x8f'
	0xff,	// '\x90'
	0xff,	// '\x91'
	0xff,	// '\x92'
	0xff,	// '\x93'
	0xff,	// '\x94'
	0xff,	// '\x95'
	0xff,	// '\x96'
	0xff,	// '\x97'
	0xff,	// '\x98'
	0xff,	// '\x99'
	0xff,	// '\x9a'
	0xff,	// '\x9b'
	0xff,	// '\x9c'
	0xff,	// '\x9d'
	0xff,	// '\x9e'
	0xff,	// '\x9f'
	0xff,	// '\xa0'
	0xff,	// '\xa1'
	0xff,	// '\xa2'
	0xff,	// '\xa3'
	0xff,	// '\xa4'
	0xff,	// '\xa5'
	0xff,	// '\xa6'
	0xff,	// '\xa7'
	0xff,	// '\xa8'
	0xff,	// '\xa9'
	0xff,	// '\xaa'
	0xff,	// '\xab'
	0xff,	// '\xac'
	0xff,	// '\xad'
	0xff,	// '\xae'
	0xff,	// '\xaf'
	0xff,	// '\xb0'
	0xff,	// '\xb1'
	0xff,	// '\xb2'
	0xff,	// '\xb3'
	0xff,	// '\xb4'
	0xff,	// '\xb5'
	0xff,	// '\xb6'
	0xff,	// '\xb7'
	0xff,	// '\xb8'
	0xff,	// '\xb9'
	0xff,	// '\xba'
	0xff,	// '\xbb'
	0xff,	// '\xbc'
	0xff,	// '\xbd'
	0xff,	// '\xbe'
	0xff,	// '\xbf'
	0xff,	// '\xc0'
	0xff,	// '\xc1'
	0xff,	// '\xc2'
	0xff,	// '\xc3'
	0xff,	// '\xc4'
	0xff,	// '\xc5'
	0xff,	// '\xc6'
	0xff,	// '\xc7'
	0xff,	// '\xc8'
	0xff,	// '\xc9'
	0xff,	// '\xca'
	0xff,	// '\xcb'
	0xff,	// '\xcc'
	0xff,	// '\xcd'
	0xff,	// '\xce'
	0xff,	// '\xcf'
	0xff,	// '\xd0'
	0xff,	// '\xd1'
	0xff,	// '\xd2'
	0xff,	// '\xd3'
	0xff,	// '\xd4'
	0xff,	// '\xd5'
	0xff,	// '\xd6'
	0xff,	// '\xd7'
	0xff,	// '\xd8'
	0xff,	// '\xd9'
	0xff,	// '\xda'
	0xff,	// '\xdb'
	0xff,	// '\xdc'
	0xff,	// '\xdd'
	0xff,	// '\xde'
	0xff,	// '\xdf'
	0xff,	// '\xe0'
	0xff,	// '\xe1'
	0xff,	// '\xe2'
	0xff,	// '\xe3'
	0xff,	// '\xe4'
	0xff,	// '\xe5'
	0xff,	// '\xe6'
	0xff,	// '\xe7'
	0xff,	// '\xe8'
	0xff,	// '\xe9'
	0xff,	// '\xea'
	0xff,	// '\xeb'
	0xff,	// '\xec'
	0xff,	// '\xed'
	0xff,	// '\xee'
	0xff,	// '\xef'
	0xff,	// '\xf0'
	0xff,	// '\xf1'
	0xff,	// '\xf2'
	0xff,	// '\xf3'
	0xff,	// '\xf4'
	0xff,	// '\xf5'
	0xff,	// '\xf6'
	0xff,	// '\xf7'
	0xff,	// '\xf8'
	0xff,	// '\xf9'
	0xff,	// '\xfa'
	0xff,	// '\xfb'
	0xff,	// '\xfc'
	0xff,	// '\xfd'
	0xff,	// '\xfe'
	0xff	// '\xff'
};
                            /*### @-HexEncoder::DIGITS_decode *****************/
                            /*### @+HexEncoder::decode {{{2 *******************/
string& HexEncoder::decode(
	string& out,
	const string& in,
	string::size_type pos,
	string::size_type n)
{
	if ( pos >= in.size() )
		return out;

	if ( n == string::npos || (pos + n) >= in.size() )
		n = in.size() - pos;
	if ( n % 2 )
		n -= 1;

	string::size_type epos = pos + n;
	uint8_t nhigh, nlow;
	for ( ; pos < epos; pos += 2 ) {
		if ( (nhigh = DIGITS_decode[static_cast<uint8_t>(in[pos])]) == 0xff
				|| (nlow = DIGITS_decode[static_cast<uint8_t>(in[pos + 1])]) == 0xff )
			return out;
		out.append(1, static_cast<char>((nhigh << 4) | nlow));
	}

	return out;
}
                            /*### @-HexEncoder::decode }}}2 *******************/
                            /*### @+HexEncoder::encode {{{2 *******************/
string& HexEncoder::encode(
	string& out,
	const string& in,
	bool lower,
	string::size_type pos,
	string::size_type n)
{
	const char* Dlower = "0123456789abcdef";
	const char* Dupper = "0123456789ABCDEF";
	const char* D = Dlower;
	uint8_t c;
	char obuf[2];

	if ( pos >= in.size() )
		return out;

	string::size_type epos = in.size();
	if ( n != string::npos ) {
		if ( (epos = (pos + n)) >= in.size() )
			epos = in.size();
	}

	if ( !lower )
		D = Dupper;

	for ( ; pos < epos; ++pos ) {
		c = static_cast<uint8_t>(in[pos]);
		obuf[0] = D [(c >> 4)];
		obuf[1] = D [(c & 0x0f)];
		out.append(obuf, 2);
	}

	return out;
}



                            /*### @-HexEncoder::encode }}}2 *******************/
                            /*### @+UrlEncoder::decode {{{2 *******************/

/*
string& UrlEncoder::decode(
	string& out,
	const string& in,
	string::size_type pos,
	string::size_type n)
{
	if ( pos >= in.size() )
		return out;

	string::size_type epos = in.size();
	if ( n != string::npos ) {
		if ( (epos = (pos + n)) >= in.size() )
			epos = in.size();
	}

	const CharClass& x = CharClass::XDIGIT;
	for ( ; pos < epos; ++pos ) {
		char c = in[pos];
		if ( c == '%' && pos <= epos - 3 && x.match(in[pos+1]) && x.match(in[pos+2]) ) {
			HexEncoder::decode(out, in, pos + 1, 2);
			pos += 2;
		}
		else {
			if ( c == '+' )
				out.append(1, ' ');
			else
				out.append(1, c);
		}
	}

	return out;
}
*/
                            /*### @-UrlEncoder::decode }}}2 *******************/
                            /*### @+UrlEncoder::encode {{{2 *******************/
/*
string& UrlEncoder::encode(
	string& out,
	const string& in,
	string::size_type pos,
	string::size_type n)
{
	if ( pos >= in.size() )
		return out;

	string::size_type epos = in.size();
	if ( n != string::npos ) {
		if ( (epos = (pos + n)) >= in.size() )
			epos = in.size();
	}

	const CharClass good(CharClass::DIGIT | CharClass::ALPHA);
	//const string ok("$-_.+!*'(),");
	const string ok("-_.~");
	string::size_type ep;

	do {
		for ( ep = pos; ep < epos
				&& (good.match(in[ep]) || ok.find(in[ep]) != string::npos); ++ep ) ;

		if ( ep != pos )
			out.append(in, pos, ep - pos);

		if ( ep == epos )
			return out;

		out.append(1, '%');
		HexEncoder::encode(out, in, true, ep, 1);
		pos = ep + 1;
	} while ( pos < epos );

	return out;
}
*/
                            /*### @-UrlEncoder::encode }}}2 *******************/


/* ==================== editors ====================== */

