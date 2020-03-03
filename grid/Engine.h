#pragma once

#include <spine/SmartMetEngine.h>
#include <gis/DEM.h>
#include <gis/LandCover.h>
#include <grid-content/contentServer/cache/CacheImplementation.h>
#include <grid-content/contentServer/redis/RedisImplementation.h>
#include <grid-content/dataServer/implementation/ServiceImplementation.h>
#include <grid-content/dataServer/cache/CacheImplementation.h>
#include <grid-content/queryServer/implementation/ServiceImplementation.h>
#include <grid-files/common/ConfigurationFile.h>
#include <grid-files/common/Typedefs.h>
#include <pthread.h>
#include "ParameterDetails.h"


namespace SmartMet
{
namespace Engine
{
namespace Grid
{

typedef std::shared_ptr<ContentServer::ServiceInterface> ContentServer_sptr;
typedef std::shared_ptr<DataServer::ServiceInterface> DataServer_sptr;
typedef std::shared_ptr<QueryServer::ServiceInterface> QueryServer_sptr;


class Engine : public SmartMet::Spine::SmartMetEngine
{
  public:

                        Engine(const char *theConfigFile);
    virtual             ~Engine();

    int                 executeQuery(QueryServer::Query& query) const;

    ContentServer_sptr  getContentServer_sptr() const;
    ContentServer_sptr  getContentSourceServer_sptr() const;
    DataServer_sptr     getDataServer_sptr() const;
    QueryServer_sptr    getQueryServer_sptr() const;

    T::ParamLevelId     getFmiParameterLevelId(
                          uint producerId,
                          int level) const;

    void                getProducerNameList(
                          const std::string& aliasName,
                          std::vector<std::string>& nameList) const;

    std::string         getProducerName(const std::string& aliasName) const;

    std::string         getProducerAlias(
                          const std::string& producerName,
                          int levelId) const;

    std::string         getParameterString(
                          std::string producer,
                          std::string parameter) const;

    void                getParameterDetails(
                          const std::string& aliasName,
                          ParameterDetails_vec& parameterDetails) const;

    void                getParameterDetails(
                          const std::string& producerName,
                          const std::string& parameterName,
                          ParameterDetails_vec& parameterDetails) const;

    void                getParameterDetails(
                          const std::string& producerName,
                          const std::string& parameterName,
                          std::string& level,
                          ParameterDetails_vec& parameterDetails) const;

    void                mapParameterDetails(ParameterDetails_vec& parameterDetails) const;

    void                getParameterMappings(
                          std::string producerName,
                          std::string parameterName,
                          T::GeometryId geometryId,
                          bool onlySearchEnabled,
                          QueryServer::ParameterMapping_vec& mappings) const;

    void                getParameterMappings(
                          std::string producerName,
                          std::string parameterName,
                          bool onlySearchEnabled,
                          QueryServer::ParameterMapping_vec& mappings) const;

    void                getParameterMappings(
                          std::string producerName,
                          std::string parameterName,
                          T::GeometryId geometryId,
                          T::ParamLevelIdType levelIdType,
                          T::ParamLevelId levelId,
                          T::ParamLevel level,
                          bool onlySearchEnabled,
                          QueryServer::ParameterMapping_vec& mappings) const;

    void                getParameterMappings(
                          std::string producerName,
                          std::string parameterName,
                          T::ParamLevelIdType levelIdType,
                          T::ParamLevelId levelId,
                          T::ParamLevel level,
                          bool onlySearchEnabled,
                          QueryServer::ParameterMapping_vec& mappings) const;

    void                getProducerList(string_vec& producerList) const;

    bool                getProducerInfoByName(const std::string& name,T::ProducerInfo& info) const;

    void                getProducerParameterLevelList(
                          const std::string& producerName,
                          T::ParamLevelId fmiParamLevelId,
                          double multiplier,
                          std::set<double>& levels) const;

    void                getProducerParameterLevelIdList(
                          const std::string& producerName,
                          std::set<T::ParamLevelId>& levelIdList) const;

    void                getVerticalGrid(
                            double lon1,
                            double lat1,
                            double lon2,
                            double lat2,
                            int steps,
                            std::string utcTime,
                            std::string valueProducerName,
                            std::string valueParameter,
                            std::string heightProducerName,
                            std::string heightParameter,
                            int geometryId,
                            int forecastType,
                            int forecastNumber,
                            short areaInterpolationMethod,
                            short timeInterpolationMethod,
                            std::vector<T::Coordinate>& coordinates,
                            std::vector<float>& gridData,
                            uint& gridWidth,
                            uint& gridHeight) const;

    bool                isGridProducer(const std::string& producer) const;
    void                setDem(boost::shared_ptr<Fmi::DEM> dem);
    void                setLandCover(boost::shared_ptr<Fmi::LandCover> landCover);
    void                updateProcessing();

  protected:

