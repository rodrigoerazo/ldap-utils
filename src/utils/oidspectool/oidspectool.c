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
/**
 *  @file lib/libldapschema/oidspectool/oidspectool.c creates compilation of
 *  OID specifications
 */
/*
 *  Simple Build:
 *     export CFLAGS='-DPROGRAM_NAME="oidspectool" -Wall -I../include'
 *     yacc -d          oidspecparser.y
 *     mv   -f  y.tab.c oidspecparser.c
 *     mv       y.tab.h oidspecparser.h
 *     lex  -t          oidspeclexer.l > oidspeclexer.c
 *     gcc ${CFLAGS} -c oidspectool.c
 *     gcc ${CFLAGS} -c oidspecparser.c
 *     gcc ${CFLAGS} -c oidspeclexer.c
 *     gcc ${CFLAGS} -o oidspectool -ll oidspectool.o oidspecparser.o \
 *         oidspeclexer.o
 *
 *  Libtool Build:
 *     export CFLAGS='-DPROGRAM_NAME="oidspectool" -Wall -I../include'
 *     yacc -d          oidspecparser.y
 *     mv   -f  y.tab.c oidspecparser.c
 *     mv       y.tab.h oidspecparser.h
 *     lex  -t          oidspeclexer.l > oidspeclexer.c
 *     libtool --mode=compile --tag=CC gcc ${CFLAGS} -c oidspectool.c
 *     libtool --mode=compile --tag=CC gcc ${CFLAGS} -c oidspecparser.c
 *     libtool --mode=compile --tag=CC gcc ${CFLAGS} -c oidspeclexer.c
 *     libtool --mode=link    --tag=CC gcc ${CFLAGS} -o oidspectool -ll \
 *         oidspectool.o oidspecparser.o oidspeclexer.o
 *
 *  Libtool Clean:
 *     libtool --mode=clean rm -f oidspectool.lo oidspecparser.lo \
 *         oidspeclexer.lo oidspectool
 */
#define _LDAP_UTILS_SRC_OIDSPECTOOL 1
#include "oidspectool.h"

///////////////
//           //
//  Headers  //
//           //
///////////////
#pragma mark - Headers

#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <getopt.h>
#include <assert.h>
#include <dirent.h>
#include <sys/stat.h>


///////////////////
//               //
//  Definitions  //
//               //
///////////////////
#pragma mark - Definitions


/////////////////
//             //
//  Datatypes  //
//             //
/////////////////
#pragma mark - Datatypes

struct my_config
{
   int            dryrun;
   int            format;
   const char   * output;
   int            verbose;
};

typedef struct my_config MyConfig;

struct my_oidspec
{
   char     * filename;
   int        lineno;
   char    ** oid;
   char    ** name;
   char    ** desc;
   char    ** examples;
   char    ** flags;
   char    ** type;
   char    ** class;
   char    ** def;
   char    ** abnf;
   char    ** re_posix;
   char    ** re_pcre;
   char    ** spec;
   char    ** spec_type;
   char    ** spec_name;
   char    ** spec_section;
   char    ** spec_source;
   char    ** spec_vendor;
};
typedef struct my_oidspec OIDSpec;


/////////////////
//             //
//  Variables  //
//             //
/////////////////
#pragma mark - Variables

char           ** string_queue;
const char      * cur_filename;
OIDSpec         * cur_oidspec;
OIDSpec        ** oidspeclist;
size_t            oidspeclist_len;
char           ** filelist;
size_t            filelist_len;

MyConfig          config;

//////////////////
//              //
//  Prototypes  //
//              //
//////////////////
#pragma mark - Prototypes

// main statement
int main(int argc, char * argv[]);

// tests filename string for specified extension
int my_fs_filename_ext(const char * nam, const char * ext);

// process individual OID spec file
int my_fs_parsefile(MyConfig * cnf, const char * file);

// process path for OID spec files
int my_fs_scanpath(MyConfig * cnf, const char * path);

// free memory from OID specification
void my_oidspec_free(OIDSpec * oidspec);

