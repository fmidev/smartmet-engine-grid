#pragma once

#include <fstream>
#include <spine/SmartMetEngine.h>
#include <gis/DEM.h>
#include <gis/LandCover.h>
#include <grid-content/contentServer/cache/CacheImplementation.h>
#include <grid-content/contentServer/redis/RedisImplementation.h>
#include <grid-content/contentServer/memory/MemoryImplementation.h>
#include <grid-content/contentServer/merge/MergeImplementation.h>
#include <grid-content/dataServer/implementation/ServiceImplementation.h>
#include <grid-content/queryServer/implementation/ServiceImplementation.h>
#include <grid-content/queryServer/definition/ParameterMappingFile.h>
#include <grid-content/queryServer/definition/UnitConversion.h>
#include <grid-files/common/ConfigurationFile.h>
#include <grid-files/common/Typedefs.h>
#include <pthread.h>
#include <atomic>
#include <unordered_map>
#include <spine/HTTP.h>
#include <spine/TableFormatter.h>
#include <spine/Table.h>
#include <spine/Value.h>
#include "ParameterDetails.h"
#include "Browser.h"
#include "MetaData.h"


namespace SmartMet
{
namespace Engine
{
namespace Grid
{

typedef std::unique_ptr<Spine::Table> ContentTable;                           //!< Owning pointer to a formatted table returned as HTTP response content.
typedef std::shared_ptr<ContentServer::ServiceInterface> ContentServer_sptr;  //!< Shared pointer to a ContentServer service interface.
typedef std::vector<ContentServer_sptr> ContentServer_sptr_vec;               //!< Ordered list of ContentServer shared pointers (one per configured source).
typedef std::shared_ptr<DataServer::ServiceInterface> DataServer_sptr;        //!< Shared pointer to a DataServer service interface.
typedef std::shared_ptr<QueryServer::ServiceInterface> QueryServer_sptr;      //!< Shared pointer to a QueryServer service interface.
typedef std::shared_ptr<QueryServer::Query> Query_sptr;                       //!< Shared pointer to a QueryServer query object.

typedef std::shared_ptr<QueryServer::ParamMappingFile_vec> ParamMappingFile_vec_sptr;  //!< Shared pointer to the live parameter mapping file vector.

/*! \brief Timestamp and content hash for detecting producer content changes. */
struct HashRec
{
  time_t checkTime;  //!< When the hash was last computed (Unix time).
  UInt64 hash;       //!< Content hash; changes indicate new grid files have been indexed.
};

typedef std::unordered_map<T::ProducerId,HashRec> ProducerHash_map;  //!< Maps each producer ID to its last-known content hash.


/*! \brief Configuration for one external content server source.
 *
 *  The engine supports multiple content sources (e.g., separate Redis instances for
 *  different model families).  Each source is independently enabled/disabled and may
 *  use Redis, CORBA, HTTP, PostgreSQL, or in-memory as its backend. */
struct ContentSource
{
  bool        mEnabled = true;                              //!< Whether this source participates in the merged content set.
  std::string mType = "redis";                              //!< Backend type: "redis", "corba", "http", "postgresql", or "memory".
  std::string mRedisAddress= "127.0.0.1";                  //!< Redis primary host address.
  int         mRedisPort = 6379;                            //!< Redis primary port.
  std::string mRedisTablePrefix = "a.";                    //!< Key prefix distinguishing this source's records in Redis.
  std::string mRedisSecondaryAddress = "127.0.0.1";        //!< Redis secondary (replica) host address.
  int         mRedisSecondaryPort = 0;                     //!< Redis secondary port (0 = disabled).
  bool        mRedisLockEnabled = false;                    //!< Whether to use Redis-level distributed locking.
  bool        mRedisReloadRequired = true;                  //!< Whether a full reload is required on reconnect.
  std::string mCorbaIor;                                    //!< CORBA IOR string when type is "corba".
  std::string mHttpUrl;                                     //!< Base URL when type is "http".
  std::string mPrimaryConnectionString;                     //!< Primary PostgreSQL connection string when type is "postgresql".
  std::string mSecondaryConnectionString;                   //!< Secondary PostgreSQL connection string when type is "postgresql".
  std::string mMemoryContentDir = "/tmp";                   //!< Directory for memory-backed content files when type is "memory".
  uint        mEventListMaxSize = 0;                        //!< Maximum number of events to buffer (0 = unlimited).
};

typedef std::vector<ContentSource> ContentSource_vec;  //!< Ordered list of content source configurations.




// ====================================================================================
/*! \brief SmartMet Server engine that provides plugins with access to grid-based
 *  meteorological data (GRIB1/GRIB2/NetCDF/QueryData).
 *
 *  Embeds and wires together a ContentServer (metadata registry backed by Redis or
 *  another source), a DataServer (memory-mapped grid file reader with an optional
 *  grid cache), and a QueryServer (high-level query executor with Lua scripting,
 *  parameter mappings, and unit conversions).  Plugins obtain service interface
 *  smart pointers via getContentServer_sptr() / getDataServer_sptr() /
 *  getQueryServer_sptr() and execute queries via executeQuery(). */
// ====================================================================================

class Engine : public SmartMet::Spine::SmartMetEngine
{
  public:

