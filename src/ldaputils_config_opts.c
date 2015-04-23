/*
 *  LDAP Utilities
 *  Copyright (C) 2012 Bindle Binaries <syzdek@bindlebinaries.com>.
 *
 *  @BINDLE_BINARIES_BSD_LICENSE_START@
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
 *
 *  @BINDLE_BINARIES_BSD_LICENSE_END@
 */
/**
 *  @file src/ldaputils_config_opts.c  contains shared functions and variables
 */
#define _LDAP_UTILS_SRC_LDAPUTILS_CONFIG_OPTS_C 1
#include "ldaputils_config_opts.h"

///////////////
//           //
//  Headers  //
//           //
///////////////

#include <inttypes.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>


/////////////////
//             //
//  Functions  //
//             //
/////////////////

/// sets LDAP server's base DN
/// @param[in] cnf   reference to common configuration struct
/// @param[in] arg   value of the command line argument
int ldaputils_config_set_basedn(LdapUtilsConfig * cnf, const char * arg)
{
   cnf->basedn = arg;
   return(0);
}


/// sets LDAP server's bind DN
/// @param[in] cnf   reference to common configuration struct
/// @param[in] arg   value of the command line argument
int ldaputils_config_set_binddn(LdapUtilsConfig * cnf, const char * arg)
{
   cnf->binddn = arg;
   return(0);
}


/// sets LDAP server's bind password
/// @param[in] cnf   reference to common configuration struct
/// @param[in] arg   value of the command line argument
int ldaputils_config_set_bindpw(LdapUtilsConfig * cnf, const char * arg)
{
   strncpy(cnf->bindpw, arg, LDAPUTILS_OPT_LEN);
   return(0);
}


/// reads LDAP server's bind password from file
/// @param[in] cnf   reference to common configuration struct
/// @param[in] arg   value of the command line argument
int ldaputils_config_set_bindpw_file(LdapUtilsConfig * cnf, const char * arg)
{
   return(ldaputils_passfile(arg, cnf->bindpw, LDAPUTILS_OPT_LEN));
}


/// prompts for LDAP server's bind password
/// @param[in] cnf   reference to common configuration struct
/// @param[in] arg   value of the command line argument
int ldaputils_config_set_bindpw_prompt(LdapUtilsConfig * cnf)
{
   // TRANSLATORS: The following string is used as a prompt when the program
   // requests the user's LDAP bind password.
   ldaputils_getpass(_("Enter LDAP Password: "), cnf->bindpw, LDAPUTILS_OPT_LEN);
   return(0);
}


/// toggles continuous mode
/// @param[in] cnf   reference to common configuration struct
int ldaputils_config_set_continuous(LdapUtilsConfig * cnf)
{
   cnf->continuous++;
   return(0);
}


/// sets LDAP debug level
/// @param[in] cnf   reference to common configuration struct
/// @param[in] arg   value of the command line argument
int ldaputils_config_set_debug(LdapUtilsConfig * cnf, const char * arg)
{
   cnf->debug = atol(arg);
   return(0);
}


/// toggles dry run
/// @param[in] cnf   reference to common configuration struct
int ldaputils_config_set_dryrun(LdapUtilsConfig * cnf)
{
   cnf->dryrun++;
   return(0);
}


/// sets LDAP server's host name
/// @param[in] cnf   reference to common configuration struct
/// @param[in] arg   value of the command line argument
int ldaputils_config_set_host(LdapUtilsConfig * cnf, const char * arg)
{
   snprintf(cnf->uribuff, LDAPUTILS_OPT_LEN, "ldap://%s:%i/", arg, cnf->port);
   ldaputils_config_set_uri(cnf, cnf->uribuff);
   return(0);
}


/// sets LDAP TCP port
/// @param[in] cnf   reference to common configuration struct
/// @param[in] arg   value of the command line argument
int ldaputils_config_set_port(LdapUtilsConfig * cnf, const char * arg)
{
   int          port;
   const char * host;
   port = atol(arg);
   if ( (port < 1) || (port > 0xffff) )
   {
      fprintf(stderr, _("%s: invalid TCP port\n"), PROGRAM_NAME);
      fprintf(stderr, _("Try `%s --help' for more information.\n"), PROGRAM_NAME);
      return(1);
   };
   if ((cnf->host))
      host = cnf->host;
   else
      host = "";
   snprintf(cnf->uribuff, LDAPUTILS_OPT_LEN, "ldap://%s:%i/", host, port);
   ldaputils_config_set_uri(cnf, cnf->uribuff);
   return(0);
}


