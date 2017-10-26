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
    mTimeMatchRequired = false;
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
    mStartTime = query.mStartTime;
    mEndTime = query.mEndTime;
    mTimeMatchRequired = query.mTimeMatchRequired;
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

    stream << space(level) << "- mStartTime              = " << mStartTime << "\n";
    stream << space(level) << "- mEndTime                = " << mEndTime << "\n";
    stream << space(level) << "- mAreaCoordinates        = " << (int)mAreaCoordinates << "\n";
    stream << space(level) << "- mTimeMatchRequired      = " << (int)mTimeMatchRequired << "\n";

    stream << space(level) << "- mForecastTimeList\n";
    for (auto it = mForecastTimeList.begin(); it != mForecastTimeList.end(); ++it)
      stream << space(level) << "   * " << *it << "\n";

    stream << space(level) << "- mProducerNameList       = \n";
    for (auto it = mProducerNameList.begin(); it != mProducerNameList.end(); ++it)
      stream << space(level) << "   * " << *it << "\n";

    stream << space(level) << "- mQueryParameterListList = \n";
    for (auto it = mQueryParameterList.begin(); it != mQueryParameterList.end(); ++it)
      it->print(stream,level+2,optionFlags);

    stream << space(level) << "- mCoordinateList         = \n";
    for (auto cList = mCoordinateList.begin(); cList != mCoordinateList.end(); ++cList)
    {
      for (auto it = cList->begin(); it != cList->end(); ++it)
        stream << space(level) << "   * " << it->x() << "," << it->y() << "\n";
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

