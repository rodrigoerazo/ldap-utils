
* Add support for auth mechanisms other than simple auth.
* Add an ldapdiff tool.
* Add an ldap2script tool.
* Add documentation.
* Add support for LDAP version 2 by updating ldaputils_initialize() to support
  a bind function which is supports LDAPv2.  ldaputils_initialize() currently
  uses ldap_sasl_bind_s() with LDAP_SASL_SIMPLE, however this results in an
  error message indicating that the method is unsupported if connecting via
  LDAPv2.