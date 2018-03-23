#include "Engine.h"

#include <spine/Exception.h>
#include <grid-files/common/GeneralFunctions.h>
#include <grid-files/grid/ValueCache.h>
#include <grid-files/identification/GridDef.h>
#include <grid-content/contentServer/corba/client/ClientImplementation.h>
#include <grid-content/dataServer/corba/client/ClientImplementation.h>
#include <grid-content/dataServer/implementation/VirtualContentFactory_type1.h>
#include <grid-content/queryServer/corba/client/ClientImplementation.h>
#include <unistd.h>



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
    return NULL;
  }
  catch (...)
  {
    SmartMet::Spine::Exception exception(BCP,exception_operation_failed,NULL);
    exception.printError();
    exit(-1);
  }
}




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
        "local-query-server.mappingUpdateFile.fmi",
        "local-query-server.mappingUpdateFile.newbase",
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
    mParameterMappingUpdateTime = 0;
    mShutdownRequested = false;

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
    mConfigurationFile.getAttributeValue("local-query-server.mappingUpdateFile.fmi",mParameterMappingUpdateFile_fmi);
    mConfigurationFile.getAttributeValue("local-query-server.mappingUpdateFile.newbase",mParameterMappingUpdateFile_newbase);
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

    mProducerAliases.init(mProducerAliasFile,true);

    startUpdateProcessing();
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





