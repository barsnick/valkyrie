/*---------------------------------------------------------------------- 
  Main include file                                         vk_include.h
  ----------------------------------------------------------------------
*/

#ifndef __VK_INCLUDE_H
#define __VK_INCLUDE_H


/* This will eventually be replaced by configure.in -------------------- */
#define VALKYRIE_PATH   "/home/de/Programs/valkyrie/trunk"
#define EXEC_PREFIX     VALKYRIE_PATH 
#define DOCS_PATH       "/docs/"
#define ICONS_PATH      "/icons/"
/* this lot are kept in ~/PACKAGE-X.X.X/ */
#define DBASE_DIR  "/dbase"
#define LOGS_DIR   "/logs"
#define SUPPS_DIR  "/suppressions"

/* The product name ---------------------------------------------------- */
#define vk_name   "valkyrie"
#define Vk_Name   "Valkyrie"
#define VK_NAME   "VALKYRIE"

/* The product version, as 'MAJOR-NUMBER.MINOR-NUMBER[.PATCHLEVEL]'.
   A version in the form 'YYYY-MM-DD' is a release-of-the-day, 
   i.e. a snapshot of the current development tree --------------------- */
#define VK_VERSION   "1.2.0"
#define VK_COPYRIGHT "(c) 2003-2005"
#define VK_AUTHOR    "Donna Robinson"
#define VK_EMAIL     "donna@valgrind.org"
/* so we don't have to update hard-wired in dates for valgrind */
#define VG_COPYRIGHT "(c) 2000-2005, and GNU GPL'd, by Julian Seward et al."

#define DEBUG_ON 1
#define oink(n) printf( "Oink %d\n", n )

/* Globally available object ------------------------------------------- */
class VkConfig;
extern VkConfig* vkConfig;



#endif
