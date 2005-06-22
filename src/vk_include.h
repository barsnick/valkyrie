/* ---------------------------------------------------------------------- 
 * Main include file                                         vk_include.h
 * Automatically generated by configure on Wed Jun 22 16:39:47 BST 2005
 * Do not edit this file - all changes will be lost
 * ----------------------------------------------------------------------
 */

#ifndef __VK_INCLUDE_H
#define __VK_INCLUDE_H

/* Install paths ------------------------------------------------------- */
#define PREFIX       "/home/de/Programs/valkyrie/trunk"
#define DOCS_PATH    "/doc"
#define ICONS_PATH   "/icons"
#define DBASE_DIR    "/dbase"
#define LOGS_DIR     "/logs"
#define SUPPS_DIR    "/suppressions"

/* Name ---------------------------------------------------------------- */
#define vk_name      "valkyrie"
#define Vk_Name      "Valkyrie"

/* Version, as 'MAJOR-NUMBER.MINOR-NUMBER[.PATCHLEVEL]'.
   A version in the form 'YYYY-MM-DD' is a release-of-the-day, 
   i.e. a snapshot of the current development tree --------------------- */
#define VK_VERSION    "1.2.0"
#define VK_COPYRIGHT  "(c) 2003-2005"
#define VK_AUTHOR     "Donna Robinson"
#define VK_EMAIL      "donna@valgrind.org"
/* so we do not have to update hard-wired-in dates for valgrind */
#define VG_COPYRIGHT  "(c) 2000-2005, and GNU GPL'd, by Julian Seward et al."

/* Other paths we need to know about ----------------------------------- */
#define VG_EXEC_PATH  "/home/sewardj/Vg3LINE/trunk/Inst/bin/valgrind"
#define VG_SUPP_DIR   "/usr/lib/valgrind"

/* These functions are implemented in vk_utils.cpp --------------------- */
const char* installPath();
const char* vkname();
const char* vkName();
const char* vkVersion();
const char* vkCopyright();
const char* vkAuthor();
const char* vkEmail();
const char* vgCopyright();
const char* vgExec();
const char* vgSuppDir();

#define DEBUG_ON 0
#define oink(n) printf( "Oink %d\n", n )

/* Globally available object ------------------------------------------- */
class VkConfig;
extern VkConfig* vkConfig;

#endif
