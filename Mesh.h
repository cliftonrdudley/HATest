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

inline bool operator==(const Edge& e1, const Edge& e2)
{
  return ( e1.startVertex == e2.startVertex && e1.endVertex == e2.endVertex )
    || ( e1.startVertex == e2.endVertex && e1.endVertex == e2.startVertex );
}
inline bool operator!=(const Edge& e1, const Edge& e2)
{
  return !(e1==e2);
}

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

  std::string toJson() const;
  
  int numVertices() const;
  int numPolygons() const;
  int numEdges() const;

  Vertex getVertex( int index ) const;
  Edge getEdge( int index ) const { return edges[index]; }
  Polygon getPolygon( int index ) const { return polygons[index]; }
  
private:
  void constructPolygons();
  std::vector< double > coordinates;
  std::vector< Edge > edges;
  std::vector< Polygon > polygons;
};

#endif // MESH_H
