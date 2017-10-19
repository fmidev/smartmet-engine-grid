#pragma once

#include <spine/SmartMetEngine.h>
#include <grid-content/contentServer/cache/CacheImplementation.h>
#include <grid-content/contentServer/redis/RedisImplementation.h>
#include <grid-content/dataServer/implementation/ServiceImplementation.h>
#include <grid-content/dataServer/cache/CacheImplementation.h>
#include <grid-content/queryServer/implementation/ServiceImplementation.h>
#include <libconfig.h++>

#include "ParameterInfo.h"


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



class Engine : public SmartMet::Spine::SmartMetEngine
{
  public:

                        Engine(const char *theConfigFile);
    virtual             ~Engine();

    int                 getGridValues(
                            std::vector<std::string> producerNameList,
                            std::string originTime,
                            T::ParamKeyType parameterKeyType,
                            T::ParamId paramKey,
                            T::ParamLevelIdType paramLevelIdType,
                            T::ParamLevelId paramLevelId,
                            T::ParamLevel paramLevel,
                            T::ForecastType forecastType,
                            T::ForecastNumber forecastNumber,
                            std::string forecastTime,
                            std::vector<T::Coordinate> coordinates,
                            bool areaSearch,                          // If true then coordinates defines the area polygon.
                            T::GridValueList& valueList);

    ContentServer_sptr  getContentServer_sptr();
    DataServer_sptr     getDataServer_sptr();
    QueryServer_sptr    getQueryServer_sptr();

  protected:

    void                init();
    void                shutdown();
    void                loadParameters();
    void                loadProducers();
    void                getParameterInfoList(std::string parameterName,std::vector<ParameterInfo>& infoList);
    void                getGeometryIdListByCoordinates(std::vector<T::Coordinate>& coordinates,std::set<T::GeometryId>& geometryIdList);

  private:

    libconfig::Config   mConfig;
    std::string         mRedisAddress;
    int                 mRedisPort;
    std::string         mRedisTablePrefix;
    std::string         mRemoteContentServerEnabled;
    std::string         mRemoteContentServerIor;
    std::string         mRemoteDataServerEnabled;
    std::string         mRemoteDataServerCache;
    std::string         mRemoteDataServerIor;
    std::string         mRemoteQueryServerEnabled;
    std::string         mRemoteQueryServerIor;
    std::string         mGridConfigDirectory;
    std::string         mServerProcessingLogFile;
    int                 mServerProcessingLogMaxSize;
    int                 mServerProcessingLogTruncateSize;
    std::string         mServerGridDirectory;
    uint                mNumOfCachedGrids;
    uint                mMaxCompressedMegaBytesOfCachedGrids;
    uint                mMaxUncompressedMegaBytesOfCachedGrids;
    uint                mCacheExpirationTime;
    Log                 mProcessingLog;
    std::string         mParameterFile;
    std::string         mProducerFile;
    ParameterInfo_vec   mParameters;
    Producer_vec        mProducerVector;
    ContentServer_sptr  mContentServerCache;
    ContentServer_sptr  mContentServerRedis;
    DataServer_sptr     mDataServer;
    DataServer_sptr     mDataServerClient;
    QueryServer_sptr    mQueryServer;
};

}  // namespace Grid
}  // namespace Engine
}  // namespace SmartMet
