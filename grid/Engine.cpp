#include "Engine.h"

#include <spine/Exception.h>
#include "grid-files/common/GeneralFunctions.h"
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
    mConfig.readFile(theConfigFile);

    mRedisAddress = "127.0.0.1";
    mRedisPort = 6379;

    if (!mConfig.exists("redis.address"))
      throw SmartMet::Spine::Exception(BCP, "The 'redis.address' attribute not specified in the config file");

    if (!mConfig.exists("redis.port"))
      throw SmartMet::Spine::Exception(BCP, "The 'redis.port' attribute not specified in the config file");

    if (!mConfig.exists("redis.tablePrefix"))
      throw SmartMet::Spine::Exception(BCP, "The 'redis.tablePrefix' attribute not specified in the config file");

    if (!mConfig.exists("grid-files.configDirectory"))
      throw SmartMet::Spine::Exception(BCP, "The 'grid-files.configDirectory' attribute not specified in the config file");

    if (!mConfig.exists("remote-content-server.enabled"))
      throw SmartMet::Spine::Exception(BCP, "The 'remote-content-server.enabled' attribute not specified in the config file");

    if (!mConfig.exists("remote-content-server.ior"))
      throw SmartMet::Spine::Exception(BCP, "The 'remote-content-server.ior' attribute not specified in the config file");

    if (!mConfig.exists("remote-data-server.enabled"))
      throw SmartMet::Spine::Exception(BCP, "The 'remote-data-server.enabled' attribute not specified in the config file");

    if (!mConfig.exists("remote-data-server.cache"))
      throw SmartMet::Spine::Exception(BCP, "The 'remote-data-server.cache' attribute not specified in the config file");

    if (!mConfig.exists("remote-data-server.ior"))
      throw SmartMet::Spine::Exception(BCP, "The 'remote-data-server.ior' attribute not specified in the config file");

    if (!mConfig.exists("remote-query-server.enabled"))
      throw SmartMet::Spine::Exception(BCP, "The 'remote-query-server.enabled' attribute not specified in the config file");

    if (!mConfig.exists("remote-query-server.ior"))
      throw SmartMet::Spine::Exception(BCP, "The 'remote-query-server.ior' attribute not specified in the config file");

    if (!mConfig.exists("local-data-server.processingLog.file"))
      throw SmartMet::Spine::Exception(BCP, "The 'local-data-server.processingLog.file' attribute not specified in the config file");

    if (!mConfig.exists("local-data-server.processingLog.maxSize"))
      throw SmartMet::Spine::Exception(BCP, "The 'local-data-server.processingLog.maxSize' attribute not specified in the config file");

    if (!mConfig.exists("local-data-server.processingLog.truncateSize"))
      throw SmartMet::Spine::Exception(BCP, "The 'local-data-server.processingLog.truncateSize' attribute not specified in the config file");

    if (!mConfig.exists("local-data-server.gridDirectory"))
      throw SmartMet::Spine::Exception(BCP, "The 'local-data-server.gridDirectory' attribute not specified in the config file");

    if (!mConfig.exists("local-data-server.cache.numOfGrids"))
      throw SmartMet::Spine::Exception(BCP, "The 'local-data-server.cache.numOfGrids' attribute not specified in the config file");

    if (!mConfig.exists("local-data-server.cache.maxUncompressedSizeInMegaBytes"))
      throw SmartMet::Spine::Exception(BCP, "The 'local-data-server.cache.maxUncompressedSizeInMegaBytes' attribute not specified in the config file");

    if (!mConfig.exists("local-data-server.cache.maxCompressedSizeInMegaBytes"))
      throw SmartMet::Spine::Exception(BCP, "The 'local-data-server.cache.maxCompressedSizeInMegaBytes' attribute not specified in the config file");

    if (!mConfig.exists("parameterFile"))
      throw SmartMet::Spine::Exception(BCP, "The 'parameterFile' attribute not specified in the config file");

    if (!mConfig.exists("producerFile"))
      throw SmartMet::Spine::Exception(BCP, "The 'producerFile' attribute not specified in the config file");

    mConfig.lookupValue("redis.address", mRedisAddress);
    mConfig.lookupValue("redis.port", mRedisPort);
    mConfig.lookupValue("redis.tablePrefix", mRedisTablePrefix);
    mConfig.lookupValue("grid-files.configDirectory", mGridConfigDirectory);
    mConfig.lookupValue("remote-content-server.enabled", mRemoteContentServerEnabled);
    mConfig.lookupValue("remote-content-server.ior", mRemoteContentServerIor);
    mConfig.lookupValue("remote-data-server.enabled", mRemoteDataServerEnabled);
    mConfig.lookupValue("remote-data-server.cache", mRemoteDataServerCache);
    mConfig.lookupValue("remote-data-server.ior", mRemoteDataServerIor);
    mConfig.lookupValue("remote-query-server.enabled", mRemoteQueryServerEnabled);
    mConfig.lookupValue("remote-query-server.ior", mRemoteQueryServerIor);
    mConfig.lookupValue("local-data-server.gridDirectory", mServerGridDirectory);
    mConfig.lookupValue("local-data-server.processingLog.file", mServerProcessingLogFile);
    mConfig.lookupValue("local-data-server.processingLog.maxSize", mServerProcessingLogMaxSize);
    mConfig.lookupValue("local-data-server.processingLog.truncateSize", mServerProcessingLogTruncateSize);
    mConfig.lookupValue("local-data-server.cache.numOfGrids", mNumOfCachedGrids);
    mConfig.lookupValue("local-data-server.cache.maxUncompressedSizeInMegaBytes", mMaxUncompressedMegaBytesOfCachedGrids);
    mConfig.lookupValue("local-data-server.cache.maxCompressedSizeInMegaBytes", mMaxCompressedMegaBytesOfCachedGrids);
    mConfig.lookupValue("parameterFile", mParameterFile);
    mConfig.lookupValue("producerFile", mProducerFile);


    // Initializing information that is needed for identifying the content of the grid files.

    SmartMet::Identification::gribDef.init(mGridConfigDirectory.c_str());


    // Initializing the size of the grid value cache.

    SmartMet::GRID::valueCache.init(mNumOfCachedGrids,mMaxUncompressedMegaBytesOfCachedGrids,mMaxCompressedMegaBytesOfCachedGrids);

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
    loadParameters();
    loadProducers();

    ContentServer::RedisImplementation *redis = new ContentServer::RedisImplementation();
    redis->init(mRedisAddress.c_str(),mRedisPort,mRedisTablePrefix.c_str());
    mContentServerRedis.reset(redis);

    ContentServer::ServiceInterface *cServer = NULL;
    DataServer::ServiceInterface *dServer = NULL;
    //QueryServer::ServiceInterface *qServer = NULL;


    if (mRemoteContentServerEnabled == "true"  &&  mRemoteContentServerIor.length() > 50)
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

    if (mRemoteDataServerEnabled == "true"  &&  mRemoteDataServerIor.length() > 50)
    {
      DataServer::Corba::ClientImplementation *client = new DataServer::Corba::ClientImplementation();
      client->init(mRemoteDataServerIor);

      if (mRemoteDataServerCache == "true")
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
      server->init(0,0,"NotRegistered","NotRegistered",mServerGridDirectory,cServer);
      //dServer->init(0,0,"NotRegistered","NotRegistered",mServerGridDirectory,cache);
      mDataServer.reset(server);
      server->startEventProcessing();
      dServer = server;

      if (mServerProcessingLogFile.length() > 0)
      {
        mProcessingLog.init(true,mServerProcessingLogFile.c_str(),mServerProcessingLogMaxSize,mServerProcessingLogTruncateSize);
        //cache->setProcessingLog(&mProcessingLog);
        server->setProcessingLog(&mProcessingLog);
      }
    }

    if (mRemoteQueryServerEnabled == "true"  &&  mRemoteQueryServerIor.length() > 50)
    {
      QueryServer::Corba::ClientImplementation *client = new QueryServer::Corba::ClientImplementation();
      client->init(mRemoteQueryServerIor);
      mQueryServer.reset(client);
      //qServer = client;
    }
    else
    {
      QueryServer::ServiceImplementation *server = new QueryServer::ServiceImplementation();
      server->init(cServer,dServer);
      mQueryServer.reset(server);
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





void Engine::loadParameters()
{
  try
  {

    FILE *file = fopen(mParameterFile.c_str(),"r");
    if (file == NULL)
    {
      SmartMet::Spine::Exception exception(BCP,"Cannot open the parameter file!");
      exception.addParameter("Filename",mParameterFile);
      throw exception;
    }

    char st[1000];

    while (!feof(file))
    {
      if (fgets(st,1000,file) != NULL  &&  st[0] != '#')
      {
        bool ind = false;
        char *field[100];
        uint c = 1;
        field[0] = st;
        char *p = st;
        while (*p != '\0'  &&  c < 100)
        {
          if (*p == '"')
            ind = !ind;

          if ((*p == ','  ||  *p == ';'  || *p == '\n') && !ind)
          {
            *p = '\0';
            p++;
            field[c] = p;
            c++;
          }
          else
          {
            p++;
          }
        }

        if (c > 6)
        {
          ParameterInfo rec;

          if (field[0][0] != '\0')
            rec.mParameterName = field[0];

          if (field[1][0] != '\0')
            rec.mParameterKeyType = (T::ParamKeyType)atoi(field[1]);

          if (field[2][0] != '\0')
            rec.mParamKey = field[2];

          if (field[3][0] != '\0')
            rec.mParamLevelIdType = (T::ParamLevelIdType)atoi(field[3]);

          if (field[4][0] != '\0')
            rec.mParamLevelId = (T::ParamLevelId)atoi(field[4]);

          if (field[5][0] != '\0')
            rec.mParamLevel = (T::ParamLevel)atoi(field[5]);

          if (field[6][0] != '\0')
            rec.mInterpolationMethod = (T::InterpolationMethod)atoi(field[6]);

          rec.print(std::cout,0,0);
          mParameters.push_back(rec);
        }
      }
    }
    fclose(file);
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,NULL);
  }
}





