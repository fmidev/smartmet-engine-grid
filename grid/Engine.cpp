#include "Engine.h"

#include <grid-content/contentServer/corba/client/ClientImplementation.h>
#include <grid-content/contentServer/http/client/ClientImplementation.h>
#include "grid-content/contentServer/postgresql/PostgresqlImplementation.h"
#include <grid-content/dataServer/corba/client/ClientImplementation.h>
#include <grid-content/queryServer/corba/client/ClientImplementation.h>
#include <grid-files/common/CoordinateConversions.h>
#include <grid-files/common/GeneralFunctions.h>
#include <grid-files/common/GraphFunctions.h>
#include <grid-files/common/ImageFunctions.h>
#include <grid-files/common/ImagePaint.h>
#include <grid-files/common/BitLine.h>
#include <grid-files/common/MemoryMapper.h>
#include <grid-files/common/ShowFunction.h>
#include <grid-files/grid/ValueCache.h>
#include <grid-files/identification/GridDef.h>
#include <grid-files/map/Topology.h>
#include <macgyver/AnsiEscapeCodes.h>
#include <macgyver/Exception.h>
#include <macgyver/StringConversion.h>
#include <macgyver/TimeFormatter.h>
#include <spine/Convenience.h>
#include <spine/Reactor.h>
#include <unistd.h>

#include <unordered_set>

#include "Browser.h"

#define FUNCTION_TRACE FUNCTION_TRACE_OFF

#define CONTENT_SERVER_SESSION_ID 111111111
#define DATA_SERVER_SESSION_ID 222222222



namespace SmartMet
{
namespace Engine
{
namespace Grid
{
static void* gridEngine_updateThread(void* arg)
{
  try
  {
    Engine* engine = static_cast<Engine*>(arg);
    engine->updateProcessing();
    return nullptr;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.printError();
    exit(-1);
  }
}

Engine::Engine(const char* theConfigFile)
{
  FUNCTION_TRACE
  try
  {
    const char* configAttribute[] =
    {
        "smartmet.library.grid-files.configFile",
        "smartmet.library.grid-files.cache.numOfGrids",
        "smartmet.library.grid-files.cache.maxSizeInMegaBytes",

        "smartmet.engine.grid.enabled",

        "smartmet.engine.grid.content-server.content-source.type",
        "smartmet.engine.grid.content-server.content-source.redis.address",
        "smartmet.engine.grid.content-server.content-source.redis.port",
        "smartmet.engine.grid.content-server.content-source.redis.tablePrefix",
        "smartmet.engine.grid.content-server.content-source.http.url",
        "smartmet.engine.grid.content-server.content-source.corba.ior",
        "smartmet.engine.grid.content-server.cache.enabled",
        "smartmet.engine.grid.content-server.cache.requestForwardEnabled",

        "smartmet.engine.grid.content-server.processing-log.enabled",
        "smartmet.engine.grid.content-server.processing-log.file",
        "smartmet.engine.grid.content-server.processing-log.maxSize",
        "smartmet.engine.grid.content-server.processing-log.truncateSize",
        "smartmet.engine.grid.content-server.debug-log.enabled",
        "smartmet.engine.grid.content-server.debug-log.file",
        "smartmet.engine.grid.content-server.debug-log.maxSize",
        "smartmet.engine.grid.content-server.debug-log.truncateSize",

        "smartmet.engine.grid.data-server.remote",
        "smartmet.engine.grid.data-server.ior",
        "smartmet.engine.grid.data-server.grid-storage.directory",
        "smartmet.engine.grid.data-server.processing-log.enabled",
        "smartmet.engine.grid.data-server.processing-log.file",
        "smartmet.engine.grid.data-server.processing-log.maxSize",
        "smartmet.engine.grid.data-server.processing-log.truncateSize",
        "smartmet.engine.grid.data-server.debug-log.enabled",
        "smartmet.engine.grid.data-server.debug-log.file",
        "smartmet.engine.grid.data-server.debug-log.maxSize",
        "smartmet.engine.grid.data-server.debug-log.truncateSize",

        "smartmet.engine.grid.query-server.remote",
        "smartmet.engine.grid.query-server.ior",
        "smartmet.engine.grid.query-server.queryCache.enabled",
        "smartmet.engine.grid.query-server.queryCache.maxAge",
        "smartmet.engine.grid.query-server.producerFile",
        "smartmet.engine.grid.query-server.producerMappingFiles",
        "smartmet.engine.grid.query-server.luaFiles",
        "smartmet.engine.grid.query-server.mappingTargetKeyType",
        "smartmet.engine.grid.query-server.mappingFiles",
        "smartmet.engine.grid.query-server.mappingUpdateFile.fmi",
        "smartmet.engine.grid.query-server.mappingUpdateFile.newbase",
        "smartmet.engine.grid.query-server.aliasFiles",
        "smartmet.engine.grid.query-server.processing-log.enabled",
        "smartmet.engine.grid.query-server.processing-log.file",
        "smartmet.engine.grid.query-server.processing-log.maxSize",
        "smartmet.engine.grid.query-server.processing-log.truncateSize",
        "smartmet.engine.grid.query-server.debug-log.enabled",
        "smartmet.engine.grid.query-server.debug-log.file",
        "smartmet.engine.grid.query-server.debug-log.maxSize",
        "smartmet.engine.grid.query-server.debug-log.truncateSize",
        nullptr
    };

    mEnabled = true;
    mMemoryMapper_enabled = false;
    mMemoryMapper_premapEnabled = true;
    mMemoryMapper_maxProcessingThreads = 30;
    mMemoryMapper_maxMessages = 100000;
    mMemoryMapper_pageCacheSize = 2000000;
    mConfigurationFile_name = theConfigFile;
    mConfigurationFile_checkTime = time(nullptr) + 120;
    mConfigurationFile_modificationTime = getFileModificationTime(mConfigurationFile_name.c_str());
    mLevelInfoList_lastUpdate = 0;
    mProducerInfoList_updateTime = 0;
    mContentSourceRedisAddress = "127.0.0.1";
    mContentSourceRedisPort = 6379;
    mContentSourceRedisSecondaryAddress = "127.0.0.1";
    mContentSourceRedisSecondaryPort = 0;
    mContentSourceRedisLockEnabled = false;
    mContentSourceRedisTablePrefix = "";
    mContentSourceRedisReloadRequired = false;
    mContentSourceHttpUrl = "";
    mContentSourceCorbaIor = "";
    mContentCacheEnabled = true;
    mRequestForwardEnabled = false;
    mMemoryContentDir = "/tmp";
    mEventListMaxSize = 0;
    mQueryCache_updateTime = time(nullptr);
    mContentServerStartTime = 0;
    mShutdownRequested = false;
    mShutdownFinished = false;

    mFileCache_enabled = false;
    mFileCache_directory = "/tmp";

    mContentServerProcessingLogEnabled = false;
    mContentServerDebugLogEnabled = false;
    mDataServerProcessingLogEnabled = false;
    mDataServerDebugLogEnabled = false;
    mQueryServerProcessingLogEnabled = false;
    mQueryServerDebugLogEnabled = false;
    mParameterMappingDefinitions_updateTime = 0;
    mParameterMappingDefinitions_autoFileKeyType = T::ParamKeyTypeValue::FMI_NAME;

    mDataServerRemote = false;
    mDataServerCleanupAge = 24*3600;
    mDataServerCleanupInterval = 600;
    mContentServerProcessingLogMaxSize = 10000000;
    mContentServerProcessingLogTruncateSize = 5000000;
    mContentServerDebugLogMaxSize = 10000000;
    mContentServerDebugLogTruncateSize = 5000000;
    mDataServerProcessingLogMaxSize = 10000000;
    mDataServerProcessingLogTruncateSize = 5000000;
    mDataServerDebugLogMaxSize = 10000000;
    mDataServerDebugLogTruncateSize = 5000000;
    mDataServerMethodsEnabled = false;
    mQueryServerRemote = false;
    mQueryServerProcessingLogMaxSize = 10000000;
    mQueryServerProcessingLogTruncateSize = 50000000;
    mQueryServerDebugLogMaxSize = 10000000;
    mQueryServerDebugLogTruncateSize = 5000000;
    mQueryCache_enabled = false;
    mQueryCache_maxAge = 300;
    mQueryCache_stats.starttime = Fmi::SecondClock::universal_time();
    mQueryServerContentCache_maxRecordsPerThread = 500000;
    mQueryServerContentCache_clearInterval = 3600 * 24 * 3;
    mQueryServerContentSearchCache_maxRecordsPerThread = 500000;
    mQueryServerContentSearchCache_clearInterval = 3600 * 24 * 3;
    mQueryServerCheckGeometryStatus = false;
    mContentSwapEnabled = false;
    mFileCacheMaxWaitTime = 0;
    mFileCacheMaxFirstWaitTime = 0;
    mContentUpdateInterval = 180;
    mBrowserEnabled = true;
    mBrowserFlags = 0;

    mCacheType = "memory";
    mCacheDir = "/tmp";
    mNumOfCachedGrids = 10000;
    mMaxSizeOfCachedGridsInMegaBytes = 10000;

    mContentServerCacheImplementation = nullptr;
    mDataServerImplementation = nullptr;
    mThread = 0;

    mParameterTable.reset(new Spine::Table);

    ConfigurationFile configurationFile;
    configurationFile.readFile(mConfigurationFile_name.c_str());

    configurationFile.getAttributeValue("smartmet.engine.grid.enabled", mEnabled);
    if (!mEnabled)
      return;

    uint t = 0;
    while (configAttribute[t] != nullptr)
    {
      if (!configurationFile.findAttribute(configAttribute[t]))
      {
        Fmi::Exception exception(BCP, "Missing configuration attribute!");
        exception.addParameter("File", theConfigFile);
        exception.addParameter("Attribute", configAttribute[t]);
        throw exception;
      }
      t++;
    }

    configurationFile.getAttributeValue("smartmet.library.grid-files.configFile", mGridConfigFile);
    configurationFile.getAttributeValue("smartmet.library.grid-files.memoryMapper.enabled", mMemoryMapper_enabled);
    configurationFile.getAttributeValue("smartmet.library.grid-files.memoryMapper.accessFile", mMemoryMapper_accessFile);
    configurationFile.getAttributeValue("smartmet.library.grid-files.memoryMapper.premapEnabled", mMemoryMapper_premapEnabled);
    configurationFile.getAttributeValue("smartmet.library.grid-files.memoryMapper.maxProsessingThreads", mMemoryMapper_maxProcessingThreads);
    configurationFile.getAttributeValue("smartmet.library.grid-files.memoryMapper.maxMessages", mMemoryMapper_maxMessages);
    configurationFile.getAttributeValue("smartmet.library.grid-files.memoryMapper.pageCacheSize", mMemoryMapper_pageCacheSize);
    configurationFile.getAttributeValue("smartmet.library.grid-files.memoryMapper.fileHandleLimit", mMemoryMapper_fileHandleLimit);

    configurationFile.getAttributeValue("smartmet.library.grid-files.cache.type", mCacheType);
    configurationFile.getAttributeValue("smartmet.library.grid-files.cache.directory", mCacheDir);
    configurationFile.getAttributeValue("smartmet.library.grid-files.cache.numOfGrids", mNumOfCachedGrids);
    configurationFile.getAttributeValue("smartmet.library.grid-files.cache.maxSizeInMegaBytes", mMaxSizeOfCachedGridsInMegaBytes);

    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.content-source.type", mContentSourceType);

    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.content-source.redis.address", mContentSourceRedisAddress);
    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.content-source.redis.port", mContentSourceRedisPort);
    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.content-source.redis.tablePrefix", mContentSourceRedisTablePrefix);
    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.content-source.redis.secondaryAddress", mContentSourceRedisSecondaryAddress);
    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.content-source.redis.secondaryPort", mContentSourceRedisSecondaryPort);
    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.content-source.redis.lockEnabled", mContentSourceRedisLockEnabled);
    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.content-source.redis.reloadRequired", mContentSourceRedisReloadRequired);

    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.content-source.postgresql.primaryConnectionString", mPrimaryConnectionString);
    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.content-source.postgresql.secondaryConnectionString", mSecondaryConnectionString);

    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.content-source.http.url", mContentSourceHttpUrl);

    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.content-source.corba.ior", mContentSourceCorbaIor);

    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.content-source.file.contentDir", mMemoryContentDir);
    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.content-source.file.eventListMaxSize", mEventListMaxSize);

    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.cache.enabled", mContentCacheEnabled);
    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.cache.requestForwardEnabled", mRequestForwardEnabled);

    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.cache.contentSwapEnabled", mContentSwapEnabled);
    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.cache.fileCache.maxFirstWaitTime", mFileCacheMaxFirstWaitTime);
    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.cache.fileCache.maxWaitTime", mFileCacheMaxWaitTime);
    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.cache.contentUpdateInterval", mContentUpdateInterval);

    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.processing-log.enabled", mContentServerProcessingLogEnabled);
    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.processing-log.file", mContentServerProcessingLogFile);
    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.processing-log.maxSize", mContentServerProcessingLogMaxSize);
    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.processing-log.truncateSize", mContentServerProcessingLogTruncateSize);
    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.debug-log.enabled", mContentServerDebugLogEnabled);
    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.debug-log.file", mContentServerDebugLogFile);
    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.debug-log.maxSize", mContentServerDebugLogMaxSize);
    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.debug-log.truncateSize", mContentServerDebugLogTruncateSize);

    configurationFile.getAttributeValue("smartmet.engine.grid.data-server.remote", mDataServerRemote);
    configurationFile.getAttributeValue("smartmet.engine.grid.data-server.ior", mDataServerIor);

    // These settings are used when the data server is embedded into the grid engine.
    configurationFile.getAttributeValue("smartmet.engine.grid.data-server.grid-storage.directory", mDataServerGridDirectory);
    configurationFile.getAttributeValue("smartmet.engine.grid.data-server.grid-storage.clean-up.age", mDataServerCleanupAge);
    configurationFile.getAttributeValue("smartmet.engine.grid.data-server.grid-storage.clean-up.checkInterval", mDataServerCleanupInterval);

    configurationFile.getAttributeValue("smartmet.engine.grid.data-server.fileCache.enabled", mFileCache_enabled);
    configurationFile.getAttributeValue("smartmet.engine.grid.data-server.fileCache.directory", mFileCache_directory);

    configurationFile.getAttributeValue("smartmet.engine.grid.data-server.processing-log.enabled", mDataServerProcessingLogEnabled);
    configurationFile.getAttributeValue("smartmet.engine.grid.data-server.processing-log.file", mDataServerProcessingLogFile);
    configurationFile.getAttributeValue("smartmet.engine.grid.data-server.processing-log.maxSize", mDataServerProcessingLogMaxSize);
    configurationFile.getAttributeValue("smartmet.engine.grid.data-server.processing-log.truncateSize", mDataServerProcessingLogTruncateSize);
    configurationFile.getAttributeValue("smartmet.engine.grid.data-server.debug-log.enabled", mDataServerDebugLogEnabled);
    configurationFile.getAttributeValue("smartmet.engine.grid.data-server.debug-log.file", mDataServerDebugLogFile);
    configurationFile.getAttributeValue("smartmet.engine.grid.data-server.debug-log.maxSize", mDataServerDebugLogMaxSize);
    configurationFile.getAttributeValue("smartmet.engine.grid.data-server.debug-log.truncateSize", mDataServerDebugLogTruncateSize);

    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.remote", mQueryServerRemote);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.ior", mQueryServerIor);

    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.queryCache.enabled", mQueryCache_enabled);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.queryCache.maxAge", mQueryCache_maxAge);

    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.contentCache.maxRecordsPerThread", mQueryServerContentCache_maxRecordsPerThread);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.contentCache.clearInterval", mQueryServerContentCache_clearInterval);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.contentSearchCache.maxRecordsPerThread", mQueryServerContentSearchCache_maxRecordsPerThread);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.contentSearchCache.clearInterval", mQueryServerContentSearchCache_clearInterval);

    // These settings are used when the query server is embedded into the grid engine.
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.producerFile", mProducerSearchList_filename);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.heightConversionFile", mHeightConversionFile);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.producerMappingFiles", mProducerMappingDefinitions_filenames);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.producerStatusFile", mProducerStatusFile);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.checkGeometryStatus",mQueryServerCheckGeometryStatus);

    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.processing-log.enabled", mQueryServerProcessingLogEnabled);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.processing-log.file", mQueryServerProcessingLogFile);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.processing-log.maxSize", mQueryServerProcessingLogMaxSize);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.processing-log.truncateSize", mQueryServerProcessingLogTruncateSize);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.debug-log.enabled", mQueryServerDebugLogEnabled);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.debug-log.file", mQueryServerDebugLogFile);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.debug-log.maxSize", mQueryServerDebugLogMaxSize);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.debug-log.truncateSize", mQueryServerDebugLogTruncateSize);

    int tmp = 0;
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.mappingTargetKeyType", tmp);
    mParameterMappingDefinitions_autoFileKeyType = C_UCHAR(tmp);

    std::vector<std::string> vec;
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.mappingLevelSimplification", vec);
    for (auto it = vec.begin(); it != vec.end(); it++)
      mParameterMapping_simplifiedLevelIdSet.insert(atoi(it->c_str()));

    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.mappingUpdateFile.fmi", mParameterMappingDefinitions_autoFile_fmi);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.mappingUpdateFile.newbase", mParameterMappingDefinitions_autoFile_newbase);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.mappingUpdateFile.netCdf", mParameterMappingDefinitions_autoFile_netCdf);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.mappingFiles", mParameterMappingDefinitions_filenames);

    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.unitConversionFile", mUnitConversionFile);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.mappingAliasFiles",mParameterMappingAliasDefinitions_filenames);

    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.aliasFiles", mParameterAliasDefinitions_filenames);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.luaFiles", mQueryServerLuaFiles);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.dataServerMethodsEnabled",mDataServerMethodsEnabled);

    configurationFile.getAttributeValue("smartmet.engine.grid.browser.enabled", mBrowserEnabled);
    configurationFile.getAttributeValue("smartmet.engine.grid.browser.flags", mBrowserFlags);


    mProducerSearchList_modificationTime = getFileModificationTime(mProducerSearchList_filename.c_str());

    // Initializing information that is needed for identifying the content of the grid files.

    SmartMet::Identification::gridDef.init(mGridConfigFile.c_str());
    Map::topology.init(mGridConfigFile.c_str(),true,true,true);

    if (!mEnabled)
      std::cout << ANSI_FG_RED << "**** Grid-engine configuration: Engine usage disabled!" << ANSI_FG_DEFAULT << std::endl;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Constructor failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}



