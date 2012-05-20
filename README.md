ber
===

C Implementation of BER assembler/disassembler package.
Minimum requirement for SNMPv1/v2 implementation.
 
Christophe MEESSEN (CPPM - IN2P3)
18 feb 1994
 
Permission to use, copy,  modify and distribute this software and it's docu-
mentation for any purpose and  without fee is hereby granted,  provided that
this notice remain part of the source files. This package is provided as is,
without any support.
 
Corrections and enhancements are welcome.
Author's address: meessen@cppm.in2p3.fr

 
A note on this BER package
--------------------------
 
This package provides the primitives to compile or disassemble SNMP packets. It is aimed to be part of an SNMP implementation.
 
It's originality is in the way SNMP packets are compiled. Packets are compiled in reverse order, that is beginning from the last byte to the first one. Due to the SNMP packet structure, it simplifies the code and thus speed up the process.
 
The SNMP packet format is like an onion for which each layer is a data block. A data block is defined as the following sequence: a tag, the data length and the data itself. The problem when compiling SNMP packets is that the length of the data is only known when it is compiled and that the data length field must be encoded in varying length.
 
Thus compiling from the most inner layer toward the most outher layer is the most simple and efficient way to proceed. The type of data to compile allows such a reverse compiling. Apparently compiling the data itself is also
simplified and thus optimized.
 
Examples:
 
compiling an sid in reverse is simply:
 
*--p = x & 0x7F;
while( x >>= 7 )
   *--p = x | 0x80;
 
where x is the sid ( must be unsigned ) and p the pointer on the first byte of the compiled data.
 
compiling an unsigned integer:
 
len = 1;
*--p = x;
while( x >>= 7 ){
   *--p = x >>= 1;
   len++;
}
*--p = len;
 
But then a buffer that may contain the biggest SNMP data must be provided at the begining of compilation process. The size of such buffer is fixed (SNMPv1) or known at the beginning of the compilation (SNMPv2 party information).
 
 
Boundary checking
-----------------
 
None of the primitives does bounds checking. It is the responsability of the package user to test for boundary overflow. But some utility functions helping in this type of tests are given for compilation.
 
Usefull primitives
------------------
 
The isObjectOf primitive allows to test if the submitted OID is a sub-object or an instance of a another given OID. This is usefull when using get_next functions to test if the received OID is out of the explored object group.
 
This function implements the test by simply comparing the OID common parts, but again in reverse order. With get_next, the last bytes of an OID
will most probably differ and the first bytes will most probably be the same. So comparing by starting from the end allows to make the decision very rappidely in the context of get_next.
 
 
No OID parsing primitive
------------------------
 
It was decided not to give OID parsing primitive because OID should be kept in compact form as long as possible. For a table retrieval with get_next it is useless to unpack the OID to pack it again to compile the next get_next. The isObjectOf function is not much slower on the compact form than on the uncompacted. OID unpacking may be only pertinent for INDEX part of some OIDs. This is the reason why no parse_oid function was given. But an sid (OID sub-identifiers) counter was given that can apply on an OID part and a parse_sid function. These two primitives allows to easely parse an OID.
 
Nevertheless an OID build function using an array of unsigned long as input was given for compatibility with existent MIB engines.
 
Macro definitions
-----------------
 
Primitives where defined as functions, not as macros. It may be interesing for the build_tag or parse_tag functions to use
macros instead. Such enhancement are left to the user of this package. The reason is because the decision between speed or space priority is context dependant. It is thus left to the user's appreciation.
 
Have fun...
 
Bien cordialement,
Ch.Meessen
