#pragma once

#include "QueryParameter.h"


namespace SmartMet
{
namespace Engine
{
namespace Grid
{


typedef std::vector<std::vector<T::Coordinate>> QueryCoordinates;
typedef std::vector<QueryParameter> QueryParameter_vec;


class Query
{
  public:
                              Query();
                              Query(Query& query);
    virtual                   ~Query();

    bool                      parameterInQuery(std::string param);
    QueryParameter*           getQueryParameterPtr(std::string param);
    uint                      getValuesPerTimeStep();
    void                      removeTemporaryParameters();
    void                      print(std::ostream& stream,uint level,uint optionFlags);

    std::vector<std::string>  mProducerNameList;
    std::vector<std::string>  mForecastTimeList;
    QueryCoordinates          mCoordinateList;
    QueryParameter_vec        mQueryParameterList;
    std::string               mStartTime;
    std::string               mEndTime;
    bool                      mTimeRangeQuery;
    bool                      mAreaQuery;
};


}  // namespace Grid
}  // namespace Engine
}  // namespace SmartMet