// free array of strings
void my_oidspec_free_strs(char ** strs);

// allocate memory for OID specifications and initialize values
OIDSpec * my_oidspec_alloc(void);

// save list of OID specifications
int my_save(MyConfig * cnf, int argc, char **argv);

// save list of OID specifications as C source file
int my_save_c(MyConfig * cnf, int argc, char **argv, FILE * fs);

// save individual OID specification
int my_save_c_oidspec(FILE * fs, OIDSpec * oidspec, size_t idx);

// save OID specification field as bit flags
int my_save_c_oidspec_flgs(FILE * fs, const char * fld, char ** vals);

// save OID specification field as const strings
int my_save_c_oidspec_strs(FILE * fs, const char * fld, char ** vals);

// save list of OID specifications as C header file
int my_save_h(MyConfig * cnf, int argc, char **argv, FILE * fs);

// save list of OID spec files as Makefile include
int my_save_makefile(MyConfig * cnf, int argc, char **argv, FILE * fs);

// prints program usage and exits
void my_usage(void);

// displays version information
void my_version(void);

// compares two OID specifications for sort order
int oidspec_cmp( const void * p1, const void * p2 );


/////////////////
//             //
//  Functions  //
//             //
/////////////////
#pragma mark - Functions

/// main statement
/// @param[in] argc   number of arguments
/// @param[in] argv   array of arguments
int main(int argc, char * argv[])
{
   int            c;
   int            err;
   int            opt_index;

   static char          short_options[]   = "hHmno:vV";
   static struct option long_options[]    =
   {
      {"help",          no_argument, 0, 'h'},
      {"header",        no_argument, 0, 'H'},
      {"makefile",      no_argument, 0, 'm'},
      {"dryrun",        no_argument, 0, 'n'},
      {"verbose",       no_argument, 0, 'v'},
      {"version",       no_argument, 0, 'V'},
      {NULL,            0,           0, 0  }
   };

   bzero(&config, sizeof(config));
   config.format = 'c';

   // loops through args
   while((c = getopt_long(argc, argv, short_options, long_options, &opt_index)) != -1)
   {
      switch(c)
      {
         case -1:       /* no more arguments */
         case 0:        /* long options toggles */
         break;

         case 'h':
         my_usage();
         return(0);

         case 'H':
         config.format = 'h';
         break;

         case 'm':
         config.format = 'm';
         break;

         case 'n':
         config.dryrun++;
         break;

         case 'o':
         config.output = optarg;
         break;

         case 'V':
         my_version();
         return(0);

         case 'v':
         config.verbose++;
         break;

         // argument error
         case '?':
         fprintf(stderr, "Try `%s --help' for more information.\n", PROGRAM_NAME);
         return(1);

         // unknown argument error
         default:
         fprintf(stderr, "%s: unrecognized option `--%c'\n", PROGRAM_NAME, c);
         fprintf(stderr, "Try `%s --help' for more information.\n", PROGRAM_NAME);
         return(1);
      };
   };
   if ((argc - optind) < 1)
   {
      fprintf(stderr, "%s: missing required argument\n", PROGRAM_NAME);
      fprintf(stderr, "Try `%s --help' for more information.\n", PROGRAM_NAME);
      return(1);
   };
   if ( (!(config.dryrun)) && (!(config.output)) )
   {
      fprintf(stderr, "%s: missing required options `-o' or `-n'\n", PROGRAM_NAME);
      fprintf(stderr, "Try `%s --help' for more information.\n", PROGRAM_NAME);
      return(1);
   };

   // initialize global variables
   string_queue      = NULL;
   oidspeclist       = NULL;
   oidspeclist_len   = 0;
   filelist          = NULL;
   filelist_len      = 0;
   if ((cur_oidspec = my_oidspec_alloc()) == NULL)
      return(2);
   if ((oidspeclist = malloc(sizeof(OIDSpec *))) == NULL)
      return(2);
   oidspeclist[0] = NULL;
   if ((filelist = malloc(sizeof(char *))) == NULL)
      return(2);
   filelist[0] = NULL;


   // loops through files
   while ((argc - optind))
   {
      if ((err = my_fs_scanpath(&config, argv[optind])) != 0)
         return(err);
      optind++;
   };


   // prints result
   if ((err = my_save(&config, argc, argv)) != 0)
      return(1);
   if ((config.verbose))
   {
      printf("stats: %zu files parsed\n", filelist_len);
      printf("stats: %zu OID specifications indexed\n", oidspeclist_len);
   };

   return(0);
}


