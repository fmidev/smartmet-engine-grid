#include "Engine.h"

#include <macgyver/Exception.h>
#include <grid-files/common/GeneralFunctions.h>
#include <grid-files/common/ShowFunction.h>
#include <grid-files/grid/ValueCache.h>
#include <grid-files/identification/GridDef.h>
#include <grid-files/common/CoordinateConversions.h>
#include <grid-files/common/ImageFunctions.h>
#include <grid-files/common/ImagePaint.h>
#include <grid-files/common/GraphFunctions.h>
#include <grid-content/contentServer/corba/client/ClientImplementation.h>
#include <grid-content/contentServer/http/client/ClientImplementation.h>
#include <grid-content/dataServer/corba/client/ClientImplementation.h>
#include <grid-content/dataServer/implementation/VirtualContentFactory_type1.h>
#include <grid-content/queryServer/corba/client/ClientImplementation.h>
#include <macgyver/StringConversion.h>
#include <unistd.h>


#define FUNCTION_TRACE FUNCTION_TRACE_OFF


namespace SmartMet
{
namespace Engine
{
namespace Grid
{


static void* gridEngine_updateThread(void *arg)
{
  try
  {
    Engine *engine = static_cast<Engine*>(arg);
    engine->updateProcessing();
    return nullptr;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP,"Operation failed!",nullptr);
    exception.printError();
    exit(-1);
  }
}




Engine::Engine(const char* theConfigFile)
{
  FUNCTION_TRACE
  try
  {
    const char *configAttribute[] =
    {
        "smartmet.library.grid-files.configFile",
        "smartmet.library.grid-files.cache.numOfGrids",
        "smartmet.library.grid-files.cache.maxSizeInMegaBytes",

        "smartmet.library.grid-files.pointCache.enabled",
        "smartmet.library.grid-files.pointCache.hitsRequired",
        "smartmet.library.grid-files.pointCache.timePeriod",

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
        "smartmet.engine.grid.query-server.producerAliasFiles",
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

    mConfigurationFilename = theConfigFile;
    mConfigurationFilename_checkTime = time(nullptr) + 120;
    mConfigurationFilename_modificationTime = getFileModificationTime(mConfigurationFilename.c_str());
    mLevelInfoList_lastUpdate = 0;
    mProducerList_updateTime = 0;
    mContentSourceRedisAddress = "127.0.0.1";
    mContentSourceRedisPort = 6379;
    mContentSourceRedisTablePrefix = "";
    mContentSourceHttpUrl = "";
    mContentSourceCorbaIor = "";
    mContentCacheEnabled = true;
    mContentCacheSortingFlags = 5;
    mPointCacheEnabled = false;
    mPointCacheHitsRequired = 20; // 20 hits required during the last 20 minutes
    mPointCacheTimePeriod = 1200;
    mPreloadMemoryLock = false;
    mRequestForwardEnabled = false;
    mMemoryContentDir = "/tmp";
    mMemoryContentSortingFlags = 5;
    mEventListMaxSize = 0;
    mQueryCacheUpdateTime = time(nullptr);

    mContentServerProcessingLogEnabled = false;
    mContentServerDebugLogEnabled = false;
    mDataServerProcessingLogEnabled = false;
    mDataServerDebugLogEnabled = false;
    mQueryServerProcessingLogEnabled = false;
    mQueryServerDebugLogEnabled = false;
    mVirtualFilesEnabled = false;
    mMemoryMapCheckEnabled = false;
    mParameterMappingUpdateTime = 0;
    mShutdownRequested = false;
    mContentPreloadEnabled = true;
    mMappingTargetKeyType = T::ParamKeyTypeValue::FMI_NAME;

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
    mQueryCacheEnabled = true;
    mQueryCacheMaxAge = 300;


    mNumOfCachedGrids = 10000;
    mMaxSizeOfCachedGridsInMegaBytes = 10000;

    mContentServerCacheImplementation = nullptr;
    mDataServerImplementation = nullptr;

    mConfigurationFile.readFile(theConfigFile);

    uint t=0;
    while (configAttribute[t] != nullptr)
    {
      if (!mConfigurationFile.findAttribute(configAttribute[t]))
      {
        Fmi::Exception exception(BCP, "Missing configuration attribute!");
        exception.addParameter("File",theConfigFile);
        exception.addParameter("Attribute",configAttribute[t]);
        throw exception;
      }
      t++;
    }

    mConfigurationFile.getAttributeValue("smartmet.library.grid-files.configFile", mGridConfigFile);
    mConfigurationFile.getAttributeValue("smartmet.library.grid-files.cache.numOfGrids", mNumOfCachedGrids);
    mConfigurationFile.getAttributeValue("smartmet.library.grid-files.cache.maxSizeInMegaBytes", mMaxSizeOfCachedGridsInMegaBytes);

    mConfigurationFile.getAttributeValue("smartmet.library.grid-files.pointCache.enabled", mPointCacheEnabled);
    mConfigurationFile.getAttributeValue("smartmet.library.grid-files.pointCache.hitsRequired", mPointCacheHitsRequired);
    mConfigurationFile.getAttributeValue("smartmet.library.grid-files.pointCache.timePeriod", mPointCacheTimePeriod);

    mConfigurationFile.getAttributeValue("smartmet.engine.grid.content-server.content-source.type", mContentSourceType);

    mConfigurationFile.getAttributeValue("smartmet.engine.grid.content-server.content-source.redis.address", mContentSourceRedisAddress);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.content-server.content-source.redis.port", mContentSourceRedisPort);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.content-server.content-source.redis.tablePrefix", mContentSourceRedisTablePrefix);

    mConfigurationFile.getAttributeValue("smartmet.engine.grid.content-server.content-source.http.url", mContentSourceHttpUrl);

    mConfigurationFile.getAttributeValue("smartmet.engine.grid.content-server.content-source.corba.ior", mContentSourceCorbaIor);

    mConfigurationFile.getAttributeValue("smartmet.engine.grid.content-server.content-source.file.contentDir", mMemoryContentDir);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.content-server.content-source.file.contentSortingFlags", mMemoryContentSortingFlags);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.content-server.content-source.file.eventListMaxSize", mEventListMaxSize);

    mConfigurationFile.getAttributeValue("smartmet.engine.grid.content-server.cache.enabled", mContentCacheEnabled);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.content-server.cache.contentSortingFlags", mContentCacheSortingFlags);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.content-server.cache.requestForwardEnabled", mRequestForwardEnabled);

