#include "Query.h"
#include "grid-files/common/GeneralFunctions.h"


namespace SmartMet
{
namespace Engine
{
namespace Grid
{




Query::Query()
{
  try
  {
    mAreaCoordinates = false;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}





Query::Query(Query& query)
{
  try
  {
    mProducerNameList = query.mProducerNameList;
    mForecastTimeList = query.mForecastTimeList;
    mCoordinateList = query.mCoordinateList;
    mQueryParameterList = query.mQueryParameterList;
    mAreaCoordinates = query.mAreaCoordinates;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}





Query::~Query()
{
  try
  {
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}





void Query::print(std::ostream& stream,uint level,uint optionFlags)
{
  try
  {
    stream << space(level) << "Query\n";

    for (auto it = mQueryParameterList.begin(); it != mQueryParameterList.end(); ++it)
    {
      it->print(stream,level+2,optionFlags);
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

