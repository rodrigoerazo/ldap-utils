/*
 *  LDAP Utilities
 *  Copyright (C) 2019 David M. Syzdek <david@syzdek.net>.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *     1. Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *
 *     2. Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *
 *     3. Neither the name of the copyright holder nor the names of its
 *        contributors may be used to endorse or promote products derived from
 *        this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 *  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 *  include/ldapschema.h - common includes and prototypes
 */
#ifndef __LDAPSCHEMA_H
#define __LDAPSCHEMA_H 1

///////////////
//           //
//  Headers  //
//           //
///////////////
#pragma mark - Headers

#ifdef WIN32
#include <windows.h>
#endif

#ifdef __APPLE__
#  include "TargetConditionals.h"
#  ifdef TARGET_OS_MAC
#     include <libkern/OSAtomic.h>
#  endif
#endif

#include <inttypes.h>
#include <regex.h>
#include <ldap.h>


//////////////
//          //
//  Macros  //
//          //
//////////////
#pragma mark - Macros

// Exports function type
#undef LDAPSCHEMA_C_DECLS
#undef LDAPSCHEMA_BEGIN_C_DECLS
#undef LDAPSCHEMA_END_C_DECLS
#undef _LDAPSCHEMA_I
#undef _LDAPSCHEMA_F
#undef _LDAPSCHEMA_V
#if defined(__cplusplus) || defined(c_plusplus)
#   define _LDAPSCHEMA_I             extern "C" inline
#   define LDAPSCHEMA_C_DECLS        "C"             ///< exports as C functions
#   define LDAPSCHEMA_BEGIN_C_DECLS  extern "C" {    ///< exports as C functions
#   define LDAPSCHEMA_END_C_DECLS    }               ///< exports as C functions
#else
#   define _LDAPSCHEMA_I             inline
#   define LDAPSCHEMA_C_DECLS        /* empty */     ///< exports as C functions
#   define LDAPSCHEMA_BEGIN_C_DECLS  /* empty */     ///< exports as C functions
#   define LDAPSCHEMA_END_C_DECLS    /* empty */     ///< exports as C functions
#endif
#ifdef WIN32
#   ifdef _LIB_LIBLDAPSCHEMA_H
#      define _LDAPSCHEMA_F   extern LDAPSCHEMA_C_DECLS __declspec(dllexport)   ///< used for library calls
#      define _LDAPSCHEMA_V   extern LDAPSCHEMA_C_DECLS __declspec(dllexport)   ///< used for library calls
#   else
#      define _LDAPSCHEMA_F   extern LDAPSCHEMA_C_DECLS __declspec(dllimport)   ///< used for library calls
#      define _LDAPSCHEMA_V   extern LDAPSCHEMA_C_DECLS __declspec(dllimport)   ///< used for library calls
#   endif
#else
#   ifdef _LIB_LIBLDAPSCHEMA_H
#      define _LDAPSCHEMA_F   /* empty */                                      ///< used for library calls
#      define _LDAPSCHEMA_V   extern LDAPSCHEMA_C_DECLS                         ///< used for library calls
#   else
#      define _LDAPSCHEMA_F   extern LDAPSCHEMA_C_DECLS                         ///< used for library calls
#      define _LDAPSCHEMA_V   extern LDAPSCHEMA_C_DECLS                         ///< used for library calls
#   endif
#endif


///////////////////
//               //
//  Definitions  //
//               //
///////////////////
#pragma mark - Definitions

// result codes
#define LDAPSCHEMA_SUCCESS                            0x00     ///< operation was successful
#define LDAPSCHEMA_INVALID_DEFINITION                 0x7001   ///< invalid defintion
#define LDAPSCHEMA_DUPLICATE                          0x7002   ///< duplicate defintion
#define LDAPSCHEMA_NO_MEMORY                          (-10)    ///< an memory allocation failed

