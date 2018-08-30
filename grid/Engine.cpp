#include "Engine.h"

#include <spine/Exception.h>
#include <grid-files/common/GeneralFunctions.h>
#include <grid-files/common/ShowFunction.h>
#include <grid-files/grid/ValueCache.h>
#include <grid-files/identification/GridDef.h>
#include <grid-content/contentServer/corba/client/ClientImplementation.h>
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
    Engine *engine = (Engine*)arg;
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
        "smartmet.library.grid-files.cache.maxUncompressedSizeInMegaBytes",
        "smartmet.library.grid-files.cache.maxCompressedSizeInMegaBytes",

        "smartmet.engine.grid.content-server.remote",
        "smartmet.engine.grid.content-server.ior",
        "smartmet.engine.grid.content-server.redis.address",
        "smartmet.engine.grid.content-server.redis.port",
        "smartmet.engine.grid.content-server.redis.tablePrefix",
        "smartmet.engine.grid.content-server.processing-log.enabled",
        "smartmet.engine.grid.content-server.processing-log.file",
        "smartmet.engine.grid.content-server.processing-log.maxSize",
        "smartmet.engine.grid.content-server.processing-log.truncateSize",
        "smartmet.engine.grid.content-server.debug-log.enabled",
        "smartmet.engine.grid.content-server.debug-log.file",
        "smartmet.engine.grid.content-server.debug-log.maxSize",
        "smartmet.engine.grid.content-server.debug-log.truncateSize",
        "smartmet.engine.grid.content-server.cache.contentSortingFlags",

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
    mRedisAddress = "127.0.0.1";
    mRedisPort = 6379;
    mDataServerCacheEnabled = false;
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
    mContentSortingFlags = 0;
    mMappingTargetKeyType = T::ParamKeyTypeValue::FMI_NAME;

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
    mConfigurationFile.getAttributeValue("smartmet.library.grid-files.cache.maxUncompressedSizeInMegaBytes", mMaxUncompressedMegaBytesOfCachedGrids);
    mConfigurationFile.getAttributeValue("smartmet.library.grid-files.cache.maxCompressedSizeInMegaBytes", mMaxCompressedMegaBytesOfCachedGrids);

    mConfigurationFile.getAttributeValue("smartmet.engine.grid.content-server.remote", mContentServerRemote);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.content-server.ior", mContentServerIor);

    // These settings are used when the content server is embedded into the grid engine.
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.content-server.redis.address", mRedisAddress);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.content-server.redis.port", mRedisPort);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.content-server.redis.tablePrefix", mRedisTablePrefix);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.content-server.processing-log.enabled", mContentServerProcessingLogEnabled);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.content-server.processing-log.file", mContentServerProcessingLogFile);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.content-server.processing-log.maxSize", mContentServerProcessingLogMaxSize);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.content-server.processing-log.truncateSize", mContentServerProcessingLogTruncateSize);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.content-server.debug-log.enabled", mContentServerDebugLogEnabled);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.content-server.debug-log.file", mContentServerDebugLogFile);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.content-server.debug-log.maxSize", mContentServerDebugLogMaxSize);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.content-server.debug-log.truncateSize", mContentServerDebugLogTruncateSize);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.content-server.cache.contentSortingFlags", mContentSortingFlags);

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
    mMappingTargetKeyType = tmp;

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
    throw SmartMet::Spine::Exception(BCP, "Constructor failed!", nullptr);
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
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", nullptr);
  }
}





