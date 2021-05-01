#include "Mesh.h"
#include "Parser.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include <sstream>

int otherVertex( const Edge& inEdge, int vertex )
{
  if ( inEdge.startVertex == vertex ) {
    return inEdge.endVertex;
  } else if ( inEdge.endVertex == vertex ) {
    return inEdge.startVertex;
  } else {
    throw std::runtime_error("vertex not in edge");
  }
}

double crossProduct( double x1, double y1, double x2, double y2 )
{
  return (x1 * y2 - x2 * y1)/(std::sqrt(x1*x1+y1*y1)*std::sqrt(x2*x2+y2*y2));
}

double crossProduct( const Mesh& mesh, Edge e1, Edge e2, int sharedVertex )
{
  if ( e1 == e2 ) {
    return std::numeric_limits<double>::max();
  }
  Vertex end1 = mesh.getVertex( otherVertex(e1, sharedVertex) );
  Vertex end2 = mesh.getVertex( otherVertex(e2, sharedVertex) );
  Vertex start = mesh.getVertex( sharedVertex );
  double cross = crossProduct( end1.xCoord-start.xCoord,
			       end1.yCoord-start.yCoord,
			       end2.xCoord-start.xCoord,
			       end2.yCoord-start.yCoord );
  return cross;
}

struct AnglesAroundVertex {
  AnglesAroundVertex( int inVertex, Edge inEdge, const Mesh& inMesh )
    : centerVertex( inVertex ), edge( inEdge ), mesh( inMesh ) {}
  int centerVertex;
  Edge edge;
  const Mesh& mesh;
  bool operator()( const int edgeIndex1, const int edgeIndex2 ) const
  {
    return crossProduct( mesh, edge, mesh.getEdge( edgeIndex1 ), centerVertex )
      < crossProduct( mesh, edge, mesh.getEdge( edgeIndex2 ), centerVertex );
  }
};


void Mesh::constructPolygons()
{
  if ( polygons.size() ) {
    // TODO: Something may be wrong, warn or error out.
    return;
  }

  // Basic algorithm:
  // For each edge in each vertex:
  //    Walk to opposite vertex and store.
  //    Find first ordered edge after one just crossed.
  //       If the angle is greater than 180, break. We're traveling around
  //       the outside of the mesh.
  //    Repeat until back at initial vertex
  // If N is the number of vertices, and M is the AVERAGE valence of each vertex,
  //    then the computational complexity should be
  //    Sorting edges -> O ( N * MlogM )
  //    Constructing a single polygon -> approx O (M ^ 2)
  //       We have to "find first ordered edge" in a ~M sized array, ~M times.
  // Note: This will attempt to construct each Polygon multiple times, so
  //    if in the process we reach a vertex lower than the initial vertex, discard.
  // Asymptotically worst case construction for all polygons is N*M times.
  // Total complexity then O( N * MlogM ) + O( N*M * M^2 ) => O( N * M^3 ), M << N
  
  std::vector< std::vector< int > > vertexEdges( numVertices() );
  for ( int edgeIndex = 0; edgeIndex < edges.size(); edgeIndex++ ) {
    vertexEdges[ edges[edgeIndex].startVertex ].push_back( edgeIndex );
    vertexEdges[ edges[edgeIndex].endVertex ].push_back( edgeIndex );
  }

  for ( int vertex = 0; vertex < vertexEdges.size(); vertex++ ) {
    for ( int edge : vertexEdges[vertex] ) {
      Polygon currentPoly;
      currentPoly.vertices.push_back( vertex );
      int currentVertex = vertex;
      int currentEdgeIndex = edge;
      do {
	Edge currentEdge = edges[currentEdgeIndex];
	int nextVertex = otherVertex( currentEdge, currentVertex );
	if ( nextVertex == vertex ) {
	  currentPoly.closed = true;
	  break;
	} else if ( nextVertex < vertex ) {
	  // we've already constructed this one, early exit
	  break;
	}
	currentPoly.vertices.push_back( nextVertex );
	const std::vector< int >& currentVertexEdges = vertexEdges[ nextVertex ];
	AnglesAroundVertex comp( nextVertex, currentEdge, *this);
	std::vector< int >::const_iterator nextEdgeInOrder =
	std::min_element( currentVertexEdges.begin(), currentVertexEdges.end(), comp );
	// the clockwise angle from current edge to next edge has to be less than 180
	// (assuming convex polygons, I'm getting stuck on the concave twist), otherwise we're
	// going around the outside of the mesh.
	double cross = crossProduct( *this, currentEdge, edges[*nextEdgeInOrder], nextVertex );
	if ( cross > 0 ) {
	  break;
	}
	currentEdgeIndex = *nextEdgeInOrder;
	currentVertex = nextVertex;
      } while ( currentVertex != vertex );
      if ( currentPoly.closed ) {
	polygons.push_back( currentPoly );
      }
    }
  }
}

