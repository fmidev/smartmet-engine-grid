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
    virtual                               ~ParameterInfo();

    void                                  print(std::ostream& stream,uint level,uint optionFlags);

    std::string                           mParameterName;
    T::ParamKeyType                       mParameterKeyType;
    T::ParamId                            mParameterKey;
    boost::optional<T::ParamLevelIdType>  mParameterLevelIdType;
    boost::optional<T::ParamLevelId>      mParameterLevelId;
    boost::optional<T::ParamLevel>        mParameterLevel;
    T::InterpolationMethod                mInterpolationMethod;
};

}  // namespace Grid
}  // namespace Engine
}  // namespace SmartMet
