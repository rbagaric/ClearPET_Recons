//
// version.h
//

#define VER_MAJOR	0
#define VER_MINOR	10
#define VER_STRING "0.10"

#ifdef RC_INVOKED
// This is for compiling version resources in Windows. Just a nicer text.
#define VER_COPYRIGHT "Copyright � 2008 raytest Isotopenme�ger�te GmbH"
#else
// This is pure ASCII text, compatible to everyone
#define VER_COPYRIGHT "Copyright (C) 2008 raytest Isotopenmessgeraete GmbH"
#endif
