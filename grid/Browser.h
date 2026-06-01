#pragma once
#include <array>
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

typedef std::map<std::string,std::string> Filenames;                          //!< Map of resource name to file path for statically served browser assets.
typedef std::vector<std::pair<std::string,std::string>> UrlPath;              //!< Ordered (URL path, label) pairs used for breadcrumb navigation.
typedef std::shared_ptr<ContentServer::ServiceInterface> ContentServer_sptr;  //!< Shared pointer to a ContentServer service interface.

class Engine;


// ====================================================================================
/*! \brief Web-based admin browser for the grid engine.
 *
 *  Handles HTTP requests from the grid-admin plugin and renders HTML pages that expose
 *  live state from all three embedded servers (ContentServer, DataServer, QueryServer):
 *  producer/generation/file/content listings, server logs, parameter mapping files, and
 *  Lua function files.  Pages are gated by the `Flags` bitmask. */
// ====================================================================================

class Browser
{
  public:

                          Browser();
    virtual               ~Browser();

    void                  init(const char *theConfigurationFile,Engine *theGridEngine);
    bool                  requestHandler(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    void                  browserContent(SessionManagement::SessionInfo& session,std::ostringstream& output);
    UInt64                getFlags();
    void                  setFlags(UInt64 flags);

  private:

    bool                  page_start(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_configuration(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_configurationFile(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_stateInformation(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
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

    /*! \brief Shared rendering helper for the six per-server log-viewer pages.  Renders
     *  the standard nav header, the per-log enable/disable/clear toolbar (gated by the
     *  logModificationEnabled flag and the user's admin group), and the tail of the log
     *  file.  Pure presentation; caller resolves the log pointer + filename for the
     *  requested server and chooses how many trailing lines to show (~100 for processing
     *  logs, ~10000 for debug logs).  readEofLines() returns lines newest-first, so
     *  chronological=true reverses the iteration to display oldest-first (used by debug
     *  logs); chronological=false leaves them newest-first (used by processing logs). */
    bool                  page_serverLog(SessionManagement::SessionInfo& session,
                                         const Spine::HTTP::Request& theRequest,
                                         Spine::HTTP::Response& theResponse,
                                         Log *log,
                                         const std::string& filename,
                                         const char *serverDisplayName,
                                         const char *logTypeDisplayName,
                                         const char *serverPageName,
                                         int maxLines,
                                         bool chronological);

    bool                  includeFile(std::ostringstream& output,const char *filename);
    void                  updateSessionParameters(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);


    static constexpr std::size_t CONTENT_CACHE_SIZE = 20;

    Engine*               mGridEngine = nullptr;         //!< Back-pointer to the owning Engine (not owned).
    ContentServer_sptr    mMainContentServer;            //!< Main (source) content server for metadata queries.
    ContentServer_sptr    mCacheContentServer;           //!< Cache-layer content server for fast read access.
    ConfigurationFile     mConfigurationFile;            //!< Parsed engine configuration file.
    std::array<T::FileId, CONTENT_CACHE_SIZE>          mCachedFileId{};           //!< Ring buffer of recently accessed file IDs.
    std::array<T::ContentInfoList, CONTENT_CACHE_SIZE> mCachedContentInfoList;    //!< Cached content info lists corresponding to mCachedFileId.
    uint                  mCachedContentCount;           //!< Number of valid entries in the content cache ring buffer.
    Filenames             mFilenames;                    //!< Map of resource name to file path for static browser assets.
    UInt64                mFlags;                        //!< Feature flags controlling which browser actions are permitted.


  public:

    /*! \brief Feature flag constants that control browser capabilities. */
    class Flags
    {
      public:
        static const UInt64 contentModificationEnabled  = 1 << 0;  //!< Allow content modifications (add/delete) via the browser.
        static const UInt64 logModificationEnabled      = 1 << 1;  //!< Allow log level changes via the browser.
    };
};

}  // namespace Grid
}  // namespace Engine
}  // namespace SmartMet

// ======================================================================
