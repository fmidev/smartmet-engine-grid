#include "Engine.h"

#include <spine/Exception.h>
#include <grid-files/common/GeneralFunctions.h>
#include <grid-files/common/ShowFunction.h>
#include <grid-files/grid/ValueCache.h>
#include <grid-files/identification/GridDef.h>
#include <grid-content/contentServer/corba/client/ClientImplementation.h>
#include <grid-content/contentServer/http/client/ClientImplementation.h>
#include <grid-content/dataServer/corba/client/ClientImplementation.h>
#include <grid-content/dataServer/implementation/VirtualContentFactory_type1.h>
#include <grid-content/queryServer/corba/client/ClientImplementation.h>
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
    SmartMet::Spine::Exception exception(BCP,exception_operation_failed,nullptr);
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
        "smartmet.library.grid-files.requestCounter.enabled",
        "smartmet.library.grid-files.requestCounter.filename",

        "smartmet.engine.grid.content-server.content-source.type",
        "smartmet.engine.grid.content-server.content-source.redis.address",
        "smartmet.engine.grid.content-server.content-source.redis.port",
        "smartmet.engine.grid.content-server.content-source.redis.tablePrefix",
        "smartmet.engine.grid.content-server.content-source.http.url",
        "smartmet.engine.grid.content-server.content-source.corba.ior",
        "smartmet.engine.grid.content-server.cache.enabled",
        "smartmet.engine.grid.content-server.cache.contentSortingFlags",

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
        "smartmet.engine.grid.data-server.grid-storage.preloadEnabled",
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
        "smartmet.engine.grid.query-server.producerFile",
        "smartmet.engine.grid.query-server.producerAliasFile",
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
    mRequestCounterEnabled = false;

    mContentServerProcessingLogEnabled = false;
    mContentServerDebugLogEnabled = false;
    mDataServerProcessingLogEnabled = false;
    mDataServerDebugLogEnabled = false;
    mQueryServerProcessingLogEnabled = false;
    mQueryServerDebugLogEnabled = false;
    mVirtualFilesEnabled = false;
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
    mNumOfCachedGrids = 10000;
    mMaxSizeOfCachedGridsInMegaBytes = 10000;

    mConfigurationFile.readFile(theConfigFile);
    //mConfigurationFile.print(std::cout,0,0);

    uint t=0;
    while (configAttribute[t] != nullptr)
    {
      if (!mConfigurationFile.findAttribute(configAttribute[t]))
      {
        SmartMet::Spine::Exception exception(BCP, "Missing configuration attribute!");
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

    mConfigurationFile.getAttributeValue("smartmet.library.grid-files.requestCounter.enabled", mRequestCounterEnabled);
    mConfigurationFile.getAttributeValue("smartmet.library.grid-files.requestCounter.filename", mRequestCounterFilename);

    mConfigurationFile.getAttributeValue("smartmet.engine.grid.content-server.content-source.type", mContentSourceType);

    mConfigurationFile.getAttributeValue("smartmet.engine.grid.content-server.content-source.redis.address", mContentSourceRedisAddress);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.content-server.content-source.redis.port", mContentSourceRedisPort);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.content-server.content-source.redis.tablePrefix", mContentSourceRedisTablePrefix);

    mConfigurationFile.getAttributeValue("smartmet.engine.grid.content-server.content-source.http.url", mContentSourceHttpUrl);

    mConfigurationFile.getAttributeValue("smartmet.engine.grid.content-server.content-source.corba.ior", mContentSourceCorbaIor);

    mConfigurationFile.getAttributeValue("smartmet.engine.grid.content-server.cache.enabled", mContentCacheEnabled);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.content-server.cache.contentSortingFlags", mContentCacheSortingFlags);

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
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.data-server.grid-storage.preloadEnabled",mContentPreloadEnabled);
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

    // These settings are used when the query server is embedded into the grid engine.
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.query-server.producerFile",mProducerFile);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.query-server.producerAliasFile",mProducerAliasFile);

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


    // Initializing information that is needed for identifying the content of the grid files.

    SmartMet::Identification::gridDef.init(mGridConfigFile.c_str());
  }
  catch (...)
  {
    SmartMet::Spine::Exception exception(BCP, "Constructor failed!", nullptr);
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
    SmartMet::Spine::Exception exception(BCP, "Operation failed!", nullptr);
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
    {
      SmartMet::Spine::Exception exception(BCP, "Unknow content source type!");
      exception.addParameter("Content source type",mContentSourceType);
    }

    if (mContentCacheEnabled)
    {
      ContentServer::CacheImplementation *cache = new ContentServer::CacheImplementation();
      cache->init(0,cServer,mContentCacheSortingFlags);
      mContentServerCache.reset(cache);
      cache->startEventProcessing();
      cServer = cache;
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
      DataServer::ServiceImplementation *server = new DataServer::ServiceImplementation();
      server->init(0,0,"NotRegistered","NotRegistered",mDataServerGridDirectory,cServer,mDataServerLuaFiles);
      server->setPointCacheEnabled(mPointCacheEnabled,mPointCacheHitsRequired,mPointCacheTimePeriod);
      server->setRequestCounterEnabled(mRequestCounterFilename,mRequestCounterEnabled);
      server->setContentPreloadEnabled(mContentPreloadEnabled);
      //dServer->init(0,0,"NotRegistered","NotRegistered",mDataServerGridDirectory,cache);

      if (mVirtualFilesEnabled)
      {
        server->setVirtualContentEnabled(true);
        DataServer::VirtualContentFactory_type1 *factory = new DataServer::VirtualContentFactory_type1();
        factory->init(mVirtualFileDefinitions);
        server->addVirtualContentFactory(factory);
      }
      else
      {
        server->setVirtualContentEnabled(false);
      }

      mDataServer.reset(server);
      server->startEventProcessing();

      if (mRequestCounterEnabled)
        server->startRequestCounting();

      dServer = server;

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
      server->init(cServer,dServer,mGridConfigFile,mParameterMappingFiles,mParameterAliasFiles,mProducerFile,mProducerAliasFile,mQueryServerLuaFiles);
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

    mProducerAliases.init(mProducerAliasFile,true);
    mParameterAliasFileCollection.init(mParameterAliasFiles);


    clearMappings();

    startUpdateProcessing();
  }
  catch (...)
  {
    SmartMet::Spine::Exception exception(BCP, "Operation failed!", nullptr);
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
    SmartMet::Spine::Exception exception(BCP, "Operation failed!", nullptr);
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
    SmartMet::Spine::Exception exception(BCP, "Operation failed!", nullptr);
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
    SmartMet::Spine::Exception exception(BCP, "Operation failed!", nullptr);
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
    SmartMet::Spine::Exception exception(BCP, "Operation failed!", nullptr);
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
    SmartMet::Spine::Exception exception(BCP, "Operation failed!", nullptr);
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
    SmartMet::Spine::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file",mConfigurationFile.getFilename());
    throw exception;
  }
}





