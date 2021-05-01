#ifndef MESH_H
#define MESH_H

#include <istream>
#include <set>
#include <vector>

struct Edge
{
  int startVertex;
  int endVertex;
};

struct Vertex
{
  double xCoord;
  double yCoord;
};
  
struct Polygon
{
  std::vector< int > vertices;
  bool closed = false;
};

class Mesh
{
 public:
  static Mesh fromJson( std::istream& in );

  int numVertices() const;
  int numPolygons() const;
  int numEdges() const;

  Vertex getVertex( int index ) const;
  Edge getEdge( int index ) const { return edges[index]; }
private:
  void constructPolygons();
  std::vector< double > coordinates;
  std::vector< Edge > edges;
  std::vector< Polygon > polygons;
};

#endif // MESH_H
