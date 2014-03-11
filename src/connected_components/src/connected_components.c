/*
 * Connected components algorithm for PostgreSQL
 *
 * Copyright (c) 2014 Adrien ANDRÉ
 *
 */
#include "postgres.h"
#include "executor/spi.h"
#include "funcapi.h"
#include "catalog/pg_type.h"
#if PGSQL_VERSION > 92
#include "access/htup_details.h"
#endif

#include "fmgr.h"

#include "connected_components.h"


// The number of tuples to fetch from the SPI cursor at each iteration
#define TUPLIMIT 1000

/* Structure to store egde columns numbers. */
typedef struct edge_columns
{
  int id;
  int source;
  int target;
} edge_columns_t;


/* Rerieves columns numbers from names,
 * and checks data types. */
static int
fetch_edge_columns(SPITupleTable *tuptable, edge_columns_t *edge_columns)
{
  // Retrieve columns indices from names
  edge_columns->id = SPI_fnumber(SPI_tuptable->tupdesc, "id");
  edge_columns->source = SPI_fnumber(SPI_tuptable->tupdesc, "source");
  edge_columns->target = SPI_fnumber(SPI_tuptable->tupdesc, "target");

  // Checks columns presence
  if (edge_columns->id == SPI_ERROR_NOATTRIBUTE ||
      edge_columns->source == SPI_ERROR_NOATTRIBUTE ||
      edge_columns->target == SPI_ERROR_NOATTRIBUTE) {
    elog(ERROR, "Error, query must return columns "
	 "'id', 'source' and 'target'");
    return -1;
  }

  // Check columns data types
  if (SPI_gettypeid(SPI_tuptable->tupdesc, edge_columns->source) != INT4OID ||
      SPI_gettypeid(SPI_tuptable->tupdesc, edge_columns->target) != INT4OID) {
    elog(ERROR, "Error, columns 'source', 'target' must be of type int4");
    return -1;
  }

  DBG("columns: id %i source %i target %i",
      edge_columns->id, edge_columns->source, edge_columns->target);

  return 0;
}


/* Fills edge from row.
 *
 * tuple: the row,
 * target_edge: edge to fill */
static void
fetch_edge(HeapTuple *tuple, TupleDesc *tupdesc, edge_columns_t *edge_columns, edge_t *target_edge)
{
  Datum binval;
  bool isnull;

  binval = SPI_getbinval(*tuple, *tupdesc, edge_columns->id, &isnull);
  if (isnull) elog(ERROR, "id contains a null value");
  target_edge->id = DatumGetInt32(binval);

  binval = SPI_getbinval(*tuple, *tupdesc, edge_columns->source, &isnull);
  if (isnull) elog(ERROR, "source contains a null value");
  target_edge->source = DatumGetInt32(binval);

  binval = SPI_getbinval(*tuple, *tupdesc, edge_columns->target, &isnull);
  if (isnull) elog(ERROR, "target contains a null value");
  target_edge->target = DatumGetInt32(binval);
}


/*  */
static int
compute_connected_components(char* sql, int *component_count)
{

  int SPIcode;
  void *SPIplan;
  Portal SPIportal;
  bool moredata = TRUE;
  int ntuples;
  edge_t *edges = NULL;
  int total_tuples = 0;
  edge_columns_t edge_columns = {.id = -1, .source = -1, .target = -1};
  int v_max_id = 0;
  int v_min_id = INT_MAX;

  int s_count = 0;
  int t_count = 0;

  char *err_msg;
  int ret = -1;
  register int z;

  DBG("start connected_components\n");

  SPIcode = SPI_connect();
  if (SPIcode != SPI_OK_CONNECT) {
    elog(ERROR, "connected_components: couldn't open a connection to SPI");
    return -1;
  }

  SPIplan = SPI_prepare(sql, 0, NULL);
  if (SPIplan == NULL) {
    elog(ERROR, "connected_components: couldn't create query plan via SPI");
    return -1;
  }

  if ((SPIportal = SPI_cursor_open(NULL, SPIplan, NULL, NULL, true)) == NULL) {
    elog(ERROR, "connected_components: SPI_cursor_open('%s') returns NULL", sql);
    return -1;
  }

  while (moredata == TRUE) {
    SPI_cursor_fetch(SPIportal, TRUE, TUPLIMIT);

    if (edge_columns.id == -1) {
      if (fetch_edge_columns(SPI_tuptable, &edge_columns) == -1)
	return finish(SPIcode, ret);
    }

    ntuples = SPI_processed;
    total_tuples += ntuples;
    if (!edges)
      edges = palloc(total_tuples * sizeof(edge_t));
    else
      edges = repalloc(edges, total_tuples * sizeof(edge_t));

    if (edges == NULL) {
      elog(ERROR, "Out of memory");
      return finish(SPIcode, ret);
    }

    if (ntuples > 0) {
      int t;
      SPITupleTable *tuptable = SPI_tuptable;
      TupleDesc tupdesc = SPI_tuptable->tupdesc;

      for (t = 0; t < ntuples; t++) {
	HeapTuple tuple = tuptable->vals[t];
	fetch_edge(&tuple, &tupdesc, &edge_columns,
		   &edges[total_tuples - ntuples + t]);
      }
      SPI_freetuptable(tuptable);
    } else {
      moredata = FALSE;
    }
  }

  //defining min and max vertex id

  DBG("Total %i tuples", total_tuples);

  for (z = 0; z < total_tuples; z++) {
    if (edges[z].source < v_min_id) v_min_id = edges[z].source;
    if (edges[z].source > v_max_id) v_max_id = edges[z].source;
    if (edges[z].target < v_min_id) v_min_id = edges[z].target;
    if (edges[z].target > v_max_id) v_max_id = edges[z].target;

    DBG("%i <-> %i", v_min_id, v_max_id);
  }

  /*
   * reducing vertex id (renumbering)
   */
  for (z = 0; z < total_tuples; z++) {

    edges[z].source -= v_min_id;
    edges[z].target -= v_min_id;
    DBG("%i - %i", edges[z].source, edges[z].target);
  }

  DBG("Total %i tuples", total_tuples);

  DBG("Calling boost_connected_components\n");

  ret = boost_connected_components(edges, total_tuples,
                                   component_count, &err_msg);

  if (ret < 0) {
    //elog(ERROR, "Error computing components: %s", err_msg);
    ereport(ERROR, (errcode(ERRCODE_E_R_E_CONTAINING_SQL_NOT_PERMITTED),
		    errmsg("Error computing components: %s", err_msg)));
  }

  DBG("SIZE %i\n", *component_count);

  DBG("ret = %i\n", ret);

  DBG("*component_count = %i\n", *component_count);

  DBG("ret = %i\n", ret);

  return finish(SPIcode, ret);
}