bool Engine::isGridProducer(const std::string& producer) const
{
  FUNCTION_TRACE
  try
  {
    AutoThreadLock lock(&mThreadLock);
    if ((time(nullptr) - mProducerList_updateTime) > 60)
    {
      mProducerList_updateTime = time(nullptr);
      getProducerList(mProducerList);

      ContentServer_sptr contentServer = getContentServer_sptr();
      contentServer->getGenerationInfoList(0,mGenerationList);
      mGenerationList.sort(T::GenerationInfo::ComparisonMethod::generationId);
    }

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
    SmartMet::Spine::Exception exception(BCP, "Operation failed!", nullptr);
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
        //parameters[t].print(std::cout,0,0);

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
    SmartMet::Spine::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file",mConfigurationFile.getFilename());
    throw exception;
  }
}





void Engine::getProducerNameList(const std::string& aliasName,std::vector<std::string>& nameList) const
{
  FUNCTION_TRACE
  try
  {
    mProducerAliases.checkUpdates();

    std::vector<std::string> aliasStrings;
    mProducerAliases.getAliasList(aliasName,aliasStrings);

    // Removing the level type information from the alias names.

    for (auto it=aliasStrings.begin(); it != aliasStrings.end(); it++)
    {
      std::vector<std::string> partList;
      splitString(*it,';',partList);
      nameList.push_back(partList[0]);
    }

    //std::cout << "ALIAS " << aliasName << "\n";
    //for (auto it = nameList.begin(); it != nameList.end(); ++it)
    //  std::cout << " - name : " << *it << "\n";

    if (nameList.size() == 0)
      nameList.push_back(aliasName);
  }
  catch (...)
  {
    SmartMet::Spine::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file",mConfigurationFile.getFilename());
    throw exception;
  }
}





void Engine::getParameterDetails(const std::string& aliasName,ParameterDetails_vec& parameterDetails) const
{
  FUNCTION_TRACE
  try
  {
    mProducerAliases.checkUpdates();

    std::vector<std::string> aliasStrings;
    mProducerAliases.getAliasList(aliasName,aliasStrings);

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
      //p.print(std::cout,0,0);
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
    SmartMet::Spine::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file",mConfigurationFile.getFilename());
    throw exception;
  }
}




