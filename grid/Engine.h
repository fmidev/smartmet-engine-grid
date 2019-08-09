#pragma once

#include <spine/SmartMetEngine.h>
#include <gis/DEM.h>
#include <grid-content/contentServer/cache/CacheImplementation.h>
#include <grid-content/contentServer/redis/RedisImplementation.h>
#include <grid-content/dataServer/implementation/ServiceImplementation.h>
#include <grid-content/dataServer/cache/CacheImplementation.h>
#include <grid-content/queryServer/implementation/ServiceImplementation.h>
#include <grid-files/common/ConfigurationFile.h>
#include <grid-files/common/Typedefs.h>
#include <pthread.h>
#include "ParameterDetails.h"


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

    int                 executeQuery(QueryServer::Query& query);

    ContentServer_sptr  getContentServer_sptr();
    ContentServer_sptr  getContentSourceServer_sptr();
    DataServer_sptr     getDataServer_sptr();
    QueryServer_sptr    getQueryServer_sptr();

    T::ParamLevelId     getFmiParameterLevelId(uint producerId,int level);
    void                getProducerNameList(const std::string& aliasName,std::vector<std::string>& nameList);
    std::string         getProducerAlias(const std::string& producerName,int levelId);

    std::string         getParameterString(std::string producer,std::string parameter);
    void                getParameterDetails(const std::string& aliasName,ParameterDetails_vec& parameterDetails);
    void                getParameterDetails(const std::string& producerName,const std::string& parameterName,ParameterDetails_vec& parameterDetails);

/*
    void                getProducerNameAndLevelIdList(const std::string& aliasName,std::vector<std::string>& nameList,std::vector<std::string>& geometryIdList,std::vector<std::string>& levelIdList,std::vector<std::string>& levelList);
    void                getProducerNameAndLevelIdList(const std::string& producerName,const std::string& parameterName,std::vector<std::string>& nameList,std::vector<std::string>& geometryIdList,std::vector<std::string>& levelIdList,std::vector<std::string>& levelList);
    void                getProducerNameAndLevelIdList(const std::string& producerName,const std::string& parameterName,int levelId,std::vector<std::string>& nameList,std::vector<std::string>& geometryIdList,std::vector<std::string>& levelIdList,std::vector<std::string>& levelList);
*/
    void                getProducerList(string_vec& producerList);
    void                getProducerParameterLevelList(const std::string& producerName,T::ParamLevelId fmiParamLevelId,double multiplier,std::set<double>& levels);
    void                getProducerParameterLevelIdList(const std::string& producerName,std::set<T::ParamLevelId>& levelIdList);

    bool                isGridProducer(const std::string& producer);

    void                setDem(boost::shared_ptr<Fmi::DEM> dem);
    void                updateProcessing();

  protected:

    void                init();
    void                shutdown();
    void                startUpdateProcessing();
    void                clearMappings();
    void                loadMappings(QueryServer::ParamMappingFile_vec& parameterMappings);
    FILE*               openMappingFile(const std::string& mappingFile);
    void                updateMappings();
    void                updateMappings(T::ParamKeyType sourceParameterKeyType,T::ParamKeyType targetParameterKeyType,const std::string& mappingFile,QueryServer::ParamMappingFile_vec& parameterMappings);


  private:

    ConfigurationFile   mConfigurationFile;
    bool                mShutdownRequested;

    ContentServer_sptr  mContentServer;
    ContentServer_sptr  mContentServerCache;

    DataServer_sptr     mDataServer;
    DataServer_sptr     mDataServerClient;
    string_vec          mDataServerLuaFiles;
    bool                mDataServerRemote;
    bool                mDataServerCacheEnabled;
    std::string         mDataServerIor;

    QueryServer_sptr    mQueryServer;
    string_vec          mQueryServerLuaFiles;

    std::string         mGridConfigFile;
    bool                mPointCacheEnabled;
    uint                mPointCacheHitsRequired;
    uint                mPointCacheTimePeriod;
    bool                mRequestCounterEnabled;
    std::string         mRequestCounterFilename;
    std::string         mDataServerGridDirectory;

    std::string         mContentSourceType;
    std::string         mContentSourceRedisAddress;
    int                 mContentSourceRedisPort;
    std::string         mContentSourceRedisTablePrefix;
    std::string         mContentSourceCorbaIor;
    std::string         mContentSourceHttpUrl;

    bool                mContentCacheEnabled;
    uint                mContentCacheSortingFlags;

    bool                mContentServerProcessingLogEnabled;
    std::string         mContentServerProcessingLogFile;
    int                 mContentServerProcessingLogMaxSize;
    int                 mContentServerProcessingLogTruncateSize;
    Log                 mContentServerProcessingLog;
    bool                mContentServerDebugLogEnabled;
    std::string         mContentServerDebugLogFile;
    int                 mContentServerDebugLogMaxSize;
    int                 mContentServerDebugLogTruncateSize;
    Log                 mContentServerDebugLog;


    bool                mDataServerProcessingLogEnabled;
    std::string         mDataServerProcessingLogFile;
    int                 mDataServerProcessingLogMaxSize;
    int                 mDataServerProcessingLogTruncateSize;
    Log                 mDataServerProcessingLog;
    bool                mDataServerDebugLogEnabled;
    std::string         mDataServerDebugLogFile;
    int                 mDataServerDebugLogMaxSize;
    int                 mDataServerDebugLogTruncateSize;
    Log                 mDataServerDebugLog;

    bool                mQueryServerRemote;
    std::string         mQueryServerIor;
    bool                mQueryServerProcessingLogEnabled;
    std::string         mQueryServerProcessingLogFile;
    int                 mQueryServerProcessingLogMaxSize;
    int                 mQueryServerProcessingLogTruncateSize;
    Log                 mQueryServerProcessingLog;
    bool                mQueryServerDebugLogEnabled;
    std::string         mQueryServerDebugLogFile;
    int                 mQueryServerDebugLogMaxSize;
    int                 mQueryServerDebugLogTruncateSize;
    Log                 mQueryServerDebugLog;

    uint                mNumOfCachedGrids;
    uint                mMaxSizeOfCachedGridsInMegaBytes;

    std::string         mProducerFile;
    std::string         mProducerAliasFile;
    bool                mVirtualFilesEnabled;
    std::string         mVirtualFileDefinitions;
    bool                mContentPreloadEnabled;

    string_vec          mParameterAliasFiles;
    string_vec          mParameterMappingFiles;

    pthread_t           mThread;
    T::ParamKeyType     mMappingTargetKeyType;
    std::string         mParameterMappingUpdateFile_fmi;
    std::string         mParameterMappingUpdateFile_newbase;
    time_t              mParameterMappingUpdateTime;

    string_vec          mProducerList;
    time_t              mProducerList_updateTime;
    T::LevelInfoList    mLevelInfoList;
    time_t              mLevelInfoList_lastUpdate;
    ThreadLock          mThreadLock;

    boost::shared_ptr<Fmi::DEM>       mDem;
    QueryServer::AliasFile            mProducerAliases;
    QueryServer::AliasFileCollection  mParameterAliasFileCollection;

};

}  // namespace Grid
}  // namespace Engine
}  // namespace SmartMet
