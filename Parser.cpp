#include "Parser.h"

#include <regex>
#include <sstream>

// Improvement: Use jsoncpp
template< typename T >
std::pair< T, T> returnPairFromString( std::string input )
{
  std::stringstream pairStream( input );
  char unusedChar;
  T firstValue, secondValue;
  pairStream >> firstValue >> unusedChar >> secondValue;
  return std::make_pair( firstValue, secondValue );
}

std::vector< double > verticesFromJson( const std::string& vertexContent )
{
  std::vector< double > result;
  std::regex pairFinder("([[:digit:][:space:].]+,[[:digit:][:space:].]+)+");
  auto pairsBegin = std::sregex_iterator( vertexContent.begin(), vertexContent.end(), pairFinder );
  auto pairsEnd = std::sregex_iterator();
  // Parse vertex results.
  for ( std::sregex_iterator pair = pairsBegin; pair != pairsEnd; pair++ ) {
    std::string pairMatch = pair->str();
    const auto [ xCoord, yCoord ] = returnPairFromString<double>( pairMatch );
    result.push_back( xCoord );
    result.push_back( yCoord );
  }
  return result;
}

std::vector< Edge > edgesFromJson( const std::string& edgeContent )
{
  std::vector< Edge > result;
  // Parse edge results
  std::regex pairFinder("([[:digit:][:space:]]+,[[:digit:][:space:]]+)+");
  auto pairsBegin = std::sregex_iterator( edgeContent.begin(), edgeContent.end(), pairFinder );
  auto pairsEnd = std::sregex_iterator();
  for ( std::sregex_iterator pair = pairsBegin; pair != pairsEnd; pair++ ) {
    std::string pairMatch = pair->str();
    const auto [ startVertex, endVertex ] = returnPairFromString<int>( pairMatch );
    result.push_back( Edge{startVertex,endVertex} );
  }
  
  return result;
}
