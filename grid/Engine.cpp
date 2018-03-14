#include "Engine.h"

#include <spine/Exception.h>
#include <grid-files/common/GeneralFunctions.h>
#include <grid-files/grid/ValueCache.h>
#include <grid-files/identification/GridDef.h>
#include <grid-content/contentServer/corba/client/ClientImplementation.h>
#include <grid-content/dataServer/corba/client/ClientImplementation.h>
#include <grid-content/dataServer/implementation/VirtualContentFactory_type1.h>
#include <grid-content/queryServer/corba/client/ClientImplementation.h>



namespace SmartMet
{
namespace Engine
{
namespace Grid
{


Engine::Engine(const char* theConfigFile)
{
  try
  {
    const char *configAttribute[] =
    {
        "grid-files.configDirectory",
        "local-content-server.redis.address",
        "local-content-server.redis.port",
        "local-content-server.redis.tablePrefix",
        "local-content-server.processing-log.file",
        "local-content-server.processing-log.maxSize",
        "local-content-server.processing-log.truncateSize",
        "local-content-server.debug-log.file",
        "local-content-server.debug-log.maxSize",
        "local-content-server.debug-log.truncateSize",
        "local-data-server.gridDirectory",
        "local-data-server.virtualFiles.enabled",
        "local-data-server.virtualFiles.definitionFile",
        "local-data-server.luaFiles",
        "local-data-server.cache.numOfGrids",
        "local-data-server.cache.maxUncompressedSizeInMegaBytes",
        "local-data-server.cache.maxCompressedSizeInMegaBytes",
        "local-data-server.processing-log.file",
        "local-data-server.processing-log.maxSize",
        "local-data-server.processing-log.truncateSize",
        "local-data-server.debug-log.file",
        "local-data-server.debug-log.maxSize",
        "local-data-server.debug-log.truncateSize",
        "local-query-server.producerFile",
        "local-query-server.producerAliasFile",
        "local-query-server.luaFiles",
        "local-query-server.mappingFiles",
        "local-query-server.aliasFiles",
        "local-query-server.processing-log.file",
        "local-query-server.processing-log.maxSize",
        "local-query-server.processing-log.truncateSize",
        "local-query-server.debug-log.file",
        "local-query-server.debug-log.maxSize",
        "local-query-server.debug-log.truncateSize",
        "remote-content-server.enabled",
        "remote-content-server.ior",
        "remote-data-server.enabled",
        "remote-data-server.cache",
        "remote-data-server.ior",
        "remote-query-server.enabled",
        "remote-query-server.ior",
        NULL
    };

    mLevelInfoList_lastUpdate = 0;
    mRedisAddress = "127.0.0.1";
    mRedisPort = 6379;
    mRemoteContentServerEnabled = false;
    mRemoteDataServerEnabled = false;
    mRemoteQueryServerEnabled = false;
    mRemoteDataServerCache = false;
    mVirtualFilesEnabled = false;

    mConfigurationFile.readFile(theConfigFile);

    uint t=0;
    while (configAttribute[t] != NULL)
    {
      if (!mConfigurationFile.findAttribute(configAttribute[t]))
      {
        SmartMet::Spine::Exception exception(BCP, "Missing configuration attribute!");
        exception.addParameter("File",theConfigFile);
        exception.addParameter("Attribute",configAttribute[t]);
      }
      t++;
    }


    mConfigurationFile.getAttributeValue("grid-files.configFile", mGridConfigFile);

    mConfigurationFile.getAttributeValue("remote-content-server.enabled", mRemoteContentServerEnabled);
    mConfigurationFile.getAttributeValue("remote-content-server.ior", mRemoteContentServerIor);

    mConfigurationFile.getAttributeValue("local-content-server.redis.address", mRedisAddress);
    mConfigurationFile.getAttributeValue("local-content-server.redis.port", mRedisPort);
    mConfigurationFile.getAttributeValue("local-content-server.redis.tablePrefix", mRedisTablePrefix);

    mConfigurationFile.getAttributeValue("remote-data-server.enabled", mRemoteDataServerEnabled);
    mConfigurationFile.getAttributeValue("remote-data-server.cache", mRemoteDataServerCache);
    mConfigurationFile.getAttributeValue("remote-data-server.ior", mRemoteDataServerIor);

    mConfigurationFile.getAttributeValue("remote-query-server.enabled", mRemoteQueryServerEnabled);
    mConfigurationFile.getAttributeValue("remote-query-server.ior", mRemoteQueryServerIor);

    mConfigurationFile.getAttributeValue("local-data-server.gridDirectory", mDataServerGridDirectory);
    mConfigurationFile.getAttributeValue("local-data-server.virtualFiles.enabled",mVirtualFilesEnabled);
    mConfigurationFile.getAttributeValue("local-data-server.virtualFiles.definitionFile",mVirtualFileDefinitions);

    mConfigurationFile.getAttributeValue("local-data-server.cache.numOfGrids", mNumOfCachedGrids);
    mConfigurationFile.getAttributeValue("local-data-server.cache.maxUncompressedSizeInMegaBytes", mMaxUncompressedMegaBytesOfCachedGrids);
    mConfigurationFile.getAttributeValue("local-data-server.cache.maxCompressedSizeInMegaBytes", mMaxCompressedMegaBytesOfCachedGrids);

    mConfigurationFile.getAttributeValue("local-query-server.producerFile",mProducerFile);
    mConfigurationFile.getAttributeValue("local-query-server.producerAliasFile",mProducerAliasFile);

    mConfigurationFile.getAttributeValue("local-content-server.processing-log.file", mContentServerProcessingLogFile);
    mConfigurationFile.getAttributeValue("local-content-server.processing-log.maxSize", mContentServerProcessingLogMaxSize);
    mConfigurationFile.getAttributeValue("local-content-server.processing-log.truncateSize", mContentServerProcessingLogTruncateSize);
    mConfigurationFile.getAttributeValue("local-content-server.debug-log.file", mContentServerDebugLogFile);
    mConfigurationFile.getAttributeValue("local-content-server.debug-log.maxSize", mContentServerDebugLogMaxSize);
    mConfigurationFile.getAttributeValue("local-content-server.debug-log.truncateSize", mContentServerDebugLogTruncateSize);

    mConfigurationFile.getAttributeValue("local-data-server.processing-log.file", mDataServerProcessingLogFile);
    mConfigurationFile.getAttributeValue("local-data-server.processing-log.maxSize", mDataServerProcessingLogMaxSize);
    mConfigurationFile.getAttributeValue("local-data-server.processing-log.truncateSize", mDataServerProcessingLogTruncateSize);
    mConfigurationFile.getAttributeValue("local-data-server.debug-log.file", mDataServerDebugLogFile);
    mConfigurationFile.getAttributeValue("local-data-server.debug-log.maxSize", mDataServerDebugLogMaxSize);
    mConfigurationFile.getAttributeValue("local-data-server.debug-log.truncateSize", mDataServerDebugLogTruncateSize);

    mConfigurationFile.getAttributeValue("local-query-server.processing-log.file", mQueryServerProcessingLogFile);
    mConfigurationFile.getAttributeValue("local-query-server.processing-log.maxSize", mQueryServerProcessingLogMaxSize);
    mConfigurationFile.getAttributeValue("local-query-server.processing-log.truncateSize", mQueryServerProcessingLogTruncateSize);
    mConfigurationFile.getAttributeValue("local-query-server.debug-log.file", mQueryServerDebugLogFile);
    mConfigurationFile.getAttributeValue("local-query-server.debug-log.maxSize", mQueryServerDebugLogMaxSize);
    mConfigurationFile.getAttributeValue("local-query-server.debug-log.truncateSize", mQueryServerDebugLogTruncateSize);
    mConfigurationFile.getAttributeValue("local-query-server.mappingFiles",mParameterMappingFiles);
    mConfigurationFile.getAttributeValue("local-query-server.aliasFiles",mParameterAliasFiles);
    mConfigurationFile.getAttributeValue("local-query-server.luaFiles",mQueryServerLuaFiles);
    mConfigurationFile.getAttributeValue("local-data-server.luaFiles",mDataServerLuaFiles);



    // Initializing information that is needed for identifying the content of the grid files.

    SmartMet::Identification::gridDef.init(mGridConfigFile.c_str());
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Constructor failed!", NULL);
  }
}




Engine::~Engine()
{
  try
  {
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}





void Engine::init()
{
  try
  {
    ContentServer::RedisImplementation *redis = new ContentServer::RedisImplementation();
    redis->init(mRedisAddress.c_str(),mRedisPort,mRedisTablePrefix.c_str());
    mContentServerRedis.reset(redis);

    ContentServer::ServiceInterface *cServer = NULL;
    DataServer::ServiceInterface *dServer = NULL;
    QueryServer::ServiceInterface *qServer = NULL;


    if (mRemoteContentServerEnabled  &&  mRemoteContentServerIor.length() > 50)
    {
      ContentServer::Corba::ClientImplementation *client = new ContentServer::Corba::ClientImplementation();
      client->init(mRemoteContentServerIor.c_str());
      mContentServerCache.reset(client);
      cServer = client;
    }
    else
    {
      ContentServer::CacheImplementation *cache = new ContentServer::CacheImplementation();
      cache->init(0,redis);
      mContentServerCache.reset(cache);
      cache->startEventProcessing();
      cServer = cache;
    }

    if (mRemoteDataServerEnabled  &&  mRemoteDataServerIor.length() > 50)
    {
      DataServer::Corba::ClientImplementation *client = new DataServer::Corba::ClientImplementation();
      client->init(mRemoteDataServerIor);

      if (mRemoteDataServerCache)
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
      //dServer->init(0,0,"NotRegistered","NotRegistered",mDataServerGridDirectory,cache);

      if (mVirtualFilesEnabled)
      {
        server->enableVirtualContent(true);
        DataServer::VirtualContentFactory_type1 *factory = new DataServer::VirtualContentFactory_type1();
        factory->init(mVirtualFileDefinitions);
        server->addVirtualContentFactory(factory);
      }
      else
      {
        server->enableVirtualContent(false);
      }

      mDataServer.reset(server);
      server->startEventProcessing();
      dServer = server;

      SmartMet::GRID::valueCache.init(mNumOfCachedGrids,mMaxUncompressedMegaBytesOfCachedGrids,mMaxCompressedMegaBytesOfCachedGrids);
    }

    if (mRemoteQueryServerEnabled  &&  mRemoteQueryServerIor.length() > 50)
    {
      QueryServer::Corba::ClientImplementation *client = new QueryServer::Corba::ClientImplementation();
      client->init(mRemoteQueryServerIor);
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


    if (mContentServerProcessingLogFile.length() > 0)
    {
      mContentServerProcessingLog.init(true,mContentServerProcessingLogFile.c_str(),mContentServerProcessingLogMaxSize,mContentServerProcessingLogTruncateSize);
      cServer->setProcessingLog(&mContentServerProcessingLog);
    }

    if (mContentServerDebugLogFile.length() > 0)
    {
      mContentServerDebugLog.init(true,mContentServerDebugLogFile.c_str(),mContentServerDebugLogMaxSize,mContentServerDebugLogTruncateSize);
      cServer->setDebugLog(&mContentServerDebugLog);
    }

    if (mDataServerProcessingLogFile.length() > 0)
    {
      mDataServerProcessingLog.init(true,mDataServerProcessingLogFile.c_str(),mDataServerProcessingLogMaxSize,mDataServerProcessingLogTruncateSize);
      dServer->setProcessingLog(&mDataServerProcessingLog);
    }

    if (mDataServerDebugLogFile.length() > 0)
    {
      mDataServerDebugLog.init(true,mDataServerDebugLogFile.c_str(),mDataServerDebugLogMaxSize,mDataServerDebugLogTruncateSize);
      dServer->setDebugLog(&mDataServerDebugLog);
    }

    if (mQueryServerProcessingLogFile.length() > 0)
    {
      mQueryServerProcessingLog.init(true,mQueryServerProcessingLogFile.c_str(),mQueryServerProcessingLogMaxSize,mQueryServerProcessingLogTruncateSize);
      qServer->setProcessingLog(&mQueryServerProcessingLog);
    }

    if (mQueryServerDebugLogFile.length() > 0)
    {
      mQueryServerDebugLog.init(true,mQueryServerDebugLogFile.c_str(),mQueryServerDebugLogMaxSize,mQueryServerDebugLogTruncateSize);
      qServer->setDebugLog(&mQueryServerDebugLog);
    }

    mProducerAliases.init(mProducerAliasFile);
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}





void Engine::shutdown()
{
  try
  {
    std::cout << "  -- Shutdown requested (grid engine)\n";

    if (!mContentServerRedis)
      mContentServerRedis->shutdown();

    if (!mContentServerCache)
      mContentServerCache->shutdown();

    if (!mDataServer)
      mDataServer->shutdown();

    if (!mQueryServer)
      mQueryServer->shutdown();
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}





void Engine::executeQuery(QueryServer::Query& query)
{
  try
  {
    mQueryServer->executeQuery(0,query);
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}





ContentServer_sptr Engine::getContentServer_sptr()
{
  try
  {
    return mContentServerCache;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}





DataServer_sptr Engine::getDataServer_sptr()
{
  try
  {
    return mDataServer;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}





QueryServer_sptr Engine::getQueryServer_sptr()
{
  try
  {
    return mQueryServer;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}





std::string Engine::getProducerName(std::string aliasName)
{
  try
  {
    mProducerAliases.checkUpdates();

    std::string name;
    if (mProducerAliases.getAlias(aliasName,name))
      return name;

    return aliasName;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}






T::ParamLevelId Engine::getFmiParameterLevelId(uint producerId,int level)
{
  try
  {
    AutoThreadLock lock(&mThreadLock);

    if (mLevelInfoList.getLength() == 0  ||  (mLevelInfoList_lastUpdate + 300) < time(0))
    {
      ContentServer_sptr  contentServer = getContentServer_sptr();
      contentServer->getLevelInfoList(0,mLevelInfoList);
      mLevelInfoList_lastUpdate = time(0);
    }

    uint len = mLevelInfoList.getLength();
    for (uint t=0; t<len; t++)
    {
      T::LevelInfo *levelInfo = mLevelInfoList.getLevelInfoByIndex(t);
      if (levelInfo != NULL  &&  levelInfo->mProducerId == producerId)
      {
        if (levelInfo->mParameterLevel == level)
          return levelInfo->mFmiParameterLevelId;
      }
    }
    return 0;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}





void Engine::getProducerList(string_vec& producerList)
{
  try
  {
    mQueryServer->getProducerList(0,producerList);
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
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
