#pragma once

#include <spine/SmartMetEngine.h>
#include <grid-content/contentServer/cache/CacheImplementation.h>
#include <grid-content/contentServer/redis/RedisImplementation.h>
#include <grid-content/dataServer/implementation/ServiceImplementation.h>
#include <grid-content/dataServer/cache/CacheImplementation.h>
#include <grid-content/queryServer/implementation/ServiceImplementation.h>
#include <grid-files/common/ConfigurationFile.h>


namespace SmartMet
{
namespace Engine
{
namespace Grid
{

typedef std::shared_ptr<ContentServer::ServiceInterface> ContentServer_sptr;
typedef std::shared_ptr<DataServer::ServiceInterface> DataServer_sptr;
typedef std::shared_ptr<QueryServer::ServiceInterface> QueryServer_sptr;
typedef std::vector<std::string> String_vec;

class Engine : public SmartMet::Spine::SmartMetEngine
{
  public:

                        Engine(const char *theConfigFile);
    virtual             ~Engine();

    void                executeQuery(QueryServer::Query& query);

    ContentServer_sptr  getContentServer_sptr();
    DataServer_sptr     getDataServer_sptr();
    QueryServer_sptr    getQueryServer_sptr();

    T::ParamLevelId     getFmiParameterLevelId(uint producerId,int level);
    std::string         getProducerName(std::string aliasName);
    void                getProducerList(string_vec& producerList);


  protected:

    void                init();
    void                shutdown();

  private:

    ConfigurationFile   mConfigurationFile;

    std::string         mRedisAddress;
    int                 mRedisPort;
    std::string         mRedisTablePrefix;

    ContentServer_sptr  mContentServerCache;
    ContentServer_sptr  mContentServerRedis;
    bool                mRemoteContentServerEnabled;
    std::string         mRemoteContentServerIor;

    DataServer_sptr     mDataServer;
    DataServer_sptr     mDataServerClient;
    String_vec          mDataServerLuaFiles;
    bool                mRemoteDataServerEnabled;
    bool                mRemoteDataServerCache;
    std::string         mRemoteDataServerIor;

    QueryServer_sptr    mQueryServer;
    String_vec          mQueryServerLuaFiles;
    bool                mRemoteQueryServerEnabled;
    std::string         mRemoteQueryServerIor;

    std::string         mGridConfigFile;
    std::string         mDataServerGridDirectory;

    std::string         mContentServerProcessingLogFile;
    int                 mContentServerProcessingLogMaxSize;
    int                 mContentServerProcessingLogTruncateSize;
    Log                 mContentServerProcessingLog;
    std::string         mContentServerDebugLogFile;
    int                 mContentServerDebugLogMaxSize;
    int                 mContentServerDebugLogTruncateSize;
    Log                 mContentServerDebugLog;

    std::string         mDataServerProcessingLogFile;
    int                 mDataServerProcessingLogMaxSize;
    int                 mDataServerProcessingLogTruncateSize;
    Log                 mDataServerProcessingLog;
    std::string         mDataServerDebugLogFile;
    int                 mDataServerDebugLogMaxSize;
    int                 mDataServerDebugLogTruncateSize;
    Log                 mDataServerDebugLog;

    std::string         mQueryServerProcessingLogFile;
    int                 mQueryServerProcessingLogMaxSize;
    int                 mQueryServerProcessingLogTruncateSize;
    Log                 mQueryServerProcessingLog;
    std::string         mQueryServerDebugLogFile;
    int                 mQueryServerDebugLogMaxSize;
    int                 mQueryServerDebugLogTruncateSize;
    Log                 mQueryServerDebugLog;

    uint                mNumOfCachedGrids;
    uint                mMaxCompressedMegaBytesOfCachedGrids;
    uint                mMaxUncompressedMegaBytesOfCachedGrids;

    std::string         mProducerFile;
    std::string         mProducerAliasFile;
    bool                mVirtualFilesEnabled;
    std::string         mVirtualFileDefinitions;

    String_vec          mParameterAliasFiles;
    String_vec          mParameterMappingFiles;

    T::LevelInfoList    mLevelInfoList;
    time_t              mLevelInfoList_lastUpdate;
    ThreadLock          mThreadLock;

    QueryServer::AliasFile mProducerAliases;
};

}  // namespace Grid
}  // namespace Engine
}  // namespace SmartMet
