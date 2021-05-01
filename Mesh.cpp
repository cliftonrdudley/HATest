#include "Mesh.h"
#include "Parser.h"

#include <algorithm>
#include <cassert>
#include <cmath>

double angleFromXAxis( const Vertex& v1, const Vertex& v2 )
{
  return std::atan2( v2.yCoord - v1.yCoord, v2.xCoord - v1.xCoord );
}

struct AnglesAroundVertex {
  AnglesAroundVertex( int inVertex, const Mesh& inMesh ) : centerVertex( inVertex ), mesh( inMesh ) {}
  int centerVertex;
  const Mesh& mesh;
  bool operator()( const int edgeIndex1, const int edgeIndex2 ) const
  {
    Edge e1 = mesh.getEdge( edgeIndex1 );
    Edge e2 = mesh.getEdge( edgeIndex2 );
    double angle1 = e1.startVertex == centerVertex
      ? angleFromXAxis( mesh.getVertex( e1.startVertex ), mesh.getVertex( e1.endVertex ) )
      : angleFromXAxis( mesh.getVertex( e1.endVertex ), mesh.getVertex( e1.startVertex ) );
    double angle2 = e2.startVertex == centerVertex
      ? angleFromXAxis( mesh.getVertex( e2.startVertex ), mesh.getVertex( e2.endVertex ) )
      : angleFromXAxis( mesh.getVertex( e2.endVertex ), mesh.getVertex( e2.startVertex ) );
    return angle1 < angle2;
  }
};


void Mesh::constructPolygons()
{
  if ( polygons.size() ) {
    // TODO: Something may be wrong, warn or error out.
    return;
  }

  // Basic algorithm:
  // Order each edge around every vertex (counter-clockwise)
  // For each edge in each vertex:
  //    Walk to opposite vertex and store.
  //    Find first ordered edge after one just crossed.
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
  
  std::vector< std::vector< int > > orderedVertexEdges( numVertices() );
  for ( int edgeIndex = 0; edgeIndex < edges.size(); edgeIndex++ ) {
    orderedVertexEdges[ edges[edgeIndex].startVertex ].push_back( edgeIndex );
    orderedVertexEdges[ edges[edgeIndex].endVertex ].push_back( edgeIndex );
  }
  for ( int vertex = 0; vertex < orderedVertexEdges.size(); vertex++ ) {
    AnglesAroundVertex comp( vertex, *this );
    std::sort( orderedVertexEdges[ vertex ].begin(), orderedVertexEdges[ vertex ].end(), comp );
  }

  for ( int vertex = 0; vertex < orderedVertexEdges.size(); vertex++ ) {
    for ( int edge : orderedVertexEdges[vertex] ) {
      Polygon currentPoly;
      currentPoly.vertices.push_back( vertex );
      int currentVertex = vertex;
      int currentEdgeIndex = edge;
      do {
	Edge currentEdge = edges[currentEdgeIndex];
	int nextVertex = currentEdge.startVertex == currentVertex ? currentEdge.endVertex : currentEdge.startVertex;
	if ( nextVertex == vertex ) {
	  currentPoly.closed = true;
	  break;
	} else if ( nextVertex < vertex ) {
	  // we've already constructed this one, early exit
	  break;
	}
	currentPoly.vertices.push_back( nextVertex );
	const std::vector< int >& currentVertexEdges = orderedVertexEdges[ nextVertex ];
	auto currentEdgeInOrder = std::find( currentVertexEdges.begin(), currentVertexEdges.end(), currentEdgeIndex );
	assert( currentEdgeInOrder != currentVertexEdges.end() );
	auto nextEdgeInOrder = currentEdgeInOrder++;
	if ( nextEdgeInOrder == currentVertexEdges.end() ) {
	  nextEdgeInOrder = currentVertexEdges.begin();
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