void Engine::loadProducers()
{
  try
  {

    FILE *file = fopen(mProducerFile.c_str(),"r");
    if (file == NULL)
    {
      SmartMet::Spine::Exception exception(BCP,"Cannot open the producer file!");
      exception.addParameter("Filename",mProducerFile);
      throw exception;
    }

    char st[1000];

    while (!feof(file))
    {
      if (fgets(st,1000,file) != NULL  &&  st[0] != '#')
      {
        bool ind = false;
        char *field[100];
        uint c = 1;
        field[0] = st;
        char *p = st;
        while (*p != '\0'  &&  c < 100)
        {
          if (*p == '"')
            ind = !ind;

          if ((*p == ','  ||  *p == ';'  || *p == '\n') && !ind)
          {
            *p = '\0';
            p++;
            field[c] = p;
            c++;
          }
          else
          {
            p++;
          }
        }

        if (c >= 2 && field[0][0] != '\0' &&  field[1][0] != '\0')
        {
          mProducerVector.push_back(std::pair<std::string,T::GeometryId>(std::string(field[0]),atoi(field[1])));
        }
      }
    }
    fclose(file);
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,NULL);
  }
}





void Engine::getParameterInfoList(std::string parameterName,std::vector<ParameterInfo>& infoList)
{
  try
  {
    for (auto it=mParameters.begin(); it != mParameters.end(); ++it)
    {
      if (strcasecmp(it->mParameterName.c_str(),parameterName.c_str()) == 0)
      {
        infoList.push_back(*it);
      }
    }
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,NULL);
  }
}