// model flags
#define LDAPSCHEMA_O_SINGLEVALUE                      0x0001   ///< attributeType: is single value
#define LDAPSCHEMA_O_OBSOLETE                         0x0002
#define LDAPSCHEMA_O_COLLECTIVE                       0x0004   ///< attributeType:
#define LDAPSCHEMA_O_NO_USER_MOD                      0x0008   ///< attributeType: is readonly
#define LDAPSCHEMA_O_OBJECTCLASS                      0x0010   ///< attributeType: is objectClass
#define LDAPSCHEMA_O_READABLE                         0x0020   ///< ldapSyntax: is human readable
#define LDAPSCHEMA_O_COMMON_ABNF                      0x0040   ///< ldapSyntax: uses common ABNF

// objectclass types
#define LDAPSCHEMA_STRUCTURAL                         0x0000
#define LDAPSCHEMA_ABSTRACT                           0x0001
#define LDAPSCHEMA_AUXILIARY                          0x0002

// attribute type usage
#define LDAPSCHEMA_USER_APP                           0x0000   ///< AttributeType usage User Applications
#define LDAPSCHEMA_DIRECTORY_OP                       0x0001   ///< AttributeType usage Directory Operation
#define LDAPSCHEMA_DISTRIBUTED_OP                     0x0002   ///< AttributeType usage Distributed Operation
#define LDAPSCHEMA_DSA_OP                             0x0003   ///< AttributeType usage DSA Operation

// LDAP schema data type
#define LDAPSCHEMA_TYPE_MASK                          0xF000
#define LDAPSCHEMA_SUBTYPE_MASK                       0x000F
#define LDAPSCHEMA_SYNTAX                             0x1000
#define LDAPSCHEMA_MATCHINGRULE                       0x2000
#define LDAPSCHEMA_ATTRIBUTETYPE                      0x3000
#define LDAPSCHEMA_OBJECTCLASS                        0x4000
#define LDAPSCHEMA_TYPE( val )                        (val & LDAPSCHEMA_TYPE_MASK )
#define LDAPSCHEMA_SUBTYPE( val )                     (val & LDAPSCHEMA_SUBTYPE_MASK )
#define LDAPSCHEMA_IS_TYPE( val, type )               ( LDAPSCHEMA_TYPE(val)         == type )
#define LDAPSCHEMA_IS_SUBTYPE( val, type )            ( LDAPSCHEMA_SUBTYPE_MASK(val) == type )


/////////////////
//             //
//  Datatypes  //
//             //
/////////////////
#pragma mark - Datatypes

/// LDAP schema descriptor state
typedef struct ldap_schema LDAPSchema;

/// LDAP schema base data model
typedef struct ldap_schema_model LDAPSchemaModel;

/// LDAP schema syntax
typedef struct ldap_schema_syntax LDAPSchemaSyntax;

typedef struct ldap_schema_objectclass LDAPSchemaObjectclass;

typedef struct ldap_schema_attributetype LDAPSchemaAttributeType;

typedef struct ldap_schema_matchingrule LDAPSchemaMatchingRule;

typedef struct ldap_schema_extension LDAPSchemaExtension;


enum ldap_schema_syntax_class
{
   LDAPSchemaSyntaxUnknown        = 0,
   LDAPSchemaSyntaxASCII          = 1,
   LDAPSchemaSyntaxUTF8           = 2,
   LDAPSchemaSyntaxInteger        = 3,
   LDAPSchemaSyntaxUnsigned       = 4,
   LDAPSchemaSyntaxBoolean        = 5,
   LDAPSchemaSyntaxData           = 6,
   LDAPSchemaSyntaxImage          = 7,
   LDAPSchemaSyntaxAudio          = 8,
   LDAPSchemaSyntaxUTF8MultiLine  = 9
};
typedef enum ldap_schema_syntax_class LDAPSchemaSyntaxClass;

/// LDAP schema extension
struct ldap_schema_extension
{
   char             * extension;
   char            ** values;
   size_t             values_len;
};


/// LDAP schema base data model
struct ldap_schema_model
{
   size_t             size;        ///< size of data struct
   uint32_t           type;        ///< LDAP schema data type
   uint32_t           flags;
   char             * definition;  ///< defintion of object
   char             * oid;         ///< oid of object
   char             * desc;        ///< description of object;
   LDAPSchemaExtension  ** extensions;
   size_t                  extensions_len;
};