/// tests filename string for specified extension
/// @param[in] nam     file name
/// @param[in] ext     file extension
int my_fs_filename_ext(const char * nam, const char * ext)
{
   size_t namlen = strlen(nam);
   size_t extlen = strlen(ext);
   if (namlen < extlen)
      return(1);
   return(strcasecmp(ext, &nam[namlen-extlen]));
}


/// process individual OID spec file
/// @param[in] cnf    configuration information
/// @param[in] file   OID specification file to process
int my_fs_parsefile(MyConfig * cnf, const char * file)
{
   size_t   size;
   void   * ptr;
   FILE   * fs;
   int      err;

   // append file to file list
   size = sizeof(char *) * (filelist_len+2);
   if ((ptr = realloc(filelist, size)) == NULL)
   {
      fprintf(stderr, "%s: out of virtual memory\n", PROGRAM_NAME);
      return(1);
   };
   filelist = ptr;
   if ((filelist[filelist_len] = strdup(file)) == NULL)
   {
      fprintf(stderr, "%s: out of virtual memory\n", PROGRAM_NAME);
      return(1);
   };
   filelist_len++;
   filelist[filelist_len] = NULL;

   // open file for parsing
   if ((cnf->verbose))
      printf("parsing \"%s\" ...\n", file);
   if ((fs = fopen(file, "r")) == NULL)
   {
      fprintf(stderr, "%s: %s: %s\n", PROGRAM_NAME, file, strerror(errno));
      return(1);
   };

   cur_filename = file;

   yyrestart(fs);
   err = yyparse();

   // close file
   fclose(fs);

   return(err);
}


/// process path for spec files
/// @param[in] cnf    configuration information
/// @param[in] path   file system path to process for OID specification files
int my_fs_scanpath(MyConfig * cnf, const char * path)
{
   DIR                * dir;
   struct dirent      * dp;
   struct stat          sb;
   int                  err;
   char                 filename[512];

   // check type of file
   if ((err = stat(path, &sb)) == -1)
   {
      fprintf(stderr, "%s: %s: %s\n", PROGRAM_NAME, path, strerror(errno));
      return(1);
   };
   switch(sb.st_mode & S_IFMT)
   {
      case S_IFREG: return(my_fs_parsefile(cnf, path));
      case S_IFDIR: break;
      default:
      fprintf(stderr, "%s: %s: not a regular file or directory\n", PROGRAM_NAME, path);
      return(1);
      break;
   };

   // open directory
   if ((cnf->verbose))
      printf("scanning \"%s\" ...\n", path);
   if ((dir = opendir(path)) == NULL)
   {
      fprintf(stderr, "%s: %s: %s\n", PROGRAM_NAME, path, strerror(errno));
      return(1);
   };

   // read directory
   while((dp = readdir(dir)) != NULL)
   {
      // skip hidden files
      if (dp->d_name[0] == '.')
         continue;

      // build path
      snprintf(filename, sizeof(filename), "%s/%s", path, dp->d_name);

      // stat files
      if ((err = stat(filename, &sb)) == -1)
      {
         fprintf(stderr, "%s: %s: %s\n", PROGRAM_NAME, filename, strerror(errno));
         return(1);
      };
      if ((sb.st_mode & S_IFMT) != S_IFREG)
         continue;

      // check for supported file extensions
      if ( ((my_fs_filename_ext(filename, ".oidspec"))) &&
           ((my_fs_filename_ext(filename, ".oidspec.c"))) )
         continue;

      // parse file
      if ((err = my_fs_parsefile(cnf, filename)) != 0)
         return(err);
   };

   // close directory
   closedir(dir);

   return(0);
}



