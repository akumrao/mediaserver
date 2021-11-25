#ifndef _BOOLEAN_HH
#define _BOOLEAN_HH

#if defined(__BORLANDC__) || (!defined(USE_LIVE555_BOOLEAN) && defined(_MSC_VER) &&  _MSC_VER >= 1400)
// Use the "bool" type defined by the Borland compiler, and MSVC++ 8.0, Visual Studio 2005 and higher
typedef bool Boolean;
#define False false
#define True true
#else
typedef unsigned char Boolean;
#ifndef __MSHTML_LIBRARY_DEFINED__
#ifndef False
const Boolean False = 0;
#endif
#ifndef True
const Boolean True = 1;
#endif

#endif
#endif

#endif
