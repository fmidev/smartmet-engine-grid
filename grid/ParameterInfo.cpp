#include "ParameterInfo.h"
#include <grid-files/common/GeneralFunctions.h>


namespace SmartMet
{
namespace Engine
{
namespace Grid
{


ParameterInfo::ParameterInfo()
{
  try
  {
    mParameterKeyType = T::ParamKeyType::UNKNOWN;
    mParamLevelIdType = T::ParamLevelIdType::ANY;
    mParamLevelId = 0;
    mParamLevel = 0;
    mInterpolationMethod = T::InterpolationMethod::Linear;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}





ParameterInfo::ParameterInfo(const ParameterInfo& info)
{
  try
  {
    mParameterName = info.mParameterName;
    mParameterKeyType = info.mParameterKeyType;
    mParamKey = info.mParamKey;
    mParamLevelIdType = info.mParamLevelIdType;
    mParamLevelId = info.mParamLevelId;
    mParamLevel = info.mParamLevel;
    mInterpolationMethod = info.mInterpolationMethod;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}





ParameterInfo::~ParameterInfo()
{
  try
  {
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}





void ParameterInfo::print(std::ostream& stream,uint level,uint optionFlags)
{
  try
  {
    stream << space(level) << "ParameterInfo\n";
    stream << space(level) << "- mParameterName       = " << mParameterName << "\n";
    stream << space(level) << "- mParameterKeyType    = " << (uint)mParameterKeyType << "\n";
    stream << space(level) << "- mParamKey            = " << mParamKey << "\n";
    stream << space(level) << "- mParamLevelIdType    = " << (uint)mParamLevelIdType << "\n";
    stream << space(level) << "- mParamLevelId        = " << (uint)mParamLevelId << "\n";
    stream << space(level) << "- mParamLevel          = " << mParamLevel << "\n";
    stream << space(level) << "- mInterpolationMethod = " << (uint)mInterpolationMethod << "\n";
}
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,NULL);
  }
}


}  // namespace Grid
}  // namespace Engine
}  // namespace SmartMet