/// LDAP schema syntax
struct ldap_schema_syntax
{
   LDAPSchemaModel         model;
   //LDAPSchemaSyntaxClass   data_class;
   //int                     readable;
   //int                     uses_abnf;
   const char            * pattern;
   const char            * source;
   const char            * abnf;
   regex_t                 re;
};

/// LDAP schema objectclass
struct ldap_schema_objectclass
{
   LDAPSchemaModel              model;
   //int                          kind;
   //size_t                       obsolete;
   size_t                       names_len;
   size_t                       must_len;
   size_t                       may_len;
   size_t                       all_must_len;
   size_t                       all_may_len;
   LDAPSchemaObjectclass      * sup;
   char                      ** names;
   LDAPSchemaAttributeType   ** must;
   LDAPSchemaAttributeType   ** may;
   LDAPSchemaAttributeType   ** all_must;
   LDAPSchemaAttributeType   ** all_may;
};

struct ldap_schema_attributetype
{
   LDAPSchemaModel                   model;
   LDAPSchemaSyntax                * syntax;
   uint64_t                          flags;
   int                               usage;
   int                               pad0;
   //uint8_t                           obsolete;
   //uint8_t                           single;
   //uint8_t                           collective;
   //uint8_t                           no_user_mod;
   //uint8_t                           is_objectclass;
   size_t                            names_len;
   size_t                            allowed_by_len;
   size_t                            required_by_len;
   uintmax_t                         min_upper;
   char                           ** names;
   LDAPSchemaObjectclass          ** allowed_by;
   LDAPSchemaObjectclass          ** required_by;
};


//////////////////
//              //
//  Prototypes  //
//              //
//////////////////
#pragma mark - Prototypes
LDAPSCHEMA_BEGIN_C_DECLS

//-----------------//
// error functions //
//-----------------//
#pragma mark error functions

_LDAPSCHEMA_F const char *
ldapschema_err2string(
         int                     err );

_LDAPSCHEMA_F int
ldapschema_errno(
         LDAPSchema            * lsd );


//-----------------//
// LDAP functions //
//-----------------//
#pragma mark LDAP functions

int
ldapschema_fetch(
         LDAPSchema            * lsd,
         LDAP                  * ld );


//-----------------//
// lexer functions //
//-----------------//
#pragma mark lexer functions

_LDAPSCHEMA_F LDAPSchemaAttributeType *
ldapschema_parse_attributetype(
         LDAPSchema            * lsd,
         const struct berval   * def );

_LDAPSCHEMA_F LDAPSchemaSyntax *
ldapschema_parse_syntax(
         LDAPSchema            * lsd,
         const struct berval   * def );


//------------------//
// memory functions //
//------------------//
#pragma mark memory functions

_LDAPSCHEMA_F int
ldap_count_values(
         char ** vals );

_LDAPSCHEMA_F int
ldap_count_values_len(
         struct berval        ** vals );

_LDAPSCHEMA_F void
ldapschema_free(
         LDAPSchema            * lsd );

_LDAPSCHEMA_F int
ldapschema_initialize(
         LDAPSchema           ** lsdp );

_LDAPSCHEMA_F void
ldapschema_value_free(
         char                 ** vals );

_LDAPSCHEMA_F void
ldapschema_value_free_len(
         struct berval        ** vals );


//----------------//
// sort functions //
//----------------//
#pragma mark sort functions

_LDAPSCHEMA_F int
ldapschema_extension_cmp(
         const void            * e1,
         const void            * e2 );

_LDAPSCHEMA_F int
ldapschema_model_cmp(
         const void            * m1,
         const void            * m2 );

_LDAPSCHEMA_F int
ldapschema_syntax_cmp(
         const LDAPSchemaSyntax * s1,
         const LDAPSchemaSyntax * s2 );


LDAPSCHEMA_END_C_DECLS
#endif /* end of header */