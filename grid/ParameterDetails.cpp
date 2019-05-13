#include "ParameterDetails.h"
#include <grid-files/common/GeneralFunctions.h>
#include <boost/functional/hash.hpp>


namespace SmartMet
{
namespace Engine
{
namespace Grid
{



ParameterDetails::ParameterDetails()
{
  try
  {
  }
  catch (...)
  {
    throw Spine::Exception(BCP, "Operation failed!", nullptr);
  }
}





ParameterDetails::ParameterDetails(const ParameterDetails& parameterDetails)
{
  try
  {
    mProducerName = parameterDetails.mProducerName;
    mGeometryId = parameterDetails.mGeometryId;
    mLevelId = parameterDetails.mLevelId;
    mLevel = parameterDetails.mLevel;
    mForecastType = parameterDetails.mForecastType;
    mForecastNumber = parameterDetails.mForecastNumber;
  }
  catch (...)
  {
    throw Spine::Exception(BCP, "Operation failed!", nullptr);
  }
}





ParameterDetails::~ParameterDetails()
{
  try
  {
  }
  catch (...)
  {
    SmartMet::Spine::Exception exception(BCP,"Destructor failed",nullptr);
    exception.printError();
  }
}





void ParameterDetails::print(std::ostream& stream,uint level,uint optionFlags)
{
  try
  {
    stream << space(level) << "ParameterDetails\n";
    stream << space(level) << "- mProducerName     = " << mProducerName << "\n";
    stream << space(level) << "- mGeometryId       = " << mGeometryId << "\n";
    stream << space(level) << "- mLevelId          = " << mLevelId << "\n";
    stream << space(level) << "- mLevel            = " << mLevel << "\n";
    stream << space(level) << "- mForecastType     = " << mForecastType << "\n";
    stream << space(level) << "- mForecastNumber   = " << mForecastNumber << "\n";
  }
  catch (...)
  {
    throw Spine::Exception(BCP,exception_operation_failed,nullptr);
  }
}



}  // namespace Grid
}  // namespace Engine
}  // namespace SmartMet
