#pragma once

#include <grid-files/grid/Typedefs.h>
#include <grid-content/queryServer/definition/ParameterMapping.h>
#include "MappingDetails.h"


namespace SmartMet
{
namespace Engine
{
namespace Grid
{


// ====================================================================================
/*! \brief Resolved parameter details for a specific producer/parameter combination.
 *
 *  Holds the original (pre-alias) producer and parameter names alongside the fully
 *  resolved query constraints and all matching parameter mappings.  Produced by
 *  Engine::getParameterDetails() and consumed by plugins building QueryServer queries. */
// ====================================================================================

class ParameterDetails
{
  public:
                                ParameterDetails();
                                ParameterDetails(const ParameterDetails& parameterDetails);
    virtual                     ~ParameterDetails();

    void                        print(std::ostream& stream,uint level,uint optionFlags);

    std::string                 mOriginalProducer;   //!< Producer name as supplied by the caller before alias resolution.
    std::string                 mOriginalParameter;  //!< Parameter name as supplied by the caller before alias resolution.

    std::string                 mProducerName;       //!< Resolved producer name.
    std::string                 mGeometryId;         //!< Requested geometry ID (empty = any).
    std::string                 mLevelId;            //!< FMI level type ID string (empty = any).
    std::string                 mLevel;              //!< Level value string (empty = any).
    std::string                 mForecastType;       //!< Forecast type string (empty = any).
    std::string                 mForecastNumber;     //!< Ensemble member or perturbation number (empty = any).
    MappingDetails_vec          mMappings;           //!< All parameter mappings matching the resolved constraints, with time windows.
};




typedef std::vector<ParameterDetails> ParameterDetails_vec;  //!< List of resolved parameter details, one entry per matched producer/parameter pair.


}  // namespace Grid
}  // namespace Engine
}  // namespace SmartMet