void Engine::getGeometryIdListByCoordinates(std::vector<T::Coordinate>& coordinates,std::set<T::GeometryId>& geometryIdList)
{
  try
  {
    std::map<T::GeometryId,uint> countList;
    uint maxCount = 0;

    for (auto coordinate = coordinates.begin(); coordinate != coordinates.end(); ++coordinate)
    {
      std::set<T::GeometryId> idList;
      Identification::gribDef.getGeometryIdListByLatLon(coordinate->y(),coordinate->x(),idList);

      for (auto g = idList.begin(); g != idList.end(); ++g)
      {
        auto c = countList.find(*g);
        if (c != countList.end())
        {
          c->second++;
          if (c->second > maxCount)
            maxCount = c->second;
        }
        else
        {
          countList.insert(std::pair<T::GeometryId,uint>(*g,1));
        }
      }
    }

    for (auto it = countList.begin(); it != countList.end(); ++it)
    {
      if (it->second == maxCount)
        geometryIdList.insert(it->first);
    }
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,NULL);
  }
}





int Engine::getGridValues(std::vector<std::string> producerNameList,
                        std::string originTime,
                        T::ParamKeyType parameterKeyType,
                        T::ParamId paramKey,
                        T::ParamLevelIdType paramLevelIdType,
                        T::ParamLevelId paramLevelId,
                        T::ParamLevel paramLevel,
                        T::ForecastType forecastType,
                        T::ForecastNumber forecastNumber,
                        std::string forecastTime,
                        std::vector<T::Coordinate> coordinates,
                        bool areaSearch,
                        T::GridValueList& valueList)
{
  try
  {
    if (coordinates.size() == 0)
      return -1; // No coordinates defined

    std::vector<ParameterInfo> parameterInfoList;

    if (parameterKeyType == T::ParamKeyType::UNKNOWN)
      getParameterInfoList(paramKey,parameterInfoList);

    if (parameterInfoList.size() == 0)
    {
      // Parameter not found from the parameter configuration file.

      switch (parameterKeyType)
      {
        case T::ParamKeyType::UNKNOWN:
        {
          // Testing if the parameterKey is a Radon name
          Identification::ParameterDefinition_fmi_cptr parameterDef = SmartMet::Identification::gribDef.mMessageIdentifier_fmi.getParameterDefByName(paramKey);

          // Testing if the parameterKey is a Newbase name
          if (parameterDef == NULL)
            parameterDef = SmartMet::Identification::gribDef.mMessageIdentifier_fmi.getParameterDefByNewbaseName(paramKey);

          if (parameterDef == NULL)
            return -2; // Unknown parameter

          ParameterInfo parameterInfo;
          parameterInfo.mParameterName = paramKey;
          parameterInfo.mParameterKeyType = T::ParamKeyType::FMI_ID;
          parameterInfo.mParamKey = parameterDef->mFmiParameterId;
          parameterInfo.mParamLevelIdType = paramLevelIdType;
          parameterInfo.mParamLevelId = paramLevelId;
          parameterInfo.mParamLevel = paramLevel;
          parameterInfo.mInterpolationMethod = parameterDef->mInterpolationMethod;
          parameterInfoList.push_back(parameterInfo);
        }
        break;

        case T::ParamKeyType::FMI_ID:
        {
          Identification::ParameterDefinition_fmi_cptr parameterDef = SmartMet::Identification::gribDef.mMessageIdentifier_fmi.getParameterDefById(paramKey);
          if (parameterDef == NULL)
            return -1; // Unknown parameter

          ParameterInfo parameterInfo;
          parameterInfo.mParameterName = paramKey;
          parameterInfo.mParameterKeyType = T::ParamKeyType::FMI_ID;
          parameterInfo.mParamKey = parameterDef->mFmiParameterId;
          parameterInfo.mParamLevelIdType = paramLevelIdType;
          parameterInfo.mParamLevelId = paramLevelId;
          parameterInfo.mParamLevel = paramLevel;
          parameterInfo.mInterpolationMethod = parameterDef->mInterpolationMethod;
          parameterInfoList.push_back(parameterInfo);
        }
        break;

        case T::ParamKeyType::FMI_NAME:
        {
          Identification::ParameterDefinition_fmi_cptr parameterDef = SmartMet::Identification::gribDef.mMessageIdentifier_fmi.getParameterDefByName(paramKey);
          if (parameterDef == NULL)
            return -2; // Unknown parameter

          ParameterInfo parameterInfo;
          parameterInfo.mParameterName = paramKey;
          parameterInfo.mParameterKeyType = T::ParamKeyType::FMI_ID;
          parameterInfo.mParamKey = parameterDef->mFmiParameterId;
          parameterInfo.mParamLevelIdType = paramLevelIdType;
          parameterInfo.mParamLevelId = paramLevelId;
          parameterInfo.mParamLevel = paramLevel;
          parameterInfo.mInterpolationMethod = parameterDef->mInterpolationMethod;
          parameterInfoList.push_back(parameterInfo);
        }
        break;

        case T::ParamKeyType::GRIB_ID:
          break;

        case T::ParamKeyType::NEWBASE_ID:
        {
          Identification::ParameterDefinition_fmi_cptr parameterDef = SmartMet::Identification::gribDef.mMessageIdentifier_fmi.getParameterDefByNewbaseId(paramKey);
          if (parameterDef == NULL)
            return -2; // Unknown parameter

          ParameterInfo parameterInfo;
          parameterInfo.mParameterName = paramKey;
          parameterInfo.mParameterKeyType = T::ParamKeyType::FMI_ID;
          parameterInfo.mParamKey = parameterDef->mFmiParameterId;
          parameterInfo.mParamLevelIdType = paramLevelIdType;
          parameterInfo.mParamLevelId = paramLevelId;
          parameterInfo.mParamLevel = paramLevel;
          parameterInfo.mInterpolationMethod = parameterDef->mInterpolationMethod;
          parameterInfoList.push_back(parameterInfo);
        }
        break;

        case T::ParamKeyType::NEWBASE_NAME:
        {
          Identification::ParameterDefinition_fmi_cptr parameterDef = SmartMet::Identification::gribDef.mMessageIdentifier_fmi.getParameterDefByNewbaseName(paramKey);
          if (parameterDef == NULL)
            return -2; // Unknown parameter

          ParameterInfo parameterInfo;
          parameterInfo.mParameterName = paramKey;
          parameterInfo.mParameterKeyType = T::ParamKeyType::FMI_ID;
          parameterInfo.mParamKey = parameterDef->mFmiParameterId;
          parameterInfo.mParamLevelIdType = paramLevelIdType;
          parameterInfo.mParamLevelId = paramLevelId;
          parameterInfo.mParamLevel = paramLevel;
          parameterInfo.mInterpolationMethod = parameterDef->mInterpolationMethod;
          parameterInfoList.push_back(parameterInfo);
        }
        break;

        case T::ParamKeyType::CDM_ID:
          break;

        case T::ParamKeyType::CDM_NAME:
          break;

        default:
          return -3; // Unknown parameter type
      }
    }

    // Getting geometries that support support the given coordinates

    std::set<T::GeometryId> geometryIdList;
    getGeometryIdListByCoordinates(coordinates,geometryIdList);
    // SmartMet::Identification::gribDef.getGeometryIdListByLatLon(coordinates[0].y(),coordinates[0].x(),geometryIdList);


    if (producerNameList.size() > 0)
    {
      // We have a list of producer names
      return -200;
    }
    else
    {
      // No producers defined. We should go through the producer list that is defined
      // in the configuration file.

      for (auto it = mProducerVector.begin(); it != mProducerVector.end(); ++it)
      {
        std::string producerName = it->first;

        T::GeometryId producerGeometryId = it->second;

        if (geometryIdList.find(producerGeometryId) != geometryIdList.end())
        {
          // The current producer supports a geometry where the current coordinates can be found.
          T::ProducerInfo producerInfo;
          if (mContentServerCache->getProducerInfoByName(0,producerName,producerInfo) == 0)
          {
            T::GenerationInfoList generationInfoList;
            int result = mContentServerCache->getGenerationInfoListByProducerId(0,producerInfo.mProducerId,generationInfoList);
            T::GenerationInfo *generationInfo = generationInfoList.getLastGenerationInfoByProducerId(producerInfo.mProducerId);

            if (generationInfo != NULL)
            {
              // We have now a valid generation

              //generationInfo->print(std::cout,0,0);

              for (auto pInfo = parameterInfoList.begin(); pInfo != parameterInfoList.end(); ++pInfo)
              {
                //printf("*** SEARCH %f,%f\n",lat,lon);
                //pInfo->print(std::cout,0,0);

                T::ContentInfoList contentList;
                int result = mContentServerCache->getContentListByParameterGenerationIdAndForecastTime(0,generationInfo->mGenerationId,pInfo->mParameterKeyType,pInfo->mParamKey,pInfo->mParamLevelIdType,pInfo->mParamLevelId,pInfo->mParamLevel,forecastType,forecastNumber,producerGeometryId,forecastTime,contentList);
  //              int result = mContentServerCache->getContentListByParameterGenerationIdAndForecastTime(0,generationInfo->mGenerationId,pInfo->mParameterKeyType,pInfo->mParamKey,T::ParamLevelIdType::IGNORE,0,0,forecastType,forecastNumber,producerGeometryId,forecastTime,contentList);

  //            contentList.print(std::cout,0,0);
                if (contentList.getLength() > 0)
                {
                  // We found content information close to the current forecast time

                  if (contentList.getLength() == 1)
                  {
                    T::ContentInfo *contentInfo = contentList.getContentInfoByIndex(0);
                    if (contentInfo->mForecastTime == forecastTime)
                    {
                      // We found a grid which forecast time is exactly the same as the requested forecast time.

                      if (!areaSearch)
                        int result = mDataServer->getGridValueListByPointList(0,contentInfo->mFileId,contentInfo->mMessageIndex,T::CoordinateType::LATLON_COORDINATES,coordinates,pInfo->mInterpolationMethod,valueList);
                      else
                        int result = mDataServer->getGridValueListByPolygon(0,contentInfo->mFileId,contentInfo->mMessageIndex,T::CoordinateType::LATLON_COORDINATES,coordinates,valueList);

                      return 0;
                    }
                    else
                    {
                      // ERROR: If there is only one content record in place then its forecast time should match to the requested forecast time.
                    }
                  }

                  if (contentList.getLength() == 2)
                  {
                    T::ContentInfo *contentInfo1 = contentList.getContentInfoByIndex(0);
                    T::ContentInfo *contentInfo2 = contentList.getContentInfoByIndex(1);

                    boost::posix_time::ptime forecastTime1 = toTimeStamp(contentInfo1->mForecastTime);
                    boost::posix_time::ptime forecastTime2 = toTimeStamp(contentInfo2->mForecastTime);

                    boost::posix_time::ptime forecastTimeParam = toTimeStamp(forecastTime);


                    if (contentInfo1->mForecastTime < forecastTime  &&  contentInfo2->mForecastTime > forecastTime)
                    {
                      T::GridValueList list1;
                      T::GridValueList list2;

                      time_t diff = toTimeT(forecastTimeParam) - toTimeT(forecastTime1);
                      time_t ttDiff = toTimeT(forecastTime2) - toTimeT(forecastTime1);

                      int result1 = 0;
                      int result2 = 0;
                      if (!areaSearch)
                      {
                        result1 = mDataServer->getGridValueListByPointList(0,contentInfo1->mFileId,contentInfo1->mMessageIndex,T::CoordinateType::LATLON_COORDINATES,coordinates,pInfo->mInterpolationMethod,list1);
                        result2 = mDataServer->getGridValueListByPointList(0,contentInfo2->mFileId,contentInfo2->mMessageIndex,T::CoordinateType::LATLON_COORDINATES,coordinates,pInfo->mInterpolationMethod,list2);
                      }
                      else
                      {
                        result1 = mDataServer->getGridValueListByPolygon(0,contentInfo1->mFileId,contentInfo1->mMessageIndex,T::CoordinateType::LATLON_COORDINATES,coordinates,list1);
                        result2 = mDataServer->getGridValueListByPolygon(0,contentInfo2->mFileId,contentInfo2->mMessageIndex,T::CoordinateType::LATLON_COORDINATES,coordinates,list2);
                      }

                      uint len1 = list1.getLength();
                      uint len2 = list2.getLength();

                      if (result1 == 0  &&  result2 == 0  &&  len1 == len2)
                      {
                        for (uint t=0; t<len1; t++)
                        {
                          T::GridValue *val1 = list1.getGridValueByIndex(t);
                          T::GridValue *val2 = list2.getGridValueByIndex(t);

                          T::ParamValue valueDiff = val2->mValue - val1->mValue;
                          T::ParamValue valueStep = valueDiff / (T::ParamValue)ttDiff;

                          T::GridValue *rec = new T::GridValue();
                          rec->mX = val1->mX;
                          rec->mY = val1->mY;
                          rec->mValue = val1->mValue + (T::ParamValue)(diff * valueStep);
                          valueList.addGridValue(rec);
                        }
                      }
                      return 0;
                    }
                    else
                    {
                      // ERROR: The given forecast time should been between the found content times.
                    }
                  }
                }
              }
            }
          }
        }
      }
    }

    return -100;
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