void Engine::init()
{
  FUNCTION_TRACE
  try
  {
    ContentServer::RedisImplementation *redis = new ContentServer::RedisImplementation();
    redis->init(mRedisAddress.c_str(),mRedisPort,mRedisTablePrefix.c_str());
    mContentServerRedis.reset(redis);

    ContentServer::ServiceInterface *cServer = nullptr;
    DataServer::ServiceInterface *dServer = nullptr;
    QueryServer::ServiceInterface *qServer = nullptr;


    if (mContentServerRemote  &&  mContentServerIor.length() > 50)
    {
      ContentServer::Corba::ClientImplementation *client = new ContentServer::Corba::ClientImplementation();
      client->init(mContentServerIor.c_str());
      mContentServerCache.reset(client);
      cServer = client;
    }
    else
    {
      ContentServer::CacheImplementation *cache = new ContentServer::CacheImplementation();
      cache->init(0,redis,mContentSortingFlags);
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
      server->enableContentPreload(mContentPreloadEnabled);
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

    startUpdateProcessing();
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", nullptr);
  }
}





void Engine::shutdown()
{
  FUNCTION_TRACE
  try
  {
    std::cout << "  -- Shutdown requested (grid engine)\n";
    mShutdownRequested = true;

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
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", nullptr);
  }
}





int Engine::executeQuery(QueryServer::Query& query)
{
  FUNCTION_TRACE
  try
  {
    return mQueryServer->executeQuery(0,query);
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", nullptr);
  }
}





ContentServer_sptr Engine::getContentServer_sptr()
{
  FUNCTION_TRACE
  try
  {
    return mContentServerCache;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", nullptr);
  }
}





DataServer_sptr Engine::getDataServer_sptr()
{
  FUNCTION_TRACE
  try
  {
    return mDataServer;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", nullptr);
  }
}





QueryServer_sptr Engine::getQueryServer_sptr()
{
  FUNCTION_TRACE
  try
  {
    return mQueryServer;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", nullptr);
  }
}





void Engine::getProducerNameList(std::string aliasName,std::vector<std::string>& nameList)
{
  FUNCTION_TRACE
  try
  {
    mProducerAliases.checkUpdates();
    mProducerAliases.getAliasList(aliasName,nameList);

    //std::cout << "ALIAS " << aliasName << "\n";
    //for (auto it = nameList.begin(); it != nameList.end(); ++it)
    //  std::cout << " - name : " << *it << "\n";

    if (nameList.size() == 0)
      nameList.push_back(aliasName);
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", nullptr);
  }
}






T::ParamLevelId Engine::getFmiParameterLevelId(uint producerId,int level)
{
  FUNCTION_TRACE
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
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", nullptr);
  }
}





void Engine::getProducerList(string_vec& producerList)
{
  FUNCTION_TRACE
  try
  {
    mQueryServer->getProducerList(0,producerList);
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", nullptr);
  }
}





void Engine::getProducerParameterLevelList(std::string producerName,T::ParamLevelId fmiParamLevelId,double multiplier,std::vector<double>& levels)
{
  FUNCTION_TRACE
  try
  {
    AutoThreadLock lock(&mThreadLock);

    ContentServer_sptr  contentServer = getContentServer_sptr();
    if (mLevelInfoList.getLength() == 0  ||  (mLevelInfoList_lastUpdate + 300) < time(0))
    {
      contentServer->getLevelInfoList(0,mLevelInfoList);
      mLevelInfoList_lastUpdate = time(0);
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
            levels.push_back(levelInfo->mParameterLevel*multiplier);
          }
        }

        if (levels.size() > 0)
          return;
      }
    }
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", nullptr);
  }
}