/// allocate memory for OID specifications and initialize values
OIDSpec * my_oidspec_alloc(void)
{
   OIDSpec * oidspec;

   if ((oidspec = malloc(sizeof(OIDSpec))) == NULL)
   {
      fprintf(stderr, "%s: out of virtual memory\n", PROGRAM_NAME);
      return(NULL);
   };
   bzero(oidspec, sizeof(OIDSpec));

   return(oidspec);
}


/// free memory from OID specification
/// @param[in] oidspec    reference to OID specification memory
void my_oidspec_free(OIDSpec * oidspec)
{
   if (!(oidspec))
      return;

   if ((oidspec->filename))
      free(oidspec->filename);

   my_oidspec_free_strs(oidspec->oid);
   my_oidspec_free_strs(oidspec->name);
   my_oidspec_free_strs(oidspec->desc);
   my_oidspec_free_strs(oidspec->examples);
   my_oidspec_free_strs(oidspec->flags);
   my_oidspec_free_strs(oidspec->type);
   my_oidspec_free_strs(oidspec->class);
   my_oidspec_free_strs(oidspec->def);
   my_oidspec_free_strs(oidspec->abnf);
   my_oidspec_free_strs(oidspec->re_posix);
   my_oidspec_free_strs(oidspec->re_pcre);
   my_oidspec_free_strs(oidspec->spec);
   my_oidspec_free_strs(oidspec->spec_type);
   my_oidspec_free_strs(oidspec->spec_name);
   my_oidspec_free_strs(oidspec->spec_section);
   my_oidspec_free_strs(oidspec->spec_source);
   my_oidspec_free_strs(oidspec->spec_vendor);

   free(oidspec);

   return;
}


/// free array of strings
/// @param[in] strs    reference to array of strings
void my_oidspec_free_strs(char ** strs)
{
   size_t pos;
   if (!(strs))
      return;
   for(pos = 0; ((strs[pos])); pos++)
      free(strs[pos]);
   free(strs);
   return;
}


/// save list of OID specifications as C source file
/// @param[in] cnf    configuration information
/// @param[in] argc   number of arguments
/// @param[in] argv   array of arguments
int my_save(MyConfig * cnf, int argc, char **argv)
{
   FILE         * fs;

   if ((cnf->dryrun))
      return(0);

   // open file for writing
   fs = stdout;
   if ( ((cnf->output)) && ((strcmp("-", cnf->output))) )
   {
      if ((cnf->verbose))
         printf("saving results to \"%s\" ...\n", cnf->output);
      if ((fs = fopen(cnf->output, "w")) == NULL)
      {
         fprintf(stderr, "%s: fopen: %s: %s\n", PROGRAM_NAME, cnf->output, strerror(errno));
         return(1);
      };
   };

   switch(cnf->format)
   {
         case 'h': my_save_h(cnf, argc, argv, fs); break;
         case 'm': my_save_makefile(cnf, argc, argv, fs); break;
         default:  my_save_c(cnf, argc, argv, fs); break;
   };

   // closes file
   if (fs != stdout)
      fclose(fs);

   return(0);
}


