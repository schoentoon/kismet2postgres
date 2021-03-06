/* * -----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Toon Schoenmakers <nighteyes1993@gmail.com>
 * wrote this file. As long as you retain this notice you can do whatever you
 * want with this stuff. If we meet some day, and you think this stuff is worth
 * it, you can buy us a beer in return.
 * -----------------------------------------------------------------------------
 */

#include "postgres.h"

#include "debug.h"

#include <event2/bufferevent.h>
#include <event2/event_struct.h>

#include <limits.h>
#include <stdlib.h>
#include <string.h>

struct query_struct {
  void (*callback)(PGresult*,void*,char*);
  void *context;
  char *query;
  unsigned char sent : 1;
  struct query_struct *next;
};

static struct database_list {
  struct connection_struct* db;
  struct database_list* next;
} *all_databases;

char* db_connect = NULL;

static void pq_timer(evutil_socket_t fd, short event, void *arg);
static int highPriorityDatabaseQuery(struct connection_struct* conn, char* query, void (*callback)(PGresult*,void*,char*), void* context);

struct connection_struct* initDatabase(struct event_base* base) {
  struct connection_struct* database = malloc(sizeof(struct connection_struct));
  database->query_count = 0;
  database->idle_ticker = 0;
  database->autocommit = 0;
  database->report_errors = 0;
  database->queries = NULL;
  database->last_query = NULL;
  database->conn = NULL;
  struct event* timer = event_new(base, -1, EV_PERSIST, pq_timer, database);
  struct timeval tv;
  evutil_timerclear(&tv);
  tv.tv_sec = 0;
  tv.tv_usec = 100;
  event_add(timer, &tv);
  if (all_databases == NULL) {
    all_databases = malloc(sizeof(struct database_list));
    memset(all_databases, 0, sizeof(struct database_list));
    all_databases->db = database;
  } else {
    struct database_list* node = all_databases;
    while (node->next)
      node = node->next;
    node->next = malloc(sizeof(struct database_list));
    memset(node->next, 0, sizeof(struct database_list));
    node->next->db = database;
  }
  return database;
};

void dispatchDatabases() {
  struct database_list* node = all_databases;
  while (node) {
    pq_timer(0, 0, node->db);
    node = node->next;
  };
};

static void pq_timer(evutil_socket_t fd, short event, void *arg) {
  struct connection_struct* database = (struct connection_struct*) arg;
  if (database->queries) {
    database->idle_ticker = 0;
    if (database->conn == NULL) {
      database->conn = PQconnectdb(db_connect);
      if (PQstatus(database->conn) != CONNECTION_OK) {
        fprintf(stderr, "%s\n", PQerrorMessage(database->conn));
        PQfinish(database->conn);
        database->conn = NULL;
      } else {
        PQsetnonblocking(database->conn, 1);
        if (database->autocommit)
          highPriorityDatabaseQuery(database, "SET AUTOCOMMIT = ON", NULL, NULL);
      }
    } else {
      if (database->queries->sent == 0) {
        PQsendQuery(database->conn, database->queries->query);
        database->queries->sent = 1;
      }
      if (PQconsumeInput(database->conn) && !PQisBusy(database->conn)) {
        PGresult* res = PQgetResult(database->conn);
        while (res) {
          if (database->queries->callback)
            database->queries->callback(res, database->queries->context, database->queries->query);
          if (database->report_errors && PQresultStatus(res) != PGRES_COMMAND_OK)
            fprintf(stderr, "Query: '%s' returned error\n\t%s\n", database->queries->query, PQresultErrorMessage(res));
          PQclear(res);
          res = PQgetResult(database->conn);
        }
        database->query_count--;
        struct query_struct* old = database->queries;
        database->queries = database->queries->next;
        free(old->query);
        free(old);
        pq_timer(fd, event, arg);
      }
    }
  } else {
    if (database->conn && ++database->idle_ticker > MAX_IDLE_TICKS) {
      PQfinish(database->conn);
      database->conn = NULL;
      database->idle_ticker = 0;
    }
  }
}

void appendQueryPool(struct connection_struct* conn, struct query_struct* query) {
  if (conn->query_count == 0) {
    conn->queries = query;
    conn->last_query = query;
    conn->query_count++;
  } else {
    conn->last_query->next = query;
    conn->last_query = query;
    conn->query_count++;
  }
}

static int highPriorityDatabaseQuery(struct connection_struct* conn, char* query, void (*callback)(PGresult*,void*,char*), void* context) {
  if (query == NULL || conn == NULL)
    return 0;
  struct query_struct* query_struct = malloc(sizeof(struct query_struct));
  if (query_struct == NULL)
    return 0;
  query_struct->query = malloc(strlen(query) + 1);
  strcpy(query_struct->query, query);
  query_struct->callback = callback;
  query_struct->context = context;
  query_struct->sent = 0;
  query_struct->next = NULL;
  if (conn->query_count == 0) {
    conn->queries = query_struct;
    conn->query_count++;
  } else {
    query_struct->next = conn->queries;
    conn->queries = query_struct;
    conn->query_count++;
  }
  return 1;
};

int databaseQuery(struct connection_struct* conn, char* query, void (*callback)(PGresult*,void*,char*), void* context) {
  if (query == NULL || conn == NULL)
    return 0;
  struct query_struct* query_struct = malloc(sizeof(struct query_struct));
  if (query_struct == NULL)
    return 0;
  query_struct->query = malloc(strlen(query) + 1);
  strcpy(query_struct->query, query);
  query_struct->callback = callback;
  query_struct->context = context;
  query_struct->sent = 0;
  query_struct->next = NULL;
  appendQueryPool(conn, query_struct);
  return 1;
}