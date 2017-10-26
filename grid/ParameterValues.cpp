#include "ParameterValues.h"
#include "grid-files/common/GeneralFunctions.h"


namespace SmartMet
{
namespace Engine
{
namespace Grid
{


ParameterValues::ParameterValues()
{
  try
  {
    mProducerId = 0;
    mGenerationId = 0;
    mGeometryId = 0;
    mParameterKeyType = T::ParamKeyType::UNKNOWN;
    mParameterLevelIdType = T::ParamLevelIdType::FMI;
    mParameterLevelId = 0;
    mParameterLevel = 0;
    mForecastType = -1;
    mForecastNumber = -1;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}




ParameterValues::ParameterValues(const ParameterValues& parameterValues)
{
  try
  {
    mForecastTime = parameterValues.mForecastTime;
    mProducerId = parameterValues.mProducerId;
    mGenerationId = parameterValues.mGenerationId;
    mGeometryId = parameterValues.mGeometryId;

    mParameterKeyType = parameterValues.mParameterKeyType;
    mParameterKey = parameterValues.mParameterKey;
    mParameterLevelIdType = parameterValues.mParameterLevelIdType;
    mParameterLevelId = parameterValues.mParameterLevelId;
    mParameterLevel = parameterValues.mParameterLevel;
    mForecastType = parameterValues.mForecastType;
    mForecastNumber = parameterValues.mForecastNumber;

    mValueList = parameterValues.mValueList;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}




ParameterValues::~ParameterValues()
{
  try
  {
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}





void ParameterValues::print(std::ostream& stream,uint level,uint optionFlags)
{
  try
  {
    stream << space(level) << "ParameterValues\n";
    stream << space(level) << "- mForecastTime         = " << mForecastTime << "\n";
    stream << space(level) << "- mProducerId           = " << mProducerId << "\n";
    stream << space(level) << "- mGenerationId         = " << mGenerationId << "\n";
    stream << space(level) << "- mGeometryId           = " << mGeometryId << "\n";
    stream << space(level) << "- mParameterKeyType     = " << (int)mParameterKeyType << "\n";
    stream << space(level) << "- mParameterKey         = " << mParameterKey << "\n";
    stream << space(level) << "- mParameterLevelIdType = " << (int)mParameterLevelIdType << "\n";
    stream << space(level) << "- mParameterLevelId     = " << (int)mParameterLevelId << "\n";
    stream << space(level) << "- mParameterLevel       = " << mParameterLevel << "\n";
    stream << space(level) << "- mForecastType         = " << mForecastType << "\n";
    stream << space(level) << "- mForecastNumber       = " << mForecastNumber << "\n";
    stream << space(level) << "- mValueList            = \n";

    uint len = mValueList.getLength();
    for (uint t=0; t<len; t++)
    {
      T::GridValue *rec = mValueList.getGridValueByIndex(t);
      rec->print(stream,level+2,optionFlags);
    }
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}



}  // namespace Grid
}  // namespace Engine
}  // namespace SmartMet