/// save list of OID specifications as C source file
/// @param[in] cnf    configuration information
/// @param[in] argc   number of arguments
/// @param[in] argv   array of arguments
/// @param[in] fs     FILE stream of output file
int my_save_c(MyConfig * cnf, int argc, char **argv, FILE * fs)
{
   size_t         pos;
   char           buff[256];
   time_t         timer;
   struct tm    * tm_info;

   assert(cnf != NULL);

   // print header
   fprintf(fs, "//\n");
   time(&timer);
   tm_info = localtime(&timer);
   strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", tm_info);
   fprintf(fs, "// Generated on:   %s\n", buff);
   fprintf(fs, "// Generated with:");
   for(pos = 0; pos < (size_t)argc; pos++)
      fprintf(fs, " %s", argv[pos]);
   fprintf(fs, "\n");
   fprintf(fs, "//\n");
   fprintf(fs, "\n");
   fprintf(fs, "#include <stdio.h>\n");
   fprintf(fs, "#include \"lspecdata.h\"\n");
   fprintf(fs, "\n");

   // sort OIDs
   qsort(oidspeclist, oidspeclist_len, sizeof(OIDSpec *), oidspec_cmp);

   // save OID specs
   for(pos = 0; pos < oidspeclist_len; pos++)
      my_save_c_oidspec(fs, oidspeclist[pos], pos);

   // generate array
   fprintf(fs, "const size_t ldapschema_oidspecs_len = %zu;\n", oidspeclist_len);
   fprintf(fs, "const struct ldapschema_spec * ldapschema_oidspecs[] =\n");
   fprintf(fs, "{\n");
   for(pos = 0; pos < oidspeclist_len; pos++)
      fprintf(fs, "  &oidspec%zu, // %s\n", pos, oidspeclist[pos]->oid[0]);
   fprintf(fs, "  NULL\n");
   fprintf(fs, "};\n");
   fprintf(fs, "\n");

   // prints footer
   fprintf(fs, "/* end of source */\n");

   return(0);
}


/// save individual OID specification
/// @param[in] fs         FILE stream of output file
/// @param[in] oidspec    OID specification to save
/// @param[in] idx        index or ID of OID specification
int my_save_c_oidspec(FILE * fs, OIDSpec * oidspec, size_t idx)
{
   size_t pos;

   fprintf(fs, "// %s\n", oidspec->oid[0]);
   fprintf(fs, "// %s:%i\n", oidspec->filename, oidspec->lineno);
   fprintf(fs, "const struct ldapschema_spec oidspec%zu =\n", idx);
   fprintf(fs, "{\n");
   my_save_c_oidspec_strs(fs, ".oid",            oidspec->oid);
   my_save_c_oidspec_strs(fs, ".name",           oidspec->name);
   my_save_c_oidspec_strs(fs, ".desc",           oidspec->desc);
   my_save_c_oidspec_flgs(fs, ".flags",          oidspec->flags);
   my_save_c_oidspec_flgs(fs, ".type",           oidspec->type);
   my_save_c_oidspec_flgs(fs, ".class",          oidspec->class);
   my_save_c_oidspec_strs(fs, ".def",            oidspec->def);
   my_save_c_oidspec_strs(fs, ".abnf",           oidspec->abnf);
   my_save_c_oidspec_strs(fs, ".re_posix",       oidspec->re_posix);
   my_save_c_oidspec_strs(fs, ".re_pcre",        oidspec->re_pcre);
   my_save_c_oidspec_strs(fs, ".spec",           oidspec->spec);
   my_save_c_oidspec_strs(fs, ".spec_type",      oidspec->spec_type);
   my_save_c_oidspec_strs(fs, ".spec_name",      oidspec->spec_name);
   my_save_c_oidspec_strs(fs, ".spec_section",   oidspec->spec_section);
   my_save_c_oidspec_strs(fs, ".spec_source",    oidspec->spec_source);
   my_save_c_oidspec_strs(fs, ".spec_vendor",    oidspec->spec_vendor);
   fprintf(fs, "   %-15s =", ".examples");
   if ((oidspec->examples))
   {
      fprintf(fs, " (const char *[])\n");
      fprintf(fs, "%20s {\n", "");
      for(pos = 0; ((oidspec->examples[pos])); pos++)
         fprintf(fs, "%20s    %s,\n", "", oidspec->examples[pos]);
      fprintf(fs, "%20s    NULL,\n", "");
      fprintf(fs, "%20s },\n", "");
   } else
   {
      fprintf(fs, " NULL,\n");
   };
   fprintf(fs, "};\n\n\n");

   return(0);
}