    mConfigurationFile.getAttributeValue("smartmet.engine.grid.content-server.processing-log.enabled", mContentServerProcessingLogEnabled);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.content-server.processing-log.file", mContentServerProcessingLogFile);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.content-server.processing-log.maxSize", mContentServerProcessingLogMaxSize);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.content-server.processing-log.truncateSize", mContentServerProcessingLogTruncateSize);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.content-server.debug-log.enabled", mContentServerDebugLogEnabled);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.content-server.debug-log.file", mContentServerDebugLogFile);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.content-server.debug-log.maxSize", mContentServerDebugLogMaxSize);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.content-server.debug-log.truncateSize", mContentServerDebugLogTruncateSize);

    mConfigurationFile.getAttributeValue("smartmet.engine.grid.data-server.remote", mDataServerRemote);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.data-server.ior", mDataServerIor);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.data-server.caching", mDataServerCacheEnabled);

    // These settings are used when the data server is embedded into the grid engine.
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.data-server.grid-storage.memoryMapCheckEnabled",mMemoryMapCheckEnabled);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.data-server.grid-storage.preloadEnabled",mContentPreloadEnabled);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.data-server.grid-storage.preloadFile",mContentPreloadFile);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.data-server.grid-storage.preloadMemoryLock",mPreloadMemoryLock);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.data-server.grid-storage.directory", mDataServerGridDirectory);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.data-server.virtualFiles.enabled",mVirtualFilesEnabled);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.data-server.virtualFiles.definitionFile",mVirtualFileDefinitions);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.data-server.luaFiles",mDataServerLuaFiles);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.data-server.processing-log.enabled", mDataServerProcessingLogEnabled);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.data-server.processing-log.file", mDataServerProcessingLogFile);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.data-server.processing-log.maxSize", mDataServerProcessingLogMaxSize);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.data-server.processing-log.truncateSize", mDataServerProcessingLogTruncateSize);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.data-server.debug-log.enabled", mDataServerDebugLogEnabled);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.data-server.debug-log.file", mDataServerDebugLogFile);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.data-server.debug-log.maxSize", mDataServerDebugLogMaxSize);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.data-server.debug-log.truncateSize", mDataServerDebugLogTruncateSize);


    mConfigurationFile.getAttributeValue("smartmet.engine.grid.query-server.remote", mQueryServerRemote);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.query-server.ior", mQueryServerIor);

    mConfigurationFile.getAttributeValue("smartmet.engine.grid.query-server.queryCache.enabled", mQueryCacheEnabled);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.query-server.queryCache.maxAge", mQueryCacheMaxAge);


    // These settings are used when the query server is embedded into the grid engine.
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.query-server.producerFile",mProducerFile);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.query-server.producerAliasFiles",mProducerAliasFiles);

    mConfigurationFile.getAttributeValue("smartmet.engine.grid.query-server.processing-log.enabled", mQueryServerProcessingLogEnabled);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.query-server.processing-log.file", mQueryServerProcessingLogFile);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.query-server.processing-log.maxSize", mQueryServerProcessingLogMaxSize);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.query-server.processing-log.truncateSize", mQueryServerProcessingLogTruncateSize);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.query-server.debug-log.enabled", mQueryServerDebugLogEnabled);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.query-server.debug-log.file", mQueryServerDebugLogFile);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.query-server.debug-log.maxSize", mQueryServerDebugLogMaxSize);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.query-server.debug-log.truncateSize", mQueryServerDebugLogTruncateSize);

    int tmp = 0;
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.query-server.mappingTargetKeyType",tmp);
    mMappingTargetKeyType = C_UCHAR(tmp);

    mConfigurationFile.getAttributeValue("smartmet.engine.grid.query-server.mappingUpdateFile.fmi",mParameterMappingUpdateFile_fmi);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.query-server.mappingUpdateFile.newbase",mParameterMappingUpdateFile_newbase);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.query-server.mappingFiles",mParameterMappingFiles);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.query-server.aliasFiles",mParameterAliasFiles);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.query-server.luaFiles",mQueryServerLuaFiles);


    mProducerFile_modificationTime = getFileModificationTime(mProducerFile.c_str());

    // Initializing information that is needed for identifying the content of the grid files.

    SmartMet::Identification::gridDef.init(mGridConfigFile.c_str());
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Constructor failed!", nullptr);
    exception.addParameter("Configuration file",mConfigurationFile.getFilename());
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
    ContentServer::ServiceInterface *cServer = nullptr;
    DataServer::ServiceInterface *dServer = nullptr;
    QueryServer::ServiceInterface *qServer = nullptr;

    clearMappings();

    if (mContentSourceType == "redis")
    {
      ContentServer::RedisImplementation *redis = new ContentServer::RedisImplementation();
      redis->init(mContentSourceRedisAddress.c_str(),mContentSourceRedisPort,mContentSourceRedisTablePrefix.c_str());
      mContentServer.reset(redis);
      cServer = redis;
    }
    else
    if (mContentSourceType == "corba")
    {
      ContentServer::Corba::ClientImplementation *client = new ContentServer::Corba::ClientImplementation();
      client->init(mContentSourceCorbaIor.c_str());
      mContentServer.reset(client);
      cServer = client;
    }
    else
    if (mContentSourceType == "http")
    {
      ContentServer::HTTP::ClientImplementation *client = new ContentServer::HTTP::ClientImplementation();
      client->init(mContentSourceHttpUrl.c_str());
      mContentServer.reset(client);
      cServer = client;
    }
    else
    if (mContentSourceType == "file")
    {
      bool eventstEnabled = true;
      if (mEventListMaxSize == 0)
      {
        eventstEnabled = false;
        mContentCacheEnabled = false;
      }

      ContentServer::MemoryImplementation *memoryImplementation = new ContentServer::MemoryImplementation();
      memoryImplementation->init(true,false,true,eventstEnabled,mMemoryContentDir,0,mMemoryContentSortingFlags);
      memoryImplementation->setEventListMaxLength(mEventListMaxSize);
      mContentServer.reset(memoryImplementation);
      cServer = memoryImplementation;
    }
    else
    {
      Fmi::Exception exception(BCP, "Unknow content source type!");
      exception.addParameter("Content source type",mContentSourceType);
    }

    if (mContentCacheEnabled)
    {
      mContentServerCacheImplementation = new ContentServer::CacheImplementation();
      mContentServerCacheImplementation->init(0,cServer,mContentCacheSortingFlags);
      mContentServerCacheImplementation->setRequestForwardEnabled(mRequestForwardEnabled);
      mContentServerCache.reset(mContentServerCacheImplementation);
      mContentServerCacheImplementation->startEventProcessing();
      cServer = mContentServerCacheImplementation;
    }

    if (mDataServerRemote  &&  mDataServerIor.length() > 50)
    {
      DataServer::Corba::ClientImplementation *client = new DataServer::Corba::ClientImplementation();
      client->init(mDataServerIor);

      if (mDataServerCacheEnabled)
      {
        DataServer::CacheImplementation *serverCache = new DataServer::CacheImplementation();
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
      mDataServerImplementation->init(0,0,"NotRegistered","NotRegistered",mDataServerGridDirectory,cServer,mDataServerLuaFiles);
      mDataServerImplementation->setPointCacheEnabled(mPointCacheEnabled,mPointCacheHitsRequired,mPointCacheTimePeriod);
      mDataServerImplementation->setPreload(mContentPreloadEnabled,mPreloadMemoryLock,mContentPreloadFile);
      mDataServerImplementation->setMemoryMapCheckEnabled(mMemoryMapCheckEnabled);

      if (mVirtualFilesEnabled)
      {
        mDataServerImplementation->setVirtualContentEnabled(true);
        DataServer::VirtualContentFactory_type1 *factory = new DataServer::VirtualContentFactory_type1();
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

      SmartMet::GRID::valueCache.init(mNumOfCachedGrids,mMaxSizeOfCachedGridsInMegaBytes);
    }

    if (mQueryServerRemote  &&  mQueryServerIor.length() > 50)
    {
      QueryServer::Corba::ClientImplementation *client = new QueryServer::Corba::ClientImplementation();
      client->init(mQueryServerIor);
      mQueryServer.reset(client);
      qServer = client;
    }
    else
    {
      QueryServer::ServiceImplementation *server = new QueryServer::ServiceImplementation();
      server->init(cServer,dServer,mGridConfigFile,mParameterMappingFiles,mParameterAliasFiles,mProducerFile,mProducerAliasFiles,mQueryServerLuaFiles);
      qServer = server;

      mQueryServer.reset(server);
    }


    if (mContentServerProcessingLogEnabled &&  mContentServerProcessingLogFile.length() > 0)
    {
      mContentServerProcessingLog.init(true,mContentServerProcessingLogFile.c_str(),mContentServerProcessingLogMaxSize,mContentServerProcessingLogTruncateSize);
      cServer->setProcessingLog(&mContentServerProcessingLog);
    }

    if (mContentServerDebugLogEnabled && mContentServerDebugLogFile.length() > 0)
    {
      mContentServerDebugLog.init(true,mContentServerDebugLogFile.c_str(),mContentServerDebugLogMaxSize,mContentServerDebugLogTruncateSize);
      cServer->setDebugLog(&mContentServerDebugLog);
    }

    if (mDataServerProcessingLogEnabled && mDataServerProcessingLogFile.length() > 0)
    {
      mDataServerProcessingLog.init(true,mDataServerProcessingLogFile.c_str(),mDataServerProcessingLogMaxSize,mDataServerProcessingLogTruncateSize);
      dServer->setProcessingLog(&mDataServerProcessingLog);
    }

    if (mDataServerDebugLogEnabled && mDataServerDebugLogFile.length() > 0)
    {
      mDataServerDebugLog.init(true,mDataServerDebugLogFile.c_str(),mDataServerDebugLogMaxSize,mDataServerDebugLogTruncateSize);
      dServer->setDebugLog(&mDataServerDebugLog);
    }

    if (mQueryServerProcessingLogEnabled && mQueryServerProcessingLogFile.length() > 0)
    {
      mQueryServerProcessingLog.init(true,mQueryServerProcessingLogFile.c_str(),mQueryServerProcessingLogMaxSize,mQueryServerProcessingLogTruncateSize);
      qServer->setProcessingLog(&mQueryServerProcessingLog);
    }

    if (mQueryServerDebugLogEnabled && mQueryServerDebugLogFile.length() > 0)
    {
      mQueryServerDebugLog.init(true,mQueryServerDebugLogFile.c_str(),mQueryServerDebugLogMaxSize,mQueryServerDebugLogTruncateSize);
      qServer->setDebugLog(&mQueryServerDebugLog);
    }

    updateProducerAndGenerationList();

    mProducerAliasFileCollection.init(mProducerAliasFiles,true);
    mParameterAliasFileCollection.init(mParameterAliasFiles);

    startUpdateProcessing();
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file",mConfigurationFile.getFilename());
    throw exception;
  }
}





