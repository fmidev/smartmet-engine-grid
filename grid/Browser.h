#pragma once
#include <spine/SmartMetPlugin.h>
#include <spine/Reactor.h>
#include <spine/HTTP.h>
#include <grid-files/common/ConfigurationFile.h>
#include <grid-content/contentServer/http/server/ServerInterface.h>



namespace SmartMet
{


namespace Engine
{
namespace Grid
{

typedef std::map<std::string,std::string> Filenames;
typedef std::vector<std::pair<std::string,std::string>> UrlPath;
typedef std::shared_ptr<ContentServer::ServiceInterface> ContentServer_sptr;

class Engine;


class Browser
{
  public:

                          Browser();
    virtual               ~Browser();

    void                  init(const char *theConfigurationFile,ContentServer_sptr theMainContentServer,Engine *theGridEngine);
    bool                  requestHandler(const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    void                  browserContent(std::ostringstream& output);
    unsigned long long    getFlags();
    void                  setFlags(unsigned long long flags);

  private:

    bool                  page_start(const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_configuration(const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_configurationFile(const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_producerFile(const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_parameterMappingFiles(const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_parameterMappingFile(const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_parameterAliasFiles(const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_parameterAliasFile(const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_producerMappingFiles(const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_producerMappingFile(const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_luaFiles(const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_luaFile(const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_contentServer(const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_dataServer(const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_queryServer(const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_contentInformation(const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_producers(const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_generations(const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_files(const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_contentList(const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_contentServer_debugLog(const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_contentServer_processingLog(const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_dataServer_debugLog(const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_dataServer_processingLog(const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_queryServer_debugLog(const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_queryServer_processingLog(const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);

    bool                  includeFile(std::ostringstream& output,const char *filename);


    Engine*               mGridEngine = nullptr;
    ContentServer_sptr    mMainContentServer;
    ContentServer_sptr    mCacheContentServer;
    ConfigurationFile     mConfigurationFile;
    uint                  mCachedFileId;
    T::ContentInfoList    mCachedContentInfoList;
    Filenames             mFilenames;
    unsigned long long    mFlags;


  public:

    class Flags
    {
      public:
        static const unsigned long long   contentModificationEnabled  = 1 << 0;
        static const unsigned long long   logModificationEnabled      = 1 << 1;
    };
};

}  // namespace Grid
}  // namespace Engine
}  // namespace SmartMet

// ======================================================================
