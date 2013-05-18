/* * -----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Toon Schoenmakers <nighteyes1993@gmail.com>
 * wrote this file. As long as you retain this notice you can do whatever you
 * want with this stuff. If we meet some day, and you think this stuff is worth
 * it, you can buy us a beer in return.
 * -----------------------------------------------------------------------------
 */

#include "debug.h"
#include "config.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>

#include <event.h>

static const struct option g_LongOpts[] = {
  { "help",       no_argument,       0, 'h' },
  { "debug",      optional_argument, 0, 'D' },
  { "foreground", no_argument,       0, 'f' },
  { "config",     required_argument, 0, 'C' },
  { "test-config",required_argument, 0, 'T' },
  { 0, 0, 0, 0 }
};

struct event_base* event_base = NULL;

static int usage() {
  fprintf(stderr, "USAGE: kismet2postgres [options]\n");
  fprintf(stderr, "-h, --help\t\tShow this help message.\n");
  fprintf(stderr, "-D, --debug\t\tIncrease debug level.\n");
  fprintf(stderr, "\t\t\tYou can also directly set a certain debug level with -D5\n");
  fprintf(stderr, "-f, --foreground\tDon't fork into the background (-D won't fork either).\n");
  fprintf(stderr, "-C, --config\t\tUse this config file.\n");
  fprintf(stderr, "-T, --test-config\tSimply test the config file for any errors.\n");
  return 0;
};

int main(int argc, char** argv) {
  int arg, optindex;
  unsigned char foreground = 0;
  while ((arg = getopt_long(argc, argv, "hD::fC:T:", g_LongOpts, &optindex)) != -1) {
    switch (arg) {
    case 'h':
      return usage();
    case 'D':
      if (optarg) {
        errno = 0;
        long tmp = strtol(optarg, NULL, 10);
        if (errno == 0 && tmp < 256)
          debug = (unsigned char) tmp;
        else
          fprintf(stderr, "%ld is an invalid debug level.\n", tmp);
      } else
        debug++;
      break;
    case 'f':
      foreground = 1;
      break;
    case 'C':
      if (parse_config(optarg) == 0)
        return 1;
      break;
    case 'T':
      if (parse_config(optarg)) {
        fprintf(stderr, "Config file seems to be valid.\n");
        return 0;
      } else {
        fprintf(stderr, "Your config seems to be invalid.\n");
        return 1;
      }
    }
  }
  if (foreground || debug || (fork() == 0)) {
    event_base = event_base_new();
    dispatch_config(event_base);
    while (1)
      event_base_dispatch(event_base);
  }
  return 0;
};