Engine::~Engine()
{
  FUNCTION_TRACE
  try
  {
    for (auto it = mDataServer_clients.begin(); it != mDataServer_clients.end(); ++it)
      delete *it;

    if (mEnabled && !mShutdownFinished) {
      std::cout << __PRETTY_FUNCTION__ << ": an attempt to destroy Grid engine before"
                    << " is shutdown is complete" << std::endl;
        abort();
    }
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.printError();
  }
}



void Engine::init()
{
  FUNCTION_TRACE
  try
  {
    if (!mEnabled)
    {
      mContentServer.reset(new ContentServer::ServiceInterface());
      mContentServer->setEnabled(false);

      mDataServer.reset(new DataServer::ServiceInterface());
      mDataServer->setEnabled(false);

      mQueryServer.reset(new QueryServer::ServiceInterface());
      mQueryServer->setEnabled(false);

      return;
    }

    if (!mMemoryMapper_accessFile.empty())
      memoryMapper.setAccessFile(mMemoryMapper_accessFile.c_str());


    memoryMapper.setMaxProcessingThreads(mMemoryMapper_maxProcessingThreads);
    memoryMapper.setMaxMessages(mMemoryMapper_maxMessages);
    memoryMapper.setPageCacheSize(mMemoryMapper_pageCacheSize);
    memoryMapper.setFileHandleLimit(mMemoryMapper_fileHandleLimit);
    memoryMapper.setPremapEnabled(mMemoryMapper_premapEnabled);
    memoryMapper.setEnabled(mMemoryMapper_enabled);

    ContentServer::ServiceInterface* cServer = nullptr;
    DataServer::ServiceInterface* dServer = nullptr;
    QueryServer::ServiceInterface* qServer = nullptr;

    clearMappings();

    if (mContentSourceType == "redis")
    {
      ContentServer::RedisImplementation* redis = new ContentServer::RedisImplementation();
      mContentServer.reset(redis);
      redis->init(mContentSourceRedisAddress.c_str(), mContentSourceRedisPort, mContentSourceRedisTablePrefix.c_str(), mContentSourceRedisSecondaryAddress.c_str(),
          mContentSourceRedisSecondaryPort, mContentSourceRedisLockEnabled, mContentSourceRedisReloadRequired);
      cServer = redis;
    }
    else if (mContentSourceType == "postgresql")
    {
      ContentServer::PostgresqlImplementation* postgres = new ContentServer::PostgresqlImplementation();
      mContentServer.reset(postgres);
      postgres->init(mPrimaryConnectionString.c_str(),mSecondaryConnectionString.c_str(),false);
      cServer = postgres;
    }
    else if (mContentSourceType == "corba")
    {
      ContentServer::Corba::ClientImplementation* client = new ContentServer::Corba::ClientImplementation();
      mContentServer.reset(client);
      client->init(mContentSourceCorbaIor.c_str());
      cServer = client;
    }
    else if (mContentSourceType == "http")
    {
      ContentServer::HTTP::ClientImplementation* client = new ContentServer::HTTP::ClientImplementation();
      mContentServer.reset(client);
      client->init(mContentSourceHttpUrl.c_str());
      cServer = client;
    }
    else if (mContentSourceType == "file")
    {
      bool eventstEnabled = true;
      if (mEventListMaxSize == 0)
      {
        eventstEnabled = false;
        mContentCacheEnabled = false;
      }

      ContentServer::MemoryImplementation* memoryImplementation = new ContentServer::MemoryImplementation();
      mContentServer.reset(memoryImplementation);
      memoryImplementation->init(true, false, true, eventstEnabled, mMemoryContentDir, 0);
      memoryImplementation->setEventListMaxLength(mEventListMaxSize);
      cServer = memoryImplementation;
    }
    else
    {
      Fmi::Exception exception(BCP, "Unknown content source type!");
      exception.addParameter("Content source type", mContentSourceType);
      throw exception;
    }

    if (mContentCacheEnabled)
    {
      mContentServerCacheImplementation = new ContentServer::CacheImplementation();
      mContentServerCacheImplementation->setRequestForwardEnabled(mRequestForwardEnabled);
      mContentServerCacheImplementation->setContentSwap(mContentSwapEnabled,mFileCacheMaxFirstWaitTime,mFileCacheMaxWaitTime);
      mContentServerCacheImplementation->setContentUpdateInterval(mContentUpdateInterval);
      mContentServerCacheImplementation->init(CONTENT_SERVER_SESSION_ID,DATA_SERVER_SESSION_ID,cServer);

      mContentServerCache.reset(mContentServerCacheImplementation);
      mContentServerCacheImplementation->startEventProcessing();
      cServer = mContentServerCacheImplementation;
    }

    if (mDataServerRemote && mDataServerIor.length() > 50)
    {
      DataServer::Corba::ClientImplementation* client = new DataServer::Corba::ClientImplementation();
      client->init(mDataServerIor);

      mDataServer.reset(client);
      dServer = client;
    }
    else
    {
      mDataServerImplementation = new DataServer::ServiceImplementation();
      mDataServerImplementation->init(DATA_SERVER_SESSION_ID, 0, "NotRegistered", "NotRegistered", mDataServerGridDirectory, cServer);
      mDataServerImplementation->setCleanup(mDataServerCleanupAge,mDataServerCleanupInterval);
      mDataServerImplementation->setFileCache(mFileCache_enabled,mFileCache_directory.c_str());


      mDataServer.reset(mDataServerImplementation);
      mDataServerImplementation->startEventProcessing();
      mDataServerImplementation->startCacheProcessing();

      dServer = mDataServerImplementation;

      if (strcasecmp(mCacheType.c_str(),"filesys") == 0)
      {
        SmartMet::GRID::valueCache.setCacheDir(mCacheDir.c_str());
        SmartMet::GRID::valueCache.init(mNumOfCachedGrids, mMaxSizeOfCachedGridsInMegaBytes,true);
      }
      else
      {
        SmartMet::GRID::valueCache.init(mNumOfCachedGrids, mMaxSizeOfCachedGridsInMegaBytes);
      }
    }

    if (mQueryServerRemote && mQueryServerIor.length() > 50)
    {
      QueryServer::Corba::ClientImplementation* client = new QueryServer::Corba::ClientImplementation();
      client->init(mQueryServerIor);
      mQueryServer.reset(client);
      qServer = client;
    }
    else
    {
      QueryServer::ServiceImplementation* server = new QueryServer::ServiceImplementation();
      server->init(cServer, dServer, mGridConfigFile, mHeightConversionFile,
          mParameterMappingDefinitions_filenames,mUnitConversionFile,mParameterMappingAliasDefinitions_filenames,
          mParameterAliasDefinitions_filenames, mProducerSearchList_filename,
          mProducerMappingDefinitions_filenames, mQueryServerLuaFiles,mQueryServerCheckGeometryStatus,mDataServerMethodsEnabled);

      server->initContentCache(mQueryServerContentCache_maxRecordsPerThread,mQueryServerContentCache_clearInterval);
      server->initContentSearchCache(mQueryServerContentSearchCache_maxRecordsPerThread,mQueryServerContentSearchCache_clearInterval);

      qServer = server;

      mQueryServer.reset(server);
    }

    if (mContentServerProcessingLogFile.length() > 0)
    {
      mContentServerProcessingLog.init(mContentServerProcessingLogEnabled, mContentServerProcessingLogFile.c_str(), mContentServerProcessingLogMaxSize, mContentServerProcessingLogTruncateSize);
      cServer->setProcessingLog(&mContentServerProcessingLog);
    }

    if (mContentServerDebugLogFile.length() > 0)
    {
      mContentServerDebugLog.init(mContentServerDebugLogEnabled, mContentServerDebugLogFile.c_str(), mContentServerDebugLogMaxSize, mContentServerDebugLogTruncateSize);
      cServer->setDebugLog(&mContentServerDebugLog);
    }

    if (mDataServerProcessingLogFile.length() > 0)
    {
      mDataServerProcessingLog.init(mDataServerProcessingLogEnabled, mDataServerProcessingLogFile.c_str(), mDataServerProcessingLogMaxSize, mDataServerProcessingLogTruncateSize);
      dServer->setProcessingLog(&mDataServerProcessingLog);
    }

    if (mDataServerDebugLogFile.length() > 0)
    {
      mDataServerDebugLog.init(mDataServerDebugLogEnabled, mDataServerDebugLogFile.c_str(), mDataServerDebugLogMaxSize, mDataServerDebugLogTruncateSize);
      dServer->setDebugLog(&mDataServerDebugLog);
    }

    if (mQueryServerProcessingLogFile.length() > 0)
    {
      mQueryServerProcessingLog.init(mQueryServerProcessingLogEnabled, mQueryServerProcessingLogFile.c_str(), mQueryServerProcessingLogMaxSize, mQueryServerProcessingLogTruncateSize);
      qServer->setProcessingLog(&mQueryServerProcessingLog);
    }

    if (mQueryServerDebugLogFile.length() > 0)
    {
      mQueryServerDebugLog.init(mQueryServerDebugLogEnabled, mQueryServerDebugLogFile.c_str(), mQueryServerDebugLogMaxSize, mQueryServerDebugLogTruncateSize);
      qServer->setDebugLog(&mQueryServerDebugLog);
    }

    // Waiting until content server is ready (i.e. all requested files are cached)
    while (!mShutdownRequested  &&  !cServer->isReady())
      boost::this_thread::sleep(boost::posix_time::seconds(1));

    if (mShutdownRequested)
      return;

    updateProducerAndGenerationList();
    updateMappings();

    mProducerMappingDefinitions.init(mProducerMappingDefinitions_filenames, true);
    mParameterAliasDefinitions.init(mParameterAliasDefinitions_filenames);

    mBrowser.init(mConfigurationFile_name.c_str(), mContentServer, this);
    mBrowser.setFlags(mBrowserFlags);

    // Register admin requests if reactor instance is available
    Spine::Reactor* reactor  = Spine::Reactor::instance;
    if (reactor)
    {
      using AdminRequestAccess = Spine::Reactor::AdminRequestAccess;

      reactor->addAdminTableRequestHandler(
        this,
        "gridgenerations",
        AdminRequestAccess::Public,
        std::bind(&Engine::requestGridGenerationInfo, this, std::placeholders::_2),
        "Grid generations");

      reactor->addAdminTableRequestHandler(
        this,
        "gridgenerationsqd",
        AdminRequestAccess::Public,
        std::bind(&Engine::requestGridQdGenerationInfo, this, std::placeholders::_2),
        "Grid newbase generations");

      reactor->addAdminTableRequestHandler(
        this,
        "gridproducers",
        AdminRequestAccess::Public,
        std::bind(&Engine::requestGridProducerInfo, this, std::placeholders::_2),
        "Grid producers");

      reactor->addAdminTableRequestHandler(
        this,
        "gridparameters",
        AdminRequestAccess::Public,
        std::bind(&Engine::requestGridParameterInfo, this, std::placeholders::_2),
        "Grid parameters");
    }

    startUpdateProcessing();
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}

void Engine::checkConfiguration()
{
  FUNCTION_TRACE
  try
  {
    if (!mEnabled)
      return;

    // ### Configuration updates when the server is running.

    time_t currentTime = time(nullptr);
    if ((currentTime - mConfigurationFile_checkTime) < 30)
      return;

    mConfigurationFile_checkTime = currentTime;

    time_t tt = getFileModificationTime(mConfigurationFile_name.c_str());
    if (tt == mConfigurationFile_modificationTime)
      return;

    ConfigurationFile configurationFile;
    configurationFile.readFile(mConfigurationFile_name.c_str());

    ContentServer_sptr contentServer = getContentServer_sptr();

    bool enabled = true;
    configurationFile.getAttributeValue("smartmet.engine.grid.enabled", enabled);
    if (mEnabled && !enabled)
    {
      // The grid-engine can be disabled when it is running. However, it cannot be enabled
      // if it is started in the disabled state.

      mEnabled = false;
      mContentServer->setEnabled(false);
      mDataServer->setEnabled(false);
      mQueryServer->setEnabled(false);

      std::cout << ANSI_FG_RED << Spine::log_time_str() << " Grid-engine configuration: engine disabled" << ANSI_FG_DEFAULT << std::endl;
      return;
    }

    // ### Content server processing log

    bool contentServerProcessingLogEnabled = false;
    std::string contentServerProcessingLogFile;
    int contentServerProcessingLogMaxSize = 0;
    int contentServerProcessingLogTruncateSize = 0;

    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.processing-log.enabled", contentServerProcessingLogEnabled);
    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.processing-log.file", contentServerProcessingLogFile);
    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.processing-log.maxSize", contentServerProcessingLogMaxSize);
    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.processing-log.truncateSize", contentServerProcessingLogTruncateSize);

    if (mContentServerProcessingLogEnabled != contentServerProcessingLogEnabled || mContentServerProcessingLogFile != contentServerProcessingLogFile
        || mContentServerProcessingLogMaxSize != contentServerProcessingLogMaxSize || mContentServerProcessingLogTruncateSize != contentServerProcessingLogTruncateSize)
    {
      mContentServerProcessingLog.close();

      mContentServerProcessingLogEnabled = contentServerProcessingLogEnabled;
      mContentServerProcessingLogFile = contentServerProcessingLogFile;
      mContentServerProcessingLogMaxSize = contentServerProcessingLogMaxSize;
      mContentServerProcessingLogTruncateSize = contentServerProcessingLogTruncateSize;

      mContentServerProcessingLog.init(mContentServerProcessingLogEnabled, mContentServerProcessingLogFile.c_str(), mContentServerProcessingLogMaxSize,
          mContentServerProcessingLogTruncateSize);
      if (contentServer->getProcessingLog() == nullptr)
        contentServer->setProcessingLog(&mContentServerProcessingLog);
    }

    // ### Content server debug log

    bool contentServerDebugLogEnabled = false;
    std::string contentServerDebugLogFile;
    int contentServerDebugLogMaxSize = 0;
    int contentServerDebugLogTruncateSize = 0;

    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.debug-log.enabled", contentServerDebugLogEnabled);
    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.debug-log.file", contentServerDebugLogFile);
    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.debug-log.maxSize", contentServerDebugLogMaxSize);
    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.debug-log.truncateSize", contentServerDebugLogTruncateSize);

    if (mContentServerDebugLogEnabled != contentServerDebugLogEnabled || mContentServerDebugLogFile != contentServerDebugLogFile
        || mContentServerDebugLogMaxSize != contentServerDebugLogMaxSize || mContentServerDebugLogTruncateSize != contentServerDebugLogTruncateSize)
    {
      mContentServerDebugLog.close();

      mContentServerDebugLogEnabled = contentServerDebugLogEnabled;
      mContentServerDebugLogFile = contentServerDebugLogFile;
      mContentServerDebugLogMaxSize = contentServerDebugLogMaxSize;
      mContentServerDebugLogTruncateSize = contentServerDebugLogTruncateSize;

      mContentServerDebugLog.init(mContentServerDebugLogEnabled, mContentServerDebugLogFile.c_str(), mContentServerDebugLogMaxSize, mContentServerDebugLogTruncateSize);
      if (contentServer->getDebugLog() == nullptr)
        contentServer->setDebugLog(&mContentServerDebugLog);
    }

    // ### Data server processing log

    bool dataServerProcessingLogEnabled = false;
    std::string dataServerProcessingLogFile;
    int dataServerProcessingLogMaxSize = 0;
    int dataServerProcessingLogTruncateSize = 0;

    configurationFile.getAttributeValue("smartmet.engine.grid.data-server.processing-log.enabled", dataServerProcessingLogEnabled);
    configurationFile.getAttributeValue("smartmet.engine.grid.data-server.processing-log.file", dataServerProcessingLogFile);
    configurationFile.getAttributeValue("smartmet.engine.grid.data-server.processing-log.maxSize", dataServerProcessingLogMaxSize);
    configurationFile.getAttributeValue("smartmet.engine.grid.data-server.processing-log.truncateSize", dataServerProcessingLogTruncateSize);

    if (mDataServerProcessingLogEnabled != dataServerProcessingLogEnabled || mDataServerProcessingLogFile != dataServerProcessingLogFile
        || mDataServerProcessingLogMaxSize != dataServerProcessingLogMaxSize || mDataServerProcessingLogTruncateSize != dataServerProcessingLogTruncateSize)
    {
      mDataServerProcessingLog.close();

      mDataServerProcessingLogEnabled = dataServerProcessingLogEnabled;
      mDataServerProcessingLogFile = dataServerProcessingLogFile;
      mDataServerProcessingLogMaxSize = dataServerProcessingLogMaxSize;
      mDataServerProcessingLogTruncateSize = dataServerProcessingLogTruncateSize;

      mDataServerProcessingLog.init(mDataServerProcessingLogEnabled, mDataServerProcessingLogFile.c_str(), mDataServerProcessingLogMaxSize, mDataServerProcessingLogTruncateSize);
      if (mDataServer->getProcessingLog() == nullptr)
        mDataServer->setProcessingLog(&mDataServerProcessingLog);
    }

    // ### Data server debug log

    bool dataServerDebugLogEnabled = false;
    std::string dataServerDebugLogFile;
    int dataServerDebugLogMaxSize = 0;
    int dataServerDebugLogTruncateSize = 0;

    configurationFile.getAttributeValue("smartmet.engine.grid.data-server.debug-log.enabled", dataServerDebugLogEnabled);
    configurationFile.getAttributeValue("smartmet.engine.grid.data-server.debug-log.file", dataServerDebugLogFile);
    configurationFile.getAttributeValue("smartmet.engine.grid.data-server.debug-log.maxSize", dataServerDebugLogMaxSize);
    configurationFile.getAttributeValue("smartmet.engine.grid.data-server.debug-log.truncateSize", dataServerDebugLogTruncateSize);

    if (mDataServerDebugLogEnabled != dataServerDebugLogEnabled || mDataServerDebugLogFile != dataServerDebugLogFile || mDataServerDebugLogMaxSize != dataServerDebugLogMaxSize
        || mDataServerDebugLogTruncateSize != dataServerDebugLogTruncateSize)
    {
      mDataServerDebugLog.close();

      mDataServerDebugLogEnabled = dataServerDebugLogEnabled;
      mDataServerDebugLogFile = dataServerDebugLogFile;
      mDataServerDebugLogMaxSize = dataServerDebugLogMaxSize;
      mDataServerDebugLogTruncateSize = dataServerDebugLogTruncateSize;

      mDataServerDebugLog.init(mDataServerDebugLogEnabled, mDataServerDebugLogFile.c_str(), mDataServerDebugLogMaxSize, mDataServerDebugLogTruncateSize);
      if (mDataServer->getDebugLog() == nullptr)
        mDataServer->setDebugLog(&mDataServerDebugLog);
    }

    // ### Query server processing log

    bool queryServerProcessingLogEnabled = false;
    std::string queryServerProcessingLogFile;
    int queryServerProcessingLogMaxSize = 0;
    int queryServerProcessingLogTruncateSize = 0;

    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.processing-log.enabled", queryServerProcessingLogEnabled);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.processing-log.file", queryServerProcessingLogFile);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.processing-log.maxSize", queryServerProcessingLogMaxSize);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.processing-log.truncateSize", queryServerProcessingLogTruncateSize);

    if (mQueryServerProcessingLogEnabled != queryServerProcessingLogEnabled || mQueryServerProcessingLogFile != queryServerProcessingLogFile
        || mQueryServerProcessingLogMaxSize != queryServerProcessingLogMaxSize || mQueryServerProcessingLogTruncateSize != queryServerProcessingLogTruncateSize)
    {
      mQueryServerProcessingLog.close();

      mQueryServerProcessingLogEnabled = queryServerProcessingLogEnabled;
      mQueryServerProcessingLogFile = queryServerProcessingLogFile;
      mQueryServerProcessingLogMaxSize = queryServerProcessingLogMaxSize;
      mQueryServerProcessingLogTruncateSize = queryServerProcessingLogTruncateSize;

      mQueryServerProcessingLog.init(mQueryServerProcessingLogEnabled, mQueryServerProcessingLogFile.c_str(), mQueryServerProcessingLogMaxSize,
          mQueryServerProcessingLogTruncateSize);
      if (mQueryServer->getProcessingLog() == nullptr)
        mQueryServer->setProcessingLog(&mQueryServerProcessingLog);
    }

    // ### Query server debug log

    bool queryServerDebugLogEnabled = false;
    std::string queryServerDebugLogFile;
    int queryServerDebugLogMaxSize = 0;
    int queryServerDebugLogTruncateSize = 0;

    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.debug-log.enabled", queryServerDebugLogEnabled);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.debug-log.file", queryServerDebugLogFile);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.debug-log.maxSize", queryServerDebugLogMaxSize);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.debug-log.truncateSize", queryServerDebugLogTruncateSize);

    if (mQueryServerDebugLogEnabled != queryServerDebugLogEnabled || mQueryServerDebugLogFile != queryServerDebugLogFile
        || mQueryServerDebugLogMaxSize != queryServerDebugLogMaxSize || mQueryServerDebugLogTruncateSize != queryServerDebugLogTruncateSize)
    {
      mQueryServerDebugLog.close();

      mQueryServerDebugLogEnabled = queryServerDebugLogEnabled;
      mQueryServerDebugLogFile = queryServerDebugLogFile;
      mQueryServerDebugLogMaxSize = queryServerDebugLogMaxSize;
      mQueryServerDebugLogTruncateSize = queryServerDebugLogTruncateSize;

      mQueryServerDebugLog.init(mQueryServerDebugLogEnabled, mQueryServerDebugLogFile.c_str(), mQueryServerDebugLogMaxSize, mQueryServerDebugLogTruncateSize);
      if (mQueryServer->getDebugLog() == nullptr)
        mQueryServer->setDebugLog(&mQueryServerDebugLog);
    }

    // ### Browser

    bool browserEnabled = false;

    configurationFile.getAttributeValue("smartmet.engine.grid.browser.enabled", browserEnabled);

    if (mBrowserEnabled != browserEnabled)
    {
      mBrowserEnabled = browserEnabled;

      if (mBrowserEnabled)
        std::cout << Spine::log_time_str() << " Grid-engine configuration: Browser enabled" << std::endl;
      else
        std::cout << Spine::log_time_str() << " Grid-engine configuration: Browser disabled" << std::endl;
    }


    unsigned long long browserFlags = 0;
    configurationFile.getAttributeValue("smartmet.engine.grid.browser.flags", browserFlags);

    if (mBrowserFlags != browserFlags)
    {
      std::cout << Spine::log_time_str() << " Grid-engine configuration: Browser flags changed (" << mBrowserFlags << " => " << browserFlags << ")" << std::endl;
      mBrowserFlags = browserFlags;
      mBrowser.setFlags(mBrowserFlags);
    }

    // ### Query cache

    bool queryCacheEnabled = false;
    int queryCacheMaxAge = 0;

    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.queryCache.enabled", queryCacheEnabled);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.queryCache.maxAge", queryCacheMaxAge);

    if (mQueryCache_enabled != queryCacheEnabled)
    {
      mQueryCache_enabled = queryCacheEnabled;

      if (mQueryCache_enabled)
        std::cout << Spine::log_time_str() << " Grid-engine configuration: query cache enabled" << std::endl;
      else
        std::cout << Spine::log_time_str() << " Grid-engine configuration: query cache disabled" << std::endl;
    }

    if (mQueryCache_maxAge != queryCacheMaxAge)
    {
      mQueryCache_maxAge = queryCacheMaxAge;
      std::cout << Spine::log_time_str() << " Grid-engine configuration: query cache max age set to " << mQueryCache_maxAge << " seconds" << std::endl;
    }

    if (mDataServerImplementation != nullptr)
    {
      time_t cleanupAge = mDataServerCleanupAge;
      time_t cleanupInterval = mDataServerCleanupInterval;

      configurationFile.getAttributeValue("smartmet.engine.grid.data-server.grid-storage.clean-up.age",cleanupAge);
      configurationFile.getAttributeValue("smartmet.engine.grid.data-server.grid-storage.clean-up.checkInterval",cleanupInterval);

      if (cleanupAge != mDataServerCleanupAge || cleanupInterval != mDataServerCleanupInterval)
      {
        mDataServerCleanupAge = cleanupAge;
        mDataServerCleanupInterval = cleanupInterval;
        mDataServerImplementation->setCleanup(mDataServerCleanupAge,mDataServerCleanupInterval);
        std::cout << Spine::log_time_str() << " Grid-engine configuration: clean-up.age = " <<  cleanupAge << ", clean-up.interval = " << cleanupInterval << std::endl;

      }
    }

    mConfigurationFile_modificationTime = tt;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Constructor failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}

