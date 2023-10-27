#pragma once
#include <spine/SmartMetPlugin.h>
#include <spine/Reactor.h>
#include <spine/HTTP.h>
#include <grid-files/common/ConfigurationFile.h>
#include <grid-content/contentServer/http/server/ServerInterface.h>
#include <grid-content/sessionManagement/implementation/ServiceImplementation.h>



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
    bool                  requestHandler(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    void                  browserContent(SessionManagement::SessionInfo& session,std::ostringstream& output);
    unsigned long long    getFlags();
    void                  setFlags(unsigned long long flags);

  private:

    bool                  page_start(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_configuration(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_configurationFile(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_producerFile(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_parameterMappingFiles(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_parameterMappingFile(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_parameterAliasFiles(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_parameterAliasFile(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_producerMappingFiles(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_producerMappingFile(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_luaFiles(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_luaFile(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_contentServer(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_dataServer(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_queryServer(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_contentInformation(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_producers(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_generations(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_files(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_contentList(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_contentServer_debugLog(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_contentServer_processingLog(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_dataServer_debugLog(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_dataServer_processingLog(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_queryServer_debugLog(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_queryServer_processingLog(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);

    bool                  includeFile(std::ostringstream& output,const char *filename);
    void                  updateSessionParameters(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);


    Engine*               mGridEngine = nullptr;
    ContentServer_sptr    mMainContentServer;
    ContentServer_sptr    mCacheContentServer;
    ConfigurationFile     mConfigurationFile;
    uint                  mCachedFileId[20];
    T::ContentInfoList    mCachedContentInfoList[20];
    uint                  mCachedContentCount;
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