void Engine::checkConfiguration()
{
  FUNCTION_TRACE
  try
  {
    // ### Configuration updates when the server is running.

    time_t currentTime = time(nullptr);
    if ((currentTime - mConfigurationFilename_checkTime) < 60)
      return;

    mConfigurationFilename_checkTime = currentTime;

    time_t tt = getFileModificationTime(mConfigurationFilename.c_str());
    if (tt == mConfigurationFilename_modificationTime)
      return;

    ConfigurationFile configurationFile;
    configurationFile.readFile(mConfigurationFilename.c_str());


    ContentServer_sptr contentServer = getContentServer_sptr();

    // ### Content server processing log

    bool contentServerProcessingLogEnabled = false;
    std::string contentServerProcessingLogFile;
    int contentServerProcessingLogMaxSize = 0;
    int contentServerProcessingLogTruncateSize = 0;

    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.processing-log.enabled", contentServerProcessingLogEnabled);
    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.processing-log.file", contentServerProcessingLogFile);
    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.processing-log.maxSize", contentServerProcessingLogMaxSize);
    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.processing-log.truncateSize", contentServerProcessingLogTruncateSize);

    if (mContentServerProcessingLogEnabled != contentServerProcessingLogEnabled || mContentServerProcessingLogFile != contentServerProcessingLogFile ||
        mContentServerProcessingLogMaxSize != contentServerProcessingLogMaxSize || mContentServerProcessingLogTruncateSize != contentServerProcessingLogTruncateSize)
    {
      mContentServerProcessingLog.close();

      mContentServerProcessingLogEnabled = contentServerProcessingLogEnabled;
      mContentServerProcessingLogFile = contentServerProcessingLogFile;
      mContentServerProcessingLogMaxSize = contentServerProcessingLogMaxSize;
      mContentServerProcessingLogTruncateSize = contentServerProcessingLogTruncateSize;

      mContentServerProcessingLog.init(mContentServerProcessingLogEnabled,mContentServerProcessingLogFile.c_str(),mContentServerProcessingLogMaxSize,mContentServerProcessingLogTruncateSize);
      if (contentServer->getProcessingLog() == nullptr)
        contentServer->setProcessingLog(&mContentServerProcessingLog);
    }


    // ### Content server debug log

    bool contentServerDebugLogEnabled = false;
    std::string contentServerDebugLogFile;
    int contentServerDebugLogMaxSize = 0;
    int contentServerDebugLogTruncateSize = 0;

    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.debug-log.enabled", mContentServerDebugLogEnabled);
    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.debug-log.file", mContentServerDebugLogFile);
    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.debug-log.maxSize", mContentServerDebugLogMaxSize);
    configurationFile.getAttributeValue("smartmet.engine.grid.content-server.debug-log.truncateSize", mContentServerDebugLogTruncateSize);

    if (mContentServerDebugLogEnabled != contentServerDebugLogEnabled || mContentServerDebugLogFile != contentServerDebugLogFile ||
        mContentServerDebugLogMaxSize != contentServerDebugLogMaxSize || mContentServerDebugLogTruncateSize != contentServerDebugLogTruncateSize)
    {
      mContentServerDebugLog.close();

      mContentServerDebugLogEnabled = contentServerDebugLogEnabled;
      mContentServerDebugLogFile = contentServerDebugLogFile;
      mContentServerDebugLogMaxSize = contentServerDebugLogMaxSize;
      mContentServerDebugLogTruncateSize = contentServerDebugLogTruncateSize;

      mContentServerDebugLog.init(mContentServerDebugLogEnabled,mContentServerDebugLogFile.c_str(),mContentServerDebugLogMaxSize,mContentServerDebugLogTruncateSize);
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

    if (mDataServerProcessingLogEnabled != dataServerProcessingLogEnabled || mDataServerProcessingLogFile != dataServerProcessingLogFile ||
        mDataServerProcessingLogMaxSize != dataServerProcessingLogMaxSize || mDataServerProcessingLogTruncateSize != dataServerProcessingLogTruncateSize)
    {
      mDataServerProcessingLog.close();

      mDataServerProcessingLogEnabled = dataServerProcessingLogEnabled;
      mDataServerProcessingLogFile = dataServerProcessingLogFile;
      mDataServerProcessingLogMaxSize = dataServerProcessingLogMaxSize;
      mDataServerProcessingLogTruncateSize = dataServerProcessingLogTruncateSize;

      mDataServerProcessingLog.init(mDataServerProcessingLogEnabled,mDataServerProcessingLogFile.c_str(),mDataServerProcessingLogMaxSize,mDataServerProcessingLogTruncateSize);
      if (mDataServer->getProcessingLog() == nullptr)
        mDataServer->setProcessingLog(&mDataServerProcessingLog);
    }


    // ### Data server debug log

    bool dataServerDebugLogEnabled = false;
    std::string dataServerDebugLogFile;
    int dataServerDebugLogMaxSize = 0;
    int dataServerDebugLogTruncateSize = 0;

    configurationFile.getAttributeValue("smartmet.engine.grid.data-server.debug-log.enabled", mDataServerDebugLogEnabled);
    configurationFile.getAttributeValue("smartmet.engine.grid.data-server.debug-log.file", mDataServerDebugLogFile);
    configurationFile.getAttributeValue("smartmet.engine.grid.data-server.debug-log.maxSize", mDataServerDebugLogMaxSize);
    configurationFile.getAttributeValue("smartmet.engine.grid.data-server.debug-log.truncateSize", mDataServerDebugLogTruncateSize);

    if (mDataServerDebugLogEnabled != dataServerDebugLogEnabled || mDataServerDebugLogFile != dataServerDebugLogFile ||
        mDataServerDebugLogMaxSize != dataServerDebugLogMaxSize || mDataServerDebugLogTruncateSize != dataServerDebugLogTruncateSize)
    {
      mDataServerDebugLog.close();

      mDataServerDebugLogEnabled = dataServerDebugLogEnabled;
      mDataServerDebugLogFile = dataServerDebugLogFile;
      mDataServerDebugLogMaxSize = dataServerDebugLogMaxSize;
      mDataServerDebugLogTruncateSize = dataServerDebugLogTruncateSize;

      mDataServerDebugLog.init(mDataServerDebugLogEnabled,mDataServerDebugLogFile.c_str(),mDataServerDebugLogMaxSize,mDataServerDebugLogTruncateSize);
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

    if (mQueryServerProcessingLogEnabled != queryServerProcessingLogEnabled || mQueryServerProcessingLogFile != queryServerProcessingLogFile ||
        mQueryServerProcessingLogMaxSize != queryServerProcessingLogMaxSize || mQueryServerProcessingLogTruncateSize != queryServerProcessingLogTruncateSize)
    {
      mQueryServerProcessingLog.close();

      mQueryServerProcessingLogEnabled = queryServerProcessingLogEnabled;
      mQueryServerProcessingLogFile = queryServerProcessingLogFile;
      mQueryServerProcessingLogMaxSize = queryServerProcessingLogMaxSize;
      mQueryServerProcessingLogTruncateSize = queryServerProcessingLogTruncateSize;

      mQueryServerProcessingLog.init(mQueryServerProcessingLogEnabled,mQueryServerProcessingLogFile.c_str(),mQueryServerProcessingLogMaxSize,mQueryServerProcessingLogTruncateSize);
      if (mQueryServer->getProcessingLog() == nullptr)
        mQueryServer->setProcessingLog(&mQueryServerProcessingLog);
    }


    // ### Query server debug log

    bool queryServerDebugLogEnabled = false;
    std::string queryServerDebugLogFile;
    int queryServerDebugLogMaxSize = 0;
    int queryServerDebugLogTruncateSize = 0;

    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.debug-log.enabled", mQueryServerDebugLogEnabled);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.debug-log.file", mQueryServerDebugLogFile);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.debug-log.maxSize", mQueryServerDebugLogMaxSize);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.debug-log.truncateSize", mQueryServerDebugLogTruncateSize);

    if (mQueryServerDebugLogEnabled != queryServerDebugLogEnabled || mQueryServerDebugLogFile != queryServerDebugLogFile ||
        mQueryServerDebugLogMaxSize != queryServerDebugLogMaxSize || mQueryServerDebugLogTruncateSize != queryServerDebugLogTruncateSize)
    {
      mQueryServerDebugLog.close();

      mQueryServerDebugLogEnabled = queryServerDebugLogEnabled;
      mQueryServerDebugLogFile = queryServerDebugLogFile;
      mQueryServerDebugLogMaxSize = queryServerDebugLogMaxSize;
      mQueryServerDebugLogTruncateSize = queryServerDebugLogTruncateSize;

      mQueryServerDebugLog.init(mQueryServerDebugLogEnabled,mQueryServerDebugLogFile.c_str(),mQueryServerDebugLogMaxSize,mQueryServerDebugLogTruncateSize);
      if (mQueryServer->getDebugLog() == nullptr)
        mQueryServer->setDebugLog(&mQueryServerDebugLog);
    }


    // ### Query cache

    bool queryCacheEnabled = false;
    int queryCacheMaxAge = 0;

    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.queryCache.enabled", queryCacheEnabled);
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.queryCache.maxAge", queryCacheMaxAge);

    if (mQueryCacheEnabled != queryCacheEnabled  || mQueryCacheMaxAge != queryCacheMaxAge)
    {
      mQueryCacheEnabled = queryCacheEnabled;
      mQueryCacheMaxAge = queryCacheMaxAge;
    }



    if (mDataServerImplementation != nullptr)
    {
      // ### Point cache

      bool pointCacheEnabled = false;
      uint pointCacheHitsRequired = 0;
      uint pointCacheTimePeriod = 0;

      configurationFile.getAttributeValue("smartmet.library.grid-files.pointCache.enabled", pointCacheEnabled);
      configurationFile.getAttributeValue("smartmet.library.grid-files.pointCache.hitsRequired", pointCacheHitsRequired);
      configurationFile.getAttributeValue("smartmet.library.grid-files.pointCache.timePeriod", pointCacheTimePeriod);

      if (mPointCacheEnabled != pointCacheEnabled || mPointCacheHitsRequired != pointCacheHitsRequired || mPointCacheTimePeriod != pointCacheTimePeriod)
      {
        mPointCacheEnabled = pointCacheEnabled;
        mPointCacheHitsRequired = pointCacheHitsRequired;
        mPointCacheTimePeriod = pointCacheTimePeriod;

        mDataServerImplementation->setPointCacheEnabled(mPointCacheEnabled,mPointCacheHitsRequired,mPointCacheTimePeriod);
      }


      // ### Memory map check

      bool memoryMapCheckEnabled = false;

      mConfigurationFile.getAttributeValue("smartmet.engine.grid.data-server.grid-storage.memoryMapCheckEnabled",memoryMapCheckEnabled);
      if (mMemoryMapCheckEnabled != memoryMapCheckEnabled)
      {
        mMemoryMapCheckEnabled = memoryMapCheckEnabled;
        mDataServerImplementation->setMemoryMapCheckEnabled(mMemoryMapCheckEnabled);
      }


      // ### Preload

      bool contentPreloadEnabled = false;
      std::string contentPreloadFile;
      bool preloadMemoryLock;

      configurationFile.getAttributeValue("smartmet.engine.grid.data-server.grid-storage.preloadEnabled",contentPreloadEnabled);
      configurationFile.getAttributeValue("smartmet.engine.grid.data-server.grid-storage.preloadFile",contentPreloadFile);
      configurationFile.getAttributeValue("smartmet.engine.grid.data-server.grid-storage.preloadMemoryLock",preloadMemoryLock);

      if (mContentPreloadEnabled != contentPreloadEnabled || mContentPreloadFile != contentPreloadFile || mPreloadMemoryLock || preloadMemoryLock)
      {
        mContentPreloadEnabled = contentPreloadEnabled;
        mContentPreloadFile = contentPreloadFile;
        mPreloadMemoryLock = preloadMemoryLock;
        mDataServerImplementation->setPreload(mContentPreloadEnabled,mPreloadMemoryLock,mContentPreloadFile);
      }
    }

    mConfigurationFilename_modificationTime = tt;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Constructor failed!", nullptr);
    exception.addParameter("Configuration file",mConfigurationFile.getFilename());
    throw exception;
  }
}





