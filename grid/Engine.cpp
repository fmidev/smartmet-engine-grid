#include "Engine.h"

#include <grid-content/contentServer/corba/client/ClientImplementation.h>
#include <grid-content/contentServer/http/client/ClientImplementation.h>
#include <grid-content/dataServer/corba/client/ClientImplementation.h>
#include <grid-content/dataServer/implementation/VirtualContentFactory_type1.h>
#include <grid-content/queryServer/corba/client/ClientImplementation.h>
#include <grid-files/common/CoordinateConversions.h>
#include <grid-files/common/GeneralFunctions.h>
#include <grid-files/common/GraphFunctions.h>
#include <grid-files/common/ImageFunctions.h>
#include <grid-files/common/ImagePaint.h>
#include <grid-files/common/ShowFunction.h>
#include <grid-files/grid/ValueCache.h>
#include <grid-files/identification/GridDef.h>
#include <macgyver/AnsiEscapeCodes.h>
#include <macgyver/Exception.h>
#include <macgyver/StringConversion.h>
#include <spine/Convenience.h>
#include <unistd.h>

#include <unordered_set>

#include "Browser.h"

#define FUNCTION_TRACE FUNCTION_TRACE_OFF

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
    const char* configAttribute[] = {
        "smartmet.library.grid-files.configFile",
        "smartmet.library.grid-files.cache.numOfGrids",
        "smartmet.library.grid-files.cache.maxSizeInMegaBytes",

        "smartmet.library.grid-files.pointCache.enabled",
        "smartmet.library.grid-files.pointCache.hitsRequired",
        "smartmet.library.grid-files.pointCache.timePeriod",

        "smartmet.engine.grid.enabled",

        "smartmet.engine.grid.content-server.content-source.type",
        "smartmet.engine.grid.content-server.content-source.redis.address",
        "smartmet.engine.grid.content-server.content-source.redis.port",
        "smartmet.engine.grid.content-server.content-source.redis.tablePrefix",
        "smartmet.engine.grid.content-server.content-source.http.url",
        "smartmet.engine.grid.content-server.content-source.corba.ior",
        "smartmet.engine.grid.content-server.cache.enabled",
        "smartmet.engine.grid.content-server.cache.contentSortingFlags",
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
        "smartmet.engine.grid.data-server.caching",
        "smartmet.engine.grid.data-server.grid-storage.directory",
        "smartmet.engine.grid.data-server.grid-storage.memoryMapCheckEnabled",
        "smartmet.engine.grid.data-server.grid-storage.preloadEnabled",
        "smartmet.engine.grid.data-server.grid-storage.preloadFile",
        "smartmet.engine.grid.data-server.grid-storage.preloadMemoryLock",
        "smartmet.engine.grid.data-server.virtualFiles.enabled",
        "smartmet.engine.grid.data-server.virtualFiles.definitionFile",
        "smartmet.engine.grid.data-server.luaFiles",
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
        nullptr};

    mEnabled = true;
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
    mContentCacheSortingFlags = 5;
    mPointCacheEnabled = false;
    mPointCacheHitsRequired = 20;  // 20 hits required during the last 20 minutes
    mPointCacheTimePeriod = 1200;
    mPreloadMemoryLock = false;
    mRequestForwardEnabled = false;
    mMemoryContentDir = "/tmp";
    mMemoryContentSortingFlags = 5;
    mEventListMaxSize = 0;
    mQueryCache_updateTime = time(nullptr);
    mContentServerStartTime = 0;

    mContentServerProcessingLogEnabled = false;
    mContentServerDebugLogEnabled = false;
    mDataServerProcessingLogEnabled = false;
    mDataServerDebugLogEnabled = false;
    mQueryServerProcessingLogEnabled = false;
    mQueryServerDebugLogEnabled = false;
    mVirtualFilesEnabled = false;
    mMemoryMapCheckEnabled = false;
    mParameterMappingDefinitions_updateTime = 0;
    mShutdownRequested = false;
    mContentPreloadEnabled = true;
    mParameterMappingDefinitions_autoFileKeyType = T::ParamKeyTypeValue::FMI_NAME;

    mDataServerCacheEnabled = false;
    mDataServerRemote = false;
    mContentServerProcessingLogMaxSize = 10000000;
    mContentServerProcessingLogTruncateSize = 5000000;
    mContentServerDebugLogMaxSize = 10000000;
    mContentServerDebugLogTruncateSize = 5000000;
    mDataServerProcessingLogMaxSize = 10000000;
    mDataServerProcessingLogTruncateSize = 5000000;
    mDataServerDebugLogMaxSize = 10000000;
    mDataServerDebugLogTruncateSize = 5000000;
    mQueryServerRemote = false;
    mQueryServerProcessingLogMaxSize = 10000000;
    mQueryServerProcessingLogTruncateSize = 50000000;
    mQueryServerDebugLogMaxSize = 10000000;
    mQueryServerDebugLogTruncateSize = 5000000;
    mQueryCache_enabled = true;
    mQueryCache_maxAge = 300;
    mContentSwapEnabled = false;
    mContentUpdateInterval = 180;

    mNumOfCachedGrids = 10000;
    mMaxSizeOfCachedGridsInMegaBytes = 10000;

    mContentServerCacheImplementation = nullptr;
    mDataServerImplementation = nullptr;

    mParameterTable.reset(new Spine::Table);

    ConfigurationFile configurationFile;
    configurationFile.readFile(mConfigurationFile_name.c_str());

    configurationFile.getAttributeValue("smartmet.engine.grid.enabled", mEnabled);
    if (!mEnabled) return;

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
    configurationFile.getAttributeValue("smartmet.library.grid-files.cache.numOfGrids",
                                        mNumOfCachedGrids);
    configurationFile.getAttributeValue("smartmet.library.grid-files.cache.maxSizeInMegaBytes",
                                        mMaxSizeOfCachedGridsInMegaBytes);

    configurationFile.getAttributeValue("smartmet.library.grid-files.pointCache.enabled",
                                        mPointCacheEnabled);
    configurationFile.getAttributeValue("smartmet.library.grid-files.pointCache.hitsRequired",
                                        mPointCacheHitsRequired);
    configurationFile.getAttributeValue("smartmet.library.grid-files.pointCache.timePeriod",
                                        mPointCacheTimePeriod);

    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.content-source.type",
                                        mContentSourceType);

    configurationFile.getAttributeValue(
        "smartmet.engine.grid.content-server.content-source.redis.address",
        mContentSourceRedisAddress);
    configurationFile.getAttributeValue(
        "smartmet.engine.grid.content-server.content-source.redis.port", mContentSourceRedisPort);
    configurationFile.getAttributeValue(
        "smartmet.engine.grid.content-server.content-source.redis.tablePrefix",
        mContentSourceRedisTablePrefix);
    configurationFile.getAttributeValue(
        "smartmet.engine.grid.content-server.content-source.redis.secondaryAddress",
        mContentSourceRedisSecondaryAddress);
    configurationFile.getAttributeValue(
        "smartmet.engine.grid.content-server.content-source.redis.secondaryPort",
        mContentSourceRedisSecondaryPort);
    configurationFile.getAttributeValue(
        "smartmet.engine.grid.content-server.content-source.redis.lockEnabled",
        mContentSourceRedisLockEnabled);
    configurationFile.getAttributeValue(
        "smartmet.engine.grid.content-server.content-source.redis.reloadRequired",
        mContentSourceRedisReloadRequired);

    configurationFile.getAttributeValue(
        "smartmet.engine.grid.content-server.content-source.http.url", mContentSourceHttpUrl);

    configurationFile.getAttributeValue(
        "smartmet.engine.grid.content-server.content-source.corba.ior", mContentSourceCorbaIor);

    configurationFile.getAttributeValue(
        "smartmet.engine.grid.content-server.content-source.file.contentDir", mMemoryContentDir);
    configurationFile.getAttributeValue(
        "smartmet.engine.grid.content-server.content-source.file.contentSortingFlags",
        mMemoryContentSortingFlags);
    configurationFile.getAttributeValue(
        "smartmet.engine.grid.content-server.content-source.file.eventListMaxSize",
        mEventListMaxSize);

    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.cache.enabled",
                                        mContentCacheEnabled);
    configurationFile.getAttributeValue(
        "smartmet.engine.grid.content-server.cache.contentSortingFlags", mContentCacheSortingFlags);
    configurationFile.getAttributeValue(
        "smartmet.engine.grid.content-server.cache.requestForwardEnabled", mRequestForwardEnabled);

    configurationFile.getAttributeValue(
        "smartmet.engine.grid.content-server.cache.contentSwapEnabled", mContentSwapEnabled);
    configurationFile.getAttributeValue(
        "smartmet.engine.grid.content-server.cache.contentUpdateInterval", mContentUpdateInterval);

    configurationFile.getAttributeValue(
        "smartmet.engine.grid.content-server.processing-log.enabled",
        mContentServerProcessingLogEnabled);
    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.processing-log.file",
                                        mContentServerProcessingLogFile);
    configurationFile.getAttributeValue(
        "smartmet.engine.grid.content-server.processing-log.maxSize",
        mContentServerProcessingLogMaxSize);
    configurationFile.getAttributeValue(
        "smartmet.engine.grid.content-server.processing-log.truncateSize",
        mContentServerProcessingLogTruncateSize);
    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.debug-log.enabled",
                                        mContentServerDebugLogEnabled);
    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.debug-log.file",
                                        mContentServerDebugLogFile);
    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.debug-log.maxSize",
                                        mContentServerDebugLogMaxSize);
    configurationFile.getAttributeValue(
        "smartmet.engine.grid.content-server.debug-log.truncateSize",
        mContentServerDebugLogTruncateSize);

    configurationFile.getAttributeValue("smartmet.engine.grid.data-server.remote",
                                        mDataServerRemote);
    configurationFile.getAttributeValue("smartmet.engine.grid.data-server.ior", mDataServerIor);
    configurationFile.getAttributeValue("smartmet.engine.grid.data-server.caching",
                                        mDataServerCacheEnabled);

    // These settings are used when the data server is embedded into the grid engine.
    configurationFile.getAttributeValue(
        "smartmet.engine.grid.data-server.grid-storage.memoryMapCheckEnabled",
        mMemoryMapCheckEnabled);
    configurationFile.getAttributeValue(
        "smartmet.engine.grid.data-server.grid-storage.preloadEnabled", mContentPreloadEnabled);
    configurationFile.getAttributeValue("smartmet.engine.grid.data-server.grid-storage.preloadFile",
                                        mContentPreloadFile);
    configurationFile.getAttributeValue(
        "smartmet.engine.grid.data-server.grid-storage.preloadMemoryLock", mPreloadMemoryLock);
    configurationFile.getAttributeValue("smartmet.engine.grid.data-server.grid-storage.directory",
                                        mDataServerGridDirectory);
    configurationFile.getAttributeValue("smartmet.engine.grid.data-server.virtualFiles.enabled",
                                        mVirtualFilesEnabled);
    configurationFile.getAttributeValue(
        "smartmet.engine.grid.data-server.virtualFiles.definitionFile", mVirtualFileDefinitions);
    configurationFile.getAttributeValue("smartmet.engine.grid.data-server.luaFiles",
                                        mDataServerLuaFiles);
    configurationFile.getAttributeValue("smartmet.engine.grid.data-server.processing-log.enabled",
                                        mDataServerProcessingLogEnabled);
    configurationFile.getAttributeValue("smartmet.engine.grid.data-server.processing-log.file",
                                        mDataServerProcessingLogFile);
    configurationFile.getAttributeValue("smartmet.engine.grid.data-server.processing-log.maxSize",
                                        mDataServerProcessingLogMaxSize);
    configurationFile.getAttributeValue(
        "smartmet.engine.grid.data-server.processing-log.truncateSize",
        mDataServerProcessingLogTruncateSize);
    configurationFile.getAttributeValue("smartmet.engine.grid.data-server.debug-log.enabled",
                                        mDataServerDebugLogEnabled);
    configurationFile.getAttributeValue("smartmet.engine.grid.data-server.debug-log.file",
                                        mDataServerDebugLogFile);
    configurationFile.getAttributeValue("smartmet.engine.grid.data-server.debug-log.maxSize",
                                        mDataServerDebugLogMaxSize);
    configurationFile.getAttributeValue("smartmet.engine.grid.data-server.debug-log.truncateSize",
                                        mDataServerDebugLogTruncateSize);

    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.remote",
                                        mQueryServerRemote);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.ior", mQueryServerIor);

    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.queryCache.enabled",
                                        mQueryCache_enabled);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.queryCache.maxAge",
                                        mQueryCache_maxAge);

    // These settings are used when the query server is embedded into the grid engine.
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.producerFile",
                                        mProducerSearchList_filename);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.producerMappingFiles",
                                        mProducerMappingDefinitions_filenames);

    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.processing-log.enabled",
                                        mQueryServerProcessingLogEnabled);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.processing-log.file",
                                        mQueryServerProcessingLogFile);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.processing-log.maxSize",
                                        mQueryServerProcessingLogMaxSize);
    configurationFile.getAttributeValue(
        "smartmet.engine.grid.query-server.processing-log.truncateSize",
        mQueryServerProcessingLogTruncateSize);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.debug-log.enabled",
                                        mQueryServerDebugLogEnabled);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.debug-log.file",
                                        mQueryServerDebugLogFile);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.debug-log.maxSize",
                                        mQueryServerDebugLogMaxSize);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.debug-log.truncateSize",
                                        mQueryServerDebugLogTruncateSize);

    int tmp = 0;
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.mappingTargetKeyType",
                                        tmp);
    mParameterMappingDefinitions_autoFileKeyType = C_UCHAR(tmp);

    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.mappingUpdateFile.fmi",
                                        mParameterMappingDefinitions_autoFile_fmi);
    configurationFile.getAttributeValue(
        "smartmet.engine.grid.query-server.mappingUpdateFile.newbase",
        mParameterMappingDefinitions_autoFile_newbase);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.mappingFiles",
                                        mParameterMappingDefinitions_filenames);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.aliasFiles",
                                        mParameterAliasDefinitions_filenames);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.luaFiles",
                                        mQueryServerLuaFiles);

    mProducerSearchList_modificationTime =
        getFileModificationTime(mProducerSearchList_filename.c_str());

    // Initializing information that is needed for identifying the content of the grid files.

    SmartMet::Identification::gridDef.init(mGridConfigFile.c_str());

    if (!mEnabled)
      std::cout << ANSI_FG_RED << "**** Grid-engine configuration: Engine usage disabled!"
                << ANSI_FG_DEFAULT << std::endl;
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

    ContentServer::ServiceInterface* cServer = nullptr;
    DataServer::ServiceInterface* dServer = nullptr;
    QueryServer::ServiceInterface* qServer = nullptr;

    clearMappings();

    if (mContentSourceType == "redis")
    {
      ContentServer::RedisImplementation* redis = new ContentServer::RedisImplementation();
      redis->init(mContentSourceRedisAddress.c_str(),
                  mContentSourceRedisPort,
                  mContentSourceRedisTablePrefix.c_str(),
                  mContentSourceRedisSecondaryAddress.c_str(),
                  mContentSourceRedisSecondaryPort,
                  mContentSourceRedisLockEnabled,
                  mContentSourceRedisReloadRequired);
      mContentServer.reset(redis);
      cServer = redis;
    }
    else if (mContentSourceType == "corba")
    {
      ContentServer::Corba::ClientImplementation* client =
          new ContentServer::Corba::ClientImplementation();
      client->init(mContentSourceCorbaIor.c_str());
      mContentServer.reset(client);
      cServer = client;
    }
    else if (mContentSourceType == "http")
    {
      ContentServer::HTTP::ClientImplementation* client =
          new ContentServer::HTTP::ClientImplementation();
      client->init(mContentSourceHttpUrl.c_str());
      mContentServer.reset(client);
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

      ContentServer::MemoryImplementation* memoryImplementation =
          new ContentServer::MemoryImplementation();
      memoryImplementation->init(
          true, false, true, eventstEnabled, mMemoryContentDir, 0, mMemoryContentSortingFlags);
      memoryImplementation->setEventListMaxLength(mEventListMaxSize);
      mContentServer.reset(memoryImplementation);
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
      mContentServerCacheImplementation->setContentSwapEnabled(mContentSwapEnabled);
      mContentServerCacheImplementation->setContentUpdateInterval(mContentUpdateInterval);
      mContentServerCacheImplementation->init(0, cServer, mContentCacheSortingFlags);
      mContentServerCache.reset(mContentServerCacheImplementation);
      mContentServerCacheImplementation->startEventProcessing();
      cServer = mContentServerCacheImplementation;
    }

    if (mDataServerRemote && mDataServerIor.length() > 50)
    {
      DataServer::Corba::ClientImplementation* client =
          new DataServer::Corba::ClientImplementation();
      client->init(mDataServerIor);

      if (mDataServerCacheEnabled)
      {
        DataServer::CacheImplementation* serverCache = new DataServer::CacheImplementation();
        serverCache->init(client);
        mDataServerClient.reset(client);
        mDataServer.reset(serverCache);
        dServer = serverCache;
      }
      else
      {
        mDataServer.reset(client);
        dServer = client;
      }
    }
    else
    {
      mDataServerImplementation = new DataServer::ServiceImplementation();
      mDataServerImplementation->init(0,
                                      0,
                                      "NotRegistered",
                                      "NotRegistered",
                                      mDataServerGridDirectory,
                                      cServer,
                                      mDataServerLuaFiles);
      mDataServerImplementation->setPointCacheEnabled(
          mPointCacheEnabled, mPointCacheHitsRequired, mPointCacheTimePeriod);
      mDataServerImplementation->setPreload(
          mContentPreloadEnabled, mPreloadMemoryLock, mContentPreloadFile);
      mDataServerImplementation->setMemoryMapCheckEnabled(mMemoryMapCheckEnabled);

      if (mVirtualFilesEnabled)
      {
        mDataServerImplementation->setVirtualContentEnabled(true);
        DataServer::VirtualContentFactory_type1* factory =
            new DataServer::VirtualContentFactory_type1();
        factory->init(mVirtualFileDefinitions);
        mDataServerImplementation->addVirtualContentFactory(factory);
      }
      else
      {
        mDataServerImplementation->setVirtualContentEnabled(false);
      }

      mDataServer.reset(mDataServerImplementation);
      mDataServerImplementation->startEventProcessing();

      dServer = mDataServerImplementation;

      SmartMet::GRID::valueCache.init(mNumOfCachedGrids, mMaxSizeOfCachedGridsInMegaBytes);
    }

    if (mQueryServerRemote && mQueryServerIor.length() > 50)
    {
      QueryServer::Corba::ClientImplementation* client =
          new QueryServer::Corba::ClientImplementation();
      client->init(mQueryServerIor);
      mQueryServer.reset(client);
      qServer = client;
    }
    else
    {
      QueryServer::ServiceImplementation* server = new QueryServer::ServiceImplementation();
      server->init(cServer,
                   dServer,
                   mGridConfigFile,
                   mParameterMappingDefinitions_filenames,
                   mParameterAliasDefinitions_filenames,
                   mProducerSearchList_filename,
                   mProducerMappingDefinitions_filenames,
                   mQueryServerLuaFiles);
      qServer = server;

      mQueryServer.reset(server);
    }

    if (mContentServerProcessingLogEnabled && mContentServerProcessingLogFile.length() > 0)
    {
      mContentServerProcessingLog.init(true,
                                       mContentServerProcessingLogFile.c_str(),
                                       mContentServerProcessingLogMaxSize,
                                       mContentServerProcessingLogTruncateSize);
      cServer->setProcessingLog(&mContentServerProcessingLog);
    }

    if (mContentServerDebugLogEnabled && mContentServerDebugLogFile.length() > 0)
    {
      mContentServerDebugLog.init(true,
                                  mContentServerDebugLogFile.c_str(),
                                  mContentServerDebugLogMaxSize,
                                  mContentServerDebugLogTruncateSize);
      cServer->setDebugLog(&mContentServerDebugLog);
    }

    if (mDataServerProcessingLogEnabled && mDataServerProcessingLogFile.length() > 0)
    {
      mDataServerProcessingLog.init(true,
                                    mDataServerProcessingLogFile.c_str(),
                                    mDataServerProcessingLogMaxSize,
                                    mDataServerProcessingLogTruncateSize);
      dServer->setProcessingLog(&mDataServerProcessingLog);
    }

    if (mDataServerDebugLogEnabled && mDataServerDebugLogFile.length() > 0)
    {
      mDataServerDebugLog.init(true,
                               mDataServerDebugLogFile.c_str(),
                               mDataServerDebugLogMaxSize,
                               mDataServerDebugLogTruncateSize);
      dServer->setDebugLog(&mDataServerDebugLog);
    }

    if (mQueryServerProcessingLogEnabled && mQueryServerProcessingLogFile.length() > 0)
    {
      mQueryServerProcessingLog.init(true,
                                     mQueryServerProcessingLogFile.c_str(),
                                     mQueryServerProcessingLogMaxSize,
                                     mQueryServerProcessingLogTruncateSize);
      qServer->setProcessingLog(&mQueryServerProcessingLog);
    }

    if (mQueryServerDebugLogEnabled && mQueryServerDebugLogFile.length() > 0)
    {
      mQueryServerDebugLog.init(true,
                                mQueryServerDebugLogFile.c_str(),
                                mQueryServerDebugLogMaxSize,
                                mQueryServerDebugLogTruncateSize);
      qServer->setDebugLog(&mQueryServerDebugLog);
    }

    updateProducerAndGenerationList();

    mProducerMappingDefinitions.init(mProducerMappingDefinitions_filenames, true);
    mParameterAliasDefinitions.init(mParameterAliasDefinitions_filenames);

    mBrowser.init(mConfigurationFile_name.c_str(), mContentServer, this);

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
    if (!mEnabled) return;

    // ### Configuration updates when the server is running.

    time_t currentTime = time(nullptr);
    if ((currentTime - mConfigurationFile_checkTime) < 30) return;

    mConfigurationFile_checkTime = currentTime;

    time_t tt = getFileModificationTime(mConfigurationFile_name.c_str());
    if (tt == mConfigurationFile_modificationTime) return;

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

      std::cout << ANSI_FG_RED << Spine::log_time_str()
                << " Grid-engine configuration: engine disabled" << ANSI_FG_DEFAULT << std::endl;
      return;
    }

    // ### Content server processing log

    bool contentServerProcessingLogEnabled = false;
    std::string contentServerProcessingLogFile;
    int contentServerProcessingLogMaxSize = 0;
    int contentServerProcessingLogTruncateSize = 0;

    configurationFile.getAttributeValue(
        "smartmet.engine.grid.content-server.processing-log.enabled",
        contentServerProcessingLogEnabled);
    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.processing-log.file",
                                        contentServerProcessingLogFile);
    configurationFile.getAttributeValue(
        "smartmet.engine.grid.content-server.processing-log.maxSize",
        contentServerProcessingLogMaxSize);
    configurationFile.getAttributeValue(
        "smartmet.engine.grid.content-server.processing-log.truncateSize",
        contentServerProcessingLogTruncateSize);

    if (mContentServerProcessingLogEnabled != contentServerProcessingLogEnabled ||
        mContentServerProcessingLogFile != contentServerProcessingLogFile ||
        mContentServerProcessingLogMaxSize != contentServerProcessingLogMaxSize ||
        mContentServerProcessingLogTruncateSize != contentServerProcessingLogTruncateSize)
    {
      mContentServerProcessingLog.close();

      mContentServerProcessingLogEnabled = contentServerProcessingLogEnabled;
      mContentServerProcessingLogFile = contentServerProcessingLogFile;
      mContentServerProcessingLogMaxSize = contentServerProcessingLogMaxSize;
      mContentServerProcessingLogTruncateSize = contentServerProcessingLogTruncateSize;

      mContentServerProcessingLog.init(mContentServerProcessingLogEnabled,
                                       mContentServerProcessingLogFile.c_str(),
                                       mContentServerProcessingLogMaxSize,
                                       mContentServerProcessingLogTruncateSize);
      if (contentServer->getProcessingLog() == nullptr)
        contentServer->setProcessingLog(&mContentServerProcessingLog);
    }

    // ### Content server debug log

    bool contentServerDebugLogEnabled = false;
    std::string contentServerDebugLogFile;
    int contentServerDebugLogMaxSize = 0;
    int contentServerDebugLogTruncateSize = 0;

    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.debug-log.enabled",
                                        contentServerDebugLogEnabled);
    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.debug-log.file",
                                        contentServerDebugLogFile);
    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.debug-log.maxSize",
                                        contentServerDebugLogMaxSize);
    configurationFile.getAttributeValue(
        "smartmet.engine.grid.content-server.debug-log.truncateSize",
        contentServerDebugLogTruncateSize);

    if (mContentServerDebugLogEnabled != contentServerDebugLogEnabled ||
        mContentServerDebugLogFile != contentServerDebugLogFile ||
        mContentServerDebugLogMaxSize != contentServerDebugLogMaxSize ||
        mContentServerDebugLogTruncateSize != contentServerDebugLogTruncateSize)
    {
      mContentServerDebugLog.close();

      mContentServerDebugLogEnabled = contentServerDebugLogEnabled;
      mContentServerDebugLogFile = contentServerDebugLogFile;
      mContentServerDebugLogMaxSize = contentServerDebugLogMaxSize;
      mContentServerDebugLogTruncateSize = contentServerDebugLogTruncateSize;

      mContentServerDebugLog.init(mContentServerDebugLogEnabled,
                                  mContentServerDebugLogFile.c_str(),
                                  mContentServerDebugLogMaxSize,
                                  mContentServerDebugLogTruncateSize);
      if (contentServer->getDebugLog() == nullptr)
        contentServer->setDebugLog(&mContentServerDebugLog);
    }

    // ### Data server processing log

    bool dataServerProcessingLogEnabled = false;
    std::string dataServerProcessingLogFile;
    int dataServerProcessingLogMaxSize = 0;
    int dataServerProcessingLogTruncateSize = 0;

    configurationFile.getAttributeValue("smartmet.engine.grid.data-server.processing-log.enabled",
                                        dataServerProcessingLogEnabled);
    configurationFile.getAttributeValue("smartmet.engine.grid.data-server.processing-log.file",
                                        dataServerProcessingLogFile);
    configurationFile.getAttributeValue("smartmet.engine.grid.data-server.processing-log.maxSize",
                                        dataServerProcessingLogMaxSize);
    configurationFile.getAttributeValue(
        "smartmet.engine.grid.data-server.processing-log.truncateSize",
        dataServerProcessingLogTruncateSize);

    if (mDataServerProcessingLogEnabled != dataServerProcessingLogEnabled ||
        mDataServerProcessingLogFile != dataServerProcessingLogFile ||
        mDataServerProcessingLogMaxSize != dataServerProcessingLogMaxSize ||
        mDataServerProcessingLogTruncateSize != dataServerProcessingLogTruncateSize)
    {
      mDataServerProcessingLog.close();

      mDataServerProcessingLogEnabled = dataServerProcessingLogEnabled;
      mDataServerProcessingLogFile = dataServerProcessingLogFile;
      mDataServerProcessingLogMaxSize = dataServerProcessingLogMaxSize;
      mDataServerProcessingLogTruncateSize = dataServerProcessingLogTruncateSize;

      mDataServerProcessingLog.init(mDataServerProcessingLogEnabled,
                                    mDataServerProcessingLogFile.c_str(),
                                    mDataServerProcessingLogMaxSize,
                                    mDataServerProcessingLogTruncateSize);
      if (mDataServer->getProcessingLog() == nullptr)
        mDataServer->setProcessingLog(&mDataServerProcessingLog);
    }

    // ### Data server debug log

    bool dataServerDebugLogEnabled = false;
    std::string dataServerDebugLogFile;
    int dataServerDebugLogMaxSize = 0;
    int dataServerDebugLogTruncateSize = 0;

    configurationFile.getAttributeValue("smartmet.engine.grid.data-server.debug-log.enabled",
                                        dataServerDebugLogEnabled);
    configurationFile.getAttributeValue("smartmet.engine.grid.data-server.debug-log.file",
                                        dataServerDebugLogFile);
    configurationFile.getAttributeValue("smartmet.engine.grid.data-server.debug-log.maxSize",
                                        dataServerDebugLogMaxSize);
    configurationFile.getAttributeValue("smartmet.engine.grid.data-server.debug-log.truncateSize",
                                        dataServerDebugLogTruncateSize);

    if (mDataServerDebugLogEnabled != dataServerDebugLogEnabled ||
        mDataServerDebugLogFile != dataServerDebugLogFile ||
        mDataServerDebugLogMaxSize != dataServerDebugLogMaxSize ||
        mDataServerDebugLogTruncateSize != dataServerDebugLogTruncateSize)
    {
      mDataServerDebugLog.close();

      mDataServerDebugLogEnabled = dataServerDebugLogEnabled;
      mDataServerDebugLogFile = dataServerDebugLogFile;
      mDataServerDebugLogMaxSize = dataServerDebugLogMaxSize;
      mDataServerDebugLogTruncateSize = dataServerDebugLogTruncateSize;

      mDataServerDebugLog.init(mDataServerDebugLogEnabled,
                               mDataServerDebugLogFile.c_str(),
                               mDataServerDebugLogMaxSize,
                               mDataServerDebugLogTruncateSize);
      if (mDataServer->getDebugLog() == nullptr) mDataServer->setDebugLog(&mDataServerDebugLog);
    }

    // ### Query server processing log

    bool queryServerProcessingLogEnabled = false;
    std::string queryServerProcessingLogFile;
    int queryServerProcessingLogMaxSize = 0;
    int queryServerProcessingLogTruncateSize = 0;

    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.processing-log.enabled",
                                        queryServerProcessingLogEnabled);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.processing-log.file",
                                        queryServerProcessingLogFile);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.processing-log.maxSize",
                                        queryServerProcessingLogMaxSize);
    configurationFile.getAttributeValue(
        "smartmet.engine.grid.query-server.processing-log.truncateSize",
        queryServerProcessingLogTruncateSize);

    if (mQueryServerProcessingLogEnabled != queryServerProcessingLogEnabled ||
        mQueryServerProcessingLogFile != queryServerProcessingLogFile ||
        mQueryServerProcessingLogMaxSize != queryServerProcessingLogMaxSize ||
        mQueryServerProcessingLogTruncateSize != queryServerProcessingLogTruncateSize)
    {
      mQueryServerProcessingLog.close();

      mQueryServerProcessingLogEnabled = queryServerProcessingLogEnabled;
      mQueryServerProcessingLogFile = queryServerProcessingLogFile;
      mQueryServerProcessingLogMaxSize = queryServerProcessingLogMaxSize;
      mQueryServerProcessingLogTruncateSize = queryServerProcessingLogTruncateSize;

      mQueryServerProcessingLog.init(mQueryServerProcessingLogEnabled,
                                     mQueryServerProcessingLogFile.c_str(),
                                     mQueryServerProcessingLogMaxSize,
                                     mQueryServerProcessingLogTruncateSize);
      if (mQueryServer->getProcessingLog() == nullptr)
        mQueryServer->setProcessingLog(&mQueryServerProcessingLog);
    }

    // ### Query server debug log

    bool queryServerDebugLogEnabled = false;
    std::string queryServerDebugLogFile;
    int queryServerDebugLogMaxSize = 0;
    int queryServerDebugLogTruncateSize = 0;

    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.debug-log.enabled",
                                        queryServerDebugLogEnabled);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.debug-log.file",
                                        queryServerDebugLogFile);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.debug-log.maxSize",
                                        queryServerDebugLogMaxSize);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.debug-log.truncateSize",
                                        queryServerDebugLogTruncateSize);

    if (mQueryServerDebugLogEnabled != queryServerDebugLogEnabled ||
        mQueryServerDebugLogFile != queryServerDebugLogFile ||
        mQueryServerDebugLogMaxSize != queryServerDebugLogMaxSize ||
        mQueryServerDebugLogTruncateSize != queryServerDebugLogTruncateSize)
    {
      mQueryServerDebugLog.close();

      mQueryServerDebugLogEnabled = queryServerDebugLogEnabled;
      mQueryServerDebugLogFile = queryServerDebugLogFile;
      mQueryServerDebugLogMaxSize = queryServerDebugLogMaxSize;
      mQueryServerDebugLogTruncateSize = queryServerDebugLogTruncateSize;

      mQueryServerDebugLog.init(mQueryServerDebugLogEnabled,
                                mQueryServerDebugLogFile.c_str(),
                                mQueryServerDebugLogMaxSize,
                                mQueryServerDebugLogTruncateSize);
      if (mQueryServer->getDebugLog() == nullptr) mQueryServer->setDebugLog(&mQueryServerDebugLog);
    }

    // ### Query cache

    bool queryCacheEnabled = false;
    int queryCacheMaxAge = 0;

    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.queryCache.enabled",
                                        queryCacheEnabled);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.queryCache.maxAge",
                                        queryCacheMaxAge);

    if (mQueryCache_enabled != queryCacheEnabled)
    {
      mQueryCache_enabled = queryCacheEnabled;

      if (mQueryCache_enabled)
        std::cout << Spine::log_time_str() << " Grid-engine configuration: query cache enabled"
                  << std::endl;
      else
        std::cout << Spine::log_time_str() << " Grid-engine configuration: query cache disabled"
                  << std::endl;
    }

    if (mQueryCache_maxAge != queryCacheMaxAge)
    {
      mQueryCache_maxAge = queryCacheMaxAge;
      std::cout << Spine::log_time_str()
                << " Grid-engine configuration: query cache max age set to " << mQueryCache_maxAge
                << " seconds" << std::endl;
    }

    if (mDataServerImplementation != nullptr)
    {
      // ### Point cache

      bool pointCacheEnabled = false;
      uint pointCacheHitsRequired = 0;
      uint pointCacheTimePeriod = 0;

      configurationFile.getAttributeValue("smartmet.library.grid-files.pointCache.enabled",
                                          pointCacheEnabled);
      configurationFile.getAttributeValue("smartmet.library.grid-files.pointCache.hitsRequired",
                                          pointCacheHitsRequired);
      configurationFile.getAttributeValue("smartmet.library.grid-files.pointCache.timePeriod",
                                          pointCacheTimePeriod);

      if (mPointCacheEnabled != pointCacheEnabled)
      {
        mPointCacheEnabled = pointCacheEnabled;
        mDataServerImplementation->setPointCacheEnabled(
            mPointCacheEnabled, mPointCacheHitsRequired, mPointCacheTimePeriod);

        if (mPointCacheEnabled)
          std::cout << Spine::log_time_str() << " Grid-engine configuration: point cache enabled"
                    << std::endl;
        else
          std::cout << Spine::log_time_str() << " Grid-engine configuration: point cache disabled"
                    << std::endl;
      }

      if (mPointCacheHitsRequired != pointCacheHitsRequired ||
          mPointCacheTimePeriod != pointCacheTimePeriod)
      {
        mPointCacheHitsRequired = pointCacheHitsRequired;
        mPointCacheTimePeriod = pointCacheTimePeriod;

        mDataServerImplementation->setPointCacheEnabled(
            mPointCacheEnabled, mPointCacheHitsRequired, mPointCacheTimePeriod);
      }

      // ### Memory map check

      bool memoryMapCheckEnabled = false;

      configurationFile.getAttributeValue(
          "smartmet.engine.grid.data-server.grid-storage.memoryMapCheckEnabled",
          memoryMapCheckEnabled);
      if (mMemoryMapCheckEnabled != memoryMapCheckEnabled)
      {
        mMemoryMapCheckEnabled = memoryMapCheckEnabled;
        mDataServerImplementation->setMemoryMapCheckEnabled(mMemoryMapCheckEnabled);

        if (mMemoryMapCheckEnabled)
          std::cout << Spine::log_time_str()
                    << " Grid-engine configuration: memory map check enabled" << std::endl;
        else
          std::cout << Spine::log_time_str()
                    << " Grid-engine configuration: memory map check disabled" << std::endl;
      }

      // ### Preload

      bool contentPreloadEnabled = false;
      std::string contentPreloadFile;
      bool preloadMemoryLock = false;

      configurationFile.getAttributeValue(
          "smartmet.engine.grid.data-server.grid-storage.preloadEnabled", contentPreloadEnabled);
      configurationFile.getAttributeValue(
          "smartmet.engine.grid.data-server.grid-storage.preloadFile", contentPreloadFile);
      configurationFile.getAttributeValue(
          "smartmet.engine.grid.data-server.grid-storage.preloadMemoryLock", preloadMemoryLock);

      if (mContentPreloadEnabled != contentPreloadEnabled)
      {
        mContentPreloadEnabled = contentPreloadEnabled;
        if (mContentPreloadEnabled)
          std::cout << Spine::log_time_str()
                    << " Grid-engine configuration: content preload enabled" << std::endl;
        else
          std::cout << Spine::log_time_str()
                    << " Grid-engine configuration: content preload disabled" << std::endl;
      }

      if (mContentPreloadFile != contentPreloadFile || mPreloadMemoryLock || preloadMemoryLock)
      {
        mContentPreloadFile = contentPreloadFile;
        mPreloadMemoryLock = preloadMemoryLock;
        mDataServerImplementation->setPreload(
            mContentPreloadEnabled, mPreloadMemoryLock, mContentPreloadFile);
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
    if (!mEnabled) return;

    std::cout << "  -- Shutdown requested (grid engine)\n";
    mShutdownRequested = true;

    if (!mContentServer) mContentServer->shutdown();

    if (!mContentServerCache) mContentServerCache->shutdown();

    if (!mDataServer) mDataServer->shutdown();

    if (!mQueryServer) mQueryServer->shutdown();
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}

bool Engine::browserRequest(const Spine::HTTP::Request& theRequest,
                            Spine::HTTP::Response& theResponse)
{
  FUNCTION_TRACE
  try
  {
    if (!mEnabled) return false;

    return mBrowser.requestHandler(theRequest, theResponse);
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}

void Engine::browserContent(std::ostringstream& output)
{
  FUNCTION_TRACE
  try
  {
    if (!mEnabled) return;

    mBrowser.browserContent(output);
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
    if (!mEnabled) return QueryServer::Result::SERVICE_DISABLED;

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
    if (!mEnabled) return query;

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
            exception.addDetail(
                "The reason for this situation is usually that the given producer is unknown");
            exception.addDetail(
                "or there are no producer list available in the grid engine's configuration "
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
        for (auto prod = it->second.producerHashMap.begin();
             prod != it->second.producerHashMap.end() && !noMatch;
             ++prod)
        {
          ulonglong producerHash = getProducerHash(prod->first);
          if (producerHash != prod->second) noMatch = true;
        }

        if (!noMatch)
        {
          // The cache entry is valid. We can return it.
          it->second.lastAccessTime = currentTime;
          it->second.accessCounter++;
          return it->second.query;
        }
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
          exception.addDetail(
              "The reason for this situation is usually that the given producer is unknown");
          exception.addDetail(
              "or there are no producer list available in the grid engine's configuration "
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

      std::set<uint> producerIdList;
      query->getResultProducerIdList(producerIdList);

      for (auto it = producerIdList.begin(); it != producerIdList.end(); ++it)
      {
        ulonglong producerHash = getProducerHash(*it);
        rec.producerHashMap.insert(std::pair<uint, ulonglong>(*it, producerHash));
      }

      AutoWriteLock lock(&mQueryCache_modificationLock);
      mQueryCache.insert(std::pair<std::size_t, CacheRec>(hash, rec));
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
    if (!mEnabled) return false;

    for (auto param = query->mQueryParameterList.begin(); param != query->mQueryParameterList.end();
         ++param)
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
    if (!mEnabled) return false;

    std::string prod = producer;
    std::string tmp;
    if ((mProducerMappingDefinitions.getAlias(producer, tmp) &&
         strchr(tmp.c_str(), ';') == nullptr))
    {
      // Replacing producer alias name with the (newbase) mapping name.
      prod = tmp;
    }

    // Finding (Radon) producer names according to the (newbase) producer name from the mappings.

    std::vector<std::string> nameList;

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
        if (strcasecmp(it->c_str(), itm->c_str()) == 0) return true;
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

std::string Engine::getParameterString(const std::string& producer,
                                       const std::string& parameter) const
{
  FUNCTION_TRACE
  try
  {
    if (!mEnabled) return parameter;

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
      for (size_t t = 0; t < len; t++)
      {
        if (parameters[t].mLevelId > "") levelId = parameters[t].mLevelId;

        if (parameters[t].mLevel > "") level = parameters[t].mLevel;

        if (parameters[t].mForecastType > "") forecastType = parameters[t].mForecastType;

        if (parameters[t].mForecastNumber > "") forecastNumber = parameters[t].mForecastNumber;

        if (parameters[t].mProducerName > "") prod = parameters[t].mProducerName;

        if (parameters[t].mGeometryId > "")
        {
          prod = parameters[t].mProducerName;
          geomId = parameters[t].mGeometryId;
        }
      }
      std::string paramStr = parameter + ":" + prod + ":" + geomId + ":" + levelId + ":" + level +
                             ":" + forecastType + ":" + forecastNumber;
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
    if (!mEnabled) return aliasName;

    // This method returns the producer's mapping name.

    mProducerMappingDefinitions.checkUpdates(false);

    // Finding the mapping name for the (newbase) producer alias. The producer name mappings look
    // like this:
    //   pal:pal_skandinavia
    //   pal_scandinavia:pal_skandinavia

    std::string prod = aliasName;
    if ((mProducerMappingDefinitions.getAlias(aliasName, prod) &&
         strchr(prod.c_str(), ';') == nullptr))
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

void Engine::getProducerNameList(const std::string& mappingName,
                                 std::vector<std::string>& nameList) const
{
  FUNCTION_TRACE
  try
  {
    if (!mEnabled) return;

    // This method returns the list of (Radon) producers according to the (newbase) mapping name.

    mProducerMappingDefinitions.checkUpdates(false);

    std::vector<std::string> mappingList;
    mProducerMappingDefinitions.getAliasList(mappingName, mappingList);

    // The producer name mapping list looks like this:
    //   pal_skandinavia:SMARTMET;1096;;;
    //   pal_skandinavia:SMARTMETMTA;1096;;;

    for (auto it = mappingList.begin(); it != mappingList.end(); it++)
    {
      std::vector<std::string> partList;
      splitString(*it, ';', partList);

      nameList.emplace_back(partList[0]);
    }

    if (nameList.size() == 0) nameList.emplace_back(mappingName);
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
    if (!mEnabled) return 0;

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

void Engine::getParameterDetails(const std::string& aliasName,
                                 ParameterDetails_vec& parameterDetails) const
{
  FUNCTION_TRACE
  try
  {
    if (!mEnabled) return;

    mProducerMappingDefinitions.checkUpdates(false);

    std::vector<std::string> aliasStrings;
    mProducerMappingDefinitions.getAliasList(aliasName, aliasStrings);

    for (auto it = aliasStrings.begin(); it != aliasStrings.end(); it++)
    {
      std::vector<std::string> partList;
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

void Engine::getParameterDetails(const std::string& producerName,
                                 const std::string& parameterName,
                                 ParameterDetails_vec& parameterDetails) const
{
  FUNCTION_TRACE
  try
  {
    if (!mEnabled) return;

    mProducerMappingDefinitions.checkUpdates(false);
    mParameterAliasDefinitions.checkUpdates(false);

    std::string prod = producerName;
    std::string tmp;

    // Finding the mapping name for the (newbase) producer. The producer name mappings look like
    // this:
    //
    //   pal:pal_skandinavia

    if (mProducerMappingDefinitions.getAlias(producerName, tmp) &&
        strchr(tmp.c_str(), ';') == nullptr)
    {
      getParameterDetails(tmp, parameterName, parameterDetails);
      return;
    }

    // Finding "official name" for the parameter alias name. This name is used in the parameter
    // mapping files when the query is executed. The parameter alias definitions look like this:
    //
    //   fog:FogIntensity
    //   rtype:PrecipitationType

    std::string param = parameterName;
    mParameterAliasDefinitions.getAlias(parameterName, param);

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

    std::string key = prod + ";" + param;
    std::vector<std::string> mappingList;
    mProducerMappingDefinitions.getAliasList(key, mappingList);

    for (auto it = mappingList.begin(); it != mappingList.end(); it++)
    {
      std::vector<std::string> partList;
      splitString(*it, ';', partList);

      ParameterDetails p;
      p.mOriginalProducer = producerName;
      p.mOriginalParameter = parameterName;

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
      p.mOriginalProducer = producerName;
      p.mOriginalParameter = param;
      p.mProducerName = key;
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

void Engine::getParameterDetails(const std::string& producerName,
                                 const std::string& parameterName,
                                 std::string& level,
                                 ParameterDetails_vec& parameterDetails) const
{
  FUNCTION_TRACE
  try
  {
    if (!mEnabled) return;

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

void Engine::getParameterMappings(const std::string& producerName,
                                  const std::string& parameterName,
                                  T::GeometryId geometryId,
                                  bool onlySearchEnabled,
                                  QueryServer::ParameterMapping_vec& mappings) const
{
  try
  {
    if (!mEnabled) return;

    if (!mParameterMappingDefinitions) return;

    AutoReadLock lock(&mParameterMappingDefinitions_modificationLock);

    for (auto m = mParameterMappingDefinitions->begin(); m != mParameterMappingDefinitions->end();
         ++m)
    {
      m->getMappings(producerName, parameterName, geometryId, onlySearchEnabled, mappings);
    }
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}

void Engine::getParameterMappings(const std::string& producerName,
                                  const std::string& parameterName,
                                  bool onlySearchEnabled,
                                  QueryServer::ParameterMapping_vec& mappings) const
{
  try
  {
    if (!mEnabled) return;

    if (!mParameterMappingDefinitions) return;

    AutoReadLock lock(&mParameterMappingDefinitions_modificationLock);

    for (auto m = mParameterMappingDefinitions->begin(); m != mParameterMappingDefinitions->end();
         ++m)
    {
      m->getMappings(producerName, parameterName, onlySearchEnabled, mappings);
    }
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}

void Engine::getParameterMappings(const std::string& producerName,
                                  const std::string& parameterName,
                                  T::GeometryId geometryId,
                                  T::ParamLevelIdType levelIdType,
                                  T::ParamLevelId levelId,
                                  T::ParamLevel level,
                                  bool onlySearchEnabled,
                                  QueryServer::ParameterMapping_vec& mappings) const
{
  try
  {
    if (!mEnabled) return;

    if (!mParameterMappingDefinitions) return;

    AutoReadLock lock(&mParameterMappingDefinitions_modificationLock);

    for (auto m = mParameterMappingDefinitions->begin(); m != mParameterMappingDefinitions->end();
         ++m)
    {
      m->getMappings(producerName,
                     parameterName,
                     geometryId,
                     levelIdType,
                     levelId,
                     level,
                     onlySearchEnabled,
                     mappings);
    }
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}

void Engine::getParameterMappings(const std::string& producerName,
                                  const std::string& parameterName,
                                  T::ParamLevelIdType levelIdType,
                                  T::ParamLevelId levelId,
                                  T::ParamLevel level,
                                  bool onlySearchEnabled,
                                  QueryServer::ParameterMapping_vec& mappings) const
{
  try
  {
    if (!mEnabled) return;

    if (!mParameterMappingDefinitions) return;

    AutoReadLock lock(&mParameterMappingDefinitions_modificationLock);

    for (auto m = mParameterMappingDefinitions->begin(); m != mParameterMappingDefinitions->end();
         ++m)
    {
      m->getMappings(
          producerName, parameterName, levelIdType, levelId, level, onlySearchEnabled, mappings);
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
    if (!mEnabled) return;

    ContentServer_sptr contentServer = getContentServer_sptr();

    for (auto rec = parameterDetails.begin(); rec != parameterDetails.end(); ++rec)
    {
      QueryServer::ParameterMapping_vec mappings;
      if (rec->mLevelId > " " || rec->mLevel > " ")
      {
        getParameterMappings(rec->mProducerName,
                             rec->mOriginalParameter,
                             atoi(rec->mGeometryId.c_str()),
                             T::ParamLevelIdTypeValue::ANY,
                             atoi(rec->mLevelId.c_str()),
                             atoi(rec->mLevel.c_str()),
                             false,
                             mappings);
        if (mappings.size() == 0 && rec->mLevel < " ")
        {
          getParameterMappings(rec->mProducerName,
                               rec->mOriginalParameter,
                               atoi(rec->mGeometryId.c_str()),
                               T::ParamLevelIdTypeValue::ANY,
                               atoi(rec->mLevelId.c_str()),
                               -1,
                               false,
                               mappings);
        }
      }
      else
      {
        getParameterMappings(rec->mProducerName,
                             rec->mOriginalParameter,
                             atoi(rec->mGeometryId.c_str()),
                             true,
                             mappings);
      }

      for (auto m = mappings.begin(); m != mappings.end(); ++m)
      {
        MappingDetails details;
        details.mMapping = *m;

        T::ContentInfoList contentInfoList;
        int result =
            contentServer->getContentListByParameterAndProducerName(0,
                                                                    m->mProducerName,
                                                                    m->mParameterKeyType,
                                                                    m->mParameterKey,
                                                                    m->mParameterLevelIdType,
                                                                    m->mParameterLevelId,
                                                                    m->mParameterLevel,
                                                                    m->mParameterLevel,
                                                                    -1,
                                                                    -1,
                                                                    m->mGeometryId,
                                                                    std::string("19000101T000000"),
                                                                    std::string("21000101T000000"),
                                                                    0,
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
              T::GenerationInfo* gInfo =
                  mGenerationInfoList.getGenerationInfoById(cInfo->mGenerationId);
              if (gInfo != nullptr)
              {
                auto tt = details.mTimes.find(gInfo->mAnalysisTime);
                if (tt != details.mTimes.end())
                {
                  tt->second.insert(cInfo->getForecastTime());
                }
                else
                {
                  std::set<std::string> ttt;
                  ttt.insert(cInfo->getForecastTime());
                  details.mTimes.insert(
                      std::pair<std::string, std::set<std::string>>(gInfo->mAnalysisTime, ttt));
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
    if (!mEnabled) return producerName;

    // This method returns the producer mapping name. Sometimes the same alias name
    // is used for different mappings. In this case the requested level type might
    // help us to identify the corret producer.

    // The mapping definitions might look like this (aliasName + levelId):
    //   ec:ecmwf_maailma_pinta
    //   ec;2:ecmwf_maailma_painepinta

    mProducerMappingDefinitions.checkUpdates(false);

    std::string prod = producerName;
    std::string tmp = producerName;
    if (levelId >= 0) tmp = producerName + ";" + std::to_string(levelId);

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

ContentTable Engine::getProducerInfo(boost::optional<std::string> producer) const
{
  try
  {
    boost::shared_ptr<Spine::Table> resultTable(new Spine::Table);

    Spine::TableFormatter::Names headers{"#",
                                         "ProducerName",
                                         "ProducerId",
                                         "Title",
                                         "Description",
                                         "NumOfGenerations",
                                         "NewestGeneration",
                                         "OldestGeneation"};

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
          generationList.sort(T::GenerationInfo::ComparisonMethod::analysisTime_generationId);

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
          resultTable->set(6, row, newest->mAnalysisTime);

          // Oldest generation
          resultTable->set(7, row, oldest->mAnalysisTime);

          row++;
        }
      }
    }

    return std::make_pair(resultTable, headers);
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}

ContentTable Engine::getParameterInfo(boost::optional<std::string> producer) const
{
  FUNCTION_TRACE
  try
  {
    Spine::TableFormatter::Names headers{"#",
                                         "Producer",
                                         "FmiParameterName",
                                         "FmiParameterId",
                                         "NewbaseParameterName",
                                         "NewbaseParameterId",
                                         "Unit",
                                         "Description"};
    boost::shared_ptr<Spine::Table> resultTable(new Spine::Table);

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

    return std::make_pair(resultTable, headers);
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
    if (!mEnabled) return 0;

    AutoReadLock lock(&mProducerInfoList_modificationLock);

    uint len = mLevelInfoList.getLength();
    for (uint t = 0; t < len; t++)
    {
      T::LevelInfo* levelInfo = mLevelInfoList.getLevelInfoByIndex(t);
      if (levelInfo != nullptr && levelInfo->mProducerId == producerId)
      {
        if (levelInfo->mParameterLevel == level) return levelInfo->mFmiParameterLevelId;
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
    if (!mEnabled) return;

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
    if (!mEnabled) return false;

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
    if (!mEnabled) return false;

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
    if (!mEnabled) return false;

    AutoReadLock lock(&mProducerInfoList_modificationLock);
    return mGenerationInfoList.getGenerationInfoById(generationId, generationInfo);
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}

void Engine::getProducerParameterLevelList(const std::string& producerName,
                                           T::ParamLevelId fmiParamLevelId,
                                           double multiplier,
                                           std::set<double>& levels) const
{
  FUNCTION_TRACE
  try
  {
    if (!mEnabled) return;

    AutoReadLock lock(&mProducerInfoList_modificationLock);

    std::vector<std::string> nameList;
    getProducerNameList(producerName, nameList);

    for (auto pname = nameList.begin(); pname != nameList.end(); ++pname)
    {
      T::ProducerInfo producerInfo;
      if (getProducerInfoByName(*pname, producerInfo))
      {
        std::string fmiParameterName;
        uint len = mLevelInfoList.getLength();
        for (uint t = 0; t < len; t++)
        {
          T::LevelInfo* levelInfo = mLevelInfoList.getLevelInfoByIndex(t);
          if (levelInfo != nullptr && levelInfo->mProducerId == producerInfo.mProducerId &&
              levelInfo->mFmiParameterLevelId == fmiParamLevelId &&
              (fmiParameterName.empty() || levelInfo->mFmiParameterName == fmiParameterName))
          {
            fmiParameterName = levelInfo->mFmiParameterName;
            levels.insert(levelInfo->mParameterLevel * multiplier);
          }
        }

        if (levels.size() > 0) return;
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

void Engine::getProducerParameterLevelIdList(const std::string& producerName,
                                             std::set<T::ParamLevelId>& levelIdList) const
{
  FUNCTION_TRACE
  try
  {
    if (!mEnabled) return;

    AutoReadLock lock(&mProducerInfoList_modificationLock);

    std::vector<std::string> nameList;
    getProducerNameList(producerName, nameList);

    for (auto pname = nameList.begin(); pname != nameList.end(); ++pname)
    {
      T::ProducerInfo producerInfo;
      if (getProducerInfoByName(*pname, producerInfo))
      {
        std::string fmiParameterName;
        uint len = mLevelInfoList.getLength();
        for (uint t = 0; t < len; t++)
        {
          T::LevelInfo* levelInfo = mLevelInfoList.getLevelInfoByIndex(t);
          if (levelInfo != nullptr && levelInfo->mProducerId == producerInfo.mProducerId)
          {
            // if (levelIdList.find(levelInfo->mFmiParameterLevelId) == levelIdList.end())
            levelIdList.insert(levelInfo->mFmiParameterLevelId);
          }
        }

        if (levelIdList.size() > 0) return;
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

void Engine::loadMappings(QueryServer::ParamMappingFile_vec& parameterMappings)
{
  FUNCTION_TRACE
  try
  {
    if (!mEnabled) return;

    for (auto it = mParameterMappingDefinitions_filenames.begin();
         it != mParameterMappingDefinitions_filenames.end();
         ++it)
    {
      QueryServer::ParameterMappingFile mapping(*it);
      parameterMappings.emplace_back(mapping);
    }

    for (auto it = parameterMappings.begin(); it != parameterMappings.end(); ++it)
    {
      // Loading parameter mappings if the mapping file exists and it is not empty.
      if (getFileSize(it->getFilename().c_str()) > 0) it->init();
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
    if (!mEnabled) return;

    QueryServer::ParamMappingFile_vec parameterMappings;

    if (!mParameterMappingDefinitions_autoFile_fmi.empty())
    {
      FILE* file = openMappingFile(mParameterMappingDefinitions_autoFile_fmi);
      if (file != nullptr) fclose(file);
    }

    if (!mParameterMappingDefinitions_autoFile_newbase.empty())
    {
      FILE* file = openMappingFile(mParameterMappingDefinitions_autoFile_newbase);
      if (file != nullptr) fclose(file);
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
    if (!mEnabled) return;

    time_t currentTime = time(nullptr);

    if ((currentTime - mParameterMappingDefinitions_updateTime) < 300) return;

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
      updateMappings(T::ParamKeyTypeValue::FMI_NAME,
                     mParameterMappingDefinitions_autoFileKeyType,
                     mParameterMappingDefinitions_autoFile_fmi,
                     *parameterMappings,
                     *paramTable);
    }

    if (!mParameterMappingDefinitions_autoFile_newbase.empty())
    {
      updateMappings(T::ParamKeyTypeValue::NEWBASE_NAME,
                     mParameterMappingDefinitions_autoFileKeyType,
                     mParameterMappingDefinitions_autoFile_newbase,
                     *parameterMappings,
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
    if (!mEnabled) return nullptr;

    FILE* file = fopen(mappingFile.c_str(), "we");
    if (file == nullptr)
    {
      Fmi::Exception exception(BCP, "Cannot open a mapping file for writing!");
      exception.addParameter("Filaname", mappingFile);
      throw exception;
    }

    fprintf(file, "# This file is automatically generated by the grid engine. The file contains\n");
    fprintf(file,
            "# mappings for the parameters found from the content server, which do not have\n");
    fprintf(file, "# mappings already defined. The point is that the query server cannot find \n");
    fprintf(
        file,
        "# requested parameters without mappings. On the other hand, the order of the mappings\n");
    fprintf(file,
            "# is also the search order of the parameters that do not contain complete search \n");
    fprintf(file, "# information (parameterIdType,levelIdType,levelId,level,etc.)\n");
    fprintf(file, "# \n");
    fprintf(file,
            "# If you want to change some of the mappings or their order, then you should move\n");
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
    fprintf(file, "#         500..999 = List\n");
    fprintf(file, "#         1000..65535 = External (interpolated by an external function)\n");
    fprintf(file, "# 10) Time interpolation method\n");
    fprintf(file, "#         0 = None\n");
    fprintf(file, "#         1 = Linear\n");
    fprintf(file, "#         2 = Nearest\n");
    fprintf(file, "#         3 = Min\n");
    fprintf(file, "#         4 = Max\n");
    fprintf(file, "#         1000..65535 = External (interpolated by an external function)\n");
    fprintf(file, "# 11) Level interpolation method\n");
    fprintf(file, "#         0 = None\n");
    fprintf(file, "#         1 = Linear\n");
    fprintf(file, "#         2 = Nearest\n");
    fprintf(file, "#         3 = Min\n");
    fprintf(file, "#         4 = Max\n");
    fprintf(file, "#         5 = Logarithmic\n");
    fprintf(file, "#         1000..65535 = External (interpolated by an external function)\n");
    fprintf(file, "# 12) Group flags\n");
    fprintf(file, "#         bit 0 = Climatological parameter (=> ignore year when searching) \n");
    fprintf(file,
            "# 13) Search match (Can this mapping used when searching mappings for incomplete "
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

void Engine::updateMappings(T::ParamKeyType sourceParameterKeyType,
                            T::ParamKeyType targetParameterKeyType,
                            const std::string& mappingFile,
                            QueryServer::ParamMappingFile_vec& parameterMappings,
                            Spine::Table& paramTable)
{
  FUNCTION_TRACE
  try
  {
    if (!mEnabled) return;

    ContentServer_sptr contentServer = getContentServer_sptr();

    T::SessionId sessionId = 0;
    uint numOfNewMappings = 0;
    std::unordered_set<std::string> mapList;
    std::unordered_set<std::string> searchList;

    T::ProducerInfoList producerInfoList;
    int result = contentServer->getProducerInfoList(sessionId, producerInfoList);
    if (result != 0)
    {
      std::cerr
          << __FILE__ << ":" << __LINE__
          << ": The 'contentServer.getProducerInfoList()' service call returns an error!  Result : "
          << result << " : " << ContentServer::getResultString(result).c_str() << "\n";
      return;
    }

    FILE* file = nullptr;
    std::unordered_set<std::string> pList;
    auto row = paramTable.maxj();

    uint plen = producerInfoList.getLength();
    for (uint t = 0; t < plen; t++)
    {
      T::ProducerInfo* producerInfo = producerInfoList.getProducerInfoByIndex(t);
      std::set<std::string> infoList;

      int result = contentServer->getProducerParameterListByProducerId(sessionId,
                                                                       producerInfo->mProducerId,
                                                                       sourceParameterKeyType,
                                                                       targetParameterKeyType,
                                                                       infoList);
      if (result == 0)
      {
        for (auto it = infoList.begin(); it != infoList.end(); ++it)
        {
          std::vector<std::string> pl;
          splitString(it->c_str(), ';', pl);
          if (pl.size() >= 8)
          {
            QueryServer::ParameterMapping m;
            m.mProducerName = pl[0];
            m.mParameterName = pl[1];
            m.mParameterKeyType = toUInt8(pl[2].c_str());
            m.mParameterKey = pl[3];
            m.mGeometryId = toInt32(pl[4].c_str());
            m.mParameterLevelIdType = toUInt8(pl[5].c_str());
            m.mParameterLevelId = toInt8(pl[6].c_str());
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
                  Identification::gridDef.getNewbaseParameterDefByFmiId(paramDef.mFmiParameterId,
                                                                        nbDef);
                  paramTable.set(4, row, nbDef.mParameterName);
                  paramTable.set(5, row, std::to_string(nbDef.mNewbaseParameterId));

                  paramTable.set(6, row, paramDef.mParameterUnits);
                  paramTable.set(7, row, paramDef.mParameterDescription);
                }

                row++;
              }
            }

            char key[200];
            sprintf(key,
                    "%s;%s;%s;%s;%s;%s;%s;%s;",
                    pl[0].c_str(),
                    pl[1].c_str(),
                    pl[2].c_str(),
                    pl[3].c_str(),
                    pl[4].c_str(),
                    pl[5].c_str(),
                    pl[6].c_str(),
                    pl[7].c_str());
            std::string searchKey =
                m.mProducerName + ":" + m.mParameterName + ":" + std::to_string(m.mGeometryId);

            if (mapList.find(std::string(key)) == mapList.end())
            {
              mapList.insert(std::string(key));

              bool found = false;
              bool searchEnabled = false;
              for (auto it = parameterMappings.begin(); it != parameterMappings.end() && !found;
                   ++it)
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
                if (!searchEnabled || (m.mParameterLevelId == 6 && m.mParameterLevel <= 10) ||
                    (m.mParameterLevelId == 1 && m.mParameterLevel == 0))
                {
                  if (m.mParameterLevelId != 2 && m.mParameterLevelId != 3 &&
                      m.mParameterLevelId != 4)
                    s = 'E';
                }

                // if (searchList.find(searchKey) == searchList.end())
                searchList.insert(searchKey);

                if (file == nullptr) file = openMappingFile(mappingFile);

                fprintf(file,
                        "%s;%s;%s;%s;%s;%s;%s;%s;",
                        pl[0].c_str(),
                        pl[1].c_str(),
                        pl[2].c_str(),
                        pl[3].c_str(),
                        pl[4].c_str(),
                        pl[5].c_str(),
                        pl[6].c_str(),
                        pl[7].c_str());

                Identification::FmiParameterDef paramDef;

                bool found = false;
                if (targetParameterKeyType == T::ParamKeyTypeValue::FMI_NAME)
                  found = Identification::gridDef.getFmiParameterDefByName(pl[3], paramDef);
                else if (targetParameterKeyType == T::ParamKeyTypeValue::FMI_ID)
                  found = Identification::gridDef.getFmiParameterDefById(toUInt32(pl[3]), paramDef);
                else if (targetParameterKeyType == T::ParamKeyTypeValue::NEWBASE_ID)
                  found = Identification::gridDef.getFmiParameterDefByNewbaseId(toUInt32(pl[3]),
                                                                                paramDef);

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

                  if (sourceParameterKeyType == T::ParamKeyTypeValue::NEWBASE_ID ||
                      sourceParameterKeyType == T::ParamKeyTypeValue::NEWBASE_NAME)
                  {
                    Identification::FmiParameterId_newbase paramMapping;
                    if (Identification::gridDef.getNewbaseParameterMappingByFmiId(
                            paramDef.mFmiParameterId, paramMapping))
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
                    fprintf(file, "%d;", (int)paramDef.mDefaultPrecision);
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

      file = openMappingFile(mappingFile);
    }

    if (file != nullptr) fclose(file);
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
    if (!mEnabled) return;

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
                mLevelInfoList.clear();
                mProducerInfoList_updateTime = 0;
                mLevelInfoList_lastUpdate = 0;
              }

              {
                AutoWriteLock lock(&mQueryCache_modificationLock);
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

      sleep(1);
    }
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}

void Engine::updateProducerAndGenerationList()
{
  FUNCTION_TRACE
  try
  {
    if (!mEnabled) return;

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
    if (!mEnabled) return;

    if (!mQueryCache_enabled) return;

    time_t currentTime = time(nullptr);
    if ((currentTime - mQueryCache_updateTime) < 60) return;

    time_t tt = getFileModificationTime(mProducerSearchList_filename.c_str());
    if (mProducerSearchList_modificationTime != tt && (tt + 3) < currentTime)
    {
      // The producer search order has changed. So we have to clear the query cache.
      mProducerSearchList_modificationTime = tt;
      AutoWriteLock lock(&mQueryCache_modificationLock);
      mQueryCache.clear();
      return;
    }

    mQueryCache_enabled = false;

    mQueryCache_updateTime = currentTime;
    time_t lastAccess = currentTime - mQueryCache_maxAge;
    std::vector<ulonglong> deleteList;

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
          for (auto prod = it->second.producerHashMap.begin();
               prod != it->second.producerHashMap.end() && !noMatch;
               ++prod)
          {
            ulonglong producerHash = getProducerHash(prod->first);
            if (producerHash != prod->second) noMatch = true;
          }

          if (noMatch) deleteList.emplace_back(it->first);
        }
      }
    }

    if (deleteList.size() > 0)
    {
      AutoWriteLock lock(&mQueryCache_modificationLock);
      for (auto it = deleteList.begin(); it != deleteList.end(); ++it)
      {
        auto pos = mQueryCache.find(*it);
        if (pos != mQueryCache.end()) mQueryCache.erase(pos);
      }
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

void Engine::getVerticalGrid(double lon1,
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
    if (!mEnabled) return;

    ContentServer_sptr contentServer = getContentServer_sptr();
    DataServer_sptr dataServer = getDataServer_sptr();
    QueryServer_sptr queryServer = getQueryServer_sptr();

    std::set<double> levels1;
    std::set<double> levels2;

    T::SessionId sessionId = 0;
    T::GenerationInfoList gInfoList1;
    T::GenerationInfoList gInfoList2;

    int result = contentServer->getGenerationInfoListByProducerName(
        sessionId, valueProducerName, gInfoList1);
    if (result != 0)
    {
      Fmi::Exception exception(BCP, ContentServer::getResultString(result).c_str(), nullptr);
      exception.addParameter("ProducerName", valueProducerName);
      throw exception;
    }

    result = contentServer->getGenerationInfoListByProducerName(
        sessionId, heightProducerName, gInfoList2);
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
      Fmi::Exception exception(BCP,
                               "The number of value parameter levels is different than the number "
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

      std::vector<T::ParamValue> valueVec;
      std::vector<T::ParamValue> heightVec;

      std::string pa(param);
      int result1 = queryServer->getParameterValuesByPointListAndTime(
          sessionId,
          valueProducerName,
          pa,
          T::CoordinateTypeValue::LATLON_COORDINATES,
          points.first,
          utcTime,
          areaInterpolationMethod,
          timeInterpolationMethod,
          1,
          valueVec);

      p = param;
      p += sprintf(p, "%s:%s", heightParameter.c_str(), heightProducerName.c_str());
      if (geometryId > 0)
        p += sprintf(p, ":%u:3:%u", geometryId, lev);
      else
        p += sprintf(p, "::3:%u", lev);

      pa = param;
      int result2 = queryServer->getParameterValuesByPointListAndTime(
          sessionId,
          heightProducerName,
          pa,
          T::CoordinateTypeValue::LATLON_COORDINATES,
          points.first,
          utcTime,
          areaInterpolationMethod,
          timeInterpolationMethod,
          1,
          heightVec);

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
    int imageHeight = 1000; // len1*mp;
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

void Engine::setDem(boost::shared_ptr<Fmi::DEM> dem)
{
  try
  {
    if (!mEnabled) return;

    mDem = dem;
    mQueryServer->setDem(dem);
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}

void Engine::setLandCover(boost::shared_ptr<Fmi::LandCover> landCover)
{
  try
  {
    if (!mEnabled) return;

    mLandCover = landCover;
    mQueryServer->setLandCover(landCover);
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
    if (!mEnabled) return;

    pthread_create(&mThread, nullptr, gridEngine_updateThread, this);
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file", mConfigurationFile_name);
    throw exception;
  }
}

}  // namespace Grid
}  // namespace Engine
}  // namespace SmartMet

// DYNAMIC MODULE CREATION TOOLS

extern "C" void* engine_class_creator(const char* configfile, void* /* user_data */)
{
  return new SmartMet::Engine::Grid::Engine(configfile);
}

extern "C" const char* engine_name() { return "grid"; }
