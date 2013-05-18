/* * -----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Toon Schoenmakers <nighteyes1993@gmail.com>
 * wrote this file. As long as you retain this notice you can do whatever you
 * want with this stuff. If we meet some day, and you think this stuff is worth
 * it, you can buy us a beer in return.
 * -----------------------------------------------------------------------------
 */

#ifndef _CALLBACKS_H
#define _CALLBACKS_H

#include "config.h"

#include <event2/bufferevent.h>
#include <event2/bufferevent_struct.h>

void kismet_conn_readcb(struct bufferevent *bev, void* args);

void kismet_conn_eventcb(struct bufferevent *bev, short events, void* args);

#endif //_CALLBACKS_H