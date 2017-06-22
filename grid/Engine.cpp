#include "Engine.h"

#include <spine/Exception.h>
#include "grid-files/grid/ValueCache.h"
#include "grid-files/identification/GribDef.h"


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

    if (!itsConfig.exists("server.gridDirectory"))
      throw SmartMet::Spine::Exception(BCP, "The 'server.gridDirectory' attribute not specified in the config file");

    if (!itsConfig.exists("server.configDirectory"))
      throw SmartMet::Spine::Exception(BCP, "The 'server.configDirectory' attribute not specified in the config file");

    if (!itsConfig.exists("server.cache.numOfGrids"))
      throw SmartMet::Spine::Exception(BCP, "The 'server.cache.numOfGrids' attribute not specified in the config file");

    if (!itsConfig.exists("server.cache.maxUncompressedSizeInMegaBytes"))
      throw SmartMet::Spine::Exception(BCP, "The 'server.cache.maxUncompressedSizeInMegaBytes' attribute not specified in the config file");

    if (!itsConfig.exists("server.cache.maxCompressedSizeInMegaBytes"))
      throw SmartMet::Spine::Exception(BCP, "The 'server.cache.maxCompressedSizeInMegaBytes' attribute not specified in the config file");

    itsConfig.lookupValue("redis.address", itsRedisAddress);
    itsConfig.lookupValue("redis.port", itsRedisPort);
    itsConfig.lookupValue("server.gridDirectory", itsServerGridDirectory);
    itsConfig.lookupValue("server.configDirectory", itsServerConfigDirectory);
    itsConfig.lookupValue("server.cache.numOfGrids", itsNumOfCachedGrids);
    itsConfig.lookupValue("server.cache.maxUncompressedSizeInMegaBytes", itsMaxUncompressedMegaBytesOfCachedGrids);
    itsConfig.lookupValue("server.cache.maxCompressedSizeInMegaBytes", itsMaxCompressedMegaBytesOfCachedGrids);


    // Initializing information that is needed for identifying the content of the grid files.

    SmartMet::Identification::gribDef.init(itsServerConfigDirectory.c_str());


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
    SmartMet::ContentServer::RedisImplementation *redis = new SmartMet::ContentServer::RedisImplementation();
    redis->init(itsRedisAddress.c_str(),itsRedisPort);
    contentServerRedis.reset(redis);


    SmartMet::ContentServer::CacheImplementation *cache = new SmartMet::ContentServer::CacheImplementation();
    cache->init(0,redis);
    contentServerCache.reset(cache);
    cache->startEventProcessing();

    SmartMet::DataServer::ServiceImplementation *dServer = new SmartMet::DataServer::ServiceImplementation();
    dServer->init(0,0,"NotRegistered","NotRegistered",itsServerGridDirectory,cache);
    dataServer.reset(dServer);
    dServer->startEventProcessing();

    SmartMet::QueryServer::ServiceImplementation *qServer = new SmartMet::QueryServer::ServiceImplementation();
    qServer->init(cache,dServer);
    queryServer.reset(qServer);
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