void Engine::shutdown()
{
  FUNCTION_TRACE
  try
  {
    std::cout << "  -- Shutdown requested (grid engine)\n";
    mShutdownRequested = true;

    if (!mContentServer)
      mContentServer->shutdown();

    if (!mContentServerCache)
      mContentServerCache->shutdown();

    if (!mDataServer)
      mDataServer->shutdown();

    if (!mQueryServer)
      mQueryServer->shutdown();
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file",mConfigurationFile.getFilename());
    throw exception;
  }
}





int Engine::executeQuery(QueryServer::Query& query) const
{
  FUNCTION_TRACE
  try
  {
    return mQueryServer->executeQuery(0,query);
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file",mConfigurationFile.getFilename());
    throw exception;
  }
}





Query_sptr Engine::executeQuery(Query_sptr query) const
{
  FUNCTION_TRACE
  try
  {
    if (!mQueryCacheEnabled)
    {
      int result = mQueryServer->executeQuery(0,*query);
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
      AutoReadLock lock(&mQueryCacheModificationLock);
      it = mQueryCache.find(hash);
      if (it != mQueryCache.end())
      {
        for (auto prod = it->second.producerHashMap.begin(); prod != it->second.producerHashMap.end()  && !noMatch; ++prod)
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
    }

    int result = mQueryServer->executeQuery(0,*query);
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

      std::set<uint> producerIdList;
      query->getResultProducerIdList(producerIdList);

      for (auto it = producerIdList.begin(); it != producerIdList.end(); ++it)
      {
        ulonglong producerHash = getProducerHash(*it);
        rec.producerHashMap.insert(std::pair<uint,ulonglong>(*it,producerHash));
      }

      AutoWriteLock lock(&mQueryCacheModificationLock);
      mQueryCache.insert(std::pair<std::size_t,CacheRec>(hash,rec));
    }
    return query;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file",mConfigurationFile.getFilename());
    throw exception;
  }
}





