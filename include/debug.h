/* * -----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Toon Schoenmakers <nighteyes1993@gmail.com>
 * wrote this file. As long as you retain this notice you can do whatever you
 * want with this stuff. If we meet some day, and you think this stuff is worth
 * it, you can buy us a beer in return.
 * -----------------------------------------------------------------------------
 */

#ifndef _DEBUG_H
#define _DEBUG_H

#ifdef NO_DEBUG
#  define DEBUG(level, format, ...)
#else
   void __internal_debug(unsigned char level, const char* format, ...);
#  define DEBUG(level, format, ...) __internal_debug(level, format, ##__VA_ARGS__);
#endif

unsigned char debug;

#endif //_DEBUG_H