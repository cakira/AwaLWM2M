/** @file awa_clientd_cmdline.h
 *  @brief The header file for the command line option parser
 *  generated by GNU Gengetopt version 2.22.6
 *  http://www.gnu.org/software/gengetopt.
 *  DO NOT modify this file, since it can be overwritten
 *  @author GNU Gengetopt by Lorenzo Bettini */

#ifndef AWA_CLIENTD_CMDLINE_H
#define AWA_CLIENTD_CMDLINE_H

/* If we use autoconf.  */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h> /* for FILE */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef CMDLINE_PARSER_PACKAGE
/** @brief the program name (used for printing errors) */
#define CMDLINE_PARSER_PACKAGE "awa_clientd"
#endif

#ifndef CMDLINE_PARSER_PACKAGE_NAME
/** @brief the complete program name (used for help and version) */
#define CMDLINE_PARSER_PACKAGE_NAME "awa_clientd"
#endif

#ifndef CMDLINE_PARSER_VERSION
/** @brief the program version */
#define CMDLINE_PARSER_VERSION VERSION
#endif

/** @brief Where the command line options are stored */
struct gengetopt_args_info
{
  const char *help_help; /**< @brief Print help and exit help description.  */
  int port_arg;	/**< @brief Use local port number PORT for CoAP communications (default='6000').  */
  char * port_orig;	/**< @brief Use local port number PORT for CoAP communications original value given at command line.  */
  const char *port_help; /**< @brief Use local port number PORT for CoAP communications help description.  */
  int addressFamily_arg;	/**< @brief Address family for network interface. AF=4 for IPv4, AF=6 for IPv6 (default='4').  */
  char * addressFamily_orig;	/**< @brief Address family for network interface. AF=4 for IPv4, AF=6 for IPv6 original value given at command line.  */
  const char *addressFamily_help; /**< @brief Address family for network interface. AF=4 for IPv4, AF=6 for IPv6 help description.  */
  int ipcPort_arg;	/**< @brief Use port number PORT for IPC communications (default='12345').  */
  char * ipcPort_orig;	/**< @brief Use port number PORT for IPC communications original value given at command line.  */
  const char *ipcPort_help; /**< @brief Use port number PORT for IPC communications help description.  */
  char * endPointName_arg;	/**< @brief Use NAME as client end point name (default='Awa Client').  */
  char * endPointName_orig;	/**< @brief Use NAME as client end point name original value given at command line.  */
  const char *endPointName_help; /**< @brief Use NAME as client end point name help description.  */
  char * bootstrap_arg;	/**< @brief Use bootstrap server URI.  */
  char * bootstrap_orig;	/**< @brief Use bootstrap server URI original value given at command line.  */
  const char *bootstrap_help; /**< @brief Use bootstrap server URI help description.  */
  char * factoryBootstrap_arg;	/**< @brief Load factory bootstrap information from FILE.  */
  char * factoryBootstrap_orig;	/**< @brief Load factory bootstrap information from FILE original value given at command line.  */
  const char *factoryBootstrap_help; /**< @brief Load factory bootstrap information from FILE help description.  */
  int secure_flag;	/**< @brief CoAP communications are secured with DTLS (default=off).  */
  const char *secure_help; /**< @brief CoAP communications are secured with DTLS help description.  */
  char ** objDefs_arg;	/**< @brief Load object and resource definitions from FILE.  */
  char ** objDefs_orig;	/**< @brief Load object and resource definitions from FILE original value given at command line.  */
  unsigned int objDefs_min; /**< @brief Load object and resource definitions from FILE's minimum occurreces */
  unsigned int objDefs_max; /**< @brief Load object and resource definitions from FILE's maximum occurreces */
  const char *objDefs_help; /**< @brief Load object and resource definitions from FILE help description.  */
  int daemonize_flag;	/**< @brief Detach process from terminal and run in the background (default=off).  */
  const char *daemonize_help; /**< @brief Detach process from terminal and run in the background help description.  */
  int verbose_flag;	/**< @brief Generate verbose output (default=off).  */
  const char *verbose_help; /**< @brief Generate verbose output help description.  */
  char * logFile_arg;	/**< @brief Log output to FILE.  */
  char * logFile_orig;	/**< @brief Log output to FILE original value given at command line.  */
  const char *logFile_help; /**< @brief Log output to FILE help description.  */
  int version_flag;	/**< @brief Print version and exit (default=off).  */
  const char *version_help; /**< @brief Print version and exit help description.  */
  