void Engine::getParameterDetails(const std::string& producerName,const std::string& parameterName,ParameterDetails_vec& parameterDetails) const
{
  FUNCTION_TRACE
  try
  {
    mProducerAliases.checkUpdates();
    mParameterAliasFileCollection.checkUpdates(false);


    std::string prod = producerName;
    //printf("GET PRODUCER [%s;%s]\n",prod.c_str(),parameterName.c_str());
    mParameterAliasFileCollection.getAlias(producerName,prod);


    std::string param = parameterName;
    mParameterAliasFileCollection.getAlias(parameterName,param);

    std::string key = prod + ";" + param;

    std::vector<std::string> aliasStrings;
    mProducerAliases.getAliasList(key,aliasStrings);

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
      //p.print(std::cout,0,0);
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
    SmartMet::Spine::Exception exception(BCP, "Operation failed!", nullptr);
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
    SmartMet::Spine::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file",mConfigurationFile.getFilename());
    throw exception;
  }
}





void Engine::getParameterMappings(std::string producerName,std::string parameterName,T::GeometryId geometryId, bool onlySearchEnabled, QueryServer::ParameterMapping_vec& mappings) const
{
  try
  {
    AutoThreadLock lock(&mThreadLock);

    for (auto m = mParameterMappings.begin(); m != mParameterMappings.end(); ++m)
    {
      m->getMappings(producerName, parameterName, geometryId, onlySearchEnabled, mappings);
      //printf("Get mappings [%s][%s][%d] %lu\n",producerName.c_str(),parameterName.c_str(),geometryId,mappings.size());
    }
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", nullptr);
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
    AutoThreadLock lock(&mThreadLock);

    for (auto m = mParameterMappings.begin(); m != mParameterMappings.end(); ++m)
    {
      m->getMappings(producerName, parameterName, geometryId, levelIdType, levelId, level, onlySearchEnabled, mappings);
      //printf("Get mappings [%s][%s][%d] [%d][%d][%d] %lu\n",producerName.c_str(),parameterName.c_str(),geometryId,levelIdType, levelId, level,mappings.size());
    }
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", nullptr);
  }
}





