#pragma once

#include <grid-files/grid/Typedefs.h>
#include <grid-content/queryServer/definition/ParameterMapping.h>


namespace SmartMet
{
namespace Engine
{
namespace Grid
{

typedef std::map<std::string,std::set<std::string>> Times;  //!< Maps an analysis time string to the set of content identifiers valid at that time.

// ====================================================================================
/*! \brief Extends a QueryServer parameter mapping with the set of forecast times for
 *  which matching content is available.
 *
 *  Used by the engine to cache resolved parameter → file mappings together with their
 *  valid time windows so that query-time lookup can skip content searches when the
 *  requested time falls outside all known windows. */
// ====================================================================================

class MappingDetails
{
  public:
                                  MappingDetails();
                                  MappingDetails(const MappingDetails& mappingDetails);
    virtual                       ~MappingDetails();

    void                          print(std::ostream& stream,uint level,uint optionFlags);

    QueryServer::ParameterMapping mMapping;  //!< Underlying parameter mapping from the QueryServer.
    Times                         mTimes;    //!< Analysis times at which this mapping has available content.
};



typedef std::vector<MappingDetails> MappingDetails_vec;  //!< Ordered list of resolved parameter mappings with time windows.


}  // namespace Grid
}  // namespace Engine
}  // namespace SmartMet
