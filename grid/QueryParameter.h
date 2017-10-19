#pragma once

#include "grid-content/contentServer/definition/ProducerInfo.h"
#include "grid-content/contentServer/definition/GenerationInfo.h"
#include "grid-content/contentServer/definition/ContentInfoList.h"
#include "grid-files/grid/Typedefs.h"
#include "grid-files/grid/GridValueList.h"


namespace SmartMet
{
namespace Engine
{
namespace Grid
{


class QueryParameter
{
  public:
                            QueryParameter();
                            QueryParameter(const QueryParameter& queryParameter);
    virtual                 ~QueryParameter();

    QueryParameter*         duplicate();
    void                    print(std::ostream& stream,uint level,uint optionFlags);

    std::string             mParam;
    std::string             mName;
    T::ParamKeyType         mParameterKeyType;
    T::ParamId              mParameterKey;
    T::ParamLevelIdType     mParameterLevelIdType;
    T::ParamLevelId         mParameterLevelId;
    T::ParamLevel           mLevel;
    T::ForecastType         mForecastType;
    T::ForecastNumber       mForecastNumber;
    T::InterpolationMethod  mInterpolationMethod;

    std::vector<std::pair<std::string,T::GridValueList>> mValueList;
};


}  // namespace GridTimeseries
}  // namespace Plugin
}  // namespace SmartMet

