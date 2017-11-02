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
    mAreaQuery = false;
    mTimeRangeQuery = false;
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
    mAreaQuery = query.mAreaQuery;
    mStartTime = query.mStartTime;
    mEndTime = query.mEndTime;
    mTimeRangeQuery = query.mTimeRangeQuery;
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





bool Query::parameterInQuery(std::string param)
{
  try
  {
    for (auto it = mQueryParameterList.rbegin(); it != mQueryParameterList.rend(); ++it)
    {
      if (it->mParam == param  ||  it->mSymbolicName == param)
        return true;
    }
    return false;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}





uint Query::getValuesPerTimeStep()
{
  try
  {
    uint valueCount = 0;
    for (auto param = mQueryParameterList.begin(); param != mQueryParameterList.end(); ++param)
    {
      for (auto vList = param->mValueList.begin(); vList != param->mValueList.end(); ++vList)
      {
        uint vCount = vList->mValueList.getLength();
        if (vCount > valueCount)
          valueCount = vCount;
      }
    }
    return valueCount;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}




QueryParameter* Query::getQueryParameterPtr(std::string param)
{
  try
  {
    for (auto it = mQueryParameterList.rbegin(); it != mQueryParameterList.rend(); ++it)
    {
      if (it->mParam == param  ||  it->mSymbolicName == param)
        return &(*it);
    }
    return NULL;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}





void Query::removeTemporaryParameters()
{
  try
  {
    while (mQueryParameterList.size() > 0)
    {
      if (mQueryParameterList[mQueryParameterList.size()-1].mTemporary)
        mQueryParameterList.pop_back();
      else
        return;
    }
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
    stream << space(level) << "- mAreaQuery        = " << (int)mAreaQuery << "\n";
    stream << space(level) << "- mTimeRangeQuery      = " << (int)mTimeRangeQuery << "\n";

    stream << space(level) << "- mForecastTimeList\n";
    for (auto it = mForecastTimeList.begin(); it != mForecastTimeList.end(); ++it)
      stream << space(level) << "   * " << *it << "\n";

    stream << space(level) << "- mProducerNameList       = \n";
    for (auto it = mProducerNameList.begin(); it != mProducerNameList.end(); ++it)
      stream << space(level) << "   * " << *it << "\n";

    stream << space(level) << "- mQueryParameterListList = \n";
    for (auto it = mQueryParameterList.begin(); it != mQueryParameterList.end(); ++it)
      it->print(stream,level+2,optionFlags);

    stream << space(level) << "- mPolygonPath            = \n";
    for (auto cList = mCoordinateList.begin(); cList != mCoordinateList.end(); ++cList)
    {
      stream << space(level) << "  - mCoordinateList         = \n";
      for (auto it = cList->begin(); it != cList->end(); ++it)
        stream << space(level) << "     * " << it->x() << "," << it->y() << "\n";
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