    void                init();
    void                shutdown();
    void                startUpdateProcessing();
    void                clearMappings();
    void                loadMappings(QueryServer::ParamMappingFile_vec& parameterMappings);
    FILE*               openMappingFile(const std::string& mappingFile);
    void                updateMappings();


    void                updateMappings(
                          T::ParamKeyType sourceParameterKeyType,
                          T::ParamKeyType targetParameterKeyType,
                          const std::string& mappingFile,
                          QueryServer::ParamMappingFile_vec& parameterMappings);


  private:

    ContentServer_sptr  mContentServer;
    ContentServer_sptr  mContentServerCache;

    DataServer_sptr     mDataServer;
    DataServer_sptr     mDataServerClient;
    string_vec          mDataServerLuaFiles;
    bool                mDataServerRemote;
    bool                mDataServerCacheEnabled;
    std::string         mDataServerIor;

    QueryServer_sptr    mQueryServer;
    string_vec          mQueryServerLuaFiles;

    std::string         mGridConfigFile;
    bool                mPointCacheEnabled;
    uint                mPointCacheHitsRequired;
    uint                mPointCacheTimePeriod;
    bool                mRequestCounterEnabled;
    std::string         mGeneratedCounterFile;
    std::string         mGeneratedPreloadFile;
    std::string         mDataServerGridDirectory;

    std::string         mContentSourceType;
    std::string         mContentSourceRedisAddress;
    int                 mContentSourceRedisPort;
    std::string         mContentSourceRedisTablePrefix;
    std::string         mContentSourceCorbaIor;
    std::string         mContentSourceHttpUrl;

    bool                mContentCacheEnabled;
    uint                mContentCacheSortingFlags;

    bool                mContentServerProcessingLogEnabled;
    std::string         mContentServerProcessingLogFile;
    int                 mContentServerProcessingLogMaxSize;
    int                 mContentServerProcessingLogTruncateSize;
    Log                 mContentServerProcessingLog;
    bool                mContentServerDebugLogEnabled;
    std::string         mContentServerDebugLogFile;
    int                 mContentServerDebugLogMaxSize;
    int                 mContentServerDebugLogTruncateSize;
    Log                 mContentServerDebugLog;

    bool                mDataServerProcessingLogEnabled;
    std::string         mDataServerProcessingLogFile;
    int                 mDataServerProcessingLogMaxSize;
    int                 mDataServerProcessingLogTruncateSize;
    Log                 mDataServerProcessingLog;
    bool                mDataServerDebugLogEnabled;
    std::string         mDataServerDebugLogFile;
    int                 mDataServerDebugLogMaxSize;
    int                 mDataServerDebugLogTruncateSize;
    Log                 mDataServerDebugLog;

    bool                mQueryServerRemote;
    std::string         mQueryServerIor;
    bool                mQueryServerProcessingLogEnabled;
    std::string         mQueryServerProcessingLogFile;
    int                 mQueryServerProcessingLogMaxSize;
    int                 mQueryServerProcessingLogTruncateSize;
    Log                 mQueryServerProcessingLog;
    bool                mQueryServerDebugLogEnabled;
    std::string         mQueryServerDebugLogFile;
    int                 mQueryServerDebugLogMaxSize;
    int                 mQueryServerDebugLogTruncateSize;
    Log                 mQueryServerDebugLog;

    uint                mNumOfCachedGrids;
    uint                mMaxSizeOfCachedGridsInMegaBytes;

    std::string         mProducerFile;
    std::string         mProducerAliasFile;
    bool                mVirtualFilesEnabled;
    std::string         mVirtualFileDefinitions;
    bool                mContentPreloadEnabled;
    std::string         mContentPreloadFile;
    std::string         mContentCounterFile;

    string_vec          mParameterAliasFiles;
    string_vec          mParameterMappingFiles;

    pthread_t           mThread;
    T::ParamKeyType     mMappingTargetKeyType;
    std::string         mParameterMappingUpdateFile_fmi;
    std::string         mParameterMappingUpdateFile_newbase;

    boost::shared_ptr<Fmi::DEM>               mDem;
    boost::shared_ptr<Fmi::LandCover>         mLandCover;
    mutable ConfigurationFile                 mConfigurationFile;
    mutable bool                              mShutdownRequested;
    mutable ThreadLock                        mThreadLock;
    mutable time_t                            mParameterMappingUpdateTime;
    mutable string_vec                        mProducerList;
    mutable T::ProducerInfoList               mProducerInfoList;
    mutable time_t                            mProducerList_updateTime;
    mutable T::LevelInfoList                  mLevelInfoList;
    mutable time_t                            mLevelInfoList_lastUpdate;
    mutable QueryServer::AliasFile            mProducerAliases;
    mutable QueryServer::AliasFileCollection  mParameterAliasFileCollection;
    mutable QueryServer::ParamMappingFile_vec mParameterMappings;
    mutable T::GenerationInfoList             mGenerationList;
};

}  // namespace Grid
}  // namespace Engine
}  // namespace SmartMet
