#pragma once

#include <spine/SmartMetEngine.h>
#include <grid-content/contentServer/cache/CacheImplementation.h>
#include <grid-content/contentServer/redis/RedisImplementation.h>
#include <grid-content/dataServer/implementation/ServiceImplementation.h>
#include <grid-content/dataServer/cache/CacheImplementation.h>
#include <grid-content/queryServer/implementation/ServiceImplementation.h>
#include <libconfig.h++>

#include "Query.h"
#include "ParameterInfo.h"
#include "ParameterAlias.h"

extern "C"
{
  #include <lua.h>
  #include <lauxlib.h>
  #include <lualib.h>
}

namespace SmartMet
{
namespace Engine
{
namespace Grid
{

typedef std::shared_ptr<ContentServer::ServiceInterface> ContentServer_sptr;
typedef std::shared_ptr<DataServer::ServiceInterface> DataServer_sptr;
typedef std::shared_ptr<QueryServer::ServiceInterface> QueryServer_sptr;
typedef std::vector<ParameterInfo> ParameterInfo_vec;
typedef std::vector<std::pair<std::string,T::GeometryId>> Producer_vec;
typedef std::map<std::string,ParameterAlias> ParameterAlias_vec;
typedef std::vector<ParameterValues> ParameterValues_vec;


class Engine : public SmartMet::Spine::SmartMetEngine
{
  public:

                        Engine(const char *theConfigFile);
    virtual             ~Engine();

    void                executeQuery(Query& query);

    ContentServer_sptr  getContentServer_sptr();
    DataServer_sptr     getDataServer_sptr();
    QueryServer_sptr    getQueryServer_sptr();

  protected:

    void                getGridValues(
                            Producer_vec& producers,
                            ParameterInfo_vec& parameterInfoList,
                            T::ForecastType forecastType,
                            T::ForecastNumber forecastNumber,
                            std::string forecastTime,
                            bool timeMatchRequired,
                            QueryCoordinates& coordinates,
                            bool areaSearch,
                            ParameterValues& valueList);

    void                  getGridValues(
                            Producer_vec& producers,
                            ParameterInfo_vec& parameterInfoList,
                            T::ForecastType forecastType,
                            T::ForecastNumber forecastNumber,
                            std::string startTime,
                            std::string endTime,
                            QueryCoordinates& coordinates,
                            bool areaSearch,
                            ParameterValues_vec& valueList);


    void                getParameterInfoList(
                            T::ParamKeyType parameterKeyType,
                            T::ParamId paramKey,
                            T::ParamLevelIdType paramLevelIdType,
                            T::ParamLevelId paramLevelId,
                            T::ParamLevel paramLevel,
                            ParameterInfo_vec& parameterInfoList);

    void                init();
    void                shutdown();

    void                checkConfigurationUpdates();
    void                getParameterLevelInfo(std::string param,std::string& key,T::ParamLevelId& paramLevelId,T::ParamLevel& paramLevel,T::ForecastType& forecastType,T::ForecastNumber& forecastNumber);
    bool                getFunctionParams(std::string functionParamsStr,std::vector<std::string>& functionParams);
    bool                getParameterFunctionInfo(std::string paramStr,std::string& function,std::string& functionParams);
    void                getParameterInfoList(std::string parameterName,T::ParamLevelIdType paramLevelIdType,T::ParamLevelId paramLevelId,T::ParamLevel paramLevel,std::vector<ParameterInfo>& infoList);
    void                getProducerList(Query& query,Producer_vec& producers);
    void                getGeometryIdListByCoordinates(QueryCoordinates& coordinates,std::set<T::GeometryId>& geometryIdList);
    T::ParamValue       executeFunction(std::string& function,std::vector<T::ParamValue>& parameters);
    void                executeQueryFunctions(Query& query);
    void                executeTimeRangeQuery(Query& query);
    void                executeTimeStepQuery(Query& query);
    void                loadLuaFunctionFile();
    void                loadParameterFile();
    void                loadParameterAliasFile();
    void                loadProducerFile();
    void                parseFunction(std::string paramStr,std::string& function,std::vector<std::string>& functionParams,std::vector<QueryParameter>& additionalParameterList);
    void                updateQueryParameters(Query& query);

  private:

    libconfig::Config   mConfig;

    std::string         mRedisAddress;
    int                 mRedisPort;
    std::string         mRedisTablePrefix;

    ContentServer_sptr  mContentServerCache;
    ContentServer_sptr  mContentServerRedis;
    std::string         mRemoteContentServerEnabled;
    std::string         mRemoteContentServerIor;

    DataServer_sptr     mDataServer;
    DataServer_sptr     mDataServerClient;
    std::string         mRemoteDataServerEnabled;
    std::string         mRemoteDataServerCache;
    std::string         mRemoteDataServerIor;

    QueryServer_sptr    mQueryServer;
    std::string         mRemoteQueryServerEnabled;
    std::string         mRemoteQueryServerIor;

    std::string         mGridConfigDirectory;

    std::string         mServerGridDirectory;
    std::string         mServerProcessingLogFile;
    int                 mServerProcessingLogMaxSize;
    int                 mServerProcessingLogTruncateSize;
    Log                 mProcessingLog;

    uint                mNumOfCachedGrids;
    uint                mMaxCompressedMegaBytesOfCachedGrids;
    uint                mMaxUncompressedMegaBytesOfCachedGrids;

    std::string         mParameterAliasFile;
    time_t              mParameterAliasFileModificationTime;
    ParameterAlias_vec  mParameterAliasList;

    std::string         mParameterFile;
    time_t              mParameterFileModificationTime;
    ParameterInfo_vec   mParameterList;

    std::string         mProducerFile;
    time_t              mProducerFileModificationTime;
    Producer_vec        mProducerList;

    std::string         mLuaFunctionFile;
    time_t              mLuaFunctionFileModificationTime;
    lua_State*          mLuaState;

    time_t              mLastConfiguratonCheck;
    ThreadLock          mThreadLock;
};

}  // namespace Grid
}  // namespace Engine
}  // namespace SmartMet
