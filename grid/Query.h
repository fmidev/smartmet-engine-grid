#pragma once

#include "QueryParameter.h"


namespace SmartMet
{
namespace Engine
{
namespace Grid
{


class Query
{
  public:
              Query();
              Query(Query& query);
    virtual   ~Query();

    void      print(std::ostream& stream,uint level,uint optionFlags);

    std::vector<std::string>     mProducerNameList;
    std::vector<std::string>     mForecastTimeList;
    std::vector<T::Coordinate>   mCoordinateList;
    std::vector<QueryParameter>  mQueryParameterList;
    bool                         mAreaCoordinates;
};


}  // namespace GridTimeseries
}  // namespace Plugin
}  // namespace SmartMet

