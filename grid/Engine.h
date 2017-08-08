#pragma once

#include <spine/SmartMetEngine.h>
#include <grid-content/contentServer/cache/CacheImplementation.h>
#include <grid-content/contentServer/redis/RedisImplementation.h>
#include <grid-content/dataServer/implementation/ServiceImplementation.h>
#include <grid-content/queryServer/implementation/ServiceImplementation.h>
#include <libconfig.h++>

namespace SmartMet
{
namespace Engine
{
namespace Grid
{


class Engine : public SmartMet::Spine::SmartMetEngine
{
  public:

    Engine(const char *theConfigFile);
    ~Engine();

    std::shared_ptr<SmartMet::ContentServer::ServiceInterface> getContentServerPtr();
    std::shared_ptr<SmartMet::DataServer::ServiceInterface>  getDataServerPtr();
    std::shared_ptr<SmartMet::QueryServer::ServiceInterface> getQueryServerPt();

  protected:

    void init();
    void shutdown();

  private:

    std::shared_ptr<SmartMet::ContentServer::ServiceInterface> contentServerCache;
    std::shared_ptr<SmartMet::ContentServer::ServiceInterface> contentServerRedis;
    std::shared_ptr<SmartMet::DataServer::ServiceInterface>  dataServer;
    std::shared_ptr<SmartMet::QueryServer::ServiceInterface> queryServer;

    libconfig::Config   itsConfig;
    std::string         itsRedisAddress;
    int                 itsRedisPort;
    std::string         itsRedisTablePrefix;
    std::string         itsServerGridDirectory;
    std::string         itsServerConfigDirectory;
    uint                itsNumOfCachedGrids;
    uint                itsMaxCompressedMegaBytesOfCachedGrids;
    uint                itsMaxUncompressedMegaBytesOfCachedGrids;
    uint                itsCacheExpirationTime;
};

}  // namespace Grid
}  // namespace Engine
}  // namespace SmartMet