                        Engine(const char *theConfigFile);
    virtual             ~Engine();

    bool                browserRequest(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    void                browserContent(SessionManagement::SessionInfo& session,std::ostringstream& output);

    int                 executeQuery(QueryServer::Query& query) const;
    Query_sptr          executeQuery(std::shared_ptr<QueryServer::Query> query) const;

    ContentServer_sptr  getContentServer_sptr() const;
    ContentServer_sptr  getContentSourceServer_sptr(uint idx) const;
    DataServer_sptr     getDataServer_sptr() const;
    QueryServer_sptr    getQueryServer_sptr() const;

    std::string         getConfigurationFileName();
    std::string         getProducerFileName();
    void                getAnalysisTimes(std::vector<std::vector<std::string>>& table) const;
    void                getExtAnalysisTimes(std::vector<std::vector<std::string>>& table) const;

    void                getCacheStats(Fmi::Cache::CacheStatistics& statistics) const;
    void                getStateAttributes(std::shared_ptr<T::AttributeNode> parent);

    Fmi::Cache::CacheStatistics getCacheStats() const;

    ContentSource_vec&  getContentSources() {return mContentSources;}

    T::ParamLevelId     getFmiParameterLevelId(
                          T::ProducerId producerId,
                          int level) const;

    void                getProducerNameList(
                          const std::string& mappingName,
                          std::vector<std::string>& nameList) const;

    std::string         getProducerName(const std::string& aliasName) const;

    std::string         getProducerAlias(
                          const std::string& producerName,
                          int levelId) const;

    UInt64              getProducerHash(T::ProducerId producerId) const;
    UInt64              getProducerHash(std::string producerName) const;

    ContentTable        getProducerInfo(std::optional<std::string> producer,std::string timeFormat) const;
    ContentTable        getGenerationInfo(std::optional<std::string> producer,std::string timeFormat) const;
    ContentTable        getExtGenerationInfo(std::optional<std::string> producer,std::string timeFormat) const;

    ContentTable        getParameterInfo(std::optional<std::string> producer) const;

    std::list<MetaData> getEngineMetadata(const char *producerName) const;

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
    bool                getProducerInfoById(T::ProducerId producerId,T::ProducerInfo& producerInfo) const;
    bool                getGenerationInfoById(T::GenerationId generationId,T::GenerationInfo& generationInfo);

    void                getProducerParameterLevelList(
                          const std::string& producerName,
                          T::ParamLevelId fmiParamLevelId,
                          double multiplier,
                          std::set<double>& levels) const;

    void                getProducerParameterLevelIdList(
                          const std::string& producerName,
                          std::set<T::ParamLevelId>& levelIdList) const;

    void                getProducerLevelIdList(
                          T::ProducerId producerId,
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
    void                setDem(std::shared_ptr<Fmi::DEM> dem);
    void                setLandCover(std::shared_ptr<Fmi::LandCover> landCover);
    void                updateProcessing();

  protected:

    void                init();
    void                shutdown();
    void                startUpdateProcessing();
    void                clearMappings();
    void                checkConfiguration();
    void                loadMappings(QueryServer::ParamMappingFile_vec& parameterMappings);
    void                loadUnitConversionFile();
    std::ofstream       openMappingFile(const std::string& mappingFile);
    void                updateMappings();

    void                updateMappings(
                          T::ParamKeyType sourceParameterKeyType,
                          T::ParamKeyType targetParameterKeyType,
                          const std::string& mappingFile,
                          QueryServer::ParamMappingFile_vec& parameterMappings,
                          Spine::Table& paramTable);

    void                updateProducerAndGenerationList() const;

  private:

    ContentTable        requestGridGenerationInfo(const Spine::HTTP::Request &theRequest);
    ContentTable        requestGridQdGenerationInfo(const Spine::HTTP::Request &theRequest);
    ContentTable        requestGridParameterInfo(const Spine::HTTP::Request &theRequest);
    ContentTable        requestGridProducerInfo(const Spine::HTTP::Request &theRequest);

  private:

    bool                mEnabled;                             //!< Whether the engine is operational (false disables all service calls).

    std::string         mConfigurationFile_name;             //!< Path to the engine configuration file.
    time_t              mConfigurationFile_modificationTime; //!< Last-modified time of the config file at last read.
    time_t              mConfigurationFile_checkTime;        //!< Wall time when the config file modification was last checked.

    ContentServer_sptr_vec  mContentServers;                 //!< Raw content source servers, one per configured ContentSource.
    ContentServer_sptr  mContentServerCache;                 //!< Cache layer that wraps and merges all mContentServers.

    DataServer_sptr     mDataServer;                         //!< Active DataServer (may be a local implementation or a client proxy).
    DataServer_sptr     mDataServerClient;                   //!< Client-side proxy used when the DataServer is remote.
    bool                mDataServerRemote;                   //!< True when the DataServer is accessed via CORBA rather than embedded locally.
    std::string         mDataServerIor;                      //!< CORBA IOR for the remote DataServer (when mDataServerRemote is true).
    time_t              mDataServerCleanupAge;               //!< Age threshold (seconds) after which unused cached grids are evicted.
    time_t              mDataServerCleanupInterval;          //!< Interval (seconds) between DataServer cache cleanup passes.

    QueryServer_sptr    mQueryServer;                        //!< Active QueryServer (local implementation).
    string_vec          mQueryServerLuaFiles;                //!< Paths to Lua script files loaded by the QueryServer.

    std::string         mGridConfigFile;                     //!< Path to the grid-files library config (parameter/geometry identifier mappings).
    bool                mRequestForwardEnabled;              //!< Whether to forward unresolved requests to another service endpoint.
    std::string         mDataServerGridDirectory;            //!< Base directory where grid files are stored on disk.

    ContentSource_vec   mContentSources;                     //!< Configurations for all external content sources.
    bool                mContentCacheEnabled;                //!< Whether the in-memory content cache layer is active.

    bool                mContentServerProcessingLogEnabled;       //!< Enable per-call timing log for the ContentServer.
    std::string         mContentServerProcessingLogFile;          //!< Path to the ContentServer processing log file.
    int                 mContentServerProcessingLogMaxSize;       //!< Maximum size (bytes) before the processing log is truncated.
    int                 mContentServerProcessingLogTruncateSize;  //!< Size (bytes) the processing log is truncated to when it overflows.
    Log                 mContentServerProcessingLog;              //!< ContentServer per-call timing log instance.
    bool                mContentServerDebugLogEnabled;            //!< Enable verbose debug log for the ContentServer.
    std::string         mContentServerDebugLogFile;               //!< Path to the ContentServer debug log file.
    int                 mContentServerDebugLogMaxSize;            //!< Maximum size (bytes) before the debug log is truncated.
    int                 mContentServerDebugLogTruncateSize;       //!< Size (bytes) the debug log is truncated to when it overflows.
    Log                 mContentServerDebugLog;                   //!< ContentServer verbose debug log instance.
    time_t              mContentServerStartTime;                  //!< Unix time when the ContentServer was started.

    bool                mDataServerProcessingLogEnabled;          //!< Enable per-call timing log for the DataServer.
    std::string         mDataServerProcessingLogFile;             //!< Path to the DataServer processing log file.
    int                 mDataServerProcessingLogMaxSize;          //!< Maximum size (bytes) before the processing log is truncated.
    int                 mDataServerProcessingLogTruncateSize;     //!< Size (bytes) the processing log is truncated to when it overflows.
    Log                 mDataServerProcessingLog;                 //!< DataServer per-call timing log instance.
    bool                mDataServerDebugLogEnabled;               //!< Enable verbose debug log for the DataServer.
    std::string         mDataServerDebugLogFile;                  //!< Path to the DataServer debug log file.
    int                 mDataServerDebugLogMaxSize;               //!< Maximum size (bytes) before the debug log is truncated.
    int                 mDataServerDebugLogTruncateSize;          //!< Size (bytes) the debug log is truncated to when it overflows.
    Log                 mDataServerDebugLog;                      //!< DataServer verbose debug log instance.

    bool                mQueryServerRemote;                       //!< True when the QueryServer is accessed via CORBA rather than embedded locally.
    std::string         mQueryServerIor;                          //!< CORBA IOR for the remote QueryServer (when mQueryServerRemote is true).
    bool                mQueryServerProcessingLogEnabled;         //!< Enable per-call timing log for the QueryServer.
    std::string         mQueryServerProcessingLogFile;            //!< Path to the QueryServer processing log file.
    int                 mQueryServerProcessingLogMaxSize;         //!< Maximum size (bytes) before the processing log is truncated.
    int                 mQueryServerProcessingLogTruncateSize;    //!< Size (bytes) the processing log is truncated to when it overflows.
    Log                 mQueryServerProcessingLog;                //!< QueryServer per-call timing log instance.
    bool                mQueryServerDebugLogEnabled;              //!< Enable verbose debug log for the QueryServer.
    std::string         mQueryServerDebugLogFile;                 //!< Path to the QueryServer debug log file.
    int                 mQueryServerDebugLogMaxSize;              //!< Maximum size (bytes) before the debug log is truncated.
    int                 mQueryServerDebugLogTruncateSize;         //!< Size (bytes) the debug log is truncated to when it overflows.
    Log                 mQueryServerDebugLog;                     //!< QueryServer verbose debug log instance.
    bool                mQueryServerCheckGeometryStatus;          //!< Whether to verify geometry validity before executing queries.
    std::size_t         mQueryServerContentCache_maxRecordsPerThread;         //!< Per-thread capacity of the query content cache.
    uint                mQueryServerContentCache_clearInterval;               //!< Interval (seconds) between content cache clear passes.
    std::size_t         mQueryServerContentSearchCache_maxRecordsPerThread;   //!< Per-thread capacity of the content-search result cache.
    uint                mQueryServerContentSearchCache_clearInterval;         //!< Interval (seconds) between content-search cache clear passes.

    std::string         mHeightConversionFile;                    //!< Path to the height/pressure conversion data file.
    std::string         mCacheType;                               //!< DataServer grid cache backend type ("memory" or "filesystem").
    std::string         mCacheDir;                                //!< Directory for filesystem-backed grid cache.
    uint                mNumOfCachedGrids;                        //!< Maximum number of decoded grids kept in the cache.
    uint                mMaxSizeOfCachedGridsInMegaBytes;         //!< Maximum total size (MiB) of the decoded grid cache.

    bool                mContentSwapEnabled;                      //!< Use atomic swap for lock-free content reads (avoids reader stalls during updates).
    uint                mFileCacheMaxWaitTime;                    //!< Maximum time (ms) to wait for a cached file before giving up.
    uint                mFileCacheMaxFirstWaitTime;               //!< Maximum time (ms) to wait for the first cache entry.
    uint                mContentUpdateInterval;                   //!< Interval (seconds) between content cache refresh cycles.

    bool                mDataServerMethodsEnabled;                //!< Expose raw DataServer methods directly through the QueryServer interface.
    bool                mMemoryMapper_premapEnabled;              //!< Pre-map all registered grid files into memory at startup.
    bool                mMemoryMapper_enabled;                    //!< Enable the userfaultfd-based lazy memory mapper.
    std::string         mMemoryMapper_accessFile;                 //!< Path to the file used to record memory-mapper page access events.
    uint                mMemoryMapper_maxProcessingThreads;       //!< Maximum number of threads used by the memory mapper.
    uint                mMemoryMapper_maxMessages;                //!< Maximum number of pending messages in the memory-mapper queue.
    std::size_t         mMemoryMapper_fileHandleLimit;            //!< Maximum number of simultaneously open file handles in the memory mapper.
    std::size_t         mMemoryMapper_pageCacheSize;              //!< Size (pages) of the memory mapper's prefetch page cache.

    pthread_t           mThread;                                  //!< Background thread that drives periodic content/mapping updates.
    std::atomic<bool>   mShutdownRequested;                       //!< Set to true to signal the background thread to stop.
    bool                mShutdownFinished;                        //!< Set to true after the background thread has exited.
    Browser             mBrowser;                                 //!< Admin browser instance.
    bool                mBrowserEnabled;                          //!< Whether the admin browser is active.
    UInt64              mBrowserFlags;                            //!< Feature flags passed to the admin browser (OR of Browser::Flags::*).


    DataServer::ServiceImplementation*        mDataServerImplementation;          //!< Raw pointer to the local DataServer implementation for direct configuration during init.
    ContentServer::CacheImplementation*       mContentServerCacheImplementation;  //!< Raw pointer to the ContentServer cache layer for direct configuration during init.
    ContentServer::MergeImplementation*       mContentServerMergeImplementation;  //!< Raw pointer to the ContentServer merge layer for direct configuration during init.

    std::shared_ptr<Fmi::DEM>                 mDem;                               //!< Optional digital elevation model (injected by the GIS engine).
    std::shared_ptr<Fmi::LandCover>           mLandCover;                         //!< Optional land cover dataset (injected by the GIS engine).

    std::vector<DataServer::ServiceInterface*> mDataServer_clients;               //!< List of DataServer client connections managed by this engine.

    mutable QueryServer::AliasFileCollection  mProducerMappingDefinitions;              //!< Hot-reloadable producer alias → name mappings.
    string_vec                                mProducerMappingDefinitions_filenames;     //!< File paths for producer mapping alias definitions.

    std::set<std::int16_t>                    mParameterMapping_simplifiedLevelIdSet;   //!< Level IDs that use simplified (non-hierarchical) parameter mappings.
    mutable ParamMappingFile_vec_sptr         mParameterMappingDefinitions;             //!< Shared, hot-swappable parameter mapping file vector.
    mutable ModificationLock                  mParameterMappingDefinitions_modificationLock; //!< Lock protecting concurrent access to mParameterMappingDefinitions.
    mutable time_t                            mParameterMappingDefinitions_updateTime;  //!< Wall time of the last parameter mapping reload.
    string_vec                                mParameterMappingDefinitions_filenames;   //!< File paths for parameter mapping definition files.

    std::string                               mUnitConversionFile;                      //!< Path to the unit conversion definition file.
    QueryServer::UnitConversion_vec           mUnitConversions;                         //!< Loaded unit conversion rules.
    mutable QueryServer::ParamMappingFile_vec mParameterAliasMappings;                  //!< Parameter alias → canonical name mappings for the QueryServer.
    string_vec                                mParameterMappingAliasDefinitions_filenames; //!< File paths for parameter alias mapping files.

    T::ParamKeyType                           mParameterMappingDefinitions_autoFileKeyType;  //!< Key type used when auto-generating parameter mapping files.
    std::string                               mParameterMappingDefinitions_autoFile_fmi;     //!< Path for the auto-generated FMI parameter mapping file.
    std::string                               mParameterMappingDefinitions_autoFile_newbase; //!< Path for the auto-generated Newbase parameter mapping file.
    std::string                               mParameterMappingDefinitions_autoFile_netCdf;  //!< Path for the auto-generated NetCDF parameter mapping file.

    mutable QueryServer::AliasFileCollection  mParameterAliasDefinitions;               //!< Hot-reloadable parameter alias definitions.
    string_vec                                mParameterAliasDefinitions_filenames;      //!< File paths for parameter alias definition files.

    mutable string_vec                        mProducerSearchList;                       //!< Ordered list of producer names to search when resolving unqualified requests.
    std::string                               mProducerSearchList_filename;              //!< Path to the producer search list file.

    mutable T::ProducerInfoList               mProducerInfoList;                         //!< Cached list of known producers.
    mutable time_t                            mProducerInfoList_updateTime;              //!< Wall time when mProducerInfoList was last refreshed.
    mutable ModificationLock                  mProducerInfoList_modificationLock;        //!< Lock protecting concurrent access to mProducerInfoList.
    mutable ProducerHash_map                  mProducerHashMap;                          //!< Per-producer content hashes for detecting new file arrivals.
    mutable T::GenerationInfoList             mGenerationInfoList;                       //!< Cached list of known generations.
    mutable T::GeometryInfoList               mGeometryInfoList;                         //!< Cached list of known geometries.

    std::string                               mProducerStatusFile;                       //!< Path to the file recording per-producer status (enabled/disabled).

    mutable T::LevelInfoList                  mLevelInfoList;                            //!< Cached list of level information records.
    mutable time_t                            mLevelInfoList_lastUpdate;                 //!< Wall time when mLevelInfoList was last refreshed.

    bool                                      mFileCache_enabled;                        //!< Whether the on-disk file cache is active.
    std::string                               mFileCache_directory;                      //!< Directory used for the on-disk file cache.

    std::shared_ptr<Spine::Table>             mParameterTable;                           //!< Shared parameter info table built from mapping files for HTTP responses.
};

}  // namespace Grid
}  // namespace Engine
}  // namespace SmartMet
