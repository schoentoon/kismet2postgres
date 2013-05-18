/* * -----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Toon Schoenmakers <nighteyes1993@gmail.com>
 * wrote this file. As long as you retain this notice you can do whatever you
 * want with this stuff. If we meet some day, and you think this stuff is worth
 * it, you can buy us a beer in return.
 * -----------------------------------------------------------------------------
 */

#include "callbacks.h"

#include "debug.h"
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <event2/event.h>
#include <event2/buffer.h>

void kismet_conn_readcb(struct bufferevent *bev, void* args) {
  DEBUG(255, "kismet_conn_readcb(%p, %p);", bev, args);
  struct evbuffer* input = bufferevent_get_input(bev);
  struct evbuffer* output = bufferevent_get_output(bev);
  struct server* server = (struct server*) args;
  size_t len;
  char* line = evbuffer_readln(input, &len, EVBUFFER_EOL_CRLF);
  while (line) {
    DEBUG(255, "Input: '%s'", line);
    static const char* CAPABILITY_SSCANF = "*CAPABILITY: %s %[^\n]";
    char type[BUFSIZ];
    char rest[BUFSIZ];
    if (sscanf(line, CAPABILITY_SSCANF, type, rest) == 2) {
      struct inserter* inserter = server->inserters;
      while (inserter) {
        if (strcmp(inserter->type, type) == 0)
          break;
        inserter = inserter->next;
      };
      if (inserter && !inserter->capabilities) {
        char* token = strtok(rest, ",");
        DEBUG(255, "sizeof(char*) = %d", sizeof(char*));
        while (token) {
          unsigned int next_token = 0;
          if (inserter->capabilities) {
            while (inserter->capabilities[++next_token]);
            inserter->capabilities = realloc(inserter->capabilities, sizeof(char*) * (next_token + 2));
          } else
            inserter->capabilities = malloc(sizeof(char*) * 2);
          inserter->capabilities[next_token] = strdup(token);
          inserter->capabilities[++next_token] = NULL;
          token = strtok(NULL, ",");
        };
        static const char* ENABLE_EVENT = "!%d ENABLE %s *\n";
        evbuffer_add_printf(output, ENABLE_EVENT, inserter->ack_id, inserter->type);
      };
    };
    free(line);
    line = evbuffer_readln(input, &len, EVBUFFER_EOL_CRLF);
  };
};

struct reconnect_struct {
  struct event_base* base;
  struct server* server;
};

static void reconnectServer(evutil_socket_t fd, short event, void* args) {
  DEBUG(255, "reconnectServer(%d, 0x%02x, %p);", fd, event, args);
  struct reconnect_struct* reconnect_struct = (struct reconnect_struct*) args;
  startConnection(reconnect_struct->server, reconnect_struct->base);
  free(reconnect_struct);
};

void kismet_conn_eventcb(struct bufferevent *bev, short event, void* args) {
  DEBUG(255, "kismet_conn_eventcb(%p, 0x%02x, %p);", bev, event, args);
  if (!(event & BEV_EVENT_CONNECTED)) {
    struct event_base* base = bufferevent_get_base(bev);
    struct server* node = (struct server*) args;
    bufferevent_free(node->conn);
    node->conn = NULL;
    struct timeval tv = {node->timeout, 0};
    struct reconnect_struct* reconnect_struct = malloc(sizeof(struct reconnect_struct));
    reconnect_struct->server = node;
    reconnect_struct->base = base;
    event_base_once(base, -1, EV_TIMEOUT, reconnectServer, reconnect_struct, &tv);
  }
};