void Engine::getProducerNameList(std::string aliasName,std::vector<std::string>& nameList)
{
  try
  {
    mProducerAliases.checkUpdates();
    mProducerAliases.getAliasList(aliasName,nameList);

    std::cout << "ALIAS " << aliasName << "\n";
    for (auto it = nameList.begin(); it != nameList.end(); ++it)
      std::cout << " - name : " << *it << "\n";

    if (nameList.size() == 0)
      nameList.push_back(aliasName);
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





void Engine::getProducerParameterLevelList(std::string producerName,T::ParamLevelId fmiParamLevelId,double multiplier,std::vector<double>& levels)
{
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
          if (levelInfo != NULL  &&  levelInfo->mProducerId == producerInfo.mProducerId  &&  levelInfo->mFmiParameterLevelId == fmiParamLevelId  &&
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
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}





void Engine::getProducerParameterLevelIdList(std::string producerName,std::set<T::ParamLevelId>& levelIdList)
{
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
          if (levelInfo != NULL  &&  levelInfo->mProducerId == producerInfo.mProducerId)
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
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}




void Engine::loadMappings(QueryServer::ParamMappingFile_vec& parameterMappings)
{
  try
  {
    for (auto it = mParameterMappingFiles.begin(); it != mParameterMappingFiles.end(); ++it)
    {
      QueryServer::ParameterMappingFile mapping(*it);
      parameterMappings.push_back(mapping);
    }

    for (auto it = parameterMappings.begin(); it != parameterMappings.end(); ++it)
    {
      it->init();
    }
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}





void Engine::updateMappings()
{
  try
  {
    if ((time(0) - mParameterMappingUpdateTime) > 20)
    {
      mParameterMappingUpdateTime = time(0);

      QueryServer::ParamMappingFile_vec parameterMappings;
      loadMappings(parameterMappings);
      updateMappings(T::ParamKeyType::FMI_NAME,mParameterMappingUpdateFile_fmi,parameterMappings);
      updateMappings(T::ParamKeyType::NEWBASE_NAME,mParameterMappingUpdateFile_newbase,parameterMappings);
    }
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}





FILE* Engine::openMappingFile(std::string mappingFile)
{
  try
  {
    FILE *file = fopen(mappingFile.c_str(),"w");
    if (file == NULL)
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
    fprintf(file,"#         50..99 = List\n");
    fprintf(file,"#         100..65535 = External (interpolated by an external function)\n");
    fprintf(file,"#  8) Time interpolation method\n");
    fprintf(file,"#         0 = None\n");
    fprintf(file,"#         1 = Linear\n");
    fprintf(file,"#         2 = Nearest\n");
    fprintf(file,"#         100..65535 = External (interpolated by an external function)\n");
    fprintf(file,"#  9) Level interpolation method\n");
    fprintf(file,"#         0 = None\n");
    fprintf(file,"#         1 = Linear\n");
    fprintf(file,"#         2 = Nearest\n");
    fprintf(file,"#         3 = Logarithmic\n");
    fprintf(file,"#         100..65535 = External (interpolated by an external function)\n");
    fprintf(file,"# 10) Search match (Can this mapping used when searching mappings for incomplete parameters)\n");
    fprintf(file,"#         E = Enabled\n");
    fprintf(file,"#         D = Disabled\n");
    fprintf(file,"# 11) Mapping function (enables data conversions during the mapping)\n");
    fprintf(file,"# \n");

    return file;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}





void Engine::updateMappings(T::ParamKeyType parameterKeyType,std::string mappingFile,QueryServer::ParamMappingFile_vec& parameterMappings)
{
  try
  {
    ContentServer_sptr  contentServer = getContentServer_sptr();

    T::SessionId sessionId = 0;

    std::set<std::string> infoList;
    int result = 0;

    result = contentServer->getProducerParameterList(sessionId,parameterKeyType,infoList);
    if (result != 0)
    {
      std::cerr << CODE_LOCATION << "The 'contentServer.getProducerParameterList()' service call returns an error!  Result : " << result << " : " << ContentServer::getResultString(result).c_str() << "\n";
      return;
    }

    FILE *file = NULL;

    uint numOfMappings = 0;

    for (auto it=infoList.begin(); it != infoList.end(); ++it)
    {
      std::vector<std::string> pl;
      splitString(it->c_str(),';',pl);
      if (pl.size() >= 7)
      {
        QueryServer::ParameterMapping m;
        m.mProducerName = pl[0];
        m.mParameterName = pl[1];
        m.mParameterKeyType = (T::ParamKeyType)atoi(pl[2].c_str());
        m.mParameterKey = pl[3];
        m.mParameterLevelIdType = (T::ParamLevelIdType)atoi(pl[4].c_str());
        m.mParameterLevelId = atoi(pl[5].c_str());
        m.mParameterLevel = atoi(pl[6].c_str());

        bool found = false;
        for (auto it = parameterMappings.begin(); it != parameterMappings.end() && !found; ++it)
        {
          if (it->getFilename() != mappingFile)
          {
            if (it->getMapping(m) != NULL)
              found = true;
          }
          else
          {
            numOfMappings = it->getNumberOfMappings();
          }
        }

        if (!found)
        {
          if (file == NULL)
            file = openMappingFile(mappingFile);

          fprintf(file,"%s;%s;%s;%s;%s;%s;%s;",pl[0].c_str(),pl[1].c_str(),pl[2].c_str(),pl[3].c_str(),pl[4].c_str(),pl[5].c_str(),pl[6].c_str());

          Identification::FmiParameterDef paramDef;
          if (Identification::gridDef.getFmiParameterDefByName(pl[3],paramDef))
          {
            fprintf(file,"%d;%d;%d;E;",(int)paramDef.mAreaInterpolationMethod,(int)paramDef.mTimeInterpolationMethod,(int)paramDef.mLevelInterpolationMethod);

            if (parameterKeyType == T::ParamKeyType::NEWBASE_ID || parameterKeyType == T::ParamKeyType::NEWBASE_NAME)
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
            fprintf(file,"1;1;1;E;;\n");
          }
        }
      }
    }

    if (file == NULL  &&  numOfMappings > 0)
    {
      // We found all mappings from the other files. That's why we should remove them
      // from the update file.

      file = openMappingFile(mappingFile);
    }

    if (file != NULL)
      fclose(file);

  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}





void Engine::updateProcessing()
{
  try
  {
    while (!mShutdownRequested)
    {
      updateMappings();
      sleep(10);
    }
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,NULL);
  }
}





void Engine::startUpdateProcessing()
{
  try
  {
    pthread_create(&mThread,NULL,gridEngine_updateThread,this);
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,NULL);
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
