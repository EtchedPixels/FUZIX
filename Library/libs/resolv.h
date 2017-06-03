//
// resolv.h
//
// DNS resolver
//
// Copyright (C) 2002 Michael Ringgaard. All rights reserved.
// Portions Copyright (C) 1996-2002  Internet Software Consortium.
// Portions Copyright (C) 1996-2001  Nominum, Inc.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 
// 1. Redistributions of source code must retain the above copyright 
//    notice, this list of conditions and the following disclaimer.  
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.  
// 3. Neither the name of the project nor the names of its contributors
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission. 
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
// SUCH DAMAGE.
// 

#ifndef RESOLV_H
#define RESOLV_H

#define NS_PACKETSZ     512             // Maximum packet size
#define NS_MAXCDNAME    255             // Maximum compressed domain name
#define NS_MAXDNAME     1025            // Maximum domain name
#define NS_CMPRSFLGS    0xc0            // Flag bits indicating name compression
#define NS_HFIXEDSZ     12              // #/bytes of fixed data in header
#define NS_QFIXEDSZ     4               // #/bytes of fixed data in query
#define NS_RRFIXEDSZ    10              // #/bytes of fixed data in r record
#define NS_DEFAULTPORT  53              // For both TCP and UDP

#define MAXHOSTNAMELEN  256

#define QUERYBUF_SIZE   1024            // Size of query buffer

//
// DNS opcodes
//

#define DNS_OP_QUERY    0       // Standard query
#define DNS_OP_IQUERY   1       // Inverse query (deprecated/unsupported)
#define DNS_OP_STATUS   2       // Name server status query (unsupported)
#define DNS_OP_NOTIFY   4       // Zone change notification
#define DNS_OP_UPDATE   5       // Zone update message

//
// DNS response codes
//

#define DNS_ERR_NOERROR   0     // No error occurred
#define DNS_ERR_FORMERR   1     // Format error
#define DNS_ERR_SERVFAIL  2     // Server failure
#define DNS_ERR_NXDOMAIN  3     // Name error
#define DNS_ERR_NOTIMPL   4     // Unimplemented
#define DNS_ERR_REFUSED   5     // Operation refused
#define DNS_ERR_YXDOMAIN  6     // Name exists
#define DNS_ERR_YXRRSET   7     // RRset exists
#define DNS_ERR_NXRRSET   8     // RRset does not exist
#define DNS_ERR_NOTAUTH   9     // Not authoritative for zone
#define DNS_ERR_NOTZONE   10    // Zone of record different from zone section

//
// DNS resource record types
//

#define DNS_TYPE_INVALID  0     // Cookie
#define DNS_TYPE_A        1     // Host address
#define DNS_TYPE_NS       2     // Authoritative server
#define DNS_TYPE_MD       3     // Mail destination
#define DNS_TYPE_MF       4     // Mail forwarder
#define DNS_TYPE_CNAME    5     // Canonical name
#define DNS_TYPE_SOA      6     // Start of authority zone
#define DNS_TYPE_MB       7     // Mailbox domain name
#define DNS_TYPE_MG       8     // Mail group member
#define DNS_TYPE_MR       9     // Mail rename name
#define DNS_TYPE_NULL     10    // Null resource record
#define DNS_TYPE_WKS      11    // Well known service
#define DNS_TYPE_PTR      12    // Domain name pointer
#define DNS_TYPE_HINFO    13    // Host information
#define DNS_TYPE_MINFO    14    // Mailbox information
#define DNS_TYPE_MX       15    // Mail routing information
#define DNS_TYPE_TXT      16    // Text strings
#define DNS_TYPE_RP       17    // Responsible person
#define DNS_TYPE_AFSDB    18    // AFS cell database
#define DNS_TYPE_X25      19    // X_25 calling address
#define DNS_TYPE_ISDN     20    // ISDN calling address
#define DNS_TYPE_RT       21    // Router
#define DNS_TYPE_NSAP     22    // NSAP address
#define DNS_TYPE_NSAP_PTR 23    // Reverse NSAP lookup (deprecated)
#define DNS_TYPE_SIG      24    // Security signature
#define DNS_TYPE_KEY      25    // Security key
#define DNS_TYPE_PX       26    // X.400 mail mapping
#define DNS_TYPE_GPOS     27    // Geographical position (withdrawn)
#define DNS_TYPE_AAAA     28    // Ip6 Address
#define DNS_TYPE_LOC      29    // Location Information
#define DNS_TYPE_NXT      30    // Next domain (security)
#define DNS_TYPE_EID      31    // Endpoint identifier
#define DNS_TYPE_NIMLOC   32    // Nimrod Locator
#define DNS_TYPE_SRV      33    // Server Selection
#define DNS_TYPE_ATMA     34    // ATM Address
#define DNS_TYPE_NAPTR    35    // Naming Authority PoinTeR
#define DNS_TYPE_KX       36    // Key Exchange
#define DNS_TYPE_CERT     37    // Certification record
#define DNS_TYPE_A6       38    // IPv6 address (deprecates AAAA)
#define DNS_TYPE_DNAME    39    // Non-terminal DNAME (for IPv6)
#define DNS_TYPE_SINK     40    // Kitchen sink (experimentatl)
#define DNS_TYPE_OPT      41    // EDNS0 option (meta-RR)

