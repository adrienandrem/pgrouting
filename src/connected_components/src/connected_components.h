/*
 * Connected components algorithm for PostgreSQL
 *
 * Copyright (c) 2014 Adrien ANDRÉ
 *
 */
#ifndef _CONNECTED_COMPONENTS_H
#define _CONNECTED_COMPONENTS_H

#include "postgres.h"

/* Structure to store edge attributes values. */
typedef struct edge 
{
    int id;
    int source;
    int target;
} edge_t;

#ifdef __cplusplus
extern "C"
#endif
int boost_connected_components(edge_t *edges, unsigned int count,
                               int *component_count, char **err_msg);

#endif // _CONNECTED_COMPONENTS_H
