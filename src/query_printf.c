/* * -----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Toon Schoenmakers <nighteyes1993@gmail.com>
 * wrote this file. As long as you retain this notice you can do whatever you
 * want with this stuff. If we meet some day, and you think this stuff is worth
 * it, you can buy us a beer in return.
 * -----------------------------------------------------------------------------
 */

#include "query_printf.h"

#include "debug.h"

#include <stdio.h>
#include <string.h>

unsigned char startsWith(char* str, char* start) {
  size_t str_len = strlen(str);
  size_t start_len = strlen(start);
  size_t lowest = (str_len < start_len) ? str_len : start_len;
  int i;
  for (i = 0; i < lowest; i++) {
    if (str[i] != start[i])
      return 0;
  };
  return 1;
};

int query_printf(char* buf, char* data, struct inserter* inserter) {
  char* fmt = inserter->query;
  for (;*fmt != '\0'; fmt++) {
    if (*fmt == '%') {
      fmt++;
      int i = 0;
      while (inserter->capabilities[i]) {
        if (startsWith(fmt, inserter->capabilities[i])) {
          fmt += (strlen(inserter->capabilities[i]) - 1);
          char* rest = data;
          if (i == 0) {
            while (*rest != ' ')
              *buf++ = *rest++;
          } else {
            int c = 1;
            while (*rest++) {
              if (*rest == ' ') {
                if (c++ == i) {
                  rest++;
                  break;
                }
              }
            };
            while (*rest != ' ')
              *buf++ = *rest++;
          }
          break;
        };
        i++;
      };
    } else
      *buf++ = *fmt;
  };
  return 1;
};