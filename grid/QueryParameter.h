#pragma once

#include "ParameterValues.h"


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
    virtual                       ~QueryParameter();

    QueryParameter*               duplicate();
    void                          print(std::ostream& stream,uint level,uint optionFlags);

    std::string                   mParam;
    std::string                   mSymbolicName;
    T::ParamKeyType               mParameterKeyType;
    T::ParamId                    mParameterKey;
    T::ParamLevelIdType           mParameterLevelIdType;
    T::ParamLevelId               mParameterLevelId;
    T::ParamLevel                 mParameterLevel;
    T::ForecastType               mForecastType;
    T::ForecastNumber             mForecastNumber;
    T::InterpolationMethod        mInterpolationMethod;
    bool                          mTemporary;
    std::string                   mFunction;
    std::vector<std::string>      mFunctionParams;
    std::vector<ParameterValues>  mValueList;
};


}  // namespace Grid
}  // namespace Engine
}  // namespace SmartMet

