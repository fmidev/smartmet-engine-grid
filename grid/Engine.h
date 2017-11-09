#pragma once

#include <spine/SmartMetEngine.h>
#include <grid-content/contentServer/cache/CacheImplementation.h>
#include <grid-content/contentServer/redis/RedisImplementation.h>
#include <grid-content/dataServer/implementation/ServiceImplementation.h>
#include <grid-content/dataServer/cache/CacheImplementation.h>
#include <grid-content/queryServer/implementation/ServiceImplementation.h>
#include <libconfig.h++>


namespace SmartMet
{
namespace Engine
{
namespace Grid
{

typedef std::shared_ptr<ContentServer::ServiceInterface> ContentServer_sptr;
typedef std::shared_ptr<DataServer::ServiceInterface> DataServer_sptr;
typedef std::shared_ptr<QueryServer::ServiceInterface> QueryServer_sptr;


class Engine : public SmartMet::Spine::SmartMetEngine
{
  public:

                        Engine(const char *theConfigFile);
    virtual             ~Engine();

    void                executeQuery(QueryServer::Query& query);

    ContentServer_sptr  getContentServer_sptr();
    DataServer_sptr     getDataServer_sptr();
    QueryServer_sptr    getQueryServer_sptr();

  protected:

    void                init();
    void                shutdown();

  private:

    libconfig::Config   mConfig;

    std::string         mRedisAddress;
    int                 mRedisPort;
    std::string         mRedisTablePrefix;

    ContentServer_sptr  mContentServerCache;
    ContentServer_sptr  mContentServerRedis;
    std::string         mRemoteContentServerEnabled;
    std::string         mRemoteContentServerIor;

    DataServer_sptr     mDataServer;
    DataServer_sptr     mDataServerClient;
    std::string         mRemoteDataServerEnabled;
    std::string         mRemoteDataServerCache;
    std::string         mRemoteDataServerIor;

    QueryServer_sptr    mQueryServer;
    std::string         mRemoteQueryServerEnabled;
    std::string         mRemoteQueryServerIor;

    std::string         mGridConfigDirectory;

    std::string         mServerGridDirectory;

    std::string         mContentServerLogFile;
    int                 mContentServerLogMaxSize;
    int                 mContentServerLogTruncateSize;
    Log                 mContentServerLog;

    std::string         mDataServerLogFile;
    int                 mDataServerLogMaxSize;
    int                 mDataServerLogTruncateSize;
    Log                 mDataServerLog;

    std::string         mQueryServerLogFile;
    int                 mQueryServerLogMaxSize;
    int                 mQueryServerLogTruncateSize;
    Log                 mQueryServerLog;

    uint                mNumOfCachedGrids;
    uint                mMaxCompressedMegaBytesOfCachedGrids;
    uint                mMaxUncompressedMegaBytesOfCachedGrids;

    std::string         mParameterAliasFile;
    std::string         mParameterFile;
    std::string         mProducerFile;
    std::string         mLuaFunctionFile;
};

}  // namespace Grid
}  // namespace Engine
}  // namespace SmartMet
