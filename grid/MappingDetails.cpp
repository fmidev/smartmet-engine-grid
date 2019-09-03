#include "MappingDetails.h"
#include <grid-files/common/GeneralFunctions.h>
#include <boost/functional/hash.hpp>


namespace SmartMet
{
namespace Engine
{
namespace Grid
{



MappingDetails::MappingDetails()
{
  try
  {
  }
  catch (...)
  {
    throw Spine::Exception(BCP, "Operation failed!", nullptr);
  }
}





MappingDetails::MappingDetails(const MappingDetails& mappingDetails)
{
  try
  {
    mMapping = mappingDetails.mMapping;
    mTimes = mappingDetails.mTimes;
  }
  catch (...)
  {
    throw Spine::Exception(BCP, "Operation failed!", nullptr);
  }
}





MappingDetails::~MappingDetails()
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





void MappingDetails::print(std::ostream& stream,uint level,uint optionFlags)
{
  try
  {
    stream << space(level) << "MappingDetails\n";
    stream << space(level) << "- mMapping        = \n";
    mMapping.print(stream,level+2,optionFlags);
    stream << space(level) << "- mTimes          = \n";
    for (auto a = mTimes.begin(); a != mTimes.end(); ++a)
    {
      stream << space(level+2) << "- analysisTime   = " << a->first << "\n";
      for (auto t = a->second.begin(); t != a->second.end(); ++t)
      {
        stream << space(level+4) << "* " << *t << "\n";
      }
    }
  }
  catch (...)
  {
    throw Spine::Exception(BCP,exception_operation_failed,nullptr);
  }
}



}  // namespace Grid
}  // namespace Engine
}  // namespace SmartMet