void Engine::mapParameterDetails(ParameterDetails_vec& parameterDetails) const
{
  try
  {
    ContentServer_sptr contentServer = getContentServer_sptr();

    if (mGenerationList.getLength() == 0)
    {
      contentServer->getGenerationInfoList(0,mGenerationList);
      mGenerationList.sort(T::GenerationInfo::ComparisonMethod::generationId);
    }

    for (auto rec = parameterDetails.begin(); rec != parameterDetails.end(); ++rec)
    {
      QueryServer::ParameterMapping_vec mappings;
      if (rec->mLevelId > " " || rec->mLevel > " ")
      {
        getParameterMappings(rec->mProducerName, rec->mOriginalParameter, atoi(rec->mGeometryId.c_str()), T::ParamLevelIdTypeValue::ANY, atoi(rec->mLevelId.c_str()), atoi(rec->mLevel.c_str()), false, mappings);
        if (mappings.size() == 0  &&  rec->mLevel < " ")
        {
          getParameterMappings(rec->mProducerName, rec->mOriginalParameter, atoi(rec->mGeometryId.c_str()), T::ParamLevelIdTypeValue::ANY, atoi(rec->mLevelId.c_str()), -1, false, mappings);
          //getParameterMappings(producerInfo.mName, parameterKey, producerGeometryId, T::ParamLevelIdTypeValue::ANY, paramLevelId, -1, false, mappings);
        }
      }
      else
      {
        getParameterMappings(rec->mProducerName, rec->mOriginalParameter, atoi(rec->mGeometryId.c_str()), true, mappings);
        //getParameterMappings(producerInfo.mName, parameterKey, producerGeometryId, true, mappings);
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
    SmartMet::Spine::Exception exception(BCP, "Operation failed!", nullptr);
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
    // printf("GetAlias [%s;%d][%s]\n",producerName.c_str(),levelId,prod.c_str());
    return prod;
  }
  catch (...)
  {
    SmartMet::Spine::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file",mConfigurationFile.getFilename());
    throw exception;
  }
}





T::ParamLevelId Engine::getFmiParameterLevelId(uint producerId,int level) const
{
  FUNCTION_TRACE
  try
  {
    AutoThreadLock lock(&mThreadLock);

    if (mLevelInfoList.getLength() == 0  ||  (mLevelInfoList_lastUpdate + 300) < time(nullptr))
    {
      ContentServer_sptr  contentServer = getContentServer_sptr();
      contentServer->getLevelInfoList(0,mLevelInfoList);

      mLevelInfoList_lastUpdate = time(nullptr);
    }

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
    SmartMet::Spine::Exception exception(BCP, "Operation failed!", nullptr);
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
    SmartMet::Spine::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file",mConfigurationFile.getFilename());
    throw exception;
  }
}





void Engine::getProducerParameterLevelList(const std::string& producerName,T::ParamLevelId fmiParamLevelId,double multiplier,std::set<double>& levels) const
{
  FUNCTION_TRACE
  try
  {
    AutoThreadLock lock(&mThreadLock);

    ContentServer_sptr  contentServer = getContentServer_sptr();
    if (mLevelInfoList.getLength() == 0  ||  (mLevelInfoList_lastUpdate + 300) < time(nullptr))
    {
      contentServer->getLevelInfoList(0,mLevelInfoList);
      mLevelInfoList_lastUpdate = time(nullptr);
    }

    std::vector<std::string> nameList;
    getProducerNameList(producerName,nameList);

    for (auto pname = nameList.begin(); pname != nameList.end(); ++pname)
    {
      T::ProducerInfo producerInfo;
      if (contentServer->getProducerInfoByName(0,*pname,producerInfo) == 0)
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
    SmartMet::Spine::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file",mConfigurationFile.getFilename());
    throw exception;
  }
}





void Engine::getProducerParameterLevelIdList(const std::string& producerName,std::set<T::ParamLevelId>& levelIdList) const
{
  FUNCTION_TRACE
  try
  {
    AutoThreadLock lock(&mThreadLock);

    ContentServer_sptr  contentServer = getContentServer_sptr();
    if (mLevelInfoList.getLength() == 0  ||  (mLevelInfoList_lastUpdate + 300) < time(nullptr))
    {
      contentServer->getLevelInfoList(0,mLevelInfoList);
      mLevelInfoList_lastUpdate = time(nullptr);
    }

    std::vector<std::string> nameList;
    getProducerNameList(producerName,nameList);

    for (auto pname = nameList.begin(); pname != nameList.end(); ++pname)
    {
      T::ProducerInfo producerInfo;
      if (contentServer->getProducerInfoByName(0,*pname,producerInfo) == 0)
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
    SmartMet::Spine::Exception exception(BCP, "Operation failed!", nullptr);
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
    SmartMet::Spine::Exception exception(BCP, "Operation failed!", nullptr);
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
      if (file != NULL)
        fclose(file);
    }

    if (!mParameterMappingUpdateFile_newbase.empty())
    {
      FILE *file = openMappingFile(mParameterMappingUpdateFile_newbase);
      if (file != NULL)
        fclose(file);
    }
  }
  catch (...)
  {
    SmartMet::Spine::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file",mConfigurationFile.getFilename());
    throw exception;
  }
}





void Engine::updateMappings()
{
  FUNCTION_TRACE
  try
  {
    if ((time(nullptr) - mParameterMappingUpdateTime) > 20)
    {
      mParameterMappingUpdateTime = time(nullptr);

      QueryServer::ParamMappingFile_vec parameterMappings;
      loadMappings(parameterMappings);

      if (parameterMappings.size() > 0)
      {
        AutoThreadLock lock(&mThreadLock);
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
  }
  catch (...)
  {
    SmartMet::Spine::Exception exception(BCP, "Operation failed!", nullptr);
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
      SmartMet::Spine::Exception exception(BCP, "Cannot open a mapping file for writing!");
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
    SmartMet::Spine::Exception exception(BCP, "Operation failed!", nullptr);
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

    std::set<std::string> infoList;
    int result = 0;

    result = contentServer->getProducerParameterList(sessionId,sourceParameterKeyType,targetParameterKeyType,infoList);
    if (result != 0)
    {
      std::cerr << CODE_LOCATION << "The 'contentServer.getProducerParameterList()' service call returns an error!  Result : " << result << " : " << ContentServer::getResultString(result).c_str() << "\n";
      return;
    }

    FILE *file = nullptr;

    uint numOfNewMappings = 0;
    std::set<std::string> mapList;
    std::set<std::string> searchList;

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
    SmartMet::Spine::Exception exception(BCP, "Operation failed!", nullptr);
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
      updateMappings();
      sleep(300);
    }
  }
  catch (...)
  {
    SmartMet::Spine::Exception exception(BCP, "Operation failed!", nullptr);
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
    SmartMet::Spine::Exception exception(BCP, "Operation failed!", nullptr);
    exception.addParameter("Configuration file",mConfigurationFile.getFilename());
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
    SmartMet::Spine::Exception exception(BCP, "Operation failed!", nullptr);
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