/// save OID specification field as bit flags
/// @param[in] fs     FILE stream of output file
/// @param[in] fld    name of field
/// @param[in] vals   array of values
int my_save_c_oidspec_flgs(FILE * fs, const char * fld, char ** vals)
{
   size_t pos;

   fprintf(fs, "   %-15s =", fld);
   if ( (!(vals)) || (!(vals[0])) )
   {
      fprintf(fs, " 0,\n");
      return(0);
   };

   fprintf(fs, " %s", vals[0]);
   for(pos = 1; ((vals[pos])); pos++)
      fprintf(fs, "| %s", vals[pos]);
   fprintf(fs, ",\n");

   return(0);
}


/// save OID specification field as const strings
/// @param[in] fs     FILE stream of output file
/// @param[in] fld    name of field
/// @param[in] vals   array of values
int my_save_c_oidspec_strs(FILE * fs, const char * fld, char ** vals)
{
   size_t pos;

   fprintf(fs, "   %-15s =", fld);
   if ( (!(vals)) || (!(vals[0])) )
   {
      fprintf(fs, " NULL,\n");
      return(0);
   };

   fprintf(fs, " %s", vals[0]);
   for(pos = 1; ((vals[pos])); pos++)
      fprintf(fs, "\n%20s %s", "", vals[pos]);
   fprintf(fs, ",\n");

   return(0);
}


// save list of OID specifications as C header file
/// @param[in] cnf    configuration information
/// @param[in] argc   number of arguments
/// @param[in] argv   array of arguments
/// @param[in] fs     FILE stream of output file
int my_save_h(MyConfig * cnf, int argc, char **argv, FILE * fs)
{
   size_t         pos;
   char           buff[256];
   time_t         timer;
   struct tm    * tm_info;

   assert(cnf != NULL);

   // print header
   fprintf(fs, "//\n");
   time(&timer);
   tm_info = localtime(&timer);
   strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", tm_info);
   fprintf(fs, "// Generated on:   %s\n", buff);
   fprintf(fs, "// Generated with:");
   for(pos = 0; pos < (size_t)argc; pos++)
      fprintf(fs, " %s", argv[pos]);
   fprintf(fs, "\n");
   fprintf(fs, "//\n");
   fprintf(fs, "\n");
   fprintf(fs, "#include <stdio.h>\n");
   fprintf(fs, "#include \"lspec.h\"\n");
   fprintf(fs, "\n");

   // save OID specs
   for(pos = 0; pos < oidspeclist_len; pos++)
      fprintf(fs, "extern const struct ldapschema_spec oidspec%zu;\n", pos);
   fprintf(fs, "extern const size_t ldapschema_oidspecs_len;\n");
   fprintf(fs, "extern const struct ldapschema_spec * ldapschema_oidspecs[];\n");
   fprintf(fs, "/* end of header */\n");

   return(0);
}


/// save list of OID spec files as Makefile include
/// @param[in] cnf    configuration information
/// @param[in] argc   number of arguments
/// @param[in] argv   array of arguments
/// @param[in] fs     FILE stream of output file
int my_save_makefile(MyConfig * cnf, int argc, char **argv, FILE * fs)
{
   size_t         pos;
   char           buff[256];
   time_t         timer;
   struct tm    * tm_info;

   assert(cnf != NULL);

   // print header
   fprintf(fs, "#\n");
   time(&timer);
   tm_info = localtime(&timer);
   strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", tm_info);
   fprintf(fs, "# Generated on:   %s\n", buff);
   fprintf(fs, "# Generated with:");
   for(pos = 0; pos < (size_t)argc; pos++)
      fprintf(fs, " %s", argv[pos]);
   fprintf(fs, "\n");
   fprintf(fs, "#\n");
   fprintf(fs, "\n");

   // save file list
   fprintf(fs, "OIDSPEC_FILES =\n");
   for(pos = 0; pos < filelist_len; pos++)
      fprintf(fs, "OIDSPEC_FILES += %s\n", filelist[pos]);

   return(0);
}



