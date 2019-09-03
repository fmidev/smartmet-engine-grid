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


class ParameterDetails
{
  public:
                                ParameterDetails();
                                ParameterDetails(const ParameterDetails& parameterDetails);
    virtual                     ~ParameterDetails();

    void                        print(std::ostream& stream,uint level,uint optionFlags);

    std::string                 mOriginalProducer;
    std::string                 mOriginalParameter;

    std::string                 mProducerName;
    std::string                 mGeometryId;
    std::string                 mLevelId;
    std::string                 mLevel;
    std::string                 mForecastType;
    std::string                 mForecastNumber;
    MappingDetails_vec          mMappings;
};





typedef std::vector<ParameterDetails> ParameterDetails_vec;


}  // namespace Grid
}  // namespace Engine
}  // namespace SmartMet