/// toggles following referrals
/// @param[in] cnf   reference to common configuration struct
int ldaputils_config_set_referrals(LdapUtilsConfig * cnf)
{
   if (cnf->referrals < 0)
      cnf->referrals = 0;
   cnf->referrals++;
   return(0);
}


// sets LDAP search scope
int ldaputils_config_set_scope(LdapUtilsConfig * cnf, const char * arg)
{
   if (!(strcasecmp(arg, "sub")))
      cnf->scope = LDAP_SCOPE_SUBTREE;
   else if (!(strcasecmp(arg, "one")))
      cnf->scope = LDAP_SCOPE_SUBTREE;
   else if (!(strcasecmp(arg, "base")))
      cnf->scope = LDAP_SCOPE_BASE;
   else
   {
      fprintf(stderr, _("%s: scope should be base, one, or sub\n"), PROGRAM_NAME);
      return(1);
   };
   return(0);
}


/// sets LDAP size limit
/// @param[in] cnf   reference to common configuration struct
/// @param[in] arg   value of the command line argument
int ldaputils_config_set_sizelimit(LdapUtilsConfig * cnf, const char * arg)
{
   cnf->sizelimit = atol(arg);
   return(0);
}


/// sets sort attribute
/// @param[in] cnf   reference to common configuration struct
/// @param[in] arg   value of the command line argument
int ldaputils_config_set_sortattr(LdapUtilsConfig * cnf, const char * arg)
{
   cnf->sortattr = arg;
   return(0);
}


/// sets LDAP time limit
/// @param[in] cnf   reference to common configuration struct
/// @param[in] arg   value of the command line argument
int ldaputils_config_set_timelimit(LdapUtilsConfig * cnf, const char * arg)
{
   cnf->timelimit = atol(arg);
   return(0);
}


/// sets LDAP server's URI
/// @param[in] cnf   reference to common configuration struct
/// @param[in] arg   value of the command line argument
int ldaputils_config_set_uri(LdapUtilsConfig * cnf, const char * arg)
{
   if ((cnf->ludp))
   {
      if (cnf->host == cnf->ludp->lud_host)
         cnf->host = NULL;
      ldap_free_urldesc(cnf->ludp);
   };
   cnf->ludp = NULL;
   
   if (!(cnf->uri = arg))
      return(0);
   
   if ((ldap_url_parse(arg, &cnf->ludp)))
   {
      // TRANSLATORS: The following strings provide an error message if the
      // URI provided on the command line is an invalid LDAP URI.
      fprintf(stderr, _("%s: invalid LDAP URI\n"), PROGRAM_NAME);
      fprintf(stderr, _("Try `%s --help' for more information.\n"), PROGRAM_NAME);
      return(1);
   };
   
   cnf->host  = cnf->ludp->lud_host;
   cnf->port  = cnf->ludp->lud_port;
   
   return(0);
}


/// toggles verbose mode
/// @param[in] cnf   reference to common configuration struct
int ldaputils_config_set_verbose(LdapUtilsConfig * cnf)
{
   cnf->verbose = 1;
   cnf->debug |= 0x01;
   return(0);
}


/// sets LDAP protocol version
/// @param[in] cnf   reference to common configuration struct
/// @param[in] arg   value of the command line argument
int ldaputils_config_set_version(LdapUtilsConfig * cnf, const char * arg)
{
   int i;
   i = atol(arg);
   switch(i)
   {
      case 2:
      case 3:
         cnf->version = i;
         return(0);
      default:
         // TRANSLATORS: The following strings provide an error message if the
         // LDAP protocol version specified on the command line is an invalid
         // protocol version or unsupported protocol version.
         fprintf(stderr, _("%s: protocol version should be 2 or 3\n"), PROGRAM_NAME);
         fprintf(stderr, _("Try `%s --help' for more information.\n"), PROGRAM_NAME);
         return(1);
   };
   return(0);
}


/* end of source file */
