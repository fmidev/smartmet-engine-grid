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
    //mParameterLevelIdType = T::ParamLevelIdType::ANY;
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
    mParameterKey = info.mParameterKey;
    mParameterLevelIdType = info.mParameterLevelIdType;
    mParameterLevelId = info.mParameterLevelId;
    mParameterLevel = info.mParameterLevel;
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
    stream << space(level) << "- mParameterName         = " << mParameterName << "\n";
    stream << space(level) << "- mParameterKeyType      = " << (uint)mParameterKeyType << "\n";
    stream << space(level) << "- mParameterKey          = " << mParameterKey << "\n";

    if (mParameterLevelIdType)
      stream << space(level) << "- mParameterLevelIdType  = " << (uint)(*mParameterLevelIdType) << "\n";
    else
      stream << space(level) << "- mParameterLevelIdType  = \n";

    if (mParameterLevelId)
      stream << space(level) << "- mParameterLevelId      = " << (uint)(*mParameterLevelId) << "\n";
    else
      stream << space(level) << "- mParameterLevelId      = \n";

    if (mParameterLevel)
      stream << space(level) << "- mParameterLevel        = " << (*mParameterLevel) << "\n";
    else
      stream << space(level) << "- mParameterLevel        = \n";

    stream << space(level) << "- mInterpolationMethod   = " << (uint)mInterpolationMethod << "\n";
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,NULL);
  }
}


}  // namespace Grid
}  // namespace Engine
}  // namespace SmartMet
