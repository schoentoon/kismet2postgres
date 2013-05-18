/* * -----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Toon Schoenmakers <nighteyes1993@gmail.com>
 * wrote this file. As long as you retain this notice you can do whatever you
 * want with this stuff. If we meet some day, and you think this stuff is worth
 * it, you can buy us a beer in return.
 * -----------------------------------------------------------------------------
 */

#ifndef _QUERY_PRINTF_H
#define _QUERY_PRINTF_H

#include "config.h"

int query_printf(char* buf, char* data, struct inserter* inserter);

#endif //_QUERY_PRINTF_H