/*
 * Connected components algorithm for PostgreSQL
 *
 * Copyright (c) 2014 Adrien ANDRÉ
 *
 */
#include "connected_components.h"
#include <cfloat>

#include <vector>
#include <algorithm>
#include <utility>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>

using namespace std;
using namespace boost;


/* Structure storing a vertex. */
struct Vertex
{
    int id;
};


/* Adds an edge to the graph.
 *
 * Edge id, source and target ids and coordinates are copied also. */
template <class G, class E>
static void
graph_add_edge(G &graph, E &e,
               int id, int source, int target)
{
  bool inserted;

  tie(e, inserted) = add_edge(source, target, graph);

  graph[e].id = id;

  typedef typename graph_traits<G>::vertex_descriptor Vertex;
  Vertex s = vertex(source, graph);
  Vertex t = vertex(target, graph);
}


/* Computes connected components.
 * 
 * Builds boost graph from egde list,
 * calls the boost connected components function and
 * returns a component id vector:
 * component[vertex_id] = component_id
 *
 * edges: the edge list,
 * count: the edge number,
 * component_count: the component number */
int
boost_connected_components(edge_t *edges, unsigned int count,
                           int *component_count,
			   char **err_msg)
{
  try {
    {
      typedef adjacency_list <vecS, vecS, undirectedS, no_property, Vertex> graph_t;
      typedef graph_traits <graph_t>::edge_descriptor edge_descriptor;

      /* Build boost graph from edge list. */    
      graph_t graph;
      for (size_t j = 0; j < count; ++j)
      {
	edge_descriptor e;
	graph_add_edge<graph_t, edge_descriptor>(graph, e,
	    edges[j].id, edges[j].source, edges[j].target);
      }
    
      vector<int> component(num_vertices(graph));
      // calling Boost function
      *component_count = connected_components(graph, &component[0]);

      // TODO: prepare component vector for output
    }
    return EXIT_SUCCESS;
  } catch(...) {
    *err_msg = (char *) "Unknown exception caught!";
    return -1;
  }
}
