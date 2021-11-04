#pragma once

#include <spine/SmartMetEngine.h>
#include <gis/DEM.h>
#include <gis/LandCover.h>
#include <grid-content/contentServer/cache/CacheImplementation.h>
#include <grid-content/contentServer/redis/RedisImplementation.h>
#include <grid-content/contentServer/memory/MemoryImplementation.h>
#include <grid-content/dataServer/implementation/ServiceImplementation.h>
#include <grid-content/dataServer/cache/CacheImplementation.h>
#include <grid-content/queryServer/implementation/ServiceImplementation.h>
#include <grid-files/common/ConfigurationFile.h>
#include <grid-files/common/Typedefs.h>
#include <pthread.h>
#include <unordered_map>
#include <spine/HTTP.h>
#include <spine/TableFormatter.h>
#include <spine/Table.h>
#include <spine/Value.h>
#include "ParameterDetails.h"
#include "Browser.h"


namespace SmartMet
{
namespace Engine
{
namespace Grid
{

typedef std::pair<boost::shared_ptr<Spine::Table>, Spine::TableFormatter::Names> ContentTable;
typedef std::shared_ptr<ContentServer::ServiceInterface> ContentServer_sptr;
typedef std::shared_ptr<DataServer::ServiceInterface> DataServer_sptr;
typedef std::shared_ptr<QueryServer::ServiceInterface> QueryServer_sptr;
typedef std::shared_ptr<QueryServer::Query> Query_sptr;

struct CacheRec
{
  Query_sptr query;
  std::unordered_map<uint,ulonglong> producerHashMap;
  time_t cacheTime;
  time_t lastAccessTime;
  uint   accessCounter;
};

typedef std::unordered_map<std::size_t,CacheRec> QueryCache;
typedef std::unordered_map<std::size_t,CacheRec>::iterator QueryCacheIterator;
typedef std::shared_ptr<QueryServer::ParamMappingFile_vec> ParamMappingFile_vec_sptr;

struct HashRec
{
  time_t checkTime;
  ulonglong hash;
};

typedef std::unordered_map<uint,HashRec> ProducerHash_map;



class Engine : public SmartMet::Spine::SmartMetEngine
{
  public:

                        Engine(const char *theConfigFile);
    virtual             ~Engine();

    bool                browserRequest(const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    void                browserContent(std::ostringstream& output);

    int                 executeQuery(QueryServer::Query& query) const;
    Query_sptr          executeQuery(std::shared_ptr<QueryServer::Query> query) const;

    ContentServer_sptr  getContentServer_sptr() const;
    ContentServer_sptr  getContentSourceServer_sptr() const;
    DataServer_sptr     getDataServer_sptr() const;
    QueryServer_sptr    getQueryServer_sptr() const;

    std::string         getConfigurationFileName();
    std::string         getProducerFileName();

    T::ParamLevelId     getFmiParameterLevelId(
                          uint producerId,
                          int level) const;

    void                getProducerNameList(
                          const std::string& mappingName,
                          std::vector<std::string>& nameList) const;

    std::string         getProducerName(const std::string& aliasName) const;

    std::string         getProducerAlias(
                          const std::string& producerName,
                          int levelId) const;

    ulonglong           getProducerHash(uint producerId) const;

    ContentTable        getProducerInfo(boost::optional<std::string> producer) const;

    ContentTable        getParameterInfo(boost::optional<std::string> producer) const;

    std::string         getParameterString(
                          const std::string& producer,
                          const std::string& parameter) const;

    void                getParameterAlias(
                          const std::string& aliasName,
                          std::string& aliasValue) const;

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
                          const std::string& producerName,
                          const std::string& parameterName,
                          T::GeometryId geometryId,
                          bool onlySearchEnabled,
                          QueryServer::ParameterMapping_vec& mappings) const;

    void                getParameterMappings(
                          const std::string& producerName,
                          const std::string& parameterName,
                          bool onlySearchEnabled,
                          QueryServer::ParameterMapping_vec& mappings) const;

    void                getParameterMappings(
                          const std::string& producerName,
                          const std::string& parameterName,
                          T::GeometryId geometryId,
                          T::ParamLevelId levelId,
                          T::ParamLevel level,
                          bool onlySearchEnabled,
                          QueryServer::ParameterMapping_vec& mappings) const;

    void                getParameterMappings(
                          const std::string& producerName,
                          const std::string& parameterName,
                          T::ParamLevelId levelId,
                          T::ParamLevel level,
                          bool onlySearchEnabled,
                          QueryServer::ParameterMapping_vec& mappings) const;

    void                getProducerList(string_vec& producerList) const;

    bool                getProducerInfoByName(const std::string& name,T::ProducerInfo& producerInfo) const;
    bool                getProducerInfoById(uint producerId,T::ProducerInfo& producerInfo) const;
    bool                getGenerationInfoById(uint generationId,T::GenerationInfo& generationInfo);

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
                            const std::string& utcTime,
                            const std::string& valueProducerName,
                            const std::string& valueParameter,
                            const std::string& heightProducerName,
                            const std::string& heightParameter,
                            int geometryId,
                            int forecastType,
                            int forecastNumber,
                            short areaInterpolationMethod,
                            short timeInterpolationMethod,
                            std::vector<T::Coordinate>& coordinates,
                            std::vector<float>& gridData,
                            uint& gridWidth,
                            uint& gridHeight) const;