bool Engine::isCacheable(std::shared_ptr<QueryServer::Query> query) const
{
  FUNCTION_TRACE
  try
  {
    for (auto param = query->mQueryParameterList.begin(); param != query->mQueryParameterList.end(); ++param)
    {
      switch(param->mType)
      {
        case QueryServer::QueryParameter::Type::PointValues:
        case QueryServer::QueryParameter::Type::Isoline:
        case QueryServer::QueryParameter::Type::Isoband:
          break;

        case QueryServer::QueryParameter::Type::Vector:
        case QueryServer::QueryParameter::Type::GridFile:
          return false;
      }

      switch(param->mLocationType)
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
    exception.addParameter("Configuration file",mConfigurationFile.getFilename());
    throw exception;
  }
}




ContentServer_sptr Engine::getContentServer_sptr() const
{
  FUNCTION_TRACE
  try
  {
    if (mContentCacheEnabled)
      return mContentServerCache;
    else
      return mContentServer;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file",mConfigurationFile.getFilename());
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
    exception.addParameter("Configuration file",mConfigurationFile.getFilename());
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
    exception.addParameter("Configuration file",mConfigurationFile.getFilename());
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
    exception.addParameter("Configuration file",mConfigurationFile.getFilename());
    throw exception;
  }
}





bool Engine::isGridProducer(const std::string& producer) const
{
  FUNCTION_TRACE
  try
  {
    AutoReadLock lock(&mProducerListModificationLock);

    std::vector<std::string> nameList;
    getProducerNameList(producer,nameList);
    for (auto it=nameList.begin(); it!=nameList.end(); ++it)
    {
      for (auto itm = mProducerList.begin(); itm != mProducerList.end(); ++itm)
      {
        if (strcasecmp(it->c_str(),itm->c_str()) == 0)
          return true;
      }
    }

    return false;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file",mConfigurationFile.getFilename());
    throw exception;
  }
}





std::string Engine::getParameterString(std::string producer,std::string parameter) const
{
  FUNCTION_TRACE
  try
  {
    std::string key = producer + ";" + parameter;

    ParameterDetails_vec parameters;
    getParameterDetails(producer,parameter,parameters);

    std::string prod;
    std::string geomId;
    std::string level;
    std::string levelId;
    std::string forecastType;
    std::string forecastNumber;

    size_t len = parameters.size();

    if (len > 0  &&  strcasecmp(parameters[0].mProducerName.c_str(),key.c_str()) != 0)
    {
      for (size_t t = 0; t < len; t++)
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
      std::string paramStr = parameter + ":" + prod + ":" + geomId + ":" + levelId + ":" + level+ ":" + forecastType + ":" + forecastNumber;
      return paramStr;
    }

    return parameter;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file",mConfigurationFile.getFilename());
    throw exception;
  }
}





std::string Engine::getProducerName(const std::string& aliasName) const
{
  FUNCTION_TRACE
  try
  {
    mProducerAliasFileCollection.checkUpdates(false);

    std::vector<std::string> aliasStrings;
    mProducerAliasFileCollection.getAliasList(aliasName,aliasStrings);

    // Removing the level type information from the alias names.

    for (auto it=aliasStrings.begin(); it != aliasStrings.end(); it++)
    {
      std::vector<std::string> partList;
      splitString(*it,';',partList);
      if (partList.size() == 1)
        return partList[0];
    }

     return aliasName;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file",mConfigurationFile.getFilename());
    throw exception;
  }
}




void Engine::getProducerNameList(const std::string& aliasName,std::vector<std::string>& nameList) const
{
  FUNCTION_TRACE
  try
  {
    mProducerAliasFileCollection.checkUpdates(false);

    std::vector<std::string> aliasStrings;
    mProducerAliasFileCollection.getAliasList(aliasName,aliasStrings);

    // Removing the level type information from the alias names.

    for (auto it=aliasStrings.begin(); it != aliasStrings.end(); it++)
    {
      std::vector<std::string> partList;
      splitString(*it,';',partList);
      nameList.push_back(partList[0]);
    }

    if (nameList.size() == 0)
      nameList.push_back(aliasName);
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file",mConfigurationFile.getFilename());
    throw exception;
  }
}





ulonglong Engine::getProducerHash(uint producerId) const
{
  FUNCTION_TRACE
  try
  {
    ContentServer_sptr contentServer = getContentServer_sptr();
    time_t currentTime = time(nullptr);
    ulonglong hash = 0;

    auto rec = mProducerHashMap.find(producerId);
    if (rec != mProducerHashMap.end())
    {
      if ((currentTime - rec->second.checkTime) > 120)
      {
        rec->second.checkTime = currentTime;
        int result = contentServer->getHashByProducerId(0,producerId,hash);
        if (result == 0)
          rec->second.hash = hash;
        else
          rec->second.hash = 0;
      }
      return rec->second.hash;
    }

    int result = contentServer->getHashByProducerId(0,producerId,hash);
    if (result == 0)
    {
      HashRec hrec;
      hrec.checkTime = currentTime;
      hrec.hash = hash;

      mProducerHashMap.insert(std::pair<uint,HashRec>(producerId,hrec));
      return hash;
    }

    return 0;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file",mConfigurationFile.getFilename());
    throw exception;
  }
}





void Engine::getParameterDetails(const std::string& aliasName,ParameterDetails_vec& parameterDetails) const
{
  FUNCTION_TRACE
  try
  {
    mProducerAliasFileCollection.checkUpdates(false);

    std::vector<std::string> aliasStrings;
    mProducerAliasFileCollection.getAliasList(aliasName,aliasStrings);

    for (auto it=aliasStrings.begin(); it != aliasStrings.end(); it++)
    {
      std::vector<std::string> partList;
      splitString(*it,';',partList);

      ParameterDetails p;
      p.mOriginalProducer = aliasName;

      uint len = partList.size();
      for (uint t=0; t<len; t++)
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

      parameterDetails.push_back(p);
    }


    if (parameterDetails.size() == 0)
    {
      ParameterDetails p;
      p.mOriginalProducer = aliasName;
      p.mProducerName = aliasName;
      parameterDetails.push_back(p);
    }
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file",mConfigurationFile.getFilename());
    throw exception;
  }
}




void Engine::getParameterDetails(const std::string& producerName,const std::string& parameterName,ParameterDetails_vec& parameterDetails) const
{
  FUNCTION_TRACE
  try
  {
    mProducerAliasFileCollection.checkUpdates(false);
    mParameterAliasFileCollection.checkUpdates(false);


    std::string prod = producerName;
    mParameterAliasFileCollection.getAlias(producerName,prod);


    std::string param = parameterName;
    mParameterAliasFileCollection.getAlias(parameterName,param);

    std::string key = prod + ";" + param;

    std::vector<std::string> aliasStrings;
    mProducerAliasFileCollection.getAliasList(key,aliasStrings);

    for (auto it=aliasStrings.begin(); it != aliasStrings.end(); it++)
    {
      std::vector<std::string> partList;
      splitString(*it,';',partList);

      ParameterDetails p;
      p.mOriginalProducer = producerName;
      p.mOriginalParameter = parameterName;

      uint len = partList.size();
      for (uint t=0; t<len; t++)
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

      parameterDetails.push_back(p);
    }


    if (parameterDetails.size() == 0)
    {
      ParameterDetails p;
      p.mOriginalProducer = producerName;
      p.mOriginalParameter = parameterName;
      p.mProducerName = key;
      parameterDetails.push_back(p);
    }
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file",mConfigurationFile.getFilename());
    throw exception;
  }
}





void Engine::getParameterDetails(const std::string& producerName,const std::string& parameterName,std::string& level,ParameterDetails_vec& parameterDetails) const
{
  FUNCTION_TRACE
  try
  {
    getParameterDetails(producerName,parameterName,parameterDetails);
    for (auto it = parameterDetails.begin(); it != parameterDetails.end(); ++it)
      it->mLevel = level;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file",mConfigurationFile.getFilename());
    throw exception;
  }
}





void Engine::getParameterMappings(std::string producerName,std::string parameterName,T::GeometryId geometryId, bool onlySearchEnabled, QueryServer::ParameterMapping_vec& mappings) const
{
  try
  {
    AutoReadLock lock(&mParameterMappingModificationLock);

    for (auto m = mParameterMappings.begin(); m != mParameterMappings.end(); ++m)
    {
      m->getMappings(producerName, parameterName, geometryId, onlySearchEnabled, mappings);
    }
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}





void Engine::getParameterMappings(std::string producerName,std::string parameterName,bool onlySearchEnabled, QueryServer::ParameterMapping_vec& mappings) const
{
  try
  {
    AutoReadLock lock(&mParameterMappingModificationLock);

    for (auto m = mParameterMappings.begin(); m != mParameterMappings.end(); ++m)
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
    std::string producerName,
    std::string parameterName,
    T::GeometryId geometryId,
    T::ParamLevelIdType levelIdType,
    T::ParamLevelId levelId,
    T::ParamLevel level,
    bool onlySearchEnabled,
    QueryServer::ParameterMapping_vec& mappings) const
{
  try
  {
    AutoReadLock lock(&mParameterMappingModificationLock);

    for (auto m = mParameterMappings.begin(); m != mParameterMappings.end(); ++m)
    {
      m->getMappings(producerName, parameterName, geometryId, levelIdType, levelId, level, onlySearchEnabled, mappings);
    }
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}





void Engine::getParameterMappings(
    std::string producerName,
    std::string parameterName,
    T::ParamLevelIdType levelIdType,
    T::ParamLevelId levelId,
    T::ParamLevel level,
    bool onlySearchEnabled,
    QueryServer::ParameterMapping_vec& mappings) const
{
  try
  {
    AutoReadLock lock(&mParameterMappingModificationLock);

    for (auto m = mParameterMappings.begin(); m != mParameterMappings.end(); ++m)
    {
      m->getMappings(producerName, parameterName, levelIdType, levelId, level, onlySearchEnabled, mappings);
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
    ContentServer_sptr contentServer = getContentServer_sptr();

    for (auto rec = parameterDetails.begin(); rec != parameterDetails.end(); ++rec)
    {
      QueryServer::ParameterMapping_vec mappings;
      if (rec->mLevelId > " " || rec->mLevel > " ")
      {
        getParameterMappings(rec->mProducerName, rec->mOriginalParameter, atoi(rec->mGeometryId.c_str()), T::ParamLevelIdTypeValue::ANY, atoi(rec->mLevelId.c_str()), atoi(rec->mLevel.c_str()), false, mappings);
        if (mappings.size() == 0  &&  rec->mLevel < " ")
        {
          getParameterMappings(rec->mProducerName, rec->mOriginalParameter, atoi(rec->mGeometryId.c_str()), T::ParamLevelIdTypeValue::ANY, atoi(rec->mLevelId.c_str()), -1, false, mappings);
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
        int result = contentServer->getContentListByParameterAndProducerName(0,m->mProducerName,m->mParameterKeyType,m->mParameterKey,m->mParameterLevelIdType,m->mParameterLevelId,m->mParameterLevel,m->mParameterLevel,-1,-1,m->mGeometryId,std::string("19000101T000000"),std::string("21000101T0000"),0,contentInfoList);
        if (result == 0)
        {
          uint len = contentInfoList.getLength();
          AutoReadLock lock(&mProducerListModificationLock);
          for (uint t=0; t<len; t++)
          {
            T::ContentInfo *cInfo = contentInfoList.getContentInfoByIndex(t);
            if (cInfo != nullptr)
            {
              T::GenerationInfo *gInfo = mGenerationList.getGenerationInfoById(cInfo->mGenerationId);
              if (gInfo != nullptr)
              {
                auto tt = details.mTimes.find(gInfo->mAnalysisTime);
                if (tt != details.mTimes.end())
                {
                  tt->second.insert(cInfo->mForecastTime);
                }
                else
                {
                  std::set<std::string> ttt;
                  ttt.insert(cInfo->mForecastTime);
                  details.mTimes.insert(std::pair<std::string,std::set<std::string>>(gInfo->mAnalysisTime,ttt));
                }
              }
            }
          }
        }

        rec->mMappings.push_back(details);
      }
    }
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file",mConfigurationFile.getFilename());
    throw exception;
  }
}





std::string Engine::getProducerAlias(const std::string& producerName,int levelId) const
{
  FUNCTION_TRACE
  try
  {
    mParameterAliasFileCollection.checkUpdates(false);

    std::string prod = producerName;

    std::string tmp = producerName;
    if (levelId >= 0)
      tmp = producerName + ";" + std::to_string(levelId);

    mParameterAliasFileCollection.getAlias(tmp,prod);
    return prod;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file",mConfigurationFile.getFilename());
    throw exception;
  }
}





T::ParamLevelId Engine::getFmiParameterLevelId(uint producerId,int level) const
{
  FUNCTION_TRACE
  try
  {
    AutoReadLock lock(&mProducerListModificationLock);

    uint len = mLevelInfoList.getLength();
    for (uint t=0; t<len; t++)
    {
      T::LevelInfo *levelInfo = mLevelInfoList.getLevelInfoByIndex(t);
      if (levelInfo != nullptr  &&  levelInfo->mProducerId == producerId)
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
    exception.addParameter("Configuration file",mConfigurationFile.getFilename());
    throw exception;
  }
}





void Engine::getProducerList(string_vec& producerList) const
{
  FUNCTION_TRACE
  try
  {
    mQueryServer->getProducerList(0,producerList);
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file",mConfigurationFile.getFilename());
    throw exception;
  }
}





bool Engine::getProducerInfoByName(const std::string& name,T::ProducerInfo& info) const
{
  FUNCTION_TRACE
  try
  {
    AutoReadLock lock(&mProducerListModificationLock);
    T::ProducerInfo *producerInfo = mProducerInfoList.getProducerInfoByName(name);
    if (producerInfo != nullptr)
    {
      info = *producerInfo;
      return true;
    }
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}





void Engine::getProducerParameterLevelList(const std::string& producerName,T::ParamLevelId fmiParamLevelId,double multiplier,std::set<double>& levels) const
{
  FUNCTION_TRACE
  try
  {
    AutoReadLock lock(&mProducerListModificationLock);

    std::vector<std::string> nameList;
    getProducerNameList(producerName,nameList);

    for (auto pname = nameList.begin(); pname != nameList.end(); ++pname)
    {
      T::ProducerInfo producerInfo;
      if (getProducerInfoByName(*pname,producerInfo))
      {
        std::string fmiParameterName;
        uint len = mLevelInfoList.getLength();
        for (uint t=0; t<len; t++)
        {
          T::LevelInfo *levelInfo = mLevelInfoList.getLevelInfoByIndex(t);
          if (levelInfo != nullptr  &&  levelInfo->mProducerId == producerInfo.mProducerId  &&  levelInfo->mFmiParameterLevelId == fmiParamLevelId  &&
              (fmiParameterName.empty() || levelInfo->mFmiParameterName == fmiParameterName))
          {
            fmiParameterName = levelInfo->mFmiParameterName;
            levels.insert(levelInfo->mParameterLevel*multiplier);
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
    exception.addParameter("Configuration file",mConfigurationFile.getFilename());
    throw exception;
  }
}





void Engine::getProducerParameterLevelIdList(const std::string& producerName,std::set<T::ParamLevelId>& levelIdList) const
{
  FUNCTION_TRACE
  try
  {
    AutoReadLock lock(&mProducerListModificationLock);

    std::vector<std::string> nameList;
    getProducerNameList(producerName,nameList);

    for (auto pname = nameList.begin(); pname != nameList.end(); ++pname)
    {
      T::ProducerInfo producerInfo;
      if (getProducerInfoByName(*pname,producerInfo))
      {
        std::string fmiParameterName;
        uint len = mLevelInfoList.getLength();
        for (uint t=0; t<len; t++)
        {
          T::LevelInfo *levelInfo = mLevelInfoList.getLevelInfoByIndex(t);
          if (levelInfo != nullptr  &&  levelInfo->mProducerId == producerInfo.mProducerId)
          {
            if (levelIdList.find(levelInfo->mFmiParameterLevelId) == levelIdList.end())
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
    exception.addParameter("Configuration file",mConfigurationFile.getFilename());
    throw exception;
  }
}




void Engine::loadMappings(QueryServer::ParamMappingFile_vec& parameterMappings)
{
  FUNCTION_TRACE
  try
  {
    for (auto it = mParameterMappingFiles.begin(); it != mParameterMappingFiles.end(); ++it)
    {
      QueryServer::ParameterMappingFile mapping(*it);
      parameterMappings.push_back(mapping);
    }

    for (auto it = parameterMappings.begin(); it != parameterMappings.end(); ++it)
    {
      // Loading parameter mappings if the mapping file exists and it is not empty.
      if (getFileSize(it->getFilename().c_str()) > 0)
        it->init();
    }
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file",mConfigurationFile.getFilename());
    throw exception;
  }
}





void Engine::clearMappings()
{
  FUNCTION_TRACE
  try
  {
    QueryServer::ParamMappingFile_vec parameterMappings;

    if (!mParameterMappingUpdateFile_fmi.empty())
    {
      FILE *file = openMappingFile(mParameterMappingUpdateFile_fmi);
      if (file != nullptr)
        fclose(file);
    }

    if (!mParameterMappingUpdateFile_newbase.empty())
    {
      FILE *file = openMappingFile(mParameterMappingUpdateFile_newbase);
      if (file != nullptr)
        fclose(file);
    }
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file",mConfigurationFile.getFilename());
    throw exception;
  }
}





void Engine::updateMappings()
{
  FUNCTION_TRACE
  try
  {
    time_t currentTime = time(nullptr);

    if ((currentTime - mParameterMappingUpdateTime) < 300)
      return;

    mParameterMappingUpdateTime = currentTime;

    QueryServer::ParamMappingFile_vec parameterMappings;
    loadMappings(parameterMappings);

    if (parameterMappings.size() > 0)
    {
      AutoWriteLock lock(&mParameterMappingModificationLock);
      mParameterMappings = parameterMappings;
    }

    if (!mParameterMappingUpdateFile_fmi.empty())
    {
      updateMappings(T::ParamKeyTypeValue::FMI_NAME,mMappingTargetKeyType,mParameterMappingUpdateFile_fmi,parameterMappings);
    }

    if (!mParameterMappingUpdateFile_newbase.empty())
    {
      updateMappings(T::ParamKeyTypeValue::NEWBASE_NAME,mMappingTargetKeyType,mParameterMappingUpdateFile_newbase,parameterMappings);
    }
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file",mConfigurationFile.getFilename());
    throw exception;
  }
}





FILE* Engine::openMappingFile(const std::string& mappingFile)
{
  FUNCTION_TRACE
  try
  {
    FILE *file = fopen(mappingFile.c_str(),"we");
    if (file == nullptr)
    {
      Fmi::Exception exception(BCP, "Cannot open a mapping file for writing!");
      exception.addParameter("Filaname",mappingFile);
      throw exception;
    }

    fprintf(file,"# This file is automatically generated by the grid engine. The file contains\n");
    fprintf(file,"# mappings for the parameters found from the content server, which do not have\n");
    fprintf(file,"# mappings already defined. The point is that the query server cannot find \n");
    fprintf(file,"# requested parameters without mappings. On the other hand, the order of the mappings\n");
    fprintf(file,"# is also the search order of the parameters that do not contain complete search \n");
    fprintf(file,"# information (parameterIdType,levelIdType,levelId,level,etc.)\n");
    fprintf(file,"# \n");
    fprintf(file,"# If you want to change some of the mappings or their order, then you should move\n");
    fprintf(file,"# them to a permanent mapping file (which is not automatically overridden.\n");
    fprintf(file,"# \n");
    fprintf(file,"# FIELDS:\n");
    fprintf(file,"#  1) Producer name\n");
    fprintf(file,"#  2) Mapping name\n");
    fprintf(file,"#  3) Parameter id type:\n");
    fprintf(file,"#         1 = FMI_ID\n");
    fprintf(file,"#         2 = FMI_NAME\n");
    fprintf(file,"#         3 = GRIB_ID\n");
    fprintf(file,"#         4 = NEWBASE_ID\n");
    fprintf(file,"#         5 = NEWBASE_NAME\n");
    fprintf(file,"#         6 = CDM_ID\n");
    fprintf(file,"#         7 = CDM_NAME\n");
    fprintf(file,"#  4) Parameter id / name\n");
    fprintf(file,"#  5) Geometry id\n");
    fprintf(file,"#  6) Parameter level id type:\n");
    fprintf(file,"#         1 = FMI\n");
    fprintf(file,"#         2 = GRIB1\n");
    fprintf(file,"#         3 = GRIB2\n");
    fprintf(file,"#  7) Level id\n");
    fprintf(file,"#         FMI level identifiers:\n");
    fprintf(file,"#            1 Gound or water surface\n");
    fprintf(file,"#            2 Pressure level\n");
    fprintf(file,"#            3 Hybrid level\n");
    fprintf(file,"#            4 Altitude\n");
    fprintf(file,"#            5 Top of atmosphere\n");
    fprintf(file,"#            6 Height above ground in meters\n");
    fprintf(file,"#            7 Mean sea level\n");
    fprintf(file,"#            8 Entire atmosphere\n");
    fprintf(file,"#            9 Depth below land surface\n");
    fprintf(file,"#            10 Depth below some surface\n");
    fprintf(file,"#            11 Level at specified pressure difference from ground to level\n");
    fprintf(file,"#            12 Max equivalent potential temperature level\n");
    fprintf(file,"#            13 Layer between two metric heights above ground\n");
    fprintf(file,"#            14 Layer between two depths below land surface\n");
    fprintf(file,"#            15 Isothermal level, temperature in 1/100 K\n");
    fprintf(file,"#  8) Level\n");
    fprintf(file,"#  9) Area interpolation method\n");
    fprintf(file,"#         0 = None\n");
    fprintf(file,"#         1 = Linear\n");
    fprintf(file,"#         2 = Nearest\n");
    fprintf(file,"#         3 = Min\n");
    fprintf(file,"#         4 = Max\n");
    fprintf(file,"#         500..999 = List\n");
    fprintf(file,"#         1000..65535 = External (interpolated by an external function)\n");
    fprintf(file,"# 10) Time interpolation method\n");
    fprintf(file,"#         0 = None\n");
    fprintf(file,"#         1 = Linear\n");
    fprintf(file,"#         2 = Nearest\n");
    fprintf(file,"#         3 = Min\n");
    fprintf(file,"#         4 = Max\n");
    fprintf(file,"#         1000..65535 = External (interpolated by an external function)\n");
    fprintf(file,"# 11) Level interpolation method\n");
    fprintf(file,"#         0 = None\n");
    fprintf(file,"#         1 = Linear\n");
    fprintf(file,"#         2 = Nearest\n");
    fprintf(file,"#         3 = Min\n");
    fprintf(file,"#         4 = Max\n");
    fprintf(file,"#         5 = Logarithmic\n");
    fprintf(file,"#         1000..65535 = External (interpolated by an external function)\n");
    fprintf(file,"# 12) Group flags\n");
    fprintf(file,"#         bit 0 = Climatological parameter (=> ignore year when searching) \n");
    fprintf(file,"# 13) Search match (Can this mapping used when searching mappings for incomplete parameters)\n");
    fprintf(file,"#         E = Enabled\n");
    fprintf(file,"#         D = Disabled\n");
    fprintf(file,"#         I = Ignore\n");
    fprintf(file,"# 14) Mapping function (enables data conversions during the mapping)\n");
    fprintf(file,"# 15) Reverse mapping function\n");
    fprintf(file,"# 16) Default precision\n");
    fprintf(file,"# \n");

    return file;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file",mConfigurationFile.getFilename());
    throw exception;
  }
}





void Engine::updateMappings(T::ParamKeyType sourceParameterKeyType,T::ParamKeyType targetParameterKeyType,const std::string& mappingFile,QueryServer::ParamMappingFile_vec& parameterMappings)
{
  FUNCTION_TRACE
  try
  {
    ContentServer_sptr  contentServer = getContentServer_sptr();

    T::SessionId sessionId = 0;
    uint numOfNewMappings = 0;
    std::set<std::string> mapList;
    std::set<std::string> searchList;

    T::ProducerInfoList producerInfoList;
    int result = contentServer->getProducerInfoList(sessionId,producerInfoList);
    if (result != 0)
    {
      std::cerr << __FILE__ << ":" << __LINE__ << ": The 'contentServer.getProducerInfoList()' service call returns an error!  Result : " << result << " : " << ContentServer::getResultString(result).c_str() << "\n";
      return;
    }

    FILE *file = nullptr;

    uint plen = producerInfoList.getLength();
    for (uint t=0; t<plen; t++)
    {
      T::ProducerInfo *producerInfo = producerInfoList.getProducerInfoByIndex(t);
      std::set<std::string> infoList;

      int result = contentServer->getProducerParameterListByProducerId(sessionId,producerInfo->mProducerId,sourceParameterKeyType,targetParameterKeyType,infoList);
      if (result == 0)
      {
        for (auto it=infoList.begin(); it != infoList.end(); ++it)
        {
          std::vector<std::string> pl;
          splitString(it->c_str(),';',pl);
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

            char key[200];
            sprintf(key,"%s;%s;%s;%s;%s;%s;%s;%s;",pl[0].c_str(),pl[1].c_str(),pl[2].c_str(),pl[3].c_str(),pl[4].c_str(),pl[5].c_str(),pl[6].c_str(),pl[7].c_str());
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
                      it->getMappings(m.mProducerName,m.mParameterName,m.mGeometryId,true,vec);
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
                if (!searchEnabled  ||  (m.mParameterLevelId == 6   &&  m.mParameterLevel <= 10) ||  (m.mParameterLevelId == 1  &&  m.mParameterLevel == 0))
                {
                  if (m.mParameterLevelId != 2  &&  m.mParameterLevelId != 3  &&  m.mParameterLevelId != 4)
                    s = 'E';
                }

                if (searchList.find(searchKey) == searchList.end())
                  searchList.insert(searchKey);

                if (file == nullptr)
                  file = openMappingFile(mappingFile);

                fprintf(file,"%s;%s;%s;%s;%s;%s;%s;%s;",pl[0].c_str(),pl[1].c_str(),pl[2].c_str(),pl[3].c_str(),pl[4].c_str(),pl[5].c_str(),pl[6].c_str(),pl[7].c_str());

                Identification::FmiParameterDef paramDef;

                bool found = false;
                if (targetParameterKeyType == T::ParamKeyTypeValue::FMI_NAME)
                  found = Identification::gridDef.getFmiParameterDefByName(pl[3],paramDef);
                else
                if (targetParameterKeyType == T::ParamKeyTypeValue::FMI_ID)
                  found = Identification::gridDef.getFmiParameterDefById(pl[3],paramDef);
                else
                if (targetParameterKeyType == T::ParamKeyTypeValue::NEWBASE_ID)
                  found = Identification::gridDef.getFmiParameterDefByNewbaseId(pl[3],paramDef);

                if (found)
                {
                  if (paramDef.mAreaInterpolationMethod >= 0)
                    fprintf(file,"%d;",paramDef.mAreaInterpolationMethod);
                  else
                    fprintf(file,";");

                  if (paramDef.mTimeInterpolationMethod >= 0)
                    fprintf(file,"%d;",paramDef.mTimeInterpolationMethod);
                  else
                    fprintf(file,";");

                  if (paramDef.mLevelInterpolationMethod >= 0)
                    fprintf(file,"%d;",paramDef.mLevelInterpolationMethod);
                  else
                    fprintf(file,";");

                  fprintf(file,"0;%c;",s);

                  if (sourceParameterKeyType == T::ParamKeyTypeValue::NEWBASE_ID || sourceParameterKeyType == T::ParamKeyTypeValue::NEWBASE_NAME)
                  {
                    Identification::FmiParameterId_newbase paramMapping;
                    if (Identification::gridDef.getNewbaseParameterMappingByFmiId(paramDef.mFmiParameterId,paramMapping))
                    {
                      fprintf(file,"%s;",paramMapping.mConversionFunction.c_str());
                      fprintf(file,"%s;",paramMapping.mReverseConversionFunction.c_str());
                    }
                    else
                    {
                      fprintf(file,";;");
                    }
                  }
                  else
                  {
                    fprintf(file,";;");
                  }

                  if (paramDef.mDefaultPrecision >= 0)
                    fprintf(file,"%d;",(int)paramDef.mDefaultPrecision);
                  else
                    fprintf(file,";");

                  fprintf(file,"\n");
                }
                else
                {
                  fprintf(file,"1;1;1;0;D;;;;\n");
                }
              }
            }
          }
        }
      }
    }

    if (file == nullptr  &&  numOfNewMappings == 0)
    {
      // We found all mappings from the other files. That's why we should remove them
      // from the update file.

      file = openMappingFile(mappingFile);
    }

    if (file != nullptr)
      fclose(file);

  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file",mConfigurationFile.getFilename());
    throw exception;
  }
}





void Engine::updateProcessing()
{
  try
  {
    while (!mShutdownRequested)
    {
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
    exception.addParameter("Configuration file",mConfigurationFile.getFilename());
    throw exception;
  }
}




void Engine::updateProducerAndGenerationList()
{
  FUNCTION_TRACE
  try
  {
    ContentServer_sptr contentServer = getContentServer_sptr();

    AutoWriteLock lock(&mProducerListModificationLock);

    if ((time(nullptr) - mProducerList_updateTime) > 60)
    {
      mProducerList_updateTime = time(nullptr);

      mQueryServer->getProducerList(0,mProducerList);

      // Producers defined in the content server
      ContentServer_sptr contentServer = getContentServer_sptr();
      contentServer->getProducerInfoList(0, mProducerInfoList);

      contentServer->getGenerationInfoList(0,mGenerationList);
      mGenerationList.sort(T::GenerationInfo::ComparisonMethod::generationId);
    }

    if (mLevelInfoList.getLength() == 0  ||  (mLevelInfoList_lastUpdate + 300) < time(nullptr))
    {
      contentServer->getLevelInfoList(0,mLevelInfoList);
      mLevelInfoList_lastUpdate = time(nullptr);
    }
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file",mConfigurationFile.getFilename());
    throw exception;
  }
}



void Engine::updateQueryCache()
{
  try
  {
    if (!mQueryCacheEnabled)
      return;

    time_t currentTime = time(nullptr);
    if ((currentTime - mQueryCacheUpdateTime) < 60)
      return;

    time_t tt = getFileModificationTime(mProducerFile.c_str());
    if (mProducerFile_modificationTime != tt  &&  (tt+3) < currentTime)
    {
      // The producer search order has changed. So we have to clear the query cache.
      mProducerFile_modificationTime = tt;
      AutoWriteLock lock(&mQueryCacheModificationLock);
      mQueryCache.clear();
      return;
    }

    mQueryCacheEnabled = false;

    mQueryCacheUpdateTime = currentTime;
    time_t lastAccess = currentTime - mQueryCacheMaxAge;
    std::vector <ulonglong> deleteList;


    {
      AutoReadLock lock(&mQueryCacheModificationLock);

      for (auto it = mQueryCache.begin(); it != mQueryCache.end(); ++it)
      {
        if (it->second.lastAccessTime < lastAccess)
        {
          // The cache entry has not been accessed for awhile, so we should remove it.
          deleteList.push_back(it->first);
        }
        else
        {
          // If the producer information has changed then we should remove the cache entry.
          bool noMatch = false;
          for (auto prod = it->second.producerHashMap.begin(); prod != it->second.producerHashMap.end()  && !noMatch; ++prod)
          {
            ulonglong producerHash = getProducerHash(prod->first);
            if (producerHash != prod->second)
              noMatch = true;
          }

          if (noMatch)
            deleteList.push_back(it->first);
        }
      }
    }

    if (deleteList.size() > 0)
    {
      AutoWriteLock lock(&mQueryCacheModificationLock);
      for (auto it = deleteList.begin(); it != deleteList.end(); ++it)
      {
        auto pos = mQueryCache.find(*it);
        if (pos != mQueryCache.end())
          mQueryCache.erase(pos);
      }
    }

    mQueryCacheEnabled = true;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file",mConfigurationFile.getFilename());
    throw exception;
  }
}





void Engine::getVerticalGrid(
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
    uint& gridHeight) const
{
  try
  {
    ContentServer_sptr  contentServer = getContentServer_sptr();
    DataServer_sptr  dataServer = getDataServer_sptr();
    QueryServer_sptr  queryServer = getQueryServer_sptr();


    std::set<double> levels1;
    std::set<double> levels2;

    T::SessionId sessionId = 0;
    T::GenerationInfoList gInfoList1;
    T::GenerationInfoList gInfoList2;

    int result = contentServer->getGenerationInfoListByProducerName(sessionId,valueProducerName,gInfoList1);
    if (result != 0)
    {
      Fmi::Exception exception(BCP, ContentServer::getResultString(result).c_str(), nullptr);
      exception.addParameter("ProducerName",valueProducerName);
      throw exception;
    }

    result = contentServer->getGenerationInfoListByProducerName(sessionId,heightProducerName,gInfoList2);
    if (result != 0)
    {
      Fmi::Exception exception(BCP, ContentServer::getResultString(result).c_str());
      exception.addParameter("ProducerName",heightProducerName);
      throw exception;
    }

    T::GenerationInfo *gInfo1 = gInfoList1.getLastGenerationInfoByAnalysisTime();
    T::GenerationInfo *gInfo2 = gInfoList2.getLastGenerationInfoByAnalysisTime();

    if (gInfo1 != nullptr  &&  gInfo2 != nullptr  &&  gInfo1->mAnalysisTime != gInfo2->mAnalysisTime)
    {
      if (gInfo1->mAnalysisTime < gInfo2->mAnalysisTime)
        gInfo2 = gInfoList2.getGenerationInfoByAnalysisTime(gInfo1->mAnalysisTime);
      else
      if (gInfo1->mAnalysisTime > gInfo2->mAnalysisTime)
        gInfo1 = gInfoList1.getGenerationInfoByAnalysisTime(gInfo2->mAnalysisTime);
    }

    if (gInfo1 != nullptr  &&  gInfo2 != nullptr  &&  gInfo1->mAnalysisTime != gInfo2->mAnalysisTime)
    {
      Fmi::Exception exception(BCP,"Cannot find the same analysis time for values and heights!");
      throw exception;
    }

    getProducerParameterLevelList(valueProducerName,3,1,levels1);
    if (levels1.size() == 0)
    {
      Fmi::Exception exception(BCP,"Cannot find valid levels for the value parameter!");
      exception.addParameter("ValueProducer",valueProducerName);
      throw exception;
    }

    getProducerParameterLevelList(heightProducerName,3,1,levels2);
    if (levels2.size() == 0)
    {
      Fmi::Exception exception(BCP,"Cannot find valid levels for the height parameter!");
      exception.addParameter("HeightProducer",heightProducerName);
      throw exception;
    }

    if (levels1.size() != levels2.size())
    {
      Fmi::Exception exception(BCP,"The number of value parameter levels is different than the number of height parameter levels!");
      throw exception;
    }

    auto points = getIsocirclePoints(lon1,lat1,lon2,lat2,steps);

    for (auto level = levels1.rbegin(); level != levels1.rend(); ++level)
    {
      int lev = C_INT(*level);
      char param[100];
      char *p = param;
      p += sprintf(p,"%s:%s",valueParameter.c_str(),valueProducerName.c_str());
      if (geometryId > 0)
        p += sprintf(p,":%u:3:%u",geometryId,lev);
      else
        p += sprintf(p,"::3:%u",lev);

      std::vector<T::ParamValue> valueVec;
      std::vector<T::ParamValue> heightVec;

      int result1 = queryServer->getParameterValuesByPointListAndTime(sessionId,valueProducerName,std::string(param),T::CoordinateTypeValue::LATLON_COORDINATES,points.first,utcTime,areaInterpolationMethod,timeInterpolationMethod,1,valueVec);

      p = param;
      p += sprintf(p,"%s:%s",heightParameter.c_str(),heightProducerName.c_str());
      if (geometryId > 0)
        p += sprintf(p,":%u:3:%u",geometryId,lev);
      else
        p += sprintf(p,"::3:%u",lev);

      int result2 = queryServer->getParameterValuesByPointListAndTime(sessionId,heightProducerName,std::string(param),T::CoordinateTypeValue::LATLON_COORDINATES,points.first,utcTime,areaInterpolationMethod,timeInterpolationMethod,1,heightVec);

      uint sz = points.first.size();
      if (result1 == 0  &&  result2 == 0  &&  valueVec.size() == sz  &&  heightVec.size() == sz)
      {
        for (uint t=0; t<sz; t++)
        {
          auto dist = points.second[t];
          coordinates.push_back(T::Coordinate(dist,heightVec[t]));

          gridData.push_back(valueVec[t]);
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
        contourLowValues.push_back(100.0);
      else
        contourLowValues.push_back(m);

      m = m + 2;
      contourHighValues.push_back(m);
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
    exception.addParameter("Configuration file",mConfigurationFile.getFilename());
    throw exception;
  }
}





void Engine::setDem(boost::shared_ptr<Fmi::DEM> dem)
{
  try
  {
    mDem = dem;
    mQueryServer->setDem(dem);
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file",mConfigurationFile.getFilename());
    throw exception;
  }
}





void Engine::setLandCover(boost::shared_ptr<Fmi::LandCover> landCover)
{
  try
  {
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
    pthread_create(&mThread,nullptr,gridEngine_updateThread,this);
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file",mConfigurationFile.getFilename());
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


extern "C" const char* engine_name()
{
  return "grid";
}