/// prints program usage and exits
void my_usage(void)
{
   printf("Usage: %s [options] [file ...]\n", PROGRAM_NAME);
   printf("       %s [options] [dir ...]\n", PROGRAM_NAME);
   printf("Options:\n");
   printf("  -h, --help                print this help and exit\n");
   printf("  -H, --header              output C header instead of C source\n");
   printf("  -m, --makefile            output makefile include instead of C source\n");
   printf("  -n, --dryrun              show what would be done, but do nothing\n");
   printf("  -o file                   output file\n");
   printf("  -v, --verbose             run in verbose mode\n");
   printf("  -V, --version             print version number and exit\n");
   printf("\n");
   return;
}


/// displays version information
void my_version(void)
{
   printf("%s (%s) %s\n", PROGRAM_NAME, PACKAGE_NAME, PACKAGE_VERSION);
#ifdef PACKAGE_COPYRIGHT
   printf("%s\n", PACKAGE_COPYRIGHT);
#endif
   return;
}


/// append string to array of queued strings
/// @param[in] str     C string to append to queue
int my_yyappend(const char * str)
{
   size_t      len;
   void      * ptr;

   assert(str != NULL);

   // increase size of array
   if ((string_queue))
   {
      for(len = 0; ((string_queue[len])); len++);
      if ((ptr = realloc(string_queue, (sizeof(char *) * (len+2)))) == NULL)
      {
         fprintf(stderr, "%s: out of virtual memory\n", PROGRAM_NAME);
         exit(EXIT_FAILURE);
      };
   } else
   {
      len = 0;
      if ((ptr = malloc((sizeof(char *) * 2))) == NULL)
      {
         fprintf(stderr, "%s: out of virtual memory\n", PROGRAM_NAME);
         exit(EXIT_FAILURE);
      };
   };
   string_queue         = ptr;
   string_queue[len+0]  = NULL;
   string_queue[len+1]  = NULL;

   // duplicate string
   if ((string_queue[len] = strdup(str)) == NULL)
   {
      fprintf(stderr, "%s: out of virtual memory\n", PROGRAM_NAME);
      exit(EXIT_FAILURE);
   };

   return(0);
}


/// commits queued strings to field
/// @param[in] type    Yacc token of field
int my_yycommit(enum yytokentype type)
{
   const char      * name;
   char          *** vals;

   vals = NULL;
   switch(type)
   {
      case FLD_ABNF:           name = ".abnf";          vals = &cur_oidspec->abnf;         break;
      case FLD_CLASS:          name = ".class";         vals = &cur_oidspec->class;        break;
      case FLD_DEF:            name = ".def";           vals = &cur_oidspec->def;          break;
      case FLD_DESC:           name = ".desc";          vals = &cur_oidspec->desc;         break;
      case FLD_EXAMPLES:       name = ".examples";      vals = &cur_oidspec->examples;     break;
      case FLD_FLAGS:          name = ".flags";         vals = &cur_oidspec->flags;        break;
      case FLD_NAME:           name = ".name";          vals = &cur_oidspec->name;         break;
      case FLD_OID:            name = ".oid";           vals = &cur_oidspec->oid;          break;
      case FLD_RE_POSIX:       name = ".re_posix";      vals = &cur_oidspec->re_posix;     break;
      case FLD_RE_PCRE:        name = ".re_pcre";       vals = &cur_oidspec->re_pcre;      break;
      case FLD_SPEC:           name = ".spec";          vals = &cur_oidspec->spec;         break;
      case FLD_SPEC_NAME:      name = ".spec_name";     vals = &cur_oidspec->spec_name;    break;
      case FLD_SPEC_SECTION:   name = ".spec_section";  vals = &cur_oidspec->spec_section; break;
      case FLD_SPEC_SOURCE:    name = ".spec_source";   vals = &cur_oidspec->spec_source;  break;
      case FLD_SPEC_TYPE:      name = ".spec_type";     vals = &cur_oidspec->spec_type;    break;
      case FLD_SPEC_VENDOR:    name = ".spec_vendor";   vals = &cur_oidspec->spec_vendor;  break;
      case FLD_TYPE:           name = ".type";          vals = &cur_oidspec->type;         break;
      default:
      fprintf(stderr, "%s: %s: %i: encountered unknown token\n", PROGRAM_NAME, cur_filename, yylineno);
      exit(1);
      break;
   };

   // saves values
   if ((*vals))
   {
      fprintf(stderr, "%s: %s: %i: duplicate %s field in spec\n", PROGRAM_NAME, cur_filename, yylineno, name);
      exit(1);
   };
   *vals = string_queue;
   string_queue = NULL;

   return(0);
}


