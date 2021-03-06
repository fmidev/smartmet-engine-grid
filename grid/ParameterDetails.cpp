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
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}





ParameterDetails::ParameterDetails(const ParameterDetails& parameterDetails)
{
  try
  {
    mOriginalProducer = parameterDetails.mOriginalProducer;
    mOriginalParameter = parameterDetails.mOriginalParameter;
    mProducerName = parameterDetails.mProducerName;
    mGeometryId = parameterDetails.mGeometryId;
    mLevelId = parameterDetails.mLevelId;
    mLevel = parameterDetails.mLevel;
    mForecastType = parameterDetails.mForecastType;
    mForecastNumber = parameterDetails.mForecastNumber;
    mMappings = parameterDetails.mMappings;
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}





ParameterDetails::~ParameterDetails()
{
  try
  {
  }
  catch (...)
  {
    Fmi::Exception exception(BCP,"Destructor failed",nullptr);
    exception.printError();
  }
}





void ParameterDetails::print(std::ostream& stream,uint level,uint optionFlags)
{
  try
  {
    stream << space(level) << "ParameterDetails\n";
    stream << space(level) << "- mOriginalProducer  = " << mOriginalProducer << "\n";
    stream << space(level) << "- mOriginalParameter = " << mOriginalParameter << "\n";
    stream << space(level) << "- mProducerName      = " << mProducerName << "\n";
    stream << space(level) << "- mGeometryId        = " << mGeometryId << "\n";
    stream << space(level) << "- mLevelId           = " << mLevelId << "\n";
    stream << space(level) << "- mLevel             = " << mLevel << "\n";
    stream << space(level) << "- mForecastType      = " << mForecastType << "\n";
    stream << space(level) << "- mForecastNumber    = " << mForecastNumber << "\n";
    stream << space(level) << "- mMappings          = \n";
    for (auto m = mMappings.begin(); m != mMappings.end(); ++m)
    {
      m->print(stream,level+2,optionFlags);
    }
  }
  catch (...)
  {
    throw Fmi::Exception(BCP,"Operation failed!",nullptr);
  }
}



}  // namespace Grid
}  // namespace Engine
}  // namespace SmartMet
