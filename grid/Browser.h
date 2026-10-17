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

using Filenames = std::map<std::string,std::string>;                          //!< Map of resource name to file path for statically served browser assets.
using UrlPath = std::vector<std::pair<std::string,std::string>>;              //!< Ordered (URL path, label) pairs used for breadcrumb navigation.
using ContentServer_sptr = std::shared_ptr<ContentServer::ServiceInterface>;  //!< Shared pointer to a ContentServer service interface.

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
    void                  browserContent(const SessionManagement::SessionInfo& session,std::ostringstream& output);
    UInt64                getFlags() const;
    void                  setFlags(UInt64 flags);

  private:

    bool                  page_start(const SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_configuration(const SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_configurationFile(const SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_stateInformation(const SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_producerFile(const SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_parameterMappingFiles(const SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_parameterMappingFile(const SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_parameterAliasFiles(const SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_parameterAliasFile(const SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_producerMappingFiles(const SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_producerMappingFile(const SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_luaFiles(const SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_luaFile(const SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_contentServer(const SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_dataServer(const SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_queryServer(const SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_contentInformation(const SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_producers(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    void                  modifyProducer(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,ContentServer::ServiceInterface& contentServer,uint mode,T::ProducerInfo& pInfo,T::ProducerId& producerId);
    static void           writeProducerRows(std::ostringstream& output,T::ProducerInfoList& producerInfoList,uint maxRecords,uint startProducerIndex,T::ProducerId producerId,bool mainSource,T::ProducerInfo& pInfo);
    static void           writeProducerEditor(std::ostringstream& output,uint mode,const T::ProducerInfo& pInfo,T::ProducerId producerId);
    bool                  page_generations(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    static void           readGenerationParameters(const Spine::HTTP::Request& theRequest,T::GenerationInfo& gInfo,uint& mode);
    void                  modifyGeneration(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,ContentServer::ServiceInterface& contentServer,uint mode,T::GenerationInfo& gInfo,T::GenerationId& generationId);
    static void           writeGenerationRows(std::ostringstream& output,T::GenerationInfoList& generationInfoList,uint maxRecords,uint startGenerationIndex,T::GenerationId generationId,bool mainSource,T::GenerationInfo& gInfo);
    static void           writeGenerationEditor(std::ostringstream& output,uint mode,const T::GenerationInfo& gInfo,T::GenerationId generationId,uint startGenerationIndex);
    bool                  page_files(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    static void           readFileParameters(const Spine::HTTP::Request& theRequest,T::FileInfo& fInfo,uint& mode);
    void                  modifyFile(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,ContentServer::ServiceInterface& contentServer,uint mode,T::FileInfo& fInfo,T::FileId& fileId);
    void                  getFileInfoPage(ContentServer::ServiceInterface& contentServer,T::GenerationId generationId,T::FileId endFileId,uint maxRecords,T::FileId& startFileId,T::FileInfoList& fileInfoList);
    static void           writeFileRows(std::ostringstream& output,T::FileInfoList& fileInfoList,uint maxRecords,T::FileId fileId,T::FileId startFileId,bool mainSource,T::FileInfo& fInfo,T::FileId& nextFileId);
    static void           writeFileEditor(std::ostringstream& output,uint mode,const T::FileInfo& fInfo,T::FileId fileId,T::FileId startFileId);
    bool                  page_contentList(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_contentServer_debugLog(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_contentServer_processingLog(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_dataServer_debugLog(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_dataServer_processingLog(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_queryServer_debugLog(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);
    bool                  page_queryServer_processingLog(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse);

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
