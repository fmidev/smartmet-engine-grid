#include "Engine.h"

#include <spine/Exception.h>
#include "grid-files/grid/ValueCache.h"
#include "grid-files/identification/GribDef.h"
#include "grid-content/contentServer/corba/client/ClientImplementation.h"
#include "grid-content/dataServer/corba/client/ClientImplementation.h"
#include "grid-content/queryServer/corba/client/ClientImplementation.h"


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
    itsConfig.readFile(theConfigFile);

    itsRedisAddress = "127.0.0.1";
    itsRedisPort = 6379;

    if (!itsConfig.exists("redis.address"))
      throw SmartMet::Spine::Exception(BCP, "The 'redis.address' attribute not specified in the config file");

    if (!itsConfig.exists("redis.port"))
      throw SmartMet::Spine::Exception(BCP, "The 'redis.port' attribute not specified in the config file");

    if (!itsConfig.exists("redis.tablePrefix"))
      throw SmartMet::Spine::Exception(BCP, "The 'redis.tablePrefix' attribute not specified in the config file");

    if (!itsConfig.exists("grid-files.configDirectory"))
      throw SmartMet::Spine::Exception(BCP, "The 'grid-files.configDirectory' attribute not specified in the config file");

    if (!itsConfig.exists("remote-content-server.enabled"))
      throw SmartMet::Spine::Exception(BCP, "The 'remote-content-server.enabled' attribute not specified in the config file");

    if (!itsConfig.exists("remote-content-server.ior"))
      throw SmartMet::Spine::Exception(BCP, "The 'remote-content-server.ior' attribute not specified in the config file");

    if (!itsConfig.exists("remote-data-server.enabled"))
      throw SmartMet::Spine::Exception(BCP, "The 'remote-data-server.enabled' attribute not specified in the config file");

    if (!itsConfig.exists("remote-data-server.ior"))
      throw SmartMet::Spine::Exception(BCP, "The 'remote-data-server.ior' attribute not specified in the config file");

    if (!itsConfig.exists("remote-query-server.enabled"))
      throw SmartMet::Spine::Exception(BCP, "The 'remote-query-server.enabled' attribute not specified in the config file");

    if (!itsConfig.exists("remote-query-server.ior"))
      throw SmartMet::Spine::Exception(BCP, "The 'remote-query-server.ior' attribute not specified in the config file");

    if (!itsConfig.exists("local-data-server.processingLog.file"))
      throw SmartMet::Spine::Exception(BCP, "The 'local-data-server.processingLog.file' attribute not specified in the config file");

    if (!itsConfig.exists("local-data-server.processingLog.maxSize"))
      throw SmartMet::Spine::Exception(BCP, "The 'local-data-server.processingLog.maxSize' attribute not specified in the config file");

    if (!itsConfig.exists("local-data-server.processingLog.truncateSize"))
      throw SmartMet::Spine::Exception(BCP, "The 'local-data-server.processingLog.truncateSize' attribute not specified in the config file");

    if (!itsConfig.exists("local-data-server.gridDirectory"))
      throw SmartMet::Spine::Exception(BCP, "The 'local-data-server.gridDirectory' attribute not specified in the config file");

    if (!itsConfig.exists("local-data-server.cache.numOfGrids"))
      throw SmartMet::Spine::Exception(BCP, "The 'local-data-server.cache.numOfGrids' attribute not specified in the config file");

    if (!itsConfig.exists("local-data-server.cache.maxUncompressedSizeInMegaBytes"))
      throw SmartMet::Spine::Exception(BCP, "The 'local-data-server.cache.maxUncompressedSizeInMegaBytes' attribute not specified in the config file");

    if (!itsConfig.exists("local-data-server.cache.maxCompressedSizeInMegaBytes"))
      throw SmartMet::Spine::Exception(BCP, "The 'local-data-server.cache.maxCompressedSizeInMegaBytes' attribute not specified in the config file");

    itsConfig.lookupValue("redis.address", itsRedisAddress);
    itsConfig.lookupValue("redis.port", itsRedisPort);
    itsConfig.lookupValue("redis.tablePrefix", itsRedisTablePrefix);
    itsConfig.lookupValue("grid-files.configDirectory", itsGridConfigDirectory);
    itsConfig.lookupValue("remote-content-server.enabled", itsRemoteContentServerEnabled);
    itsConfig.lookupValue("remote-content-server.ior", itsRemoteContentServerIor);
    itsConfig.lookupValue("remote-data-server.enabled", itsRemoteDataServerEnabled);
    itsConfig.lookupValue("remote-data-server.ior", itsRemoteDataServerIor);
    itsConfig.lookupValue("remote-query-server.enabled", itsRemoteQueryServerEnabled);
    itsConfig.lookupValue("remote-query-server.ior", itsRemoteQueryServerIor);
    itsConfig.lookupValue("local-data-server.gridDirectory", itsServerGridDirectory);
    itsConfig.lookupValue("local-data-server.processingLog.file", itsServerProcessingLogFile);
    itsConfig.lookupValue("local-data-server.processingLog.maxSize", itsServerProcessingLogMaxSize);
    itsConfig.lookupValue("local-data-server.processingLog.truncateSize", itsServerProcessingLogTruncateSize);
    itsConfig.lookupValue("local-data-server.cache.numOfGrids", itsNumOfCachedGrids);
    itsConfig.lookupValue("local-data-server.cache.maxUncompressedSizeInMegaBytes", itsMaxUncompressedMegaBytesOfCachedGrids);
    itsConfig.lookupValue("local-data-server.cache.maxCompressedSizeInMegaBytes", itsMaxCompressedMegaBytesOfCachedGrids);


    // Initializing information that is needed for identifying the content of the grid files.

    SmartMet::Identification::gribDef.init(itsGridConfigDirectory.c_str());


    // Initializing the size of the grid value cache.

    SmartMet::GRID::valueCache.init(itsNumOfCachedGrids,itsMaxUncompressedMegaBytesOfCachedGrids,itsMaxCompressedMegaBytesOfCachedGrids);

  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
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
    redis->init(itsRedisAddress.c_str(),itsRedisPort,itsRedisTablePrefix.c_str());
    contentServerRedis.reset(redis);

    ContentServer::ServiceInterface *cServer = NULL;
    DataServer::ServiceInterface *dServer = NULL;
    //QueryServer::ServiceInterface *qServer = NULL;


    if (itsRemoteContentServerEnabled == "true"  &&  itsRemoteContentServerIor.length() > 50)
    {
      ContentServer::Corba::ClientImplementation *client = new ContentServer::Corba::ClientImplementation();
      client->init(itsRemoteContentServerIor.c_str());
      contentServerCache.reset(client);
      cServer = client;
    }
    else
    {
      ContentServer::CacheImplementation *cache = new ContentServer::CacheImplementation();
      cache->init(0,redis);
      contentServerCache.reset(cache);
      cache->startEventProcessing();
      cServer = cache;
    }

    if (itsRemoteDataServerEnabled == "true"  &&  itsRemoteDataServerIor.length() > 50)
    {
      DataServer::Corba::ClientImplementation *client = new DataServer::Corba::ClientImplementation();
      client->init(itsRemoteDataServerIor);
      dataServer.reset(client);
      dServer = client;
    }
    else
    {
      DataServer::ServiceImplementation *server = new DataServer::ServiceImplementation();
      server->init(0,0,"NotRegistered","NotRegistered",itsServerGridDirectory,cServer);
      //dServer->init(0,0,"NotRegistered","NotRegistered",itsServerGridDirectory,cache);
      dataServer.reset(server);
      server->startEventProcessing();

      if (itsServerProcessingLogFile.length() > 0)
      {
        itsProcessingLog.init(true,itsServerProcessingLogFile.c_str(),itsServerProcessingLogMaxSize,itsServerProcessingLogTruncateSize);
        //cache->setProcessingLog(&itsProcessingLog);
        server->setProcessingLog(&itsProcessingLog);
      }
    }

    if (itsRemoteQueryServerEnabled == "true"  &&  itsRemoteQueryServerIor.length() > 50)
    {
      QueryServer::Corba::ClientImplementation *client = new QueryServer::Corba::ClientImplementation();
      client->init(itsRemoteQueryServerIor);
      queryServer.reset(client);
      //qServer = client;
    }
    else
    {
      QueryServer::ServiceImplementation *server = new QueryServer::ServiceImplementation();
      server->init(cServer,dServer);
      queryServer.reset(server);
    }
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
    std::cout << "  -- Shutdown requested (grid-content engine)\n";

    if (!contentServerRedis)
      contentServerRedis->shutdown();

    if (!contentServerCache)
      contentServerCache->shutdown();

    if (!dataServer)
      dataServer->shutdown();

    if (!queryServer)
      queryServer->shutdown();
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}




std::shared_ptr<SmartMet::ContentServer::ServiceInterface> Engine::getContentServerPtr()
{
  try
  {
    return contentServerCache;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}




std::shared_ptr<SmartMet::DataServer::ServiceInterface>  Engine::getDataServerPtr()
{
  try
  {
    return dataServer;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}




std::shared_ptr<SmartMet::QueryServer::ServiceInterface> Engine::getQueryServerPt()
{
  try
  {
    return queryServer;
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