bool Engine::isEnabled() const
{
  FUNCTION_TRACE
  try
  {
    return mEnabled;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}

void Engine::shutdown()
{
  FUNCTION_TRACE
  try
  {
    if (!mEnabled)
      return;

    memoryMapper.stopFaultHandler();

    if (mShutdownRequested.exchange(true)) {
        std::cout << __PRETTY_FUNCTION__ << " called more than once" << std::endl;
        return;
    }

    if (mEnabled && mThread)
        pthread_join(mThread, nullptr);

    std::cout << "  -- Shutdown requested (grid engine)\n";

    if (mQueryServer)
    {
      mQueryServer->shutdown();
      boost::this_thread::sleep(boost::posix_time::seconds(1));
    }

    if (mDataServer)
    {
      mDataServer->shutdown();
      boost::this_thread::sleep(boost::posix_time::seconds(1));
    }

    if (mContentServerCache)
      mContentServerCache->shutdown();

    if (mContentServer)
      mContentServer->shutdown();

    mShutdownFinished = true;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}

bool Engine::browserRequest(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest, Spine::HTTP::Response& theResponse)
{
  FUNCTION_TRACE
  try
  {
    if (!mEnabled || !mBrowserEnabled  || Spine::Reactor::isShuttingDown())
      return false;

    return mBrowser.requestHandler(session,theRequest, theResponse);
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}

void Engine::browserContent(SessionManagement::SessionInfo& session,std::ostringstream& output)
{
  FUNCTION_TRACE
  try
  {
    if (!mEnabled || !mBrowserEnabled || Spine::Reactor::isShuttingDown())
      return;

    mBrowser.browserContent(session,output);
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}

int Engine::executeQuery(QueryServer::Query& query) const
{
  FUNCTION_TRACE
  try
  {
    if (!mEnabled)
      return QueryServer::Result::SERVICE_DISABLED;

    if (Spine::Reactor::isShuttingDown())
      return QueryServer::Result::SERVICE_DISABLED;

    return mQueryServer->executeQuery(0, query);
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}

Query_sptr Engine::executeQuery(Query_sptr query) const
{
  FUNCTION_TRACE
  try
  {
    if (!mEnabled)
      return query;

    if (Spine::Reactor::isShuttingDown())
      return query;

    if (!mQueryCache_enabled)
    {
      int result = mQueryServer->executeQuery(0, *query);
      if (result != 0)
      {
        Fmi::Exception exception(BCP, "The query server returns an error message!");
        exception.addParameter("Result", Fmi::to_string(result));
        exception.addParameter("Message", QueryServer::getResultString(result));

        switch (result)
        {
          case QueryServer::Result::NO_PRODUCERS_FOUND:
            exception.addDetail("The reason for this situation is usually that the given producer is unknown");
            exception.addDetail("or there are no producer list available in the grid engine's configuration "
                "file.");
            break;
        }
        throw exception;
      }
      return query;
    }

    time_t currentTime = time(nullptr);
    std::size_t hash = query->getHash();

    QueryCacheIterator it;
    bool noMatch = false;

    {
      AutoReadLock lock(&mQueryCache_modificationLock);
      it = mQueryCache.find(hash);
      if (it != mQueryCache.end())
      {
        mQueryCache_stats.hits++;
        for (auto prod = it->second.producerHashMap.begin(); prod != it->second.producerHashMap.end() && !noMatch; ++prod)
        {
          ulonglong producerHash = getProducerHash(prod->first);
          if (producerHash != prod->second)
            noMatch = true;
        }

        if (!noMatch)
        {
          // The cache entry is valid. We can return it.
          it->second.lastAccessTime = currentTime;
          it->second.accessCounter++;
          return it->second.query;
        }
      }
      else
      {
        mQueryCache_stats.misses++;
      }
    }

    int result = mQueryServer->executeQuery(0, *query);
    if (result != 0)
    {
      Fmi::Exception exception(BCP, "The query server returns an error message!");
      exception.addParameter("Result", Fmi::to_string(result));
      exception.addParameter("Message", QueryServer::getResultString(result));

      switch (result)
      {
        case QueryServer::Result::NO_PRODUCERS_FOUND:
          exception.addDetail("The reason for this situation is usually that the given producer is unknown");
          exception.addDetail("or there are no producer list available in the grid engine's configuration "
              "file.");
          break;
      }
      throw exception;
    }

    if (isCacheable(query))
    {
      CacheRec rec;
      rec.query = query;
      rec.cacheTime = currentTime;
      rec.lastAccessTime = currentTime;
      rec.accessCounter = 0;

      std::set < uint > producerIdList;
      query->getResultProducerIdList(producerIdList);

      for (auto it = producerIdList.begin(); it != producerIdList.end(); ++it)
      {
        ulonglong producerHash = getProducerHash(*it);
        rec.producerHashMap.insert(std::pair<uint, ulonglong>(*it, producerHash));
      }

      AutoWriteLock lock(&mQueryCache_modificationLock);
      mQueryCache.insert(std::pair<std::size_t, CacheRec>(hash, rec));
      mQueryCache_stats.inserts++;
      mQueryCache_stats.size++;
    }
    return query;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}

bool Engine::isCacheable(std::shared_ptr<QueryServer::Query> query) const
{
  FUNCTION_TRACE
  try
  {
    if (!mEnabled)
      return false;

    for (auto param = query->mQueryParameterList.begin(); param != query->mQueryParameterList.end(); ++param)
    {
      switch (param->mType)
      {
        case QueryServer::QueryParameter::Type::PointValues:
        case QueryServer::QueryParameter::Type::Isoline:
        case QueryServer::QueryParameter::Type::Isoband:
          break;

        case QueryServer::QueryParameter::Type::Vector:
        case QueryServer::QueryParameter::Type::GridFile:
          return false;
      }

      switch (param->mLocationType)
      {
        case QueryServer::QueryParameter::LocationType::Point:
        case QueryServer::QueryParameter::LocationType::Polygon:
        case QueryServer::QueryParameter::LocationType::Path:
        case QueryServer::QueryParameter::LocationType::Circle:
          break;

        case QueryServer::QueryParameter::LocationType::Grid:
          return false;

        case QueryServer::QueryParameter::LocationType::Geometry:
          break;
      }
    }
    return true;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}

std::string Engine::getConfigurationFileName()
{
  FUNCTION_TRACE
  try
  {
    return mConfigurationFile_name;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}

std::string Engine::getProducerFileName()
{
  FUNCTION_TRACE
  try
  {
    return mProducerSearchList_filename;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}

ContentServer_sptr Engine::getContentServer_sptr() const
{
  FUNCTION_TRACE
  try
  {
    if (mEnabled && mContentCacheEnabled)
      return mContentServerCache;
    else
      return mContentServer;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}

ContentServer_sptr Engine::getContentSourceServer_sptr() const
{
  FUNCTION_TRACE
  try
  {
    return mContentServer;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}

DataServer_sptr Engine::getDataServer_sptr() const
{
  FUNCTION_TRACE
  try
  {
    return mDataServer;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}

QueryServer_sptr Engine::getQueryServer_sptr() const
{
  FUNCTION_TRACE
  try
  {
    return mQueryServer;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}

bool Engine::isGridProducer(const std::string& producer) const
{
  FUNCTION_TRACE
  try
  {
    if (!mEnabled)
      return false;

    std::string prod = producer;
    std::string tmp;
    if ((mProducerMappingDefinitions.getAlias(producer, tmp) && strchr(tmp.c_str(), ';') == nullptr))
    {
      // Replacing producer alias name with the (newbase) mapping name.
      prod = tmp;
    }

    // Finding (Radon) producer names according to the (newbase) producer name from the mappings.

    std::vector < std::string > nameList;

    AutoReadLock lock(&mProducerInfoList_modificationLock);
    getProducerNameList(prod, nameList);

    // Checking that the search of the current (Radon) producers is allower (defined in the
    // producers search file).

    for (auto it = nameList.begin(); it != nameList.end(); ++it)
    {
      if (it->empty())
        return true;  // The (newbase) producer is mapped to an empty (Radon) producer (=> default
                      // producer)

      for (auto itm = mProducerSearchList.begin(); itm != mProducerSearchList.end(); ++itm)
      {
        if (strcasecmp(it->c_str(), itm->c_str()) == 0)
          return true;
      }
    }
    return false;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}

std::string Engine::getParameterString(const std::string& producer, const std::string& parameter) const
{
  FUNCTION_TRACE
  try
  {
    if (!mEnabled)
      return parameter;

    std::string key = producer + ";" + parameter;

    ParameterDetails_vec parameters;
    getParameterDetails(producer, parameter, parameters);

    std::string prod;
    std::string geomId;
    std::string level;
    std::string levelId;
    std::string forecastType;
    std::string forecastNumber;

    size_t len = parameters.size();

    if (len > 0 && strcasecmp(parameters[0].mProducerName.c_str(), key.c_str()) != 0)
    {
      for (size_t t = 0; t < 1; t++)
      {
        if (parameters[t].mLevelId > "")
          levelId = parameters[t].mLevelId;

        if (parameters[t].mLevel > "")
          level = parameters[t].mLevel;

        if (parameters[t].mForecastType > "")
          forecastType = parameters[t].mForecastType;

        if (parameters[t].mForecastNumber > "")
          forecastNumber = parameters[t].mForecastNumber;

        if (parameters[t].mProducerName > "")
          prod = parameters[t].mProducerName;

        if (parameters[t].mGeometryId > "")
        {
          prod = parameters[t].mProducerName;
          geomId = parameters[t].mGeometryId;
        }
      }
      std::string paramStr = parameter + ":" + prod + ":" + geomId + ":" + levelId + ":" + level + ":" + forecastType + ":" + forecastNumber;
      return paramStr;
    }

    return parameter;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}

std::string Engine::getProducerName(const std::string& aliasName) const
{
  FUNCTION_TRACE
  try
  {
    if (!mEnabled)
      return aliasName;

    // This method returns the producer's mapping name.

    mProducerMappingDefinitions.checkUpdates(false);

    // Finding the mapping name for the (newbase) producer alias. The producer name mappings look
    // like this:
    //   pal:pal_skandinavia
    //   pal_scandinavia:pal_skandinavia

    std::string prod = aliasName;
    if ((mProducerMappingDefinitions.getAlias(aliasName, prod) && strchr(prod.c_str(), ';') == nullptr))
      return prod;

    return aliasName;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}

void Engine::getProducerNameList(const std::string& mappingName, std::vector<std::string>& nameList) const
{
  FUNCTION_TRACE
  try
  {
    if (!mEnabled)
      return;

    // This method returns the list of (Radon) producers according to the (newbase) mapping name.

    mProducerMappingDefinitions.checkUpdates(false);

    std::vector < std::string > mappingList;
    mProducerMappingDefinitions.getAliasList(mappingName, mappingList);

    // The producer name mapping list looks like this:
    //   pal_skandinavia:SMARTMET;1096;;;
    //   pal_skandinavia:SMARTMETMTA;1096;;;

    for (auto it = mappingList.begin(); it != mappingList.end(); it++)
    {
      std::vector < std::string > partList;
      splitString(*it, ';', partList);

      nameList.emplace_back(partList[0]);
    }

    if (nameList.size() == 0)
      nameList.emplace_back(mappingName);
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}

ulonglong Engine::getProducerHash(uint producerId) const
{
  FUNCTION_TRACE
  try
  {
    if (!mEnabled)
      return 0;

    // This method returns the hash of the producer's content information in the Content
    // Server. This is the fastest way to check if the cached content information is still
    // valid. The hash is updated if it is older than 120 seconds.

    ContentServer_sptr contentServer = getContentServer_sptr();
    time_t currentTime = time(nullptr);
    ulonglong hash = 0;

    auto rec = mProducerHashMap.find(producerId);
    if (rec != mProducerHashMap.end())
    {
      if ((currentTime - rec->second.checkTime) > 120)
      {
        rec->second.checkTime = currentTime;
        int result = contentServer->getHashByProducerId(0, producerId, hash);
        if (result == 0)
          rec->second.hash = hash;
        else
          rec->second.hash = 0;
      }
      return rec->second.hash;
    }

    int result = contentServer->getHashByProducerId(0, producerId, hash);
    if (result == 0)
    {
      HashRec hrec;
      hrec.checkTime = currentTime;
      hrec.hash = hash;

      mProducerHashMap.insert(std::pair<uint, HashRec>(producerId, hrec));
      return hash;
    }

    return 0;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}



ulonglong Engine::getProducerHash(std::string producerName) const
{
  try
  {
    ulonglong hash = 0;
    std::vector<std::string> nameList;
    getProducerNameList(producerName,nameList);
    if (nameList.size() > 0)
    {
      for (auto it = nameList.begin(); it != nameList.end(); ++it)
      {
        T::ProducerInfo producerInfo;
        getProducerInfoByName(*it,producerInfo);
        if (producerInfo.mProducerId != 0)
        {
          hash += getProducerHash(producerInfo.mProducerId);
        }
      }
    }
    else
    {
      T::ProducerInfo producerInfo;
      getProducerInfoByName(producerName,producerInfo);
      if (producerInfo.mProducerId != 0)
      {
        hash += getProducerHash(producerInfo.mProducerId);
      }
    }

    return hash;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}



void Engine::getParameterDetails(const std::string& aliasName, ParameterDetails_vec& parameterDetails) const
{
  FUNCTION_TRACE
  try
  {
    if (!mEnabled)
      return;

    mProducerMappingDefinitions.checkUpdates(false);

    std::vector <std::string> aliasStrings;
    mProducerMappingDefinitions.getAliasList(aliasName, aliasStrings);

    for (auto it = aliasStrings.begin(); it != aliasStrings.end(); it++)
    {
      std::vector < std::string > partList;
      splitString(*it, ';', partList);

      ParameterDetails p;
      p.mOriginalProducer = aliasName;

      uint len = partList.size();
      for (uint t = 0; t < len; t++)
      {
        switch (t)
        {
          case 0:
            p.mProducerName = partList[t];
            break;

          case 1:
            p.mGeometryId = partList[t];
            break;

          case 2:
            p.mLevelId = partList[t];
            break;

          case 3:
            p.mLevel = partList[t];
            break;

          case 4:
            p.mForecastType = partList[t];
            break;

          case 5:
            p.mForecastNumber = partList[t];
            break;
        }
      }

      parameterDetails.emplace_back(p);
    }

    if (parameterDetails.size() == 0)
    {
      ParameterDetails p;
      p.mOriginalProducer = aliasName;
      p.mProducerName = aliasName;
      parameterDetails.emplace_back(p);
    }
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}

void Engine::getParameterAlias(const std::string& aliasName, std::string& aliasValue) const
{
  try
  {
    mParameterAliasDefinitions.getAlias(aliasName, aliasValue);
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}


void Engine::getParameterDetails(const std::string& producerName, const std::string& parameterName, ParameterDetails_vec& parameterDetails) const
{
  FUNCTION_TRACE
  try
  {
    if (!mEnabled)
      return;

    mProducerMappingDefinitions.checkUpdates(false);
    mParameterAliasDefinitions.checkUpdates(false);


    //std::cout << "DETAILS [" << producerName << "] [" << parameterName << "]\n";
    std::string prod = producerName;
    std::string tmp;

    // Finding the mapping name for the (newbase) producer. The producer name mappings look like
    // this:
    //
    //   pal:pal_skandinavia

    if (mProducerMappingDefinitions.getAlias(producerName, tmp) && strchr(tmp.c_str(), ';') == nullptr)
    {
      getParameterDetails(tmp, parameterName, parameterDetails);
      return;
    }

    // Finding "official name" for the parameter alias name. This name is used in the parameter
    // mapping files when the query is executed. The parameter alias definitions look like this:
    //
    //   fog:FogIntensity
    //   rtype:PrecipitationType

    std::string fullParam = parameterName;
    mParameterAliasDefinitions.getAlias(parameterName, fullParam);

    // Finding producer mapping for the parameter. The point is that different (newbase) parameters
    // might be mapped to different (Radon) producers.The mapping list looks like this:
    //
    //   pal_skandinavia;FogIntensity:SMARTMET;1096;;;;;
    //   pal_skandinavia;PrecipitationType:SMARTMETMTA;1096;;;;;
    //
    // Notice that this search only maps the (newbase) producer and the paramer information into
    // the producer name, geometry, levelId, level, forecast type and forecast number used by the
    // Radon. This means that the current (newbase) parameter name should be found from the
    // parameter mapping files (this is not checked here). For example:
    //
    //   SMARTMET;FogIntensity;2;FOGINT-N;1096;1;6;00000;2;2;2;0;E;;;;
    //   SMARTMETMTA;PrecipitationType;2;PRECTYPE-N;1096;1;6;00000;2;2;2;0;E;;;;


    std::vector<std::string> list;
    splitString(fullParam,':',list);

    std::string param;
    uint plen = list.size();
    if (plen > 1)
      param = list[0];
    else
      param = fullParam;

    //std::cout << "PARAM [" << fullParam << "] [" << param << "] [" << parameterName << "]\n";

    std::string key = prod + ";" + param;
    std::vector < std::string > mappingList;
    mProducerMappingDefinitions.getAliasList(key, mappingList);

    for (auto it = mappingList.begin(); it != mappingList.end(); it++)
    {
      std::vector <std::string> partList;
      splitString(*it, ';', partList);

      ParameterDetails p;
      p.mOriginalProducer = producerName;
      p.mOriginalParameter = parameterName;

      uint len = partList.size();

      for (uint t = 0; t < len; t++)
      {
        std::string val = partList[t];
        if (plen > (t+1)  &&  !list[t+1].empty())
          val = list[t+1];

        switch (t)
        {
          case 0:
            p.mProducerName = val;
            break;

          case 1:
            p.mGeometryId = val;
            break;

          case 2:
            p.mLevelId = val;
            break;

          case 3:
            p.mLevel = val;
            break;

          case 4:
            p.mForecastType = val;
            break;

          case 5:
            p.mForecastNumber = val;
            break;
        }
      }

      //p.print(std::cout,0,0);
      parameterDetails.emplace_back(p);
    }

    if (parameterDetails.size() == 0)
    {
      ParameterDetails p;
      p.mOriginalProducer = producerName;
      p.mOriginalParameter = fullParam;
      p.mProducerName = producerName + ";" + fullParam;
      //p.print(std::cout,0,0);
      parameterDetails.emplace_back(p);
    }
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}



void Engine::getParameterDetails(const std::string& producerName, const std::string& parameterName, std::string& level, ParameterDetails_vec& parameterDetails) const
{
  FUNCTION_TRACE
  try
  {
    if (!mEnabled)
      return;

    getParameterDetails(producerName, parameterName, parameterDetails);
    for (auto it = parameterDetails.begin(); it != parameterDetails.end(); ++it)
      it->mLevel = level;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}

void Engine::getParameterMappings(
    const std::string& producerName,
    const std::string& parameterName,
    T::GeometryId geometryId,
    bool onlySearchEnabled,
    QueryServer::ParameterMapping_vec& mappings) const
{
  try
  {
    if (!mEnabled)
      return;

    if (!mParameterMappingDefinitions)
      return;

    AutoReadLock lock(&mParameterMappingDefinitions_modificationLock);

    for (auto m = mParameterMappingDefinitions->begin(); m != mParameterMappingDefinitions->end(); ++m)
    {
      m->getMappings(producerName, parameterName, geometryId, onlySearchEnabled, mappings);
    }

    for (auto m = mParameterAliasMappings.begin(); m != mParameterAliasMappings.end(); ++m)
    {
      m->getMappings(producerName, parameterName, geometryId, onlySearchEnabled, mappings);
    }
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}

void Engine::getParameterMappings(const std::string& producerName, const std::string& parameterName, bool onlySearchEnabled, QueryServer::ParameterMapping_vec& mappings) const
{
  try
  {
    if (!mEnabled)
      return;

    if (!mParameterMappingDefinitions)
      return;

    AutoReadLock lock(&mParameterMappingDefinitions_modificationLock);

    for (auto m = mParameterMappingDefinitions->begin(); m != mParameterMappingDefinitions->end(); ++m)
    {
      m->getMappings(producerName, parameterName, onlySearchEnabled, mappings);
    }

    for (auto m = mParameterAliasMappings.begin(); m != mParameterAliasMappings.end(); ++m)
    {
      m->getMappings(producerName, parameterName, onlySearchEnabled, mappings);
    }
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}

void Engine::getParameterMappings(
    const std::string& producerName,
    const std::string& parameterName,
    T::GeometryId geometryId,
    T::ParamLevelId levelId,
    T::ParamLevel level,
    bool onlySearchEnabled,
    QueryServer::ParameterMapping_vec& mappings) const
{
  try
  {
    if (!mEnabled)
      return;

    if (!mParameterMappingDefinitions)
      return;

    AutoReadLock lock(&mParameterMappingDefinitions_modificationLock);

    for (auto m = mParameterMappingDefinitions->begin(); m != mParameterMappingDefinitions->end(); ++m)
    {
      m->getMappings(producerName, parameterName, geometryId, levelId, level, onlySearchEnabled, mappings);
    }

    for (auto m = mParameterAliasMappings.begin(); m != mParameterAliasMappings.end(); ++m)
    {
      m->getMappings(producerName, parameterName, geometryId, levelId, level, onlySearchEnabled, mappings);
    }
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}

void Engine::getParameterMappings(
    const std::string& producerName,
    const std::string& parameterName,
    T::ParamLevelId levelId,
    T::ParamLevel level,
    bool onlySearchEnabled,
    QueryServer::ParameterMapping_vec& mappings) const
{
  try
  {
    if (!mEnabled)
      return;

    if (!mParameterMappingDefinitions)
      return;

    AutoReadLock lock(&mParameterMappingDefinitions_modificationLock);

    for (auto m = mParameterMappingDefinitions->begin(); m != mParameterMappingDefinitions->end(); ++m)
    {
      m->getMappings(producerName, parameterName, levelId, level, onlySearchEnabled, mappings);
    }

    for (auto m = mParameterAliasMappings.begin(); m != mParameterAliasMappings.end(); ++m)
    {
      m->getMappings(producerName, parameterName, levelId, level, onlySearchEnabled, mappings);
    }
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}

void Engine::mapParameterDetails(ParameterDetails_vec& parameterDetails) const
{
  try
  {
    if (!mEnabled)
      return;

    ContentServer_sptr contentServer = getContentServer_sptr();

    for (auto rec = parameterDetails.begin(); rec != parameterDetails.end(); ++rec)
    {
      QueryServer::ParameterMapping_vec mappings;
      if (rec->mLevelId > " " || rec->mLevel > " ")
      {
        getParameterMappings(rec->mProducerName, rec->mOriginalParameter, atoi(rec->mGeometryId.c_str()), atoi(rec->mLevelId.c_str()),
            atoi(rec->mLevel.c_str()), false, mappings);
        if (mappings.size() == 0 && rec->mLevel < " ")
        {
          getParameterMappings(rec->mProducerName, rec->mOriginalParameter, atoi(rec->mGeometryId.c_str()), atoi(rec->mLevelId.c_str()), -1, false,
              mappings);
        }
      }
      else
      {
        getParameterMappings(rec->mProducerName, rec->mOriginalParameter, atoi(rec->mGeometryId.c_str()), true, mappings);
      }

      for (auto m = mappings.begin(); m != mappings.end(); ++m)
      {
        MappingDetails details;
        details.mMapping = *m;

        T::ContentInfoList contentInfoList;
        int result = contentServer->getContentListByParameterAndProducerName(0, m->mProducerName, m->mParameterKeyType, m->mParameterKey,
            m->mParameterLevelId, m->mParameterLevel, m->mParameterLevel, -1, -1, m->mGeometryId, std::string("19000101T000000"), std::string("21000101T000000"), 0,
            contentInfoList);
        if (result == 0)
        {
          uint len = contentInfoList.getLength();
          AutoReadLock lock(&mProducerInfoList_modificationLock);
          for (uint t = 0; t < len; t++)
          {
            T::ContentInfo* cInfo = contentInfoList.getContentInfoByIndex(t);
            if (cInfo != nullptr)
            {
              T::GenerationInfo* gInfo = mGenerationInfoList.getGenerationInfoById(cInfo->mGenerationId);
              if (gInfo != nullptr)
              {
                auto tt = details.mTimes.find(gInfo->mAnalysisTime);
                if (tt != details.mTimes.end())
                {
                  tt->second.insert(cInfo->getForecastTime());
                }
                else
                {
                  std::set < std::string > ttt;
                  ttt.insert(cInfo->getForecastTime());
                  details.mTimes.insert(std::pair<std::string, std::set<std::string>>(gInfo->mAnalysisTime, ttt));
                }
              }
            }
          }
        }

        rec->mMappings.emplace_back(details);
      }
    }
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}

std::string Engine::getProducerAlias(const std::string& producerName, int levelId) const
{
  FUNCTION_TRACE
  try
  {
    if (!mEnabled)
      return producerName;

    // This method returns the producer mapping name. Sometimes the same alias name
    // is used for different mappings. In this case the requested level type might
    // help us to identify the corret producer.

    // The mapping definitions might look like this (aliasName + levelId):
    //   ec:ecmwf_maailma_pinta
    //   ec;2:ecmwf_maailma_painepinta

    mProducerMappingDefinitions.checkUpdates(false);

    std::string prod = producerName;
    std::string tmp = producerName;
    if (levelId >= 0)
      tmp = producerName + ";" + std::to_string(levelId);

    if (mProducerMappingDefinitions.getAlias(tmp, prod) && strchr(prod.c_str(), ';') == nullptr)
      return prod;

    return producerName;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}

ContentTable Engine::getProducerInfo(std::optional<std::string> producer,std::string timeFormat) const
{
  try
  {
    updateProducerAndGenerationList();

    std::unique_ptr<Fmi::TimeFormatter> timeFormatter(Fmi::TimeFormatter::create(timeFormat));
    std::unique_ptr< Spine::Table > resultTable(new Spine::Table);

    Spine::TableFormatter::Names headers
    { "#", "ProducerName", "ProducerId", "Title", "Description", "NumOfGenerations", "NewestGeneration", "OldestGeneration" };
    resultTable->setNames(headers);
    resultTable->setTitle("SmartMet Admin");

    AutoReadLock lock(&mProducerInfoList_modificationLock);

    uint len = mProducerInfoList.getLength();
    uint row = 0;
    for (uint t = 0; t < len; t++)
    {
      T::ProducerInfo* info = mProducerInfoList.getProducerInfoByIndex(t);

      if (!producer || strcasecmp(producer->c_str(), info->mName.c_str()) == 0)
      {
        T::GenerationInfoList generationList;
        mGenerationInfoList.getGenerationInfoListByProducerId(info->mProducerId, generationList);

        uint glen = generationList.getLength();
        if (glen > 0)
        {
          generationList.sort(T::GenerationInfo::ComparisonMethod::analysisTime);

          // Row number
          resultTable->set(0, row, Fmi::to_string(row + 1));

          // Producer name
          resultTable->set(1, row, info->mName);

          // Producer id
          resultTable->set(2, row, Fmi::to_string(info->mProducerId));

          // Producer title
          resultTable->set(3, row, info->mTitle);

          // Producer description
          resultTable->set(4, row, info->mDescription);

          // Number of generations
          resultTable->set(5, row, Fmi::to_string(glen));

          T::GenerationInfo* newest = generationList.getGenerationInfoByIndex(glen - 1);
          T::GenerationInfo* oldest = generationList.getGenerationInfoByIndex(0);

          // Newest generation

          if (!timeFormat.empty()  &&  strcasecmp(timeFormat.c_str(),"ISO") != 0)
          {
            // Analysis time
            Fmi::DateTime fTime = toTimeStamp(newest->mAnalysisTime);
            resultTable->set(6, row, timeFormatter->format(fTime));

            // Oldest generation
            Fmi::DateTime lTime = toTimeStamp(oldest->mAnalysisTime);
            resultTable->set(7, row, timeFormatter->format(lTime));
          }
          else
          {
            resultTable->set(6, row, newest->mAnalysisTime);

            // Oldest generation
            resultTable->set(7, row, oldest->mAnalysisTime);
          }

          row++;
        }
      }
    }

    return resultTable;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}



ContentTable Engine::getGenerationInfo(std::optional<std::string> producer,std::string timeFormat) const
{
  try
  {
    updateProducerAndGenerationList();

    /*
     * Almost same information can be fetched with getEngineMetadata() method.
     * Maybe this could be used also here.

    if (producer)
    {
      auto list = getEngineMetadata(producer->c_str());
      for (auto it=list.begin(); it != list.end();++it)
        it->print(std::cout,0,0);
    }
    else
    {
      auto list = getEngineMetadata("");
      for (auto it=list.begin(); it != list.end();++it)
        it->print(std::cout,0,0);
    }
    */

    if (timeFormat.empty())
      timeFormat = "iso";

    std::unique_ptr<Fmi::TimeFormatter> timeFormatter(Fmi::TimeFormatter::create(timeFormat));

    std::unique_ptr<Spine::Table> resultTable(new Spine::Table);

    Spine::TableFormatter::Names headers
    {"ProducerName", "GeometryId", "Timesteps", "AnalysisTime", "MinTime", "MaxTime", "ModificationTime", "FmiParameters", "ParameterAliases" };
    resultTable->setNames(headers);
    resultTable->setTitle("SmartMet Admin");

    ContentServer_sptr contentServer = getContentServer_sptr();
    time_t currentTime = time(nullptr);
    AutoReadLock lock(&mProducerInfoList_modificationLock);

    uint len = mProducerInfoList.getLength();
    uint row = 0;
    for (uint t = 0; t < len; t++)
    {
      T::ProducerInfo* info = mProducerInfoList.getProducerInfoByIndex(t);

      if (!producer || strcasecmp(producer->c_str(), info->mName.c_str()) == 0)
      {
        T::GenerationInfoList generationList;
        mGenerationInfoList.getGenerationInfoListByProducerId(info->mProducerId, generationList);

        uint glen = generationList.getLength();
        if (glen > 0)
        {
          generationList.sort(T::GenerationInfo::ComparisonMethod::analysisTime);

          uint glen = generationList.getLength();
          for (uint g=0; g<glen; g++)
          {
            T::GenerationInfo *gInfo = generationList.getGenerationInfoByIndex(g);
            time_t deletionTime = gInfo->mDeletionTime;
            if (deletionTime == 0 || (currentTime + 120) < deletionTime)
            {
              T::GeometryInfoList geometryInfoList;
              mGeometryInfoList.getGeometryInfoListByGenerationId(gInfo->mGenerationId,geometryInfoList);

              uint geomLen = geometryInfoList.getLength();
              if (geomLen == 0)
              {
                // It seems that there are no geometry status information available. Let's generate it.

                std::set<T::GeometryId> geometryIdList;
                contentServer->getContentGeometryIdListByGenerationId(0,gInfo->mGenerationId,geometryIdList);
                if (geometryIdList.size() > 0)
                {
                  for (auto gi = geometryIdList.begin(); gi != geometryIdList.end(); ++gi)
                  {
                    T::GeometryInfo *geom = new T::GeometryInfo();
                    geom->mProducerId = info->mProducerId;
                    geom->mGenerationId = gInfo->mGenerationId;
                    geom->mGeometryId = *gi;
                    geom->mModificationTime = gInfo->mModificationTime;
                    geom->mDeletionTime = gInfo->mDeletionTime;
                    geom->mStatus = gInfo->mStatus;
                    geometryInfoList.addGeometryInfo(geom);
                  }
                }
                geomLen = geometryInfoList.getLength();
              }

              for (uint gg = 0; gg < geomLen; gg++)
              {
                T::GeometryInfo *geom = geometryInfoList.getGeometryInfoByIndex(gg);
                if (geom != nullptr &&  geom->mStatus == T::GeometryInfo::Status::Ready)
                {
                  std::set<std::string> contentTimeList;
                  contentServer->getContentTimeListByGenerationAndGeometryId(0,geom->mGenerationId,geom->mGeometryId,contentTimeList);

                  std::ostringstream output1;
                  std::ostringstream output2;

                  if (mParameterMappingDefinitions)
                  {
                    AutoReadLock lock(&mParameterMappingDefinitions_modificationLock);
                    std::set<std::string> paramKeyList;
                    contentServer->getContentParamKeyListByGenerationAndGeometryId(0,geom->mGenerationId,geom->mGeometryId,T::ParamKeyTypeValue::FMI_NAME,paramKeyList);


                    std::set<std::string> paramList;
                    for (auto pk = paramKeyList.begin(); pk != paramKeyList.end(); ++pk)
                    {
                      QueryServer::ParameterMapping_vec mappings;
                      for (auto mf = mParameterMappingDefinitions->begin(); mf != mParameterMappingDefinitions->end(); ++mf)
                      {
                        mf->getMappingsByParamKey(info->mName,T::ParamKeyTypeValue::FMI_NAME,*pk,-1,-1,-1,mappings);
                      }

                      if (mappings.size() > 0)
                      {
                        for (auto pm = mappings.begin(); pm != mappings.end(); ++pm)
                        {
                          if (strcasecmp(pk->c_str(),pm->mParameterName.c_str()) != 0)
                            paramList.insert(pm->mParameterName);
                        }
                      }
                      if (pk != paramKeyList.begin())
                       output1 << ",";

                      output1 << *pk;
                    }

                    for (auto pp = paramList.begin(); pp != paramList.end(); ++pp)
                    {
                      if (pp != paramList.begin())
                       output2 << ",";

                      output2 << *pp;
                    }
                  }

                  uint slen = contentTimeList.size();
                  if (slen > 0)
                  {
                    auto first = contentTimeList.begin();
                    auto last = contentTimeList.rbegin();

                    // Producer name
                    resultTable->set(0, row, info->mName);

                    // Geometry id
                    resultTable->set(1, row, std::to_string(geom->mGeometryId));

                    // Timesteps
                    resultTable->set(2, row, std::to_string(slen));


                    if (!timeFormat.empty()  &&  strcasecmp(timeFormat.c_str(),"ISO") != 0  && timeFormatter)
                    {
                      // Analysis time
                      Fmi::DateTime aTime = toTimeStamp(gInfo->mAnalysisTime);
                      resultTable->set(3, row, timeFormatter->format(aTime));

                      // Min time
                      Fmi::DateTime fTime = toTimeStamp(*first);
                      resultTable->set(4, row, timeFormatter->format(fTime));

                      // Max time
                      Fmi::DateTime lTime = toTimeStamp(*last);
                      resultTable->set(5, row, timeFormatter->format(lTime));

                      // Modification time
                      Fmi::DateTime mTime = toTimeStamp(utcTimeFromTimeT(geom->mModificationTime));
                      resultTable->set(6, row, timeFormatter->format(mTime));
                    }
                    else
                    {
                      // Analysis time
                      resultTable->set(3, row, gInfo->mAnalysisTime);

                      // Min time
                      resultTable->set(4, row, *first);

                      // Max time
                      resultTable->set(5, row, *last);

                      // Modification time
                      resultTable->set(6, row, utcTimeFromTimeT(geom->mModificationTime));
                    }

                    // FMI Parameters
                    resultTable->set(7, row, output1.str());

                    // Parameter alias names
                    resultTable->set(8, row, output2.str());

                    row++;
                  }
                }
              }
            }
          }
        }
      }
    }

    return resultTable;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}



ContentTable Engine::getExtGenerationInfo(std::optional<std::string> producer,std::string timeFormat) const
{
  try
  {
    updateProducerAndGenerationList();

    if (timeFormat.empty())
      timeFormat = "iso";

    T::GeometryInfoList tmpGeometryInfoList;

    std::unique_ptr<Fmi::TimeFormatter> timeFormatter(Fmi::TimeFormatter::create(timeFormat));
    std::unique_ptr<Spine::Table> resultTable(new Spine::Table);
    Spine::TableFormatter::Names headers
    { "ProducerName", "GeometryId", "Timesteps", "AnalysisTime", "MinTime", "MaxTime", "ModificationTime", "FmiParameters", "ParameterAliases" };
    resultTable->setNames(headers);
    resultTable->setTitle("SmartMet Admin");

    if (mProducerStatusFile.empty())
      return resultTable;

    ContentServer_sptr contentServer = getContentServer_sptr();

    std::vector<std::vector<std::string>> records;
    readCsvFile(mProducerStatusFile.c_str(),records);

    time_t currentTime = time(nullptr);
    uint row = 0;

    //printf("RECORDS %ld\n",records.size());
    for (auto rec = records.begin(); rec != records.end(); ++rec)
    {
      uint sz = rec->size();
      if (sz > 1 && (!producer ||  producer->empty() || strcasecmp(producer->c_str(),(*rec)[0].c_str()) == 0))
      {
        bool available = true;
        std::map<std::string,std::vector<std::pair<uint,int>>> counterList;

        for (uint t=1; t<sz && available; t++)
        {
          AutoReadLock lock(&mProducerInfoList_modificationLock);

          std::vector <std::string> list;
          splitString((*rec)[t], ':',list);

          std::string producerName = list[0];
          int geometryId = 0;
          if (list.size() > 1)
            geometryId = toInt32(list[1]);

          T::ProducerInfo *pInfo = mProducerInfoList.getProducerInfoByName(producerName);
          if (pInfo)
          {
            //printf("PRODUCER %s\n",pInfo->mName.c_str());
            T::GenerationInfoList generationInfoList;
            mGenerationInfoList.getGenerationInfoListByProducerId(pInfo->mProducerId,generationInfoList);
            //printf("GENERATIONS %u/%u\n",generationInfoList.getLength(),mGenerationInfoList.getLength());

            uint len = generationInfoList.getLength();
            for (uint g = 0; g < len; g++)
            {
              T::GenerationInfo *gInfo = generationInfoList.getGenerationInfoByIndex(g);
              time_t deletionTime = gInfo->mDeletionTime;

              if (deletionTime == 0 || (currentTime + 120) < deletionTime)
              {
                T::GeometryInfoList geometryInfoList;
                mGeometryInfoList.getGeometryInfoListByGenerationId(gInfo->mGenerationId,geometryInfoList);

                uint geomLen = geometryInfoList.getLength();

                if (geomLen == 0)
                {
                  // It seems that there are no geometry status information available. Let's generate it.

                  std::set<T::GeometryId> geometryIdList;
                  contentServer->getContentGeometryIdListByGenerationId(0,gInfo->mGenerationId,geometryIdList);
                  if (geometryIdList.size() > 0)
                  {
                    for (auto gi = geometryIdList.begin(); gi != geometryIdList.end(); ++gi)
                    {
                      T::GeometryInfo *geom = new T::GeometryInfo();
                      geom->mProducerId = gInfo->mProducerId;
                      geom->mGenerationId = gInfo->mGenerationId;
                      geom->mGeometryId = *gi;
                      geom->mModificationTime = gInfo->mModificationTime;
                      geom->mDeletionTime = gInfo->mDeletionTime;
                      geom->mStatus = gInfo->mStatus;
                      tmpGeometryInfoList.addGeometryInfo(geom);
                    }
                  }

                  tmpGeometryInfoList.getGeometryInfoListByGenerationId(gInfo->mGenerationId,geometryInfoList);
                  geomLen = geometryInfoList.getLength();
                }

                for (uint gg = 0; gg < geomLen; gg++)
                {
                  T::GeometryInfo *geom = geometryInfoList.getGeometryInfoByIndex(gg);
                  if (geom != nullptr &&  geom->mStatus == T::GeometryInfo::Status::Ready  &&  (geometryId == geom->mGeometryId  ||  geometryId == 0))
                  {
                    std::string key = gInfo->mAnalysisTime + ":" + std::to_string(geom->mGeometryId);
                    auto p = counterList.find(key);
                    if (p != counterList.end())
                      p->second.push_back(std::pair<uint,int>(geom->mGenerationId,geom->mGeometryId));
                    else
                    {
                      std::vector<std::pair<uint,int>> cntList;
                      cntList.push_back(std::pair<uint,int>(geom->mGenerationId,geom->mGeometryId));
                      counterList.insert(std::pair<std::string,std::vector<std::pair<uint,int>>>(key,cntList));
                    }
                  }
                }
              }
            }
          }
          else
          {
            //printf("PRODUCER [%s] NOT FOUND\n",(*rec)[t].c_str());
            available = false;
          }
        }

        tmpGeometryInfoList.sort(T::GeometryInfo::ComparisonMethod::generationId);


        for (auto it = counterList.begin(); it != counterList.end(); ++it)
        {
          time_t modTime = 0;
          if (it->second.size() == (sz-1))
          {
            std::ostringstream output1;
            std::ostringstream output2;

            std::set<std::string> contentTimeList;
            //printf("CONTENT [%s] [%u][%d]\n",it->first.c_str(),it->second[0].first,it->second[0].second);
            contentServer->getContentTimeListByGenerationAndGeometryId(0,it->second[0].first,it->second[0].second,contentTimeList);

            uint slen = contentTimeList.size();
            if (slen > 0)
            {
              std::set<std::string> paramList1;
              std::set<std::string> paramList2;

              for (auto g = it->second.begin(); g != it->second.end(); ++g)
              {
                //printf(" --- %u %d\n",g->first,g->second);
                //T::GenerationInfo *gInfo = mGenerationInfoList.getGenerationInfoById(g->first);
                T::GeometryInfo *geom = mGeometryInfoList.getGeometryInfoById(g->first,g->second,0);
                if (!geom)
                  geom = tmpGeometryInfoList.getGeometryInfoById(g->first,g->second,0);

                if (geom && geom->mModificationTime > modTime)
                {
                  modTime = geom->mModificationTime;
                }

                if (geom &&  mParameterMappingDefinitions)
                {
                  T::ProducerInfo *pInfo = mProducerInfoList.getProducerInfoById(geom->mProducerId);

                  if (pInfo)
                  {
                    AutoReadLock lock(&mParameterMappingDefinitions_modificationLock);
                    std::set<std::string> paramKeyList;
                    contentServer->getContentParamKeyListByGenerationAndGeometryId(0,geom->mGenerationId,geom->mGeometryId,T::ParamKeyTypeValue::FMI_NAME,paramKeyList);

                    for (auto pk = paramKeyList.begin(); pk != paramKeyList.end(); ++pk)
                    {
                      QueryServer::ParameterMapping_vec mappings;
                      for (auto mf = mParameterMappingDefinitions->begin(); mf != mParameterMappingDefinitions->end(); ++mf)
                      {
                        mf->getMappingsByParamKey(pInfo->mName,T::ParamKeyTypeValue::FMI_NAME,*pk,-1,-1,-1,mappings);
                      }

                      if (mappings.size() > 0)
                      {
                        for (auto pm = mappings.begin(); pm != mappings.end(); ++pm)
                        {
                          if (strcasecmp(pk->c_str(),pm->mParameterName.c_str()) != 0)
                          {
                            std::string name = (*rec)[0] + ";" + pm->mParameterName;
                            std::string alias;
                            if (mProducerMappingDefinitions.getAlias(name,alias))
                            {
                              paramList1.insert(*pk);
                              paramList2.insert(pm->mParameterName);
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }

              for (auto pp = paramList1.begin(); pp != paramList1.end(); ++pp)
              {
                if (pp != paramList1.begin())
                  output1 << ",";

                output1 << *pp;
              }

              for (auto pp = paramList2.begin(); pp != paramList2.end(); ++pp)
              {
                if (pp != paramList2.begin())
                  output2 << ",";

                output2 << *pp;
              }

              auto first = contentTimeList.begin();
              auto last = contentTimeList.rbegin();
              std::vector<std::string> ss;
              splitString(it->first,':',ss);

              // Producer name
              resultTable->set(0, row, (*rec)[0]);

              // Geometry id
              resultTable->set(1, row, ss[1]);

              // Timesteps
              resultTable->set(2, row, std::to_string(slen));

              if (!timeFormat.empty()  &&  strcasecmp(timeFormat.c_str(),"iso") != 0  && timeFormatter)
              {
                // Analysis time
                Fmi::DateTime aTime = toTimeStamp(ss[0]);
                resultTable->set(3, row, timeFormatter->format(aTime));

                // Min time
                Fmi::DateTime fTime = toTimeStamp(*first);
                resultTable->set(4, row, timeFormatter->format(fTime));

                // Max time
                Fmi::DateTime lTime = toTimeStamp(*last);
                resultTable->set(5, row, timeFormatter->format(lTime));

                // Modification time
                Fmi::DateTime mTime = toTimeStamp(utcTimeFromTimeT(modTime));
                resultTable->set(6, row, timeFormatter->format(mTime));
              }
              else
              {
                // Analysis time
                resultTable->set(3, row, ss[0]);

                // Min time
                resultTable->set(4, row, *first);

                // Max time
                resultTable->set(5, row, *last);

                // Modification time
                resultTable->set(6, row, utcTimeFromTimeT(modTime));
              }

              // FMI Parameters
              resultTable->set(7, row, output1.str());

              // Parameter alias names
              resultTable->set(8, row, output2.str());

              row++;
            }
          }
        }
      }
    }

    return resultTable;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}




ContentTable Engine::getParameterInfo(std::optional<std::string> producer) const
{
  FUNCTION_TRACE
  try
  {
    Spine::TableFormatter::Names headers
    { "#", "Producer", "FmiParameterName", "FmiParameterId", "NewbaseParameterName", "NewbaseParameterId", "Unit", "Description" };
    std::unique_ptr<Spine::Table> resultTable(new Spine::Table);
    resultTable->setNames(headers);
    resultTable->setTitle("Grid parameters");

    AutoReadLock lock(&mParameterMappingDefinitions_modificationLock);

    if (!mParameterTable->empty())
    {
      auto startRow = mParameterTable->minj();
      auto endRow = mParameterTable->maxj();
      auto endCol = mParameterTable->maxi();

      uint row = 0;
      if (producer)
      {
        for (std::size_t y = startRow; y <= endRow; y++)
        {
          auto p = mParameterTable->get(1, y);
          if (strcasecmp(p.c_str(), producer->c_str()) == 0)
          {
            resultTable->set(0, row, std::to_string(row + 1));
            for (std::size_t x = 1; x <= endCol; x++)
            {
              resultTable->set(x, row, mParameterTable->get(x, y));
            }
            row++;
          }
        }
      }
      else
      {
        for (std::size_t y = startRow; y <= endRow; y++)
        {
          for (std::size_t x = 0; x <= endCol; x++)
          {
            resultTable->set(x, row, mParameterTable->get(x, y));
          }
          row++;
        }
      }
    }

    return resultTable;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}



std::list<MetaData> Engine::getEngineMetadata(const char *producerName) const
{
  FUNCTION_TRACE
  try
  {
    std::list<MetaData> metaDataList;
    if (!mEnabled)
      return metaDataList;

    updateProducerAndGenerationList();

    T::GeometryInfoList tmpGeometryInfoList;

    ContentServer_sptr contentServer = getContentServer_sptr();
    time_t currentTime = time(nullptr);
    AutoReadLock lock(&mProducerInfoList_modificationLock);

    uint len = mProducerInfoList.getLength();
    for (uint t = 0; t < len; t++)
    {
      T::ProducerInfo* pInfo = mProducerInfoList.getProducerInfoByIndex(t);

      if (!producerName || producerName[0] == '\0' || strcasecmp(producerName, pInfo->mName.c_str()) == 0)
      {
        std::set<T::ParamLevelId> levelIdList;
        getProducerLevelIdList(pInfo->mProducerId,levelIdList);

        T::GenerationInfoList generationList;
        mGenerationInfoList.getGenerationInfoListByProducerId(pInfo->mProducerId, generationList);

        uint glen = generationList.getLength();
        if (glen > 0  && levelIdList.size() > 0)
        {
          generationList.sort(T::GenerationInfo::ComparisonMethod::analysisTime);

          uint glen = generationList.getLength();
          for (uint g=0; g<glen; g++)
          {
            T::GenerationInfo *gInfo = generationList.getGenerationInfoByIndex(g);
            time_t deletionTime = gInfo->mDeletionTime;

            if (deletionTime == 0 || (currentTime + 120) < deletionTime)
            {
              T::GeometryInfoList geometryInfoList;
              mGeometryInfoList.getGeometryInfoListByGenerationId(gInfo->mGenerationId,geometryInfoList);

              uint geomLen = geometryInfoList.getLength();
              if (geomLen == 0)
              {
                std::set<T::GeometryId> geometryIdList;
                contentServer->getContentGeometryIdListByGenerationId(0,gInfo->mGenerationId,geometryIdList);
                if (geometryIdList.size() > 0)
                {
                  for (auto gi = geometryIdList.begin(); gi != geometryIdList.end(); ++gi)
                  {
                    T::GeometryInfo *geom = new T::GeometryInfo();
                    geom->mProducerId = gInfo->mProducerId;
                    geom->mGenerationId = gInfo->mGenerationId;
                    geom->mGeometryId = *gi;
                    geom->mModificationTime = gInfo->mModificationTime;
                    geom->mDeletionTime = gInfo->mDeletionTime;
                    geom->mStatus = gInfo->mStatus;
                    geometryInfoList.addGeometryInfo(geom);
                  }
                }
                geomLen = geometryInfoList.getLength();
              }

              for (uint gg = 0; gg < geomLen; gg++)
              {
                T::GeometryInfo *geom = geometryInfoList.getGeometryInfoByIndex(gg);
                if (geom != nullptr &&  geom->mStatus == T::GeometryInfo::Status::Ready)
                {
                  for (auto levelId = levelIdList.begin(); levelId != levelIdList.end(); ++levelId)
                  {
                    MetaData metaData;
                    contentServer->getContentTimeListByGenerationGeometryAndLevelId(0,geom->mGenerationId,geom->mGeometryId,*levelId,metaData.times);
                    uint slen = metaData.times.size();
                    if (slen > 0)
                    {
                      std::set<std::string> paramKeyList;
                      contentServer->getContentParamKeyListByGenerationGeometryAndLevelId(0,geom->mGenerationId,geom->mGeometryId,*levelId,T::ParamKeyTypeValue::FMI_NAME,paramKeyList);
                      contentServer->getContentLevelListByGenerationGeometryAndLevelId(0,geom->mGenerationId,geom->mGeometryId,*levelId,metaData.levels);

                      metaData.producerId = pInfo->mProducerId;
                      metaData.producerName = pInfo->mName;
                      metaData.producerDescription = pInfo->mDescription;
                      metaData.generationId = gInfo->mGenerationId;
                      metaData.analysisTime = gInfo->mAnalysisTime;
                      metaData.geometryId = geom->mGeometryId;


                      auto g = Identification::gridDef.getGrib2DefinitionByGeometryId(geom->mGeometryId);
                      if (g)
                      {
                        metaData.xNumber = g->getGridColumnCount();
                        metaData.yNumber = g->getGridRowCount();
                        g->getGridLatLonArea(metaData.latlon_topLeft,metaData.latlon_topRight,metaData.latlon_bottomLeft,metaData.latlon_bottomRight);
                        metaData.projectionId = g->getGridProjection();
                        metaData.projectionName = T::get_gridProjectionString(metaData.projectionId);

                        metaData.wkt = g->getWKT();
                        metaData.proj4 = g->getProj4();
                      }

                      metaData.levelId = *levelId;

                      Identification::LevelDef levelDef;
                      if (Identification::gridDef.getFmiLevelDef(*levelId,levelDef))
                      {
                        metaData.levelName = levelDef.mName;
                        metaData.levelDescription = levelDef.mDescription;
                      }


                      //std::cout << metaData.producerName << ":" << metaData.analysisTime << ":" << metaData.geometryId << ":" << metaData.levelId << " Levels : " << metaData.levels.size() <<  " Times : " << metaData.times.size() << " Params : " << paramKeyList.size() << "\n";

                      metaData.parameters.reserve(paramKeyList.size());
                      for (auto pk = paramKeyList.begin(); pk != paramKeyList.end(); ++pk)
                      {
                        Identification::FmiParameterDef paramDef;
                        if (Identification::gridDef.getFmiParameterDefByName(pk->c_str(), paramDef))
                        {
                          Parameter param;
                          param.parameterId = paramDef.mFmiParameterId;
                          param.parameterName = paramDef.mParameterName;
                          param.parameterUnits = paramDef.mParameterUnits;
                          param.parameterDescription = paramDef.mParameterDescription;

                          metaData.parameters.push_back(param);
                        }
                      }
                      metaDataList.push_back(metaData);
                    }
                  }
                }
              }
            }
          }
        }
      }
    }


    return metaDataList;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}





void Engine::getCacheStats(Fmi::Cache::CacheStatistics& statistics) const
{
  try
  {
    if (!mEnabled)
      return;

    statistics.insert(std::make_pair("Grid::Query_cache", mQueryCache_stats));

    auto cs = getContentServer_sptr();
    if (cs)
      cs->getCacheStats(statistics);


    GRID::valueCache.getCacheStats(statistics);

    auto ds = getDataServer_sptr();
    if (ds)
      ds->getCacheStats(statistics);

    auto qs =getQueryServer_sptr();
    if (qs)
      qs->getCacheStats(statistics);
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}





Fmi::Cache::CacheStatistics Engine::getCacheStats() const
{
  try
  {
    Fmi::Cache::CacheStatistics stat;
    getCacheStats(stat);
    return stat;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}




void Engine::getStateAttributes(std::shared_ptr<T::AttributeNode> parent)
{
  try
  {
    if (!mEnabled)
      return;

    auto configuration = parent->addAttribute("Configuration");
    configuration->addAttribute("Configuration file",mConfigurationFile_name);

    auto mm = parent->addAttribute("Memory mapper");
    memoryMapper.getStateAttributes(mm);

    auto modules = parent->addAttribute("Modules");

    // ### Content Server

    auto contentServer = modules->addAttribute("Content Server");
    auto contentSource = contentServer->addAttribute("Content Source");
    mContentServer->getStateAttributes(contentSource);

    if (mContentServerCache)
    {
      auto contentCache = contentServer->addAttribute("Content Cache");
      mContentServerCache->getStateAttributes(contentCache);
    }

    // ### Data Server

    auto dataServer = parent->addAttribute("Data Server");
    mDataServer->getStateAttributes(dataServer);

    // ### Query Server

    auto queryServer = parent->addAttribute("Query Server");
    mQueryServer->getStateAttributes(queryServer);
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}



T::ParamLevelId Engine::getFmiParameterLevelId(uint producerId, int level) const
{
  FUNCTION_TRACE
  try
  {
    if (!mEnabled)
      return 0;

    AutoReadLock lock(&mProducerInfoList_modificationLock);

    uint len = mLevelInfoList.getLength();
    for (uint t = 0; t < len; t++)
    {
      T::LevelInfo* levelInfo = mLevelInfoList.getLevelInfoByIndex(t);
      if (levelInfo != nullptr && levelInfo->mProducerId == producerId)
      {
        if (levelInfo->mParameterLevel == level)
          return levelInfo->mFmiParameterLevelId;
      }
    }
    return 0;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}

void Engine::getProducerList(string_vec& producerList) const
{
  FUNCTION_TRACE
  try
  {
    if (!mEnabled)
      return;

    mQueryServer->getProducerList(0, producerList);
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}

bool Engine::getProducerInfoByName(const std::string& name, T::ProducerInfo& producerInfo) const
{
  FUNCTION_TRACE
  try
  {
    if (!mEnabled)
      return false;

    AutoReadLock lock(&mProducerInfoList_modificationLock);
    return mProducerInfoList.getProducerInfoByName(name, producerInfo);
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}

bool Engine::getProducerInfoById(uint producerId, T::ProducerInfo& producerInfo) const
{
  FUNCTION_TRACE
  try
  {
    if (!mEnabled)
      return false;

    AutoReadLock lock(&mProducerInfoList_modificationLock);
    return mProducerInfoList.getProducerInfoById(producerId, producerInfo);
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}

bool Engine::getGenerationInfoById(uint generationId, T::GenerationInfo& generationInfo)
{
  FUNCTION_TRACE
  try
  {
    if (!mEnabled)
      return false;

    AutoReadLock lock(&mProducerInfoList_modificationLock);
    return mGenerationInfoList.getGenerationInfoById(generationId, generationInfo);
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}

void Engine::getProducerParameterLevelList(const std::string& producerName, T::ParamLevelId fmiParamLevelId, double multiplier, std::set<double>& levels) const
{
  FUNCTION_TRACE
  try
  {
    if (!mEnabled)
      return;

    AutoReadLock lock(&mProducerInfoList_modificationLock);

    std::vector < std::string > nameList;
    getProducerNameList(producerName, nameList);

    for (auto pname = nameList.begin(); pname != nameList.end(); ++pname)
    {
      T::ProducerInfo *producerInfo = mProducerInfoList.getProducerInfoByName(*pname);
      if (producerInfo != nullptr)
      {
        std::string fmiParameterName;
        uint len = mLevelInfoList.getLength();
        for (uint t = 0; t < len; t++)
        {
          T::LevelInfo* levelInfo = mLevelInfoList.getLevelInfoByIndex(t);
          if (levelInfo != nullptr && levelInfo->mProducerId == producerInfo->mProducerId && levelInfo->mFmiParameterLevelId == fmiParamLevelId
              && (fmiParameterName.empty() || levelInfo->mFmiParameterName == fmiParameterName))
          {
            fmiParameterName = levelInfo->mFmiParameterName;
            levels.insert(levelInfo->mParameterLevel * multiplier);
          }
        }

        if (levels.size() > 0)
          return;
      }
    }
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}

void Engine::getProducerParameterLevelIdList(const std::string& producerName, std::set<T::ParamLevelId>& levelIdList) const
{
  FUNCTION_TRACE
  try
  {
    if (!mEnabled)
      return;

    AutoReadLock lock(&mProducerInfoList_modificationLock);

    std::vector < std::string > nameList;
    getProducerNameList(producerName, nameList);

    for (auto pname = nameList.begin(); pname != nameList.end(); ++pname)
    {
      T::ProducerInfo *producerInfo = mProducerInfoList.getProducerInfoByName(*pname);
      if (producerInfo != nullptr)
      {
        std::string fmiParameterName;
        uint len = mLevelInfoList.getLength();
        for (uint t = 0; t < len; t++)
        {
          T::LevelInfo* levelInfo = mLevelInfoList.getLevelInfoByIndex(t);
          if (levelInfo != nullptr && levelInfo->mProducerId == producerInfo->mProducerId)
          {
            // if (levelIdList.find(levelInfo->mFmiParameterLevelId) == levelIdList.end())
            levelIdList.insert(levelInfo->mFmiParameterLevelId);
          }
        }

        if (levelIdList.size() > 0)
          return;
      }
    }
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}




void Engine::getProducerLevelIdList(uint producerId, std::set<T::ParamLevelId>& levelIdList) const
{
  FUNCTION_TRACE
  try
  {
    if (!mEnabled)
      return;

    // AutoReadLock lock(&mProducerInfoList_modificationLock);

    uint len = mLevelInfoList.getLength();
    for (uint t = 0; t < len; t++)
    {
      T::LevelInfo* levelInfo = mLevelInfoList.getLevelInfoByIndex(t);
      if (levelInfo != nullptr && levelInfo->mProducerId == producerId)
      {
        levelIdList.insert(levelInfo->mFmiParameterLevelId);
      }
    }
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}




void Engine::loadUnitConversionFile()
{
  FUNCTION_TRACE
  try
  {
    if (mShutdownRequested)
      return;

    FILE* file = fopen(mUnitConversionFile.c_str(), "re");
    if (file == nullptr)
    {
      Fmi::Exception exception(BCP, "Cannot open the unit conversion file!");
      exception.addParameter("Filename", mUnitConversionFile);
      throw exception;
    }

    mUnitConversions.clear();

    char st[1000];

    while (!feof(file))
    {
      if (fgets(st, 1000, file) != nullptr && st[0] != '#')
      {
        bool ind = false;
        char* field[100];
        uint c = 1;
        field[0] = st;
        char* p = st;
        while (*p != '\0' && c < 100)
        {
          if (*p == '"')
            ind = !ind;

          if ((*p == ';' || *p == '\n') && !ind)
          {
            *p = '\0';
            p++;
            field[c] = p;
            c++;
          }
          else
          {
            p++;
          }
        }
        c--;

        if (c >= 4)
        {
          QueryServer::UnitConversion rec;

          rec.mSourceUnit = field[0];
          rec.mTargetUnit = field[1];
          rec.mConversionFunction = field[2];
          rec.mReverseConversionFunction = field[3];

          //rec.print(std::cout,0,0);

          mUnitConversions.push_back(rec);
        }
      }
    }
    fclose(file);
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}




void Engine::loadMappings(QueryServer::ParamMappingFile_vec& parameterMappings)
{
  FUNCTION_TRACE
  try
  {
    if (!mEnabled)
      return;

    if (!mUnitConversionFile.empty())
      loadUnitConversionFile();

    for (auto it = mParameterMappingDefinitions_filenames.begin(); it != mParameterMappingDefinitions_filenames.end(); ++it)
    {
      QueryServer::ParameterMappingFile mapping(*it);
      parameterMappings.emplace_back(mapping);
    }

    for (auto it = parameterMappings.begin(); it != parameterMappings.end(); ++it)
    {
      // Loading parameter mappings if the mapping file exists and it is not empty.
      if (getFileSize(it->getFilename().c_str()) > 0)
      {
        it->init();

        for (auto itm = mParameterMappingAliasDefinitions_filenames.begin(); itm != mParameterMappingAliasDefinitions_filenames.end(); ++itm)
        {
          if (*itm == it->getFilename())
          {
            QueryServer::ParameterMapping_vec aliasMappings;
            it->getAliasMappings(aliasMappings,mUnitConversions);

            QueryServer::ParameterMappingFile mapping;
            mapping.setParameterMappings(aliasMappings);
            mParameterAliasMappings.emplace_back(mapping);
          }
        }
      }
    }
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}





void Engine::getAnalysisTimes(std::vector<std::vector<std::string>>& table) const
{
  FUNCTION_TRACE
  try
  {
    if (!mEnabled)
      return;

    if (mProducerStatusFile.empty())
      return;

    table.clear();
    ContentServer_sptr contentServer = getContentServer_sptr();
    time_t currentTime = time(nullptr);

    AutoReadLock lock(&mProducerInfoList_modificationLock);
    uint plen = mProducerInfoList.getLength();
    for (uint p=0; p<plen; p++)
    {
      T::ProducerInfo *pInfo = mProducerInfoList.getProducerInfoByIndex(p);
      T::GenerationInfoList generationInfoList;
      mGenerationInfoList.getGenerationInfoListByProducerId(pInfo->mProducerId,generationInfoList);
      uint len = generationInfoList.getLength();
      for (uint g = 0; g < len; g++)
      {
        T::GenerationInfo *gInfo = generationInfoList.getGenerationInfoByIndex(g);
         time_t deletionTime = gInfo->mDeletionTime;

        if (gInfo->mStatus == T::GenerationInfo::Status::Ready)
        {
          if (deletionTime == 0 || (currentTime + 120) < deletionTime)
          {
            std::set<std::string> contentTimeList;
            contentServer->getContentTimeListByGenerationId(0,gInfo->mGenerationId,contentTimeList);

            uint slen = contentTimeList.size();
            if (slen > 0)
            {
              auto first = contentTimeList.begin();
              auto last = contentTimeList.rbegin();
              //printf("%s:%s:%s:%s:%u\n",pInfo->mName.c_str(),gInfo->mAnalysisTime.c_str(),first->c_str(),last->c_str(),slen);

              std::vector<std::string> vec;
              vec.push_back(pInfo->mName);
              vec.push_back(gInfo->mAnalysisTime);
              vec.push_back(*first);
              vec.push_back(*last);
              vec.push_back(std::to_string(slen));

              table.push_back(vec);
            }
          }
        }
      }
    }
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}


void Engine::getExtAnalysisTimes(std::vector<std::vector<std::string>>& table) const
{
  FUNCTION_TRACE
  try
  {
    //printf("\n\n**************** ANALYSIS TIMES %s *****************************\n",mProducerStatusFile.c_str());

    if (!mEnabled)
      return;

    if (mProducerStatusFile.empty())
      return;

    table.clear();
    ContentServer_sptr contentServer = getContentServer_sptr();

    std::vector<std::vector<std::string>> records;
    readCsvFile(mProducerStatusFile.c_str(),records);

    time_t currentTime = time(nullptr);

    //printf("RECORDS %ld\n",records.size());
    for (auto rec = records.begin(); rec != records.end(); ++rec)
    {
      uint sz = rec->size();
      bool available = true;
      std::map<std::string,uint> counterList;
      std::map<std::string,uint> generationList;
      for (uint t=1; t<sz && available; t++)
      {
        AutoReadLock lock(&mProducerInfoList_modificationLock);
        T::ProducerInfo *pInfo = mProducerInfoList.getProducerInfoByName((*rec)[t]);
        if (pInfo)
        {
          //printf("PRODUCER %s\n",pInfo->mName.c_str());
          T::GenerationInfoList generationInfoList;
          mGenerationInfoList.getGenerationInfoListByProducerId(pInfo->mProducerId,generationInfoList);
          //printf("GENERATIONS %u/%u\n",generationInfoList.getLength(),mGenerationInfoList.getLength());

          uint len = generationInfoList.getLength();
          for (uint g = 0; g < len; g++)
          {
            T::GenerationInfo *gInfo = generationInfoList.getGenerationInfoByIndex(g);
            time_t deletionTime = gInfo->mDeletionTime;

            if (gInfo->mStatus == T::GenerationInfo::Status::Ready)
            {
              if (deletionTime == 0 || (currentTime + 120) < deletionTime)
              {
                auto p = counterList.find(gInfo->mAnalysisTime);
                if (p != counterList.end())
                  p->second++;
                else
                {
                  counterList.insert(std::pair<std::string,uint>(gInfo->mAnalysisTime,1));
                  generationList.insert(std::pair<std::string,uint>(gInfo->mAnalysisTime,gInfo->mGenerationId));
                }
              }
            }
          }
        }
        else
        {
          //printf("PRODUCER [%s] NOT FOUND\n",(*rec)[t].c_str());
          available = false;
        }
      }

      for (auto it = counterList.begin(); it != counterList.end(); ++it)
      {
        if (it->second == (sz-1))
        {
          auto g = generationList.find(it->first);
          if (g != generationList.end())
          {
            std::set<std::string> contentTimeList;
            contentServer->getContentTimeListByGenerationId(0,g->second,contentTimeList);

            uint slen = contentTimeList.size();
            if (slen > 0)
            {
              auto first = contentTimeList.begin();
              auto last = contentTimeList.rbegin();
              //printf("%s:%s:%s:%s:%u\n",(*rec)[0].c_str(),it->first.c_str(),first->c_str(),last->c_str(),slen);

              std::vector<std::string> vec;
              vec.push_back((*rec)[0]);
              vec.push_back(it->first);
              vec.push_back(*first);
              vec.push_back(*last);
              vec.push_back(std::to_string(slen));

              table.push_back(vec);
            }
          }
        }
      }
    }
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}



void Engine::clearMappings()
{
  FUNCTION_TRACE
  try
  {
    if (!mEnabled)
      return;

    QueryServer::ParamMappingFile_vec parameterMappings;

    if (!mParameterMappingDefinitions_autoFile_fmi.empty())
    {
      FILE* file = openMappingFile(mParameterMappingDefinitions_autoFile_fmi);
      if (file != nullptr)
        fclose(file);
    }

    if (!mParameterMappingDefinitions_autoFile_newbase.empty())
    {
      FILE* file = openMappingFile(mParameterMappingDefinitions_autoFile_newbase);
      if (file != nullptr)
        fclose(file);
    }

    if (!mParameterMappingDefinitions_autoFile_netCdf.empty())
    {
      FILE* file = openMappingFile(mParameterMappingDefinitions_autoFile_netCdf);
      if (file != nullptr)
        fclose(file);
    }
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}

void Engine::updateMappings()
{
  FUNCTION_TRACE
  try
  {
    if (!mEnabled)
      return;

    time_t currentTime = time(nullptr);

    if ((currentTime - mParameterMappingDefinitions_updateTime) < 300)
      return;

    mParameterMappingDefinitions_updateTime = currentTime;

    QueryServer::ParamMappingFile_vec* parameterMappings = new QueryServer::ParamMappingFile_vec();

    loadMappings(*parameterMappings);

    if (parameterMappings->size() > 0)
    {
      AutoWriteLock lock(&mParameterMappingDefinitions_modificationLock);
      mParameterMappingDefinitions.reset(parameterMappings);
    }
    else
    {
      delete parameterMappings;
      return;
    }

    Spine::Table* paramTable = new Spine::Table();
    if (!mParameterMappingDefinitions_autoFile_fmi.empty())
    {
      updateMappings(T::ParamKeyTypeValue::FMI_NAME, mParameterMappingDefinitions_autoFileKeyType, mParameterMappingDefinitions_autoFile_fmi, *parameterMappings, *paramTable);
    }

    if (!mParameterMappingDefinitions_autoFile_newbase.empty())
    {
      updateMappings(T::ParamKeyTypeValue::NEWBASE_NAME, mParameterMappingDefinitions_autoFileKeyType, mParameterMappingDefinitions_autoFile_newbase, *parameterMappings,
          *paramTable);
    }

    if (!mParameterMappingDefinitions_autoFile_netCdf.empty())
    {
      updateMappings(T::ParamKeyTypeValue::NETCDF_NAME, mParameterMappingDefinitions_autoFileKeyType, mParameterMappingDefinitions_autoFile_netCdf, *parameterMappings,
          *paramTable);
    }

    AutoWriteLock lock(&mParameterMappingDefinitions_modificationLock);
    mParameterTable.reset(paramTable);
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}

FILE* Engine::openMappingFile(const std::string& mappingFile)
{
  FUNCTION_TRACE
  try
  {
    if (!mEnabled)
      return nullptr;

    FILE* file = fopen(mappingFile.c_str(), "we");
    if (file == nullptr)
    {
      Fmi::Exception exception(BCP, "Cannot open a mapping file for writing!");
      exception.addParameter("Filaname", mappingFile);
      throw exception;
    }

    fprintf(file, "# This file is automatically generated by the grid engine. The file contains\n");
    fprintf(file, "# mappings for the parameters found from the content server, which do not have\n");
    fprintf(file, "# mappings already defined. The point is that the query server cannot find \n");
    fprintf(file, "# requested parameters without mappings. On the other hand, the order of the mappings\n");
    fprintf(file, "# is also the search order of the parameters that do not contain complete search \n");
    fprintf(file, "# information (parameterIdType,levelIdType,levelId,level,etc.)\n");
    fprintf(file, "# \n");
    fprintf(file, "# If you want to change some of the mappings or their order, then you should move\n");
    fprintf(file, "# them to a permanent mapping file (which is not automatically overridden.\n");
    fprintf(file, "# \n");
    fprintf(file, "# FIELDS:\n");
    fprintf(file, "#  1) Producer name\n");
    fprintf(file, "#  2) Mapping name\n");
    fprintf(file, "#  3) Parameter id type:\n");
    fprintf(file, "#         1 = FMI_ID\n");
    fprintf(file, "#         2 = FMI_NAME\n");
    fprintf(file, "#         3 = GRIB_ID\n");
    fprintf(file, "#         4 = NEWBASE_ID\n");
    fprintf(file, "#         5 = NEWBASE_NAME\n");
    fprintf(file, "#         6 = NETCDF_NAME\n");
    fprintf(file, "#  4) Parameter id / name\n");
    fprintf(file, "#  5) Geometry id\n");
    fprintf(file, "#  6) Parameter level id type:\n");
    fprintf(file, "#         1 = FMI\n");
    fprintf(file, "#         2 = GRIB1\n");
    fprintf(file, "#         3 = GRIB2\n");
    fprintf(file, "#  7) Level id\n");
    fprintf(file, "#         FMI level identifiers:\n");
    fprintf(file, "#            1 Gound or water surface\n");
    fprintf(file, "#            2 Pressure level\n");
    fprintf(file, "#            3 Hybrid level\n");
    fprintf(file, "#            4 Altitude\n");
    fprintf(file, "#            5 Top of atmosphere\n");
    fprintf(file, "#            6 Height above ground in meters\n");
    fprintf(file, "#            7 Mean sea level\n");
    fprintf(file, "#            8 Entire atmosphere\n");
    fprintf(file, "#            9 Depth below land surface\n");
    fprintf(file, "#            10 Depth below some surface\n");
    fprintf(file, "#            11 Level at specified pressure difference from ground to level\n");
    fprintf(file, "#            12 Max equivalent potential temperature level\n");
    fprintf(file, "#            13 Layer between two metric heights above ground\n");
    fprintf(file, "#            14 Layer between two depths below land surface\n");
    fprintf(file, "#            15 Isothermal level, temperature in 1/100 K\n");
    fprintf(file, "#  8) Level\n");
    fprintf(file, "#  9) Area interpolation method\n");
    fprintf(file, "#         0 = None\n");
    fprintf(file, "#         1 = Linear\n");
    fprintf(file, "#         2 = Nearest\n");
    fprintf(file, "#         3 = Min\n");
    fprintf(file, "#         4 = Max\n");
    fprintf(file, "#         9 = Landscape\n");
    fprintf(file, "#         10 = Forbidden\n");
    fprintf(file, "#         500..999 = List\n");
    fprintf(file, "#         1000..65535 = External (interpolated by an external function)\n");
    fprintf(file, "# 10) Time interpolation method\n");
    fprintf(file, "#         0 = None\n");
    fprintf(file, "#         1 = Linear\n");
    fprintf(file, "#         2 = Nearest\n");
    fprintf(file, "#         3 = Min\n");
    fprintf(file, "#         4 = Max\n");
    fprintf(file, "#         6 = Previous\n");
    fprintf(file, "#         7 = Next\n");
    fprintf(file, "#         1000..65535 = External (interpolated by an external function)\n");
    fprintf(file, "# 11) Level interpolation method\n");
    fprintf(file, "#         0 = None\n");
    fprintf(file, "#         1 = Linear\n");
    fprintf(file, "#         2 = Nearest\n");
    fprintf(file, "#         3 = Min\n");
    fprintf(file, "#         4 = Max\n");
    fprintf(file, "#         5 = Logarithmic\n");
    fprintf(file, "#         6 = Previous\n");
    fprintf(file, "#         7 = Next\n");
    fprintf(file, "#         1000..65535 = External (interpolated by an external function)\n");
    fprintf(file, "# 12) Group flags\n");
    fprintf(file, "#         bit 0 = Climatological parameter (=> ignore year when searching)\n");
    fprintf(file, "#         bit 1 = Global parameter (=> ignore timestamp when searching, for example LandSeaMask)\n");
    fprintf(file, "# 13) Search match (Can this mapping used when searching mappings for incomplete "
        "parameters)\n");
    fprintf(file, "#         E = Enabled\n");
    fprintf(file, "#         D = Disabled\n");
    fprintf(file, "#         I = Ignore\n");
    fprintf(file, "# 14) Mapping function (enables data conversions during the mapping)\n");
    fprintf(file, "# 15) Reverse mapping function\n");
    fprintf(file, "# 16) Default precision\n");
    fprintf(file, "# \n");

    return file;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}



bool filesEqual(const char *filename1,const char *filename2)
{
  FUNCTION_TRACE
  try
  {
    if (getFileSize(filename1) != getFileSize(filename2))
      return false;

    FILE* file1 = fopen(filename1, "re");
    if (file1 == nullptr)
    {
      Fmi::Exception exception(BCP, "Cannot open a file for reading!");
      exception.addParameter("Filaname", filename1);
      throw exception;
    }

    FILE* file2 = fopen(filename2, "re");
    if (file2 == nullptr)
    {
      Fmi::Exception exception(BCP, "Cannot open a file for reading!");
      exception.addParameter("Filaname", filename2);
      throw exception;
    }

    uchar buf1[10000];
    uchar buf2[10000];

    while (!feof(file1) &&  !feof(file2))
    {
      int n1 = fread(buf1,1,10000,file1);
      int n2 = fread(buf2,1,10000,file2);

      if (n1 != n2 || memcmp(buf1,buf2,n1) != 0)
      {
        fclose(file1);
        fclose(file2);
        return false;
      }
    }
    fclose(file1);
    fclose(file2);
    return true;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    throw exception;
  }
}




void Engine::updateMappings(
    T::ParamKeyType sourceParameterKeyType,
    T::ParamKeyType targetParameterKeyType,
    const std::string& mappingFile,
    QueryServer::ParamMappingFile_vec& parameterMappings,
    Spine::Table& paramTable)
{
  FUNCTION_TRACE
  try
  {
    if (!mEnabled)
      return;

    if (Spine::Reactor::isShuttingDown())
      return;

    std::string tmpMappingFile = mappingFile + ".tmp." + std::to_string(getpid());

    ContentServer_sptr contentServer = getContentServer_sptr();

    T::SessionId sessionId = 0;
    uint numOfNewMappings = 0;
    std::unordered_set < std::string > mapList;
    std::unordered_set < std::string > searchList;

    T::ProducerInfoList producerInfoList;
    int result = contentServer->getProducerInfoList(sessionId, producerInfoList);
    if (result != 0)
    {
      std::cerr << __FILE__ << ":" << __LINE__ << ": The 'contentServer.getProducerInfoList()' service call returns an error!  Result : " << result << " : "
          << ContentServer::getResultString(result).c_str() << "\n";
      return;
    }

    FILE* file = nullptr;
    std::unordered_set < std::string > pList;
    auto row = paramTable.maxj();

    uint plen = producerInfoList.getLength();
    for (uint t = 0; t < plen; t++)
    {
      if (Spine::Reactor::isShuttingDown())
        return;

      T::ProducerInfo* producerInfo = producerInfoList.getProducerInfoByIndex(t);
      std::set < std::string > infoList;

      int result = contentServer->getProducerParameterListByProducerId(sessionId, producerInfo->mProducerId, sourceParameterKeyType, targetParameterKeyType, infoList);
      if (result == 0)
      {
        for (auto it = infoList.begin(); it != infoList.end(); ++it)
        {
          if (Spine::Reactor::isShuttingDown())
            return;

          std::vector < std::string > pl;
          splitString(it->c_str(), ';', pl);
          if (pl.size() >= 8)
          {
            QueryServer::ParameterMapping m;
            m.mProducerName = pl[0];
            m.mParameterName = pl[1];
            m.mParameterKeyType = toUInt8(pl[2].c_str());
            m.mParameterKey = pl[3];
            m.mGeometryId = toInt32(pl[4].c_str());
            //m.mParameterLevelIdType = toUInt8(pl[5].c_str());
            m.mParameterLevelId = toInt8(pl[6].c_str());
            if (mParameterMapping_simplifiedLevelIdSet.find(m.mParameterLevelId) != mParameterMapping_simplifiedLevelIdSet.end())
              m.mParameterLevel = 0;
            else
              m.mParameterLevel = toInt32(pl[7].c_str());

            if (sourceParameterKeyType == T::ParamKeyTypeValue::FMI_NAME)
            {
              char tmp[200];
              sprintf(tmp, "%s;%s", pl[0].c_str(), pl[1].c_str());

              auto res = pList.insert(std::string(tmp));
              if (res.second)
              {
                paramTable.set(0, row, std::to_string(row + 1));
                paramTable.set(1, row, pl[0]);
                paramTable.set(2, row, pl[1]);

                Identification::FmiParameterDef paramDef;
                if (Identification::gridDef.getFmiParameterDefByName(pl[1], paramDef))
                {
                  paramTable.set(3, row, std::to_string(paramDef.mFmiParameterId));

                  Identification::NewbaseParameterDef nbDef;
                  Identification::gridDef.getNewbaseParameterDefByFmiId(paramDef.mFmiParameterId, nbDef);
                  paramTable.set(4, row, nbDef.mParameterName);
                  paramTable.set(5, row, std::to_string(nbDef.mNewbaseParameterId));

                  paramTable.set(6, row, paramDef.mParameterUnits);
                  paramTable.set(7, row, paramDef.mParameterDescription);
                }

                row++;
              }
            }

            char key[200];
            std::string level = pl[7];
            if (mParameterMapping_simplifiedLevelIdSet.find(m.mParameterLevelId) != mParameterMapping_simplifiedLevelIdSet.end())
              level = "*";

            sprintf(key, "%s;%s;%s;%s;%s;%s;%s;%s;", pl[0].c_str(), pl[1].c_str(), pl[2].c_str(), pl[3].c_str(), pl[4].c_str(), pl[5].c_str(), pl[6].c_str(), level.c_str());
            std::string searchKey = m.mProducerName + ":" + m.mParameterName + ":" + std::to_string(m.mGeometryId);

            if (mapList.find(std::string(key)) == mapList.end())
            {
              mapList.insert(std::string(key));

              bool found = false;
              bool searchEnabled = false;
              for (auto it = parameterMappings.begin(); it != parameterMappings.end() && !found; ++it)
              {
                if (it->getFilename() != mappingFile)
                {
                  if (it->getMapping(m) != nullptr)
                  {
                    found = true;
                  }
                  else
                  {
                    if (searchList.find(searchKey) != searchList.end())
                    {
                      searchEnabled = true;
                    }
                    else
                    {
                      QueryServer::ParameterMapping_vec vec;
                      it->getMappings(m.mProducerName, m.mParameterName, m.mGeometryId, true, vec);
                      if (vec.size() > 0)
                      {
                        searchEnabled = true;
                      }
                    }
                  }
                }
              }

              if (!found)
              {
                numOfNewMappings++;

                char s = 'D';
                if (!searchEnabled || (m.mParameterLevelId == 6 && m.mParameterLevel <= 10) || (m.mParameterLevelId == 1 && m.mParameterLevel == 0))
                {
                  if (mParameterMapping_simplifiedLevelIdSet.find(m.mParameterLevelId) == mParameterMapping_simplifiedLevelIdSet.end())
                    s = 'E';
                }

                // if (searchList.find(searchKey) == searchList.end())
                searchList.insert(searchKey);

                if (file == nullptr)
                  file = openMappingFile(tmpMappingFile);

                fprintf(file, "%s;%s;%s;%s;%s;%s;%s;%s;", pl[0].c_str(), pl[1].c_str(), pl[2].c_str(), pl[3].c_str(), pl[4].c_str(), pl[5].c_str(), pl[6].c_str(), level.c_str());

                Identification::FmiParameterDef paramDef;

                bool found = false;
                if (targetParameterKeyType == T::ParamKeyTypeValue::FMI_NAME)
                  found = Identification::gridDef.getFmiParameterDefByName(pl[3], paramDef);
                else if (targetParameterKeyType == T::ParamKeyTypeValue::FMI_ID)
                  found = Identification::gridDef.getFmiParameterDefById(toUInt32(pl[3]), paramDef);
                else if (targetParameterKeyType == T::ParamKeyTypeValue::NEWBASE_ID)
                  found = Identification::gridDef.getFmiParameterDefByNewbaseId(toUInt32(pl[3]), paramDef);
                else if (targetParameterKeyType == T::ParamKeyTypeValue::NETCDF_NAME)
                  found = Identification::gridDef.getFmiParameterDefByNetCdfName(pl[3], paramDef);

                if (found)
                {
                  if (paramDef.mAreaInterpolationMethod >= 0)
                    fprintf(file, "%d;", paramDef.mAreaInterpolationMethod);
                  else
                    fprintf(file, ";");

                  if (paramDef.mTimeInterpolationMethod >= 0)
                    fprintf(file, "%d;", paramDef.mTimeInterpolationMethod);
                  else
                    fprintf(file, ";");

                  if (paramDef.mLevelInterpolationMethod >= 0)
                    fprintf(file, "%d;", paramDef.mLevelInterpolationMethod);
                  else
                    fprintf(file, ";");

                  fprintf(file, "0;%c;", s);

                  if (sourceParameterKeyType == T::ParamKeyTypeValue::NEWBASE_ID || sourceParameterKeyType == T::ParamKeyTypeValue::NEWBASE_NAME)
                  {
                    Identification::FmiParameterId_newbase paramMapping;
                    if (Identification::gridDef.getNewbaseParameterMappingByFmiId(paramDef.mFmiParameterId, paramMapping))
                    {
                      fprintf(file, "%s;", paramMapping.mConversionFunction.c_str());
                      fprintf(file, "%s;", paramMapping.mReverseConversionFunction.c_str());
                    }
                    else
                    {
                      fprintf(file, ";;");
                    }
                  }
                  if (sourceParameterKeyType == T::ParamKeyTypeValue::NETCDF_NAME)
                  {
                    Identification::FmiParameterId_netCdf paramMapping;
                    if (Identification::gridDef.getNetCdfParameterMappingByFmiId(paramDef.mFmiParameterId, paramMapping))
                    {
                      fprintf(file, "%s;", paramMapping.mConversionFunction.c_str());
                      fprintf(file, "%s;", paramMapping.mReverseConversionFunction.c_str());
                    }
                    else
                    {
                      fprintf(file, ";;");
                    }
                  }
                  else
                  {
                    fprintf(file, ";;");
                  }

                  if (paramDef.mDefaultPrecision >= 0)
                    fprintf(file, "%d;", (int) paramDef.mDefaultPrecision);
                  else
                    fprintf(file, ";");

                  fprintf(file, "\n");
                }
                else
                {
                  fprintf(file, "1;1;1;0;D;;;;\n");
                }
              }
            }
          }
        }
      }
    }

    if (file == nullptr && numOfNewMappings == 0)
    {
      // We found all mappings from the other files. That's why we should remove them
      // from the update file.

      file = openMappingFile(tmpMappingFile);
    }

    if (file != nullptr)
      fclose(file);

    if (!filesEqual(tmpMappingFile.c_str(), mappingFile.c_str()))
      rename(tmpMappingFile.c_str(), mappingFile.c_str());
    else
      remove(tmpMappingFile.c_str());
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}

void Engine::updateProcessing()
{
  try
  {
    if (!mEnabled)
      return;

    ContentServer_sptr contentServer = getContentServer_sptr();
    while (!mShutdownRequested)
    {
      try
      {
        T::EventInfo eventInfo;
        int result = contentServer->getLastEventInfo(0, 0, eventInfo);
        if (result == ContentServer::Result::DATA_NOT_FOUND || result == ContentServer::Result::OK)
        {
          if (eventInfo.mServerTime > mContentServerStartTime)
          {
            if (mContentServerStartTime > 0)
            {
              // Content Server has been restarted. We should delete all cached information.

              {
                AutoWriteLock lock(&mProducerInfoList_modificationLock);
                mProducerInfoList.clear();
                mGenerationInfoList.clear();
                mGeometryInfoList.clear();
                mLevelInfoList.clear();
                mProducerInfoList_updateTime = 0;
                mLevelInfoList_lastUpdate = 0;
              }

              {
                AutoWriteLock lock(&mQueryCache_modificationLock);
                mQueryCache_stats.size = 0;
                mQueryCache.clear();
              }
            }
          }
          mContentServerStartTime = eventInfo.mServerTime;
        }
      }
      catch (...)
      {
      }

      try
      {
        updateMappings();
      }
      catch (...)
      {
      }

      try
      {
        updateQueryCache();
      }
      catch (...)
      {
      }

      try
      {
        updateProducerAndGenerationList();
      }
      catch (...)
      {
      }

      try
      {
        checkConfiguration();
      }
      catch (...)
      {
      }
      if (!mShutdownRequested)
	      boost::this_thread::sleep(boost::posix_time::seconds(1));
    }
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}

void Engine::updateProducerAndGenerationList() const
{
  FUNCTION_TRACE
  try
  {
    if (!mEnabled)
      return;

    ContentServer_sptr contentServer = getContentServer_sptr();

    if ((time(nullptr) - mProducerInfoList_updateTime) > 60)
    {
      mProducerInfoList_updateTime = time(nullptr);

      AutoWriteLock lock(&mProducerInfoList_modificationLock);

      mQueryServer->getProducerList(0, mProducerSearchList);

      // Producers defined in the content server
      ContentServer_sptr contentServer = getContentServer_sptr();
      contentServer->getProducerInfoList(0, mProducerInfoList);

      contentServer->getGenerationInfoList(0, mGenerationInfoList);
      mGenerationInfoList.sort(T::GenerationInfo::ComparisonMethod::generationId);

      contentServer->getGeometryInfoList(0, mGeometryInfoList);
      mGeometryInfoList.sort(T::GeometryInfo::ComparisonMethod::generationId);
    }

    if (mLevelInfoList.getLength() == 0 || (mLevelInfoList_lastUpdate + 300) < time(nullptr))
    {
      mLevelInfoList_lastUpdate = time(nullptr);

      T::LevelInfoList levelInfoList;

      if (contentServer->getLevelInfoList(0, levelInfoList) == 0)
      {
        AutoWriteLock lock(&mProducerInfoList_modificationLock);
        mLevelInfoList = levelInfoList;
      }
    }
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}

void Engine::updateQueryCache()
{
  try
  {
    if (!mEnabled)
      return;

    if (!mQueryCache_enabled)
      return;

    time_t currentTime = time(nullptr);
    if ((currentTime - mQueryCache_updateTime) < 60)
      return;

    time_t tt = getFileModificationTime(mProducerSearchList_filename.c_str());
    if (mProducerSearchList_modificationTime != tt && (tt + 3) < currentTime)
    {
      // The producer search order has changed. So we have to clear the query cache.
      mProducerSearchList_modificationTime = tt;
      AutoWriteLock lock(&mQueryCache_modificationLock);
      mQueryCache_stats.size = 0;
      mQueryCache.clear();
      return;
    }

    mQueryCache_enabled = false;

    mQueryCache_updateTime = currentTime;
    time_t lastAccess = currentTime - mQueryCache_maxAge;
    std::vector < ulonglong > deleteList;

    {
      AutoReadLock lock(&mQueryCache_modificationLock);

      for (auto it = mQueryCache.begin(); it != mQueryCache.end(); ++it)
      {
        if (it->second.lastAccessTime < lastAccess)
        {
          // The cache entry has not been accessed for awhile, so we should remove it.
          deleteList.emplace_back(it->first);
        }
        else
        {
          // If the producer information has changed then we should remove the cache entry.
          bool noMatch = false;
          for (auto prod = it->second.producerHashMap.begin(); prod != it->second.producerHashMap.end() && !noMatch; ++prod)
          {
            ulonglong producerHash = getProducerHash(prod->first);
            if (producerHash != prod->second)
              noMatch = true;
          }

          if (noMatch)
            deleteList.emplace_back(it->first);
        }
      }
    }

    if (deleteList.size() > 0)
    {
      AutoWriteLock lock(&mQueryCache_modificationLock);
      for (auto it = deleteList.begin(); it != deleteList.end(); ++it)
      {
        auto pos = mQueryCache.find(*it);
        if (pos != mQueryCache.end())
          mQueryCache.erase(pos);
      }
      mQueryCache_stats.size -= deleteList.size();
    }

    mQueryCache_enabled = true;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}

void Engine::getVerticalGrid(
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
    uint& gridHeight) const
{
  try
  {
    if (!mEnabled)
      return;

    ContentServer_sptr contentServer = getContentServer_sptr();
    DataServer_sptr dataServer = getDataServer_sptr();
    QueryServer_sptr queryServer = getQueryServer_sptr();

    std::set<double> levels1;
    std::set<double> levels2;

    T::SessionId sessionId = 0;
    T::GenerationInfoList gInfoList1;
    T::GenerationInfoList gInfoList2;

    int result = contentServer->getGenerationInfoListByProducerName(sessionId, valueProducerName, gInfoList1);
    if (result != 0)
    {
      Fmi::Exception exception(BCP, ContentServer::getResultString(result).c_str(), nullptr);
      exception.addParameter("ProducerName", valueProducerName);
      throw exception;
    }

    result = contentServer->getGenerationInfoListByProducerName(sessionId, heightProducerName, gInfoList2);
    if (result != 0)
    {
      Fmi::Exception exception(BCP, ContentServer::getResultString(result).c_str());
      exception.addParameter("ProducerName", heightProducerName);
      throw exception;
    }

    T::GenerationInfo* gInfo1 = gInfoList1.getLastGenerationInfoByAnalysisTime();
    T::GenerationInfo* gInfo2 = gInfoList2.getLastGenerationInfoByAnalysisTime();

    if (gInfo1 != nullptr && gInfo2 != nullptr && gInfo1->mAnalysisTime != gInfo2->mAnalysisTime)
    {
      if (gInfo1->mAnalysisTime < gInfo2->mAnalysisTime)
        gInfo2 = gInfoList2.getGenerationInfoByAnalysisTime(gInfo1->mAnalysisTime);
      else if (gInfo1->mAnalysisTime > gInfo2->mAnalysisTime)
        gInfo1 = gInfoList1.getGenerationInfoByAnalysisTime(gInfo2->mAnalysisTime);
    }

    if (gInfo1 != nullptr && gInfo2 != nullptr && gInfo1->mAnalysisTime != gInfo2->mAnalysisTime)
    {
      Fmi::Exception exception(BCP, "Cannot find the same analysis time for values and heights!");
      throw exception;
    }

    getProducerParameterLevelList(valueProducerName, 3, 1, levels1);
    if (levels1.size() == 0)
    {
      Fmi::Exception exception(BCP, "Cannot find valid levels for the value parameter!");
      exception.addParameter("ValueProducer", valueProducerName);
      throw exception;
    }

    getProducerParameterLevelList(heightProducerName, 3, 1, levels2);
    if (levels2.size() == 0)
    {
      Fmi::Exception exception(BCP, "Cannot find valid levels for the height parameter!");
      exception.addParameter("HeightProducer", heightProducerName);
      throw exception;
    }

    if (levels1.size() != levels2.size())
    {
      Fmi::Exception exception(BCP, "The number of value parameter levels is different than the number "
          "of height parameter levels!");
      throw exception;
    }

    auto points = getIsocirclePoints(lon1, lat1, lon2, lat2, steps);

    for (auto level = levels1.rbegin(); level != levels1.rend(); ++level)
    {
      int lev = C_INT(*level);
      char param[100];
      char* p = param;
      p += sprintf(p, "%s:%s", valueParameter.c_str(), valueProducerName.c_str());
      if (geometryId > 0)
        p += sprintf(p, ":%u:3:%u", geometryId, lev);
      else
        p += sprintf(p, "::3:%u", lev);

      std::vector < T::ParamValue > valueVec;
      std::vector < T::ParamValue > heightVec;

      std::string pa(param);
      int result1 = queryServer->getParameterValuesByPointListAndTime(sessionId, valueProducerName, pa, T::CoordinateTypeValue::LATLON_COORDINATES, points.first, utcTime,
          areaInterpolationMethod, timeInterpolationMethod, 1, valueVec);

      p = param;
      p += sprintf(p, "%s:%s", heightParameter.c_str(), heightProducerName.c_str());
      if (geometryId > 0)
        p += sprintf(p, ":%u:3:%u", geometryId, lev);
      else
        p += sprintf(p, "::3:%u", lev);

      pa = param;
      int result2 = queryServer->getParameterValuesByPointListAndTime(sessionId, heightProducerName, pa, T::CoordinateTypeValue::LATLON_COORDINATES, points.first, utcTime,
          areaInterpolationMethod, timeInterpolationMethod, 1, heightVec);

      uint sz = points.first.size();
      if (result1 == 0 && result2 == 0 && valueVec.size() == sz && heightVec.size() == sz)
      {
        for (uint t = 0; t < sz; t++)
        {
          auto dist = points.second[t];
          coordinates.emplace_back(T::Coordinate(dist, heightVec[t]));

          gridData.emplace_back(valueVec[t]);
        }
      }
    }

    gridWidth = points.first.size();
    gridHeight = levels1.size();

#if 0
    int imageWidth = 1000; // points.first.size()*mp;
    int imageHeight = 1000;// len1*mp;
    bool rotate = true;
    double mpy = (double)imageHeight / maxHeight;
    double mpx = (double)imageWidth / maxDistance;

    std::vector<float> contourLowValues;
    std::vector<float> contourHighValues;
    T::ByteData_vec contours;

    float m = minValue;
    uint ss = C_UINT((maxValue - minValue) / 2) + 1;
    for (uint t=0; t<ss; t++)
    {
      if (t == 0)
      contourLowValues.emplace_back(100.0);
      else
      contourLowValues.emplace_back(m);

      m = m + 2;
      contourHighValues.emplace_back(m);
    }

    getIsobands(gridData,&coordinates,points.first.size(),levels1.size(),contourLowValues,contourHighValues,T::AreaInterpolationMethod::Linear,0,1,contours);

    ImagePaint imagePaint(imageWidth,imageHeight,0xFFFFFFFF,false,rotate);

    int sz = imageWidth * imageHeight;
    unsigned long *image = new unsigned long[sz];
    for (int t=0; t<sz; t++)
    image[t] = 0xFF0000;

    // ### Painting contours into the image:

    if (contours.size() > 0)
    {
      uint c = 250;
      uint step = 250 / contours.size();

      uint t = 0;
      for (auto it = contours.begin(); it != contours.end(); ++it)
      {
        uint col = (c << 16) + (c << 8) + c;

        imagePaint.paintWkb(mpx,mpy,0,0,*it,col);
        c = c - step;
        t++;
      }
    }

    T::ByteData_vec contours2;
    getIsolines(gridData,&coordinates,points.first.size(),levels1.size(),contourLowValues,T::AreaInterpolationMethod::Linear,0,1,contours2);
    for (auto it = contours2.begin(); it != contours2.end(); ++it)
    {
      imagePaint.paintWkb(mpx,mpy,0,0,*it,0x000000);
    }

    imagePaint.savePngImage("/tmp/test.png");
#endif
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}

void Engine::setDem(std::shared_ptr<Fmi::DEM> dem)
{
  try
  {
    if (!mEnabled)
      return;

    mDem = dem;
    mQueryServer->setDem(dem);
    mDataServer->setDem(dem);
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}


inline bool setBit(uchar *_bits,uint _pos)
{
  uint bt = _pos / 8;
  uint bit = _pos % 8;

  uchar v = 1 << bit;

  if (_bits[bt] & v)
    return true;

  return false;
}


void Engine::setLandCover(std::shared_ptr<Fmi::LandCover> landCover)
{
  try
  {
    if (!mEnabled)
      return;

    mLandCover = landCover;
    mQueryServer->setLandCover(landCover);
    mDataServer->setLandCover(landCover);
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    throw exception;
  }
}

void Engine::startUpdateProcessing()
{
  try
  {
    if (!mEnabled)
      return;

    pthread_create(&mThread, nullptr, gridEngine_updateThread, this);
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}


ContentTable  Engine::requestGridGenerationInfo(const Spine::HTTP::Request &theRequest)
try
{
  auto producer = theRequest.getParameter("producer");
  std::string timeFormat = Spine::optional_string(theRequest.getParameter("timeformat"), "sql");
  auto producerInfo = getGenerationInfo(producer, timeFormat);
  return producerInfo;
}
catch (...)
{
  throw Fmi::Exception(BCP, "Operation failed!");
}


ContentTable  Engine::requestGridQdGenerationInfo(const Spine::HTTP::Request &theRequest)
try
{
  auto producer = theRequest.getParameter("producer");
  std::string timeFormat = Spine::optional_string(theRequest.getParameter("timeformat"), "sql");
  auto producerInfo = getExtGenerationInfo(producer, timeFormat);
  return producerInfo;
}
catch (...)
{
  throw Fmi::Exception(BCP, "Operation failed!");
}


ContentTable  Engine::requestGridParameterInfo(const Spine::HTTP::Request &theRequest)
try
{
  auto producer = theRequest.getParameter("producer");
  auto producerInfo = getParameterInfo(producer);
  return producerInfo;
}
catch (...)
{
  throw Fmi::Exception(BCP, "Operation failed!");
}


ContentTable  Engine::requestGridProducerInfo(const Spine::HTTP::Request &theRequest)
try
{
  auto producer = theRequest.getParameter("producer");
  std::string timeFormat = Spine::optional_string(theRequest.getParameter("timeformat"), "sql");
  auto producerInfo = getProducerInfo(producer, timeFormat);
  return producerInfo;
}
catch (...)
{
  throw Fmi::Exception(BCP, "Operation failed!");
}


}  // namespace Grid
}  // namespace Engine
}  // namespace SmartMet

// DYNAMIC MODULE CREATION TOOLS

extern "C" void* engine_class_creator(const char* configfile, void* /* user_data */)
{
  return new SmartMet::Engine::Grid::Engine(configfile);
}

extern "C" const char* engine_name()
{
  return "grid";
}