    bool                isEnabled() const;
    bool                isGridProducer(const std::string& producer) const;
    void                setDem(boost::shared_ptr<Fmi::DEM> dem);
    void                setLandCover(boost::shared_ptr<Fmi::LandCover> landCover);
    void                updateProcessing();

  protected:

    void                init();
    void                shutdown();
    void                startUpdateProcessing();
    void                clearMappings();
    void                checkConfiguration();
    void                loadMappings(QueryServer::ParamMappingFile_vec& parameterMappings);
    FILE*               openMappingFile(const std::string& mappingFile);
    void                updateMappings();

    bool                isCacheable(std::shared_ptr<QueryServer::Query> query) const;

    void                updateMappings(
                          T::ParamKeyType sourceParameterKeyType,
                          T::ParamKeyType targetParameterKeyType,
                          const std::string& mappingFile,
                          QueryServer::ParamMappingFile_vec& parameterMappings,
                          Spine::Table& paramTable);

    void                updateQueryCache();
    void                updateProducerAndGenerationList();

  private:

    bool                mEnabled;

    std::string         mConfigurationFile_name;
    time_t              mConfigurationFile_modificationTime;
    time_t              mConfigurationFile_checkTime;

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
    bool                mRequestForwardEnabled;
    std::string         mDataServerGridDirectory;

    std::string         mContentSourceType;
    std::string         mContentSourceRedisAddress;
    int                 mContentSourceRedisPort;
    std::string         mContentSourceRedisTablePrefix;
    std::string         mContentSourceRedisSecondaryAddress;
    int                 mContentSourceRedisSecondaryPort;
    bool                mContentSourceRedisLockEnabled;
    bool                mContentSourceRedisReloadRequired;
    std::string         mContentSourceCorbaIor;
    std::string         mContentSourceHttpUrl;

    std::string         mMemoryContentDir;
    uint                mEventListMaxSize;
    bool                mContentCacheEnabled;

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
    time_t              mContentServerStartTime;

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

    bool                mMemoryMapCheckEnabled;
    bool                mVirtualFilesEnabled;
    std::string         mVirtualFileDefinitions;
    bool                mContentPreloadEnabled;
    std::string         mContentPreloadFile;
    bool                mPreloadMemoryLock;

    bool                mContentSwapEnabled;
    uint                mContentUpdateInterval;


    pthread_t           mThread;
    Browser             mBrowser;
    bool                mBrowserEnabled;
    unsigned long long  mBrowserFlags;


    DataServer::ServiceImplementation*        mDataServerImplementation;
    ContentServer::CacheImplementation*       mContentServerCacheImplementation;

    boost::shared_ptr<Fmi::DEM>               mDem;
    boost::shared_ptr<Fmi::LandCover>         mLandCover;

    mutable QueryServer::AliasFileCollection  mProducerMappingDefinitions;
    string_vec                                mProducerMappingDefinitions_filenames;

    mutable ParamMappingFile_vec_sptr         mParameterMappingDefinitions;
    mutable ModificationLock                  mParameterMappingDefinitions_modificationLock;
    mutable time_t                            mParameterMappingDefinitions_updateTime;
    string_vec                                mParameterMappingDefinitions_filenames;

    T::ParamKeyType                           mParameterMappingDefinitions_autoFileKeyType;
    std::string                               mParameterMappingDefinitions_autoFile_fmi;
    std::string                               mParameterMappingDefinitions_autoFile_newbase;
    std::string                               mParameterMappingDefinitions_autoFile_netCdf;

    mutable QueryServer::AliasFileCollection  mParameterAliasDefinitions;
    string_vec                                mParameterAliasDefinitions_filenames;

    mutable string_vec                        mProducerSearchList;
    std::string                               mProducerSearchList_filename;
    time_t                                    mProducerSearchList_modificationTime;

    mutable T::ProducerInfoList               mProducerInfoList;
    mutable time_t                            mProducerInfoList_updateTime;
    mutable ModificationLock                  mProducerInfoList_modificationLock;
    mutable ProducerHash_map                  mProducerHashMap;
    mutable T::GenerationInfoList             mGenerationInfoList;

    mutable T::LevelInfoList                  mLevelInfoList;
    mutable time_t                            mLevelInfoList_lastUpdate;

    mutable QueryCache                        mQueryCache;
    mutable ModificationLock                  mQueryCache_modificationLock;
    mutable time_t                            mQueryCache_updateTime;
    bool                                      mQueryCache_enabled;
    int                                       mQueryCache_maxAge;

    boost::shared_ptr<Spine::Table>           mParameterTable;
};

}  // namespace Grid
}  // namespace Engine
}  // namespace SmartMet