#define DNS_TYPE_TSIG     250   // Transaction signature
#define DNS_TYPE_IXFR     251   // Incremental zone transfer
#define DNS_TYPE_AXFR     252   // Transfer zone of authority
#define DNS_TYPE_MAILB    253   // Transfer mailbox records
#define DNS_TYPE_MAILA    254   // Transfer mail agent records
#define DNS_TYPE_ANY      255   // Wildcard match

//
// DNS classes
//

#define DNS_CLASS_INVALID  0    // Cookie
#define DNS_CLASS_IN       1    // Internet
#define DNS_CLASS_2        2    // Unallocated/unsupported
#define DNS_CLASS_CHAOS    3    // MIT Chaos-net
#define DNS_CLASS_HS       4    // MIT Hesiod

#define DNS_CLASS_NONE     254  // For prereq. sections in update request
#define DNS_CLASS_ANY      255  // Wildcard match

//
// DNS message header
//

/* FIXME: we need to ifdef this for the CPU bitendian packing */

struct dns_hdr {
  unsigned short id;            // Query identification number

  // Fields in third byte
  unsigned char rd : 1;         // Recursion desired
  unsigned char tc : 1;         // Truncated message
  unsigned char aa : 1;         // Authoritive answer
  unsigned char opcode : 4;     // Purpose of message
  unsigned char vqr : 1;        // Response flag

  // Fields in fourth byte
  unsigned char rcode : 4;      // Response code
  unsigned char cd: 1;          // Checking disabled by resolver
  unsigned char ad: 1;          // Authentic data from named
  unsigned char unused : 1;     // Unused bits (MBZ as of 4.9.3a3)
  unsigned char ra : 1;         // Recursion available

  // Remaining bytes
  unsigned short qdcount;       // Number of question entries
  unsigned short ancount;       // Number of answer entries
  unsigned short nscount;       // Number of authority entries
  unsigned short arcount;       // Number of resource entries
};

//
// DNS resolver state
//

#define MAXDNSRCH               6       // Max # domains in search path
#define MAXNS                   3       // Max # name servers we'll track
#define RES_TIMEOUT             5       // Min. seconds between retries
#define RES_DFLRETRY            2       // Default #/tries

struct res_state {
  unsigned long options;                  // Option flags - see below
  int retry;                              // Number of times to retransmit
  int retrans;                            // Retransmition time interval
  int nscount;                            // Number of name servers
  struct sockaddr_in nsaddr_list[MAXNS];  // Address of name server
  unsigned short id;                      // Current message id
  int ndots;                              // Threshold for initial abs. query
  char *dnsrch[MAXDNSRCH + 1];            // Components of domain to search
  char defdname[256];                     // Default domain
};

//
// Resolver options
//

#define RES_USEVC       0x00000008      // Use virtual circuit
#define RES_IGNTC       0x00000020      // Ignore trucation errors
#define RES_RECURSE     0x00000040      // Recursion desired
#define RES_DEFNAMES    0x00000080      // Use default domain name
#define RES_DNSRCH      0x00000200      // Search up local domain tree
#define RES_ROTATE      0x00004000      // Rotate ns list after each query

#define RES_DEFAULT     (RES_RECURSE | RES_DEFNAMES | RES_DNSRCH)
  
int res_init(void); 

#endif