void Engine::getProducerParameterLevelIdList(std::string producerName,std::set<T::ParamLevelId>& levelIdList)
{
  FUNCTION_TRACE
  try
  {
    AutoThreadLock lock(&mThreadLock);

    ContentServer_sptr  contentServer = getContentServer_sptr();
    if (mLevelInfoList.getLength() == 0  ||  (mLevelInfoList_lastUpdate + 300) < time(0))
    {
      contentServer->getLevelInfoList(0,mLevelInfoList);
      mLevelInfoList_lastUpdate = time(0);
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
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", nullptr);
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
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", nullptr);
  }
}





void Engine::updateMappings()
{
  FUNCTION_TRACE
  try
  {
    if ((time(0) - mParameterMappingUpdateTime) > 20)
    {
      mParameterMappingUpdateTime = time(0);

      QueryServer::ParamMappingFile_vec parameterMappings;
      loadMappings(parameterMappings);

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
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", nullptr);
  }
}





FILE* Engine::openMappingFile(std::string mappingFile)
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
    fprintf(file,"#  5) Parameter level id type:\n");
    fprintf(file,"#         1 = FMI\n");
    fprintf(file,"#         2 = GRIB1\n");
    fprintf(file,"#         3 = GRIB2\n");
    fprintf(file,"#  6) Level id\n");
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
    fprintf(file,"#  7) Area interpolation method\n");
    fprintf(file,"#         0 = None\n");
    fprintf(file,"#         1 = Linear\n");
    fprintf(file,"#         2 = Nearest\n");
    fprintf(file,"#         500..999 = List\n");
    fprintf(file,"#         1000..65535 = External (interpolated by an external function)\n");
    fprintf(file,"#  8) Time interpolation method\n");
    fprintf(file,"#         0 = None\n");
    fprintf(file,"#         1 = Linear\n");
    fprintf(file,"#         2 = Nearest\n");
    fprintf(file,"#         1000..65535 = External (interpolated by an external function)\n");
    fprintf(file,"#  9) Level interpolation method\n");
    fprintf(file,"#         0 = None\n");
    fprintf(file,"#         1 = Linear\n");
    fprintf(file,"#         2 = Nearest\n");
    fprintf(file,"#         3 = Logarithmic\n");
    fprintf(file,"#         1000..65535 = External (interpolated by an external function)\n");
    fprintf(file,"# 10) Group flags\n");
    fprintf(file,"#         bit 0 = Climatological parameter (=> ignore year when searching) \n");
    fprintf(file,"# 11) Search match (Can this mapping used when searching mappings for incomplete parameters)\n");
    fprintf(file,"#         E = Enabled\n");
    fprintf(file,"#         D = Disabled\n");
    fprintf(file,"# 12) Mapping function (enables data conversions during the mapping)\n");
    fprintf(file,"# \n");

    return file;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", nullptr);
  }
}





void Engine::updateMappings(T::ParamKeyType sourceParameterKeyType,T::ParamKeyType targetParameterKeyType,std::string mappingFile,QueryServer::ParamMappingFile_vec& parameterMappings)
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
      if (pl.size() >= 7)
      {
        QueryServer::ParameterMapping m;
        m.mProducerName = pl[0];
        m.mParameterName = pl[1];
        m.mParameterKeyType = toInt64(pl[2].c_str());
        m.mParameterKey = pl[3];
        m.mParameterLevelIdType = toInt64(pl[4].c_str());
        m.mParameterLevelId = static_cast<char>(toInt64(pl[5].c_str()));
        m.mParameterLevel = toInt64(pl[6].c_str());

        char key[200];
        sprintf(key,"%s;%s;%s;%s;%s;%s;%s;",pl[0].c_str(),pl[1].c_str(),pl[2].c_str(),pl[3].c_str(),pl[4].c_str(),pl[5].c_str(),pl[6].c_str());
        std::string searchKey = m.mProducerName + ":" + m.mParameterName;

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
                  it->getMappings(m.mProducerName,m.mParameterName,true,vec);
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

            fprintf(file,"%s;%s;%s;%s;%s;%s;%s;",pl[0].c_str(),pl[1].c_str(),pl[2].c_str(),pl[3].c_str(),pl[4].c_str(),pl[5].c_str(),pl[6].c_str());

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
                fprintf(file,"%d;",(int)paramDef.mAreaInterpolationMethod);
              else
                fprintf(file,";");

              if (paramDef.mTimeInterpolationMethod >= 0)
                fprintf(file,"%d;",(int)paramDef.mTimeInterpolationMethod);
              else
                fprintf(file,";");

              if (paramDef.mLevelInterpolationMethod >= 0)
                fprintf(file,"%d;",(int)paramDef.mLevelInterpolationMethod);
              else
                fprintf(file,";");

              fprintf(file,"0;%c;",s);

              if (sourceParameterKeyType == T::ParamKeyTypeValue::NEWBASE_ID || sourceParameterKeyType == T::ParamKeyTypeValue::NEWBASE_NAME)
              {
                Identification::FmiParameterId_newbase paramMapping;
                if (Identification::gridDef.getNewbaseParameterMappingByFmiId(paramDef.mFmiParameterId,paramMapping))
                {
                  fprintf(file,"%s",paramMapping.mConversionFunction.c_str());
                }
              }
              fprintf(file,";\n");
            }
            else
            {
              fprintf(file,"1;1;1;0;D;;\n");
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
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", nullptr);
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
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,nullptr);
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
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,nullptr);
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