  unsigned int help_given ;	/**< @brief Whether help was given.  */
  unsigned int port_given ;	/**< @brief Whether port was given.  */
  unsigned int addressFamily_given ;	/**< @brief Whether addressFamily was given.  */
  unsigned int ipcPort_given ;	/**< @brief Whether ipcPort was given.  */
  unsigned int endPointName_given ;	/**< @brief Whether endPointName was given.  */
  unsigned int bootstrap_given ;	/**< @brief Whether bootstrap was given.  */
  unsigned int factoryBootstrap_given ;	/**< @brief Whether factoryBootstrap was given.  */
  unsigned int secure_given ;	/**< @brief Whether secure was given.  */
  unsigned int objDefs_given ;	/**< @brief Whether objDefs was given.  */
  unsigned int daemonize_given ;	/**< @brief Whether daemonize was given.  */
  unsigned int verbose_given ;	/**< @brief Whether verbose was given.  */
  unsigned int logFile_given ;	/**< @brief Whether logFile was given.  */
  unsigned int version_given ;	/**< @brief Whether version was given.  */

  char **inputs ; /**< @brief unamed options (options without names) */
  unsigned inputs_num ; /**< @brief unamed options number */
} ;

/** @brief The additional parameters to pass to parser functions */
struct cmdline_parser_params
{
  int override; /**< @brief whether to override possibly already present options (default 0) */
  int initialize; /**< @brief whether to initialize the option structure gengetopt_args_info (default 1) */
  int check_required; /**< @brief whether to check that all required options were provided (default 1) */
  int check_ambiguity; /**< @brief whether to check for options already specified in the option structure gengetopt_args_info (default 0) */
  int print_errors; /**< @brief whether getopt_long should print an error message for a bad option (default 1) */
} ;

/** @brief the purpose string of the program */
extern const char *gengetopt_args_info_purpose;
/** @brief the usage string of the program */
extern const char *gengetopt_args_info_usage;
/** @brief the description string of the program */
extern const char *gengetopt_args_info_description;
/** @brief all the lines making the help output */
extern const char *gengetopt_args_info_help[];

/**
 * The command line parser
 * @param argc the number of command line options
 * @param argv the command line options
 * @param args_info the structure where option information will be stored
 * @return 0 if everything went fine, NON 0 if an error took place
 */
int cmdline_parser (int argc, char **argv,
  struct gengetopt_args_info *args_info);

/**
 * The command line parser (version with additional parameters - deprecated)
 * @param argc the number of command line options
 * @param argv the command line options
 * @param args_info the structure where option information will be stored
 * @param override whether to override possibly already present options
 * @param initialize whether to initialize the option structure my_args_info
 * @param check_required whether to check that all required options were provided
 * @return 0 if everything went fine, NON 0 if an error took place
 * @deprecated use cmdline_parser_ext() instead
 */
int cmdline_parser2 (int argc, char **argv,
  struct gengetopt_args_info *args_info,
  int override, int initialize, int check_required);

/**
 * The command line parser (version with additional parameters)
 * @param argc the number of command line options
 * @param argv the command line options
 * @param args_info the structure where option information will be stored
 * @param params additional parameters for the parser
 * @return 0 if everything went fine, NON 0 if an error took place
 */
int cmdline_parser_ext (int argc, char **argv,
  struct gengetopt_args_info *args_info,
  struct cmdline_parser_params *params);

/**
 * Save the contents of the option struct into an already open FILE stream.
 * @param outfile the stream where to dump options
 * @param args_info the option struct to dump
 * @return 0 if everything went fine, NON 0 if an error took place
 */
int cmdline_parser_dump(FILE *outfile,
  struct gengetopt_args_info *args_info);

/**
 * Save the contents of the option struct into a (text) file.
 * This file can be read by the config file parser (if generated by gengetopt)
 * @param filename the file where to save
 * @param args_info the option struct to save
 * @return 0 if everything went fine, NON 0 if an error took place
 */
int cmdline_parser_file_save(const char *filename,
  struct gengetopt_args_info *args_info);

/**
 * Print the help
 */
void cmdline_parser_print_help(void);
/**
 * Print the version
 */
void cmdline_parser_print_version(void);

/**
 * Initializes all the fields a cmdline_parser_params structure 
 * to their default values
 * @param params the structure to initialize
 */
void cmdline_parser_params_init(struct cmdline_parser_params *params);

/**
 * Allocates dynamically a cmdline_parser_params structure and initializes
 * all its fields to their default values
 * @return the created and initialized cmdline_parser_params structure
 */
struct cmdline_parser_params *cmdline_parser_params_create(void);

/**
 * Initializes the passed gengetopt_args_info structure's fields
 * (also set default values for options that have a default)
 * @param args_info the structure to initialize
 */
void cmdline_parser_init (struct gengetopt_args_info *args_info);
/**
 * Deallocates the string fields of the gengetopt_args_info structure
 * (but does not deallocate the structure itself)
 * @param args_info the structure to deallocate
 */
void cmdline_parser_free (struct gengetopt_args_info *args_info);

/**
 * Checks that all the required options were specified
 * @param args_info the structure to check
 * @param prog_name the name of the program that will be used to print
 *   possible errors
 * @return
 */
int cmdline_parser_required (struct gengetopt_args_info *args_info,
  const char *prog_name);

extern const char *cmdline_parser_addressFamily_values[];  /**< @brief Possible values for addressFamily. */


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* AWA_CLIENTD_CMDLINE_H */
