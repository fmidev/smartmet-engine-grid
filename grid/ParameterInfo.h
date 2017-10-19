#pragma once

#include <grid-files/grid/Typedefs.h>


namespace SmartMet
{
namespace Engine
{
namespace Grid
{


class ParameterInfo
{
  public:
                           ParameterInfo();
                           ParameterInfo(const ParameterInfo& info);
    virtual                ~ParameterInfo();

    void                   print(std::ostream& stream,uint level,uint optionFlags);

    std::string            mParameterName;
    T::ParamKeyType        mParameterKeyType;
    T::ParamId             mParamKey;
    T::ParamLevelIdType    mParamLevelIdType;
    T::ParamLevelId        mParamLevelId;
    T::ParamLevel          mParamLevel;
    T::InterpolationMethod mInterpolationMethod;
};

}  // namespace Grid
}  // namespace Engine
}  // namespace SmartMet
