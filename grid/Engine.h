#pragma once

#include <spine/SmartMetEngine.h>
#include <grid-content/contentServer/cache/CacheImplementation.h>
#include <grid-content/contentServer/redis/RedisImplementation.h>
#include <grid-content/dataServer/implementation/ServiceImplementation.h>
#include <grid-content/dataServer/cache/CacheImplementation.h>
#include <grid-content/queryServer/implementation/ServiceImplementation.h>
#include <grid-files/common/ConfigurationFile.h>
#include <pthread.h>


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
    void                getProducerNameList(std::string aliasName,std::vector<std::string>& nameList);
    void                getProducerList(string_vec& producerList);
    void                getProducerParameterLevelList(std::string producerName,T::ParamLevelId fmiParamLevelId,double multiplier,std::vector<double>& levels);
    void                getProducerParameterLevelIdList(std::string producerName,std::set<T::ParamLevelId>& levelIdList);

    void                updateProcessing();

  protected:

    void                init();
    void                shutdown();
    void                startUpdateProcessing();
    void                loadMappings(QueryServer::ParamMappingFile_vec& parameterMappings);
    FILE*               openMappingFile(std::string mappingFile);
    void                updateMappings();
    void                updateMappings(T::ParamKeyType parameterKeyType,std::string mappingFile,QueryServer::ParamMappingFile_vec& parameterMappings);


  private:

    ConfigurationFile   mConfigurationFile;
    bool                mShutdownRequested;

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

    pthread_t           mThread;
    std::string         mParameterMappingUpdateFile_fmi;
    std::string         mParameterMappingUpdateFile_newbase;
    time_t              mParameterMappingUpdateTime;

    T::LevelInfoList    mLevelInfoList;
    time_t              mLevelInfoList_lastUpdate;
    ThreadLock          mThreadLock;

    QueryServer::AliasFile mProducerAliases;
};

}  // namespace Grid
}  // namespace Engine
}  // namespace SmartMet