Mesh Mesh::fromJson( std::istream& in )
{
  Mesh result;
  
  std::string contents( std::istreambuf_iterator<char>(in),{});
  auto vertsLoc = contents.find("vertices");
  auto edgesLoc = contents.find("edges");
  if ( vertsLoc == std::string::npos || edgesLoc == std::string::npos ) {
    throw std::runtime_error( "Insufficient data" );
  }
  // Assume Vertices come first. TODO: Generalize
  if ( edgesLoc < vertsLoc ) {
    throw std::runtime_error( "Vertices should come first in the JSON" );
  }
  std::string vertexContent = contents.substr( vertsLoc, edgesLoc - 2 );
  result.coordinates = verticesFromJson( vertexContent );
  std::string edgeContent = contents.substr( edgesLoc );
  result.edges = edgesFromJson( edgeContent );

  result.constructPolygons();
  return result;
}

std::string Mesh::toJson() const
{
  std::stringstream jsonStream;
  jsonStream << std::string( "{\n\t\"vertices\": [" );
  // Improvement, use Boost string join.
  int nVertices = numVertices();
  for ( int i = 0; i < nVertices-1; i++ ) {
    Vertex vertex = getVertex(i);
    jsonStream << "[" << vertex.xCoord << "," << vertex.yCoord << "],";
  }
  Vertex lastVertex = getVertex(nVertices-1);
  jsonStream << "[" << lastVertex.xCoord << "," << lastVertex.yCoord << "]],";

  jsonStream << "\n\t\"edges\": [";
  int nEdges = numEdges();
  for ( int i = 0; i < nEdges-1; i++ ) {
    Edge edge = getEdge(i);
    jsonStream << "[" << edge.startVertex << "," << edge.endVertex << "],";
  }
  Edge lastEdge = getEdge(nEdges-1);
  jsonStream << "[" << lastEdge.startVertex << "," << lastEdge.endVertex << "]],";

  jsonStream << "\n\t\"polygons\": [";
  int nPolygons = numPolygons();
  for ( int i = 0; i < nPolygons-1; i++ ) {
    Polygon polygon = getPolygon(i);
    jsonStream << "[";
    int nPolyVertices = polygon.vertices.size();
    for ( int j = 0; j < nPolyVertices - 1; j++ ) {
      jsonStream << polygon.vertices[j] << ",";
    }
    jsonStream << polygon.vertices[nPolyVertices-1] << "],";
  }
  Polygon lastPolygon = getPolygon(nPolygons-1);
  jsonStream << "[";
  int lastPolyVertices = lastPolygon.vertices.size();
  for ( int j = 0; j < lastPolyVertices - 1; j++ ) {
    jsonStream << lastPolygon.vertices[j] << ",";
  }
  jsonStream << lastPolygon.vertices[lastPolyVertices-1] << "]]\n}";
  return jsonStream.str();
}

int Mesh::numVertices() const
{
  return coordinates.size() / 2;
}

int Mesh::numPolygons() const
{
  return polygons.size();
}

int Mesh::numEdges() const
{
  return edges.size();
}

Vertex Mesh::getVertex( int index ) const
{
  assert( coordinates.size() > 2 * index + 1 );
  return Vertex{ coordinates[ 2 * index ], coordinates[ 2 * index + 1 ] };
}


// Out of time
// How I would do Algorithm 2:
//   Call the functions in Parser.cpp, plus an additional one
//   to read the polygons, and construct a Mesh object.
//   Create an unordered_map between edges and pairs of polygon
//   indices (using -1 in the second slot if it's an outer edge).
//   Runtime is O(N) N number of edges, and lookup is constant order
//   (for each edge, just return the index that doesn't match the input
//   polygon).

// How I would do step 3: With tutorials :)

// How I would do Algorithm 3:
//   For speed, I'd create a quadtree so the search for nearby polygons
//   was only log(N), N being # polygons. Then for each polygon "close enough",
//   perform a cross product between ordered edges and an edge to the point
//   from the start vertex. If the sign is the same on all of them, it's inside the polygon (again, assuming convex polygons).

// How I would do Algorithm 4: Hadn't even thought about it yet.
