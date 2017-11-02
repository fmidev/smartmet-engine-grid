#include "ParameterAlias.h"
#include <grid-files/common/GeneralFunctions.h>


namespace SmartMet
{
namespace Engine
{
namespace Grid
{


ParameterAlias::ParameterAlias()
{
  try
  {
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}





ParameterAlias::ParameterAlias(const ParameterAlias& alias)
{
  try
  {
    mName = alias.mName;
    mTitle = alias.mTitle;
    mParameterString = alias.mParameterString;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}





ParameterAlias::~ParameterAlias()
{
  try
  {
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}





void ParameterAlias::print(std::ostream& stream,uint level,uint optionFlags)
{
  try
  {
    stream << space(level) << "ParameterAlias\n";
    stream << space(level) << "- mName             = " << mName << "\n";
    stream << space(level) << "- mTitle            = " << mTitle << "\n";
    stream << space(level) << "- mParameterString  = " << mParameterString << "\n";
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,NULL);
  }
}


}  // namespace Grid
}  // namespace Engine
}  // namespace SmartMet
