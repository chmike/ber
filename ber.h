/***************************************************************************
 
	   C Implementation of BER assembler/disassembler package.
	      Minimum requirement for SNMPv1/v2 implementation.
 
		    Christophe MEESSEN (CPPM - IN2P3)
			     18 feb 1994
 
 Permission to use, copy,  modify and distribute this software and it's docu-
 mentation for any purpose and  without fee is hereby granted,  provided that
 this notice remain part of the source files. This package is provided as is,
 without any support.
 
	       Corrections and enhancements are welcome.
	       Author's address: meessen@marina.in2p3.fr
 
 ****************************************************************************/
 
#ifndef __BER__
#define __BER__
 
/* -- various type tags used in SNMP -- */
 
/* -- Tag class encoding -- */
#define UNIVERSAL         0x00
#define APPLICATION       0x40
#define CONTEXTSPECIFIC   0x80
#define PRIVATE           0xC0
 
/* -- Tag primitive/contructed form -- */
#define PRIMITIVE         0x00
#define CONSTRUCT         0x20
 
/* -- SNMP Tags -- */
#define INTEGER_tag          ( UNIVERSAL   | PRIMITIVE | 0x02 )
#define OCTET_STRING_tag     ( UNIVERSAL   | PRIMITIVE | 0x04 )
#define NULL_tag             ( UNIVERSAL   | PRIMITIVE | 0x05 )
#define OBJECT_ID_tag        ( UNIVERSAL   | PRIMITIVE | 0x06 )
#define SEQUENCE_tag         ( UNIVERSAL   | CONSTRUCT | 0x10 )
#define SET_OF_tag           ( UNIVERSAL   | CONSTRUCT | 0x11 )
 
#define IPADDRESS_tag        ( APPLICATION | PRIMITIVE | 0x00 )
#define COUNTER32_tag        ( APPLICATION | PRIMITIVE | 0x01 )
#define GAUGE32_tag          ( APPLICATION | PRIMITIVE | 0x02 )
#define TIMETICKS_tag        ( APPLICATION | PRIMITIVE | 0x03 )
#define OPAQUE_tag           ( APPLICATION | PRIMITIVE | 0x04 )
#define NSAP_tag             ( APPLICATION | PRIMITIVE | 0x05 )
#define COUNTER64_tag        ( APPLICATION | PRIMITIVE | 0x06 )
#define UINTEGER32_tag       ( APPLICATION | PRIMITIVE | 0x07 )
 
#define GET_cmd              ( CONTEXTSPECIFIC | CONSTRUCT | 0x00 )
#define GET_NEXT_cmd         ( CONTEXTSPECIFIC | CONSTRUCT | 0x01 )
#define RESPONSE_cmd         ( CONTEXTSPECIFIC | CONSTRUCT | 0x02 )
#define SET_cmd              ( CONTEXTSPECIFIC | CONSTRUCT | 0x03 )
#define TRAP_V1_cmd          ( CONTEXTSPECIFIC | CONSTRUCT | 0x04 )
#define GET_BULK_cmd         ( CONTEXTSPECIFIC | CONSTRUCT | 0x05 )
#define INFORM_cmd           ( CONTEXTSPECIFIC | CONSTRUCT | 0x06 )
#define TRAP_V2_cmd          ( CONTEXTSPECIFIC | CONSTRUCT | 0x07 )
 
#define NoSuchObject_tag     ( CONTEXTSPECIFIC | PRIMITIVE | 0x00 )
#define NoSuchInstance_tag   ( CONTEXTSPECIFIC | PRIMITIVE | 0x01 )
#define EndOfMibView_tag     ( CONTEXTSPECIFIC | PRIMITIVE | 0x02 )
 
 
/*___________________________________________________________________________
 |                                                                           |
 |                     BER integer internal representation.                  |
 |___________________________________________________________________________|
 |                                                                           |
 |   Any BER integer value is stored in C long integer type. It is supposed  |
 |   by default that sizeof(long) is 4 bytes, thus 32 bits.                  |
 |                                                                           |
 |   It is asserted that BER integer number representation used in SNMP are  |
 |   UNSIGNED INTEGER OF 32 BIT SIZE :     0 <= x < 2^32  --> nat32          |
 |   SIGNED   INTEGER OF 32 BIT SIZE : -2^31 <= x < 2^31  --> int32          |
 |   UNSIGNED INTEGER OF 64 BIT SIZE :     0 <= x < 2^64  --> nat64          |
 |                                                                           |
 |   a nat32 is mapped into an unsigned long.                                |
 |   an int32 is mapped into a signed long.                                  |
 |   a nat64 is mapped into an array of two unsigned long.                   |
 |___________________________________________________________________________|
 |                                                                           |
 |   A BER object identifier is made of a sequence of sub-identifiers.       |
 |   sub-identifiers are stored as C long integer type. It is supposed       |
 |   by default that sizeof(long) is 4 bytes, thus 32 bits.                  |
 |   a sid (sub-identifier) is mapped into an unsigned long.                 |
 |___________________________________________________________________________|
*/
 
typedef unsigned long nat32;
typedef long          int32;
typedef unsigned long nat64[2];
typedef unsigned long sid;
 
/* -- utility definition -- */
typedef unsigned long  ulong;
typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;
 
 
/*___________________________________________________________________________
 |                                                                           |
 |                     BER disassembling functions                           |
 |___________________________________________________________________________|
 |                                                                           |
 |   For all disassembling functions, p is given as argument and p' is       |
 |   returned as result. p' is always bigger than p.                         |
 |        _________                                                          |
 |    ___|__VALUE__|___      p = argument, p' = returned value.              |
 |        ^p   ->   ^p'                                                      |
 |                                                                           |
 |  PARSE_TAG put at x (uchar*) the tag pointed by p.                        |
 |  PARSE_LEN put at x (uint*)  the length value pointed by p.               |
 |  PARSE_SID put at x (sid*)   the sub identifier pointed by p.             |
 |  PARSE_DAT put in array x (uchar*) the l (uint) bytes pointed by p.       |
 |___________________________________________________________________________|
 |                                                                           |
 | For the following functions, p points on the first byte of the length of  |
 | the value. p' is always bigger than p.                                    |
 |        ___________________                                                |
 |    ___|_length_|__VALUE__|___      p = argument, p' = returned value.     |
 |        ^p       ->        ^p'                                             |
 |                                                                           |
 |  PARSE_NAT32 put at x (nat32*) the nat32 value.                           |
 |  PARSE_INT32 put at x (int32*) the int32 value.                           |
 |  PARSE_INT64 put at x (nat64)  the nat64 value.                           |
 |  PARSE_OCT put at x the octet string pointed by p truncated to l bytes.   |
 |            on return l contains the octet string length.                  |
 |___________________________________________________________________________|
 |                                                                           |
 |  BER disassembling utility functions                                      |
 |                                                                           |
 |  NUMBER_SID return the number of sub-identifiers in sub-identifier list   |
 |             pointed by p and of length l.                                 |
 |                                                                           |
 |  ISOBJECTOF return true if sub-identifier list A begins with sub-         |
 |             identifier list B or sid list A equal sid list B.             |
 |___________________________________________________________________________|
*/
 
uchar* parse_tag  ( register uchar* x, register uchar* p );
uchar* parse_len  ( register uint*  x, register uchar* p );
uchar* parse_sid  ( register sid*   x, register uchar* p );
uchar* parse_dat  ( register uchar* x, register uint l, register uchar* p );
uchar* parse_nat32( register nat32* x, register uchar* p );
uchar* parse_int32( register int32* x, register uchar* p );
uchar* parse_nat64( register nat64  x, register uchar* p );
uchar* parse_oct  ( register uchar* x, register uint* l, register uchar* p );
ulong  number_sid ( register uchar* p, register uint l );
int isObjectOf( uchar *a, uint al, uchar *b, uint bl );
 
 
/*___________________________________________________________________________
 |                                                                           |
 |                        BER assembling functions                           |
 |___________________________________________________________________________|
 |                                                                           |
 |   For all assembling functions, p is given as argument, the value is      |
 |   stored in the bytes preceeding p and p' is returned as result.          |
 |   Assembling is done in reverse order, thus p' is always smaller than p.  |
 |   The user is responsible to check for buffer overflow. He may use the    |
 |   buffer overflow check utility functions or a "watch dog" pointer.       |
 |                                                                           |
 |        _________                                                          |
 |    ___|__VALUE__|___      p = argument, p' = returned value.              |
 |        ^p'  <-   ^p                                                       |
 |                                                                           |
 |  BUILD_TAG put before p the tag x (uchar).                                |
 |  BUILD_LEN put before p the length x (uint).                              |
 |  BUILD_SID put before p the sub-identifier x (sid).                       |
 |  BUILD_DAT put before p the array x (uchar*) of l (uint) bytes.           |
 |___________________________________________________________________________|
 |                                                                           |
 | For the following functions, assembing function prepend the value AND the |
 | value length. p' is always smaller than p.                                |
 |        ___________________                                                |
 |    ___|_length_|__VALUE__|___      p = argument, p' = returned value.     |
 |        ^p'      <-        ^p                                              |
 |                                                                           |
 |  BUILD_NAT32 put before p the int value x (nat32).                        |
 |  BUILD_INT32 put before p the int value x (int32).                        |
 |  BUILD_NAT64 put before p the integer64 value x (nat64).                  |
 |  BUILD_OCT put before p the array x (uchar*) of length l (uint) bytes.    |
 |  BUILD_OID put before p the oid x (ulong*) of l (uint) sid.               |
 |___________________________________________________________________________|
 |                                                                           |
 |  BER assembling utility functions                                         |
 |                                                                           |
 |  These utilities permit to check for potential buffer overflow.           |
 |  buf is a pointer on the first byte of the buffer and p a pointer on      |
 |  first byte of already compiled data. Test includes tag and value length. |
 |                                                                           |
 |  SPACEFOR_LENTAG return true if there is space for tag and length l.      |
 |  SPACEFOR_INT32  return true if there is space for the biggest int32/nat32|
 |  SPACEFOR_NAT64  return true if there is space for tag biggest nat64.     |
 |  SPACEFOR_OCT    return true if there is space for the octet string of    |
 |                  length l.                                                |
 |___________________________________________________________________________|
*/
 
uchar* build_tag  ( register uchar  x, register uchar* p );
uchar* build_len  ( register uint   x, register uchar* p );
uchar* build_sid  ( register sid    x, register uchar* p );
uchar* build_dat  ( register uchar* x, register uint l, register uchar* p );
uchar* build_nat32( register nat32  x, register uchar* p );
uchar* build_int32( register int32  x, register uchar* p );
uchar* build_nat64( register nat64  x, register uchar* p );
uchar* build_oct  ( register uchar* x, register uint  l, register uchar* p );
uchar* build_oid  ( register ulong* x, register uint  l, register uchar* p );
int spaceFor_lentag( register uchar* buf, register uchar* p, register uint l );
int spaceFor_int32 ( register uchar* buf, register uchar* p);
int spaceFor_nat64 ( register uchar* buf, register uchar* p);
int spaceFor_oct   ( register uchar* buf, register uchar* p, register uint l );
 
#endif  __BER__
