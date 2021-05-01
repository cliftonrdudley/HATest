#include "Mesh.h"

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
  throwIfWrong( mesh.numVertices(), 4 );
  throwIfWrong( mesh.numPolygons(), 2 );
  throwIfWrong( mesh.numEdges(), 5 );
}

int main()
{
  commonExample();
  return 0;
}
