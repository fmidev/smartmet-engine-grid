#include "QueryParameter.h"
#include "grid-files/common/GeneralFunctions.h"

namespace SmartMet
{
namespace Engine
{
namespace Grid
{


QueryParameter::QueryParameter()
{
  try
  {
    mParameterKeyType = T::ParamKeyType::UNKNOWN;
    mParameterLevelIdType = T::ParamLevelIdType::FMI;
    mParameterLevelId = 0;
    mLevel = 0;
    mForecastType = -1;
    mForecastNumber = -1;
    mInterpolationMethod = T::InterpolationMethod::Linear;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}





QueryParameter::QueryParameter(const QueryParameter& queryParameter)
{
  try
  {
    mParam = queryParameter.mParam;
    mName = queryParameter.mName;
    mParameterKeyType = queryParameter.mParameterKeyType;
    mParameterKey = queryParameter.mParameterKey;
    mParameterLevelIdType = queryParameter.mParameterLevelIdType;
    mParameterLevelId = queryParameter.mParameterLevelId;
    mLevel = queryParameter.mLevel;
    mForecastType = queryParameter.mForecastType;
    mForecastNumber = queryParameter.mForecastNumber;
    mInterpolationMethod = queryParameter.mInterpolationMethod;

    mValueList = queryParameter.mValueList;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}





QueryParameter::~QueryParameter()
{
  try
  {
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}





QueryParameter* QueryParameter::duplicate()
{
  try
  {
    return new QueryParameter(*this);
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}





void QueryParameter::print(std::ostream& stream,uint level,uint optionFlags)
{
  try
  {
    stream << space(level) << "QueryParameter\n";
    stream << space(level) << "- mParam                = " << mParam << "\n";
    stream << space(level) << "- mName                 = " << mName << "\n";
    stream << space(level) << "- mParameterKeyType     = " << (uint)mParameterKeyType << "\n";
    stream << space(level) << "- mParameterKey         = " << mParameterKey << "\n";
    stream << space(level) << "- mParameterLevelIdType = " << (uint)mParameterLevelIdType << "\n";
    stream << space(level) << "- mParameterLevelId     = " << mParameterLevelId << "\n";
    stream << space(level) << "- mLevel                = " << mLevel << "\n";
    stream << space(level) << "- mForecastType         = " << mForecastType << "\n";
    stream << space(level) << "- mForecastNumber       = " << mForecastNumber << "\n";
    stream << space(level) << "- mInterpolationMethod  = " << (uint)mInterpolationMethod << "\n";

    stream << space(level) << "- mValueList            = \n";

    for (auto it = mValueList.begin();  it != mValueList.end(); ++it)
    {
      stream << space(level) << "  * " << it->first << "\n";

      uint len = it->second.getLength();
      for (uint t=0; t<len; t++)
      {
        T::GridValue *rec = it->second.getGridValueByIndex(t);
        rec->print(stream,level+2,optionFlags);
      }
    }

  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}






}  // namespace GridTimeseries
}  // namespace Plugin
}  // namespace SmartMet

