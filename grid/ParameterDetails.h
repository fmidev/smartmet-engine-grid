#pragma once

#include <grid-files/grid/Typedefs.h>


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

    std::string                 mProducerName;
    std::string                 mGeometryId;
    std::string                 mLevelId;
    std::string                 mLevel;
    std::string                 mForecastType;
    std::string                 mForecastNumber;
};





typedef std::vector<ParameterDetails> ParameterDetails_vec;


}  // namespace Grid
}  // namespace Engine
}  // namespace SmartMet
