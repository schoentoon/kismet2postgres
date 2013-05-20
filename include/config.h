/* * -----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Toon Schoenmakers <nighteyes1993@gmail.com>
 * wrote this file. As long as you retain this notice you can do whatever you
 * want with this stuff. If we meet some day, and you think this stuff is worth
 * it, you can buy us a beer in return.
 * -----------------------------------------------------------------------------
 */

#ifndef _CONFIG_H
#define _CONFIG_H

#include "postgres.h"

#include <event2/dns.h>
#include <event2/bufferevent.h>

struct inserter {
  char* type;
  char* query;
  char** capabilities;
  unsigned char ack_id;
  struct inserter* next;
};

struct server {
  char* address;
  unsigned short port;
  unsigned char timeout;
  unsigned char disable_time : 1;
  struct connection_struct* db;
  struct inserter* inserters;
  struct bufferevent* conn;
  struct server* next;
};

struct config {
  char* conninfo;
  struct server* servers;
};

struct config* global_config;

struct evdns_base* dns;

int parse_config(char* config_file);

int dispatch_config(struct event_base* base);

int startConnection(struct server* server, struct event_base* base);

#endif //_CONFIG_H