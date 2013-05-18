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

#include <event2/dns.h>
#include <event2/bufferevent.h>

struct server {
  char* address;
  unsigned short port;
  unsigned char timeout;
  struct server* next;
  struct bufferevent* conn;
};

struct config {
  struct server* servers;
};

struct config* global_config;

struct evdns_base* dns;

int parse_config(char* config_file);

int dispatch_config(struct event_base* base);

int startConnection(struct server* server, struct event_base* base);

#endif //_CONFIG_H