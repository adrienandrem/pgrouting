/*
 * Connected components algorithm for PostgreSQL
 *
 * Copyright (c) 2014 Adrien André
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */
#include <iostream>
#include <vector>
#include <algorithm>
#include <utility>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>

using namespace std;
using namespace boost;

/*
 * pgr_dijkstra('SELECT id, source, target, cost FROM network ;', 172, 253, FALSE, FALSE)
 *
 */

int main(int , char* [])
{
  {
    typedef adjacency_list <vecS, vecS, undirectedS> Graph;

    Graph G;
    add_edge(0, 1, G);
    add_edge(1, 4, G);
    add_edge(4, 0, G);
    add_edge(2, 5, G);

    std::vector<int> component(num_vertices(G));
    // calling Boost function
    int component_count = connected_components(G, &component[0]);

    std::vector<int>::size_type i;
    cout << "Total number of components: " << component_count << endl;
    for (i = 0; i != component.size(); ++i)
      cout << "Vertex " << i <<" is in component " << component[i] << endl;
    cout << endl;
  }
  return 0;
}
