/* * -----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Toon Schoenmakers <nighteyes1993@gmail.com>
 * wrote this file. As long as you retain this notice you can do whatever you
 * want with this stuff. If we meet some day, and you think this stuff is worth
 * it, you can buy us a beer in return.
 * -----------------------------------------------------------------------------
 */

#include "config.h"

#include "debug.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

struct config* global_config = NULL;

struct evdns_base* dns = NULL;

int parse_config(char* config_file) {
  if (global_config)
    return 0;
  FILE *f = fopen(config_file, "r");
  if (f == NULL) {
    fprintf(stderr, "Error '%s' while opening '%s'.\n", strerror(errno), config_file);
    return 0;
  }
  global_config = malloc(sizeof(struct config));
  struct server* current_server = NULL;
  char line_buffer[BUFSIZ];
  unsigned int line_count = 0;
  while (fgets(line_buffer, sizeof(line_buffer), f)) {
    line_count++;
    if (strlen(line_buffer) == 1 || line_buffer[0] == '#')
      continue;
    char key[BUFSIZ];
    char value[BUFSIZ];
    if (sscanf(line_buffer, "%[a-z_] = %[^\t\n]", key, value) == 2) {
      DEBUG(255, "key: '%s', value: '%s'", key, value);
      if (strcasecmp(key, "address") == 0) {
        if (current_server) {
          current_server->next = malloc(sizeof(struct server));
          current_server = current_server->next;
          memset(current_server, 0, sizeof(struct server));
        } else {
          current_server = malloc(sizeof(struct server));
          memset(current_server, 0, sizeof(struct server));
          global_config->servers = current_server;
        };
        current_server->timeout = 90;
        current_server->address = strdup(value);
      } else if (strcasecmp(key, "port") == 0) {
        long port = strtol(value, NULL, 10);
        if ((errno == ERANGE || (port == LONG_MAX || port == LONG_MIN)) || (errno != 0 && port == 0) || port < 0 || port > 65535) {
          fprintf(stderr, "Error at line %d. Port %ld is out of range.\n", line_count, port);
          return 0;
        } else
          current_server->port = port;
      } else if (strcasecmp(key, "timeout") == 0) {
        long timeout = strtol(value, NULL, 10);
        if ((errno == ERANGE || (timeout == LONG_MAX || timeout == LONG_MIN)) || (errno != 0 && timeout == 0) || timeout < 0 || timeout > 255) {
          fprintf(stderr, "Error at line %d. Timeout %ld is out of range. (0-255)\n", line_count, timeout);
          return 0;
        } else
          current_server->port = (unsigned char) timeout;
      };
    } else {
      fprintf(stderr, "Parsing error at line %d.\n", line_count);
      return 0;
    }
  };
  fclose(f);
  return 1;
};

int dispatch_config(struct event_base* base) {
  struct server* node = global_config->servers;
  if (!node)
    return 0;
  if (!dns)
    dns = evdns_base_new(base, 1);
  while (node) {
    startConnection(node, base);
    node = node->next;
  };
  return 1;
};

int startConnection(struct server* server, struct event_base* base) {
  if (server->conn)
    return 0;
  server->conn = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
  struct timeval timeout = {server->timeout, 0};
  bufferevent_set_timeouts(server->conn, &timeout, NULL);
  bufferevent_socket_connect_hostname(server->conn, dns, AF_INET, server->address, server->port);
  return 1;
};