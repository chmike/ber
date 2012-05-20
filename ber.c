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
 
#include "BER.h"
 
extern void bcopy( char *src, char *dst, int nbytes );
 
 
/* -- REQUIRES: tag number < 31 -- */
uchar* parse_tag  ( register uchar* x, register uchar* p ){
   *x = *p++;
   return p;
}
 
 
/* -- REQUIRES: sizeof(length) <= sizeof(uint) -- */
uchar* parse_len  ( register uint* x, register uchar* p ){
   register uchar len = *p++;
   if( (len & 0x80) == 0 ){
      *x = len;
   } else {
      register uint val = *p++;
      len &= 0x7f;
      while( --len){
	 val <<= 8;
	 val |= *p++;
      }
      *x = val;
   }
   return p;
}
 
uchar* parse_sid  ( register sid* x, register uchar* p ){
   register ulong val = *p & 0x7f;
   while( *p++ & 0x80 ){
      val <<= 7;
      val |= *p & 0x7f;
   }
   *x = val;
   return p;
}
 
 
uchar* parse_dat  ( register uchar* x, register uint l, register uchar* p ){
   bcopy( p, x, l );
   return p + l;
}
 
 
/* -- REQUIRES: sizeof(ber_int32) < 127 -- */
uchar* parse_nat32( register nat32* x, register uchar* p ){
   register uchar len = *p++;    /* assert len < 127 -- */
   register ulong val = 0;
   if( len ){
      val = *p++;
      while( --len ){
	 val <<= 8;
	 val |= *p++;
      }
   }
   *x = val;
   return p;
}
 
 
/* -- REQUIRES: sizeof(ber_int32) < 127 -- */
uchar* parse_int32( register int32* x, register uchar* p ){
   register uchar len = *p++;    /* assert len < 127 -- */
   register int val = 0;
   if( len ){
      val = (long)((char)(*p++)); /* expands the sign */
      while( --len ){
	 val <<= 8;
	 val |= *p++;
      }
   }
   *x = val;
   return p;
}
 
/* -- REQUIRES: sizeof(ber_int64) < 127 -- */
uchar* parse_nat64( register nat64  x, register uchar* p ){
   register uchar len = *p++; /* assert len < 127 */
   if( len == 0 ){
      *x++ = 0;
      *x = 0;
   } else {
      register long val = *p++;
      if( len > sizeof(ulong) ){
	 register uchar hlen = len - sizeof(ulong);
	 len = sizeof(ulong);
	 while( --hlen ){
	    val <<=  8;
	    val |= *p++;
	 }
	 *x++ = val;
	 val = *p++;
      } else {
	 *x++ = 0;
      }
      while( --len ){
	 val <<= 8;
	 val |= *p++;
      }
      *x = val;
   }
   return p;
}
 
 
 
uchar* parse_oct( register uchar* x, register uint* l, register uchar* p ){
   uint len;
   register ulong bufsize = *l;
   p = parse_len( &len, p );
   if( bufsize > len ) bufsize = len;
   bcopy( p, x, bufsize );
   *l = bufsize;
   return p + len;
}
 
 
ulong  number_sid( register uchar* p, register uint l ){
   register ulong x = 0;
   while( l-- )
      if( (char)(*p++) >= 0 ) x++;
   return x;
}
 
/* -- REQUIRES: bl > 0 -- */
int isObjectOf( uchar *a, uint al, uchar *b, uint bl ){
   if( bl-- >= al ) return 0;
   a += bl;
   b += bl;
   if( *a != *b ) return 0;
   while( bl-- )
      if( *--a != *--b )
	 return 0;
   return 1;
}
 
 
/* -- REQUIRES: tag number < 31 -- */
uchar* build_tag( register uchar x, register uchar* p ){
   *--p = x;
   return p;
}
 
 
uchar* build_len( register uint  x, register uchar* p ){
   if( x > 127 ){
      register uchar len = 0x81;
      *--p = x;
      while( x >>= 8 ){
	 *--p = x;
	 len++;
      }
      *--p = len;
   } else
      *--p = x;
   return p;
}
 
uchar* build_sid( register sid x, register uchar* p ){
   *--p = x & 0x7f;
   while( x >>= 7 )
      *--p = x | 0x80;
   return p;
}
 
 
uchar* build_dat( register uchar* x, register uint l, register uchar* p ){
   bcopy( x, p -= l, l );
   return p;
}
 
 
/* -- REQUIRES: sizeof(ber_int32) < 127 -- */
uchar* build_nat32( register nat32 x, register uchar* p ){
   register uchar len = 1;
 
   *--p = x;   /* assert x value < 2^32 */
   while( x >>= 7 ){
      *--p = x >>= 1;
      len++;
   }
   *--p = len;
   return p;
}
 
 
/* -- REQUIRES: sizeof(ber_int32) < 127 -- */
uchar* build_int32( register long x, register uchar* p ){
   register uchar len = 1;
   register ulong ctrl = ((x < 0 ) ? -x : x) >> 7;
 
#ifdef BIGLONG
   ctrl &= 0x01FFFFFF;
#endif
   *--p = x;
   while( ctrl ){
      *--p = x >>= 8;
      len++;
      ctrl >>= 8;
   }
   *--p = len;
   return p;
}
 
 
/* -- REQUIRES: sizeof(ber_int64) < 127 -- */
uchar* build_nat64( register nat64 x, register uchar* p ){
   register long A = *x++;
   register long B = *x;
 
   if( A ) {
      p -= sizeof(long);
      bcopy( (uchar*)(x), p, sizeof(long) ); /* store B */
      p = build_nat32( A, p ); /* store A */
      *p += sizeof(long); /* adjust stored length */
   } else {
      p = build_nat32( B, p ); /* store B */
   }
   return p;
}
 
 
uchar* build_oct  ( register uchar* x, register uint  l, register uchar* p ){
   bcopy( p -= l, x, l );
   return build_len( l, p );
}
 
 
/* -- REQUIRES: l > 0 -- */
uchar* build_oid  ( register sid* x, register uint l, register uchar* p ){
   register ulong *sidp = x + l, val;
   register uchar *op = p;
 
   while( l-- ){
      val = *--sidp;
      *--p = val & 0x7f;
      while( val >>= 7 )
	 *--p = val | 0x80;
   }
   return  build_len( op - p, p );
}
 
 
/* -- REQUIRES: l < 32767 -- */
int spaceFor_lentag( register uchar* buf, register uchar* p, register uint l ){
   if( l < 128 ) l = 1;
   else if( l < 256 ) l = 2;
   else l = 3;
   return ( p - buf > l );
}
 
 
int spaceFor_int( register uchar* buf, register uchar* p ){
   return( (p - buf) > (sizeof(long) + 1) );
}
 
 
int spaceFor_int64( register uchar* buf, register uchar* p ){
   return( (p - buf) > (sizeof(long)*2 + 1) );
}
 
 
/* -- REQUIRES: l < 32767 -- */
int spaceFor_oct( register uchar* buf, register uchar* p, register uint l ){
   if( l < 128 ) l += 1;
   else if( l < 256 ) l += 2;
   else l = +3;
   return ( (p - buf) > l );
}