/// validates OID spec and appends to list of OID specs
int my_yyoidspec(void)
{
   size_t         pos;
   void         * ptr;

   // checks current OID spec
   if (!(cur_oidspec->oid))
   {
      fprintf(stderr, "%s: %s: %i: spec missing .oid field\n", PROGRAM_NAME, cur_filename, yylineno);
      return(1);
   };
   if (!(cur_oidspec->type))
   {
      fprintf(stderr, "%s: %s: %i: spec missing .type field\n", PROGRAM_NAME, cur_filename, yylineno);
      return(1);
   };
   if (!(cur_oidspec->desc))
   {
      fprintf(stderr, "%s: %s: %i: spec missing .desc field\n", PROGRAM_NAME, cur_filename, yylineno);
      return(1);
   };
   if ((config.verbose))
      printf("adding %s (%s) ...\n", cur_oidspec->oid[0], cur_oidspec->desc[0]);

   // searches for duplicate (I know, I know, I am being lazy)
   for(pos = 0; pos < oidspeclist_len; pos++)
   {
      if (!(strcasecmp(oidspeclist[pos]->oid[0], cur_oidspec->oid[0])))
      {
         fprintf(stderr, "%s: %s: %i: duplicate entry for %s\n", PROGRAM_NAME, cur_filename, yylineno, oidspeclist[pos]->oid[0]);
         fprintf(stderr, "%s: %s: %i: duplicate entry for %s\n", PROGRAM_NAME, oidspeclist[pos]->filename, oidspeclist[pos]->lineno, cur_oidspec->oid[0]);
         exit(1);
      };
   };

   // saves file information
   cur_oidspec->lineno = yylineno;
   if ((cur_oidspec->filename = strdup(cur_filename)) == NULL)
   {
      fprintf(stderr, "%s: out of virtual memory\n", PROGRAM_NAME);
      exit(EXIT_FAILURE);
   };

   // increase size of OID spec list
   if ((ptr = realloc(oidspeclist, (sizeof(OIDSpec *)*(oidspeclist_len+2)))) == NULL)
   {
      fprintf(stderr, "%s: out of virtual memory\n", PROGRAM_NAME);
      exit(EXIT_FAILURE);
   };
   oidspeclist = ptr;
   oidspeclist[oidspeclist_len+0] = cur_oidspec;
   oidspeclist[oidspeclist_len+1] = NULL;
   oidspeclist_len++;

   // allocates next OID spec
   if ((cur_oidspec = my_oidspec_alloc()) == NULL)
   {
      fprintf(stderr, "%s: out of virtual memory\n", PROGRAM_NAME);
      exit(EXIT_FAILURE);
   };

   return(0);
}


/// appends string to array of queued strings then commits queue to field
/// @param[in] type    Yacc token of field
/// @param[in] str     C string to append to queue
int my_yysubmit(enum yytokentype type, const char * str)
{
   my_yyappend(str);
   return(my_yycommit(type));
}


/// compares two OID specifications for sort order
/// @param[in] p1   reference to first OID specification
/// @param[in] p2   reference to second OID specification
int oidspec_cmp( const void * p1, const void * p2 )
{
   const OIDSpec * const * o1 = p1;
   const OIDSpec * const * o2 = p2;
   return(strcasecmp((*o1)->oid[0], (*o2)->oid[0]));
}


void yyerror (char *s)
{
   fprintf(stderr, "%s: %s: %i: %s\n", PROGRAM_NAME, cur_filename, yylineno, s);
   return;
}

/* end of source file */
