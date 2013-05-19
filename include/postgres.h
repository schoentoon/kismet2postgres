/* * -----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Toon Schoenmakers <nighteyes1993@gmail.com>
 * wrote this file. As long as you retain this notice you can do whatever you
 * want with this stuff. If we meet some day, and you think this stuff is worth
 * it, you can buy us a beer in return.
 * -----------------------------------------------------------------------------
 */

#ifndef _POSTGRES_H
#define _POSTGRES_H

#include <event2/event.h>

#define MAX_CONNECTIONS 10
#define MAX_IDLE_TICKS 100000

#include <libpq-fe.h>

char* db_connect;

struct connection_struct {
  PGconn *conn;
  struct query_struct *queries;
  struct query_struct *last_query;
  unsigned int query_count;
  unsigned int idle_ticker;
  unsigned char autocommit : 1;
  unsigned char report_errors : 1;
};

/** Initialize our database pool
 * @param base The event_base used for our internal timer
 * @return Basically your private database connection
 */
struct connection_struct* initDatabase(struct event_base* base);

/** Execute a query on our database pool
 * @param conn The database connection to launch the query on
 * @param query The query to execute
 * @param callback The function to call after our query is done
 * @param context A pointer to pass to your callback
 * @return 1 in case the query was valid and put onto our database pool
 */
int databaseQuery(struct connection_struct* conn, char* query, void (*callback)(PGresult*,void*,char*), void* context);

void dispatchDatabases();

#endif //_POSTGRES_H