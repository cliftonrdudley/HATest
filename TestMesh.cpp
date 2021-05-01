#include "Mesh.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

void throwIfWrong( int found, int expected )
{
  if ( found != expected ) {
    std::string errorMessage = "Bad data discovered in TestMesh. Expected ";
    errorMessage += std::to_string( expected ) + " but found " + std::to_string( found );
    throw std::runtime_error( errorMessage );
  }
}

// TODO: Corner cases and bad data.

void commonExample()
{
  std::stringstream input;
  input << "{";
  input << "\"vertices\": [[0, 0], [2, 0], [2, 2], [0, 2]],";
  input << "\"edges\": [[0, 1], [1, 2], [0, 2], [0, 3], [2, 3]]";
  input << "}";
  Mesh mesh = Mesh::fromJson( input );
  std::ofstream fout("with_polys.json");
  fout << mesh.toJson() << std::endl;

  throwIfWrong( mesh.numVertices(), 4 );
  throwIfWrong( mesh.numEdges(), 5 );
  throwIfWrong( mesh.numPolygons(), 2 );
  throwIfWrong( mesh.getPolygon(0).vertices.size(), 3 );
  throwIfWrong( mesh.getPolygon(1).vertices.size(), 3 );
}

int main()
{
  commonExample();
  return 0;
}
