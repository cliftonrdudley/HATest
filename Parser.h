#ifndef PARSER_H
#define PARSER_H

#include "Mesh.h"

#include <string>
#include <vector>

// Improvement: namespaces

std::vector< double > verticesFromJson( const std::string& vertexContent );

std::vector< Edge > edgesFromJson( const std::string& edgeContent );

//std::vector< Polygon > polygonsFromJson( const std::string& polygonContent );
#endif // PARSER_H
