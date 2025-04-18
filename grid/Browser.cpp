#include "Browser.h"
#include "Engine.h"
#include <grid-files/grid/GridFile.h>


namespace SmartMet
{

namespace Engine
{
namespace Grid
{


#define MODE_NONE              0
#define MODE_ADD_REQUEST       1
#define MODE_EDIT_REQUEST      2
#define MODE_DELETE_REQUEST    3
#define MODE_GENERATE_REQUEST  4

#define MODE_LOG_ENABLE        10
#define MODE_LOG_DISABLE       11
#define MODE_LOG_CLEAR         12

#define MODE_ADD_CONFIRM       101
#define MODE_EDIT_CONFIRM      102
#define MODE_DELETE_CONFIRM    103
#define MODE_GENERATE_CONFIRM  104


Browser::Browser()
{
  try
  {
    for (uint t=0; t<20; t++)
      mCachedFileId[t] = 0;

    mFlags = 0;
    mCachedContentCount = 0;
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}





Browser::~Browser()
{
}





void Browser::init(const char *theConfigurationFile,ContentServer_sptr theMainContentServer,Engine *theGridEngine)
{
  try
  {
    mConfigurationFile.readFile(theConfigurationFile);
    mGridEngine = theGridEngine;
    mMainContentServer = theMainContentServer;
    mCacheContentServer = mGridEngine->getContentServer_sptr();
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}





void Browser::setFlags(unsigned long long flags)
{
  try
  {
    mFlags = flags;
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}





unsigned long long Browser::getFlags()
{
  try
  {
    return mFlags;
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}





bool Browser::includeFile(std::ostringstream& output,const char *filename)
{
  try
  {
    FILE *file = fopen(filename,"r");
    if (file == nullptr)
      return false;

    char st[100000];
    while (!feof(file))
    {
      if (fgets(st,100000,file) != 0)
        output << st;
    }
    fclose(file);

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}




void Browser::updateSessionParameters(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse)
{
  try
  {
    auto v = theRequest.getParameter("producerId");
    if (v)
      session.setAttribute("grid-engine","producerId",v->c_str());

    v = theRequest.getParameter("producerName");
    if (v)
      session.setAttribute("grid-engine","producerName",v->c_str());

    v = theRequest.getParameter("startProducerIndex");
    if (v)
      session.setAttribute("grid-engine","startProducerIndex",v->c_str());

    v = theRequest.getParameter("generationId");
    if (v)
      session.setAttribute("grid-engine","generationId",v->c_str());

    v = theRequest.getParameter("generationName");
    if (v)
      session.setAttribute("grid-engine","generationName",v->c_str());

    v = theRequest.getParameter("startGenerationIndex");
    if (v)
      session.setAttribute("grid-engine","startGenerationIndex",v->c_str());

    v = theRequest.getParameter("fileId");
    if (v)
      session.setAttribute("grid-engine","fileId",v->c_str());

    v = theRequest.getParameter("startFileId");
    if (v)
      session.setAttribute("grid-engine","startFileId",v->c_str());

    v = theRequest.getParameter("endFileId");
    if (v)
      session.setAttribute("grid-engine","endFileId",v->c_str());

    v = theRequest.getParameter("startMessageIndex");
    if (v)
      session.setAttribute("grid-engine","startMessageIndex",v->c_str());

    v = theRequest.getParameter("messageIndex");
    if (v)
      session.setAttribute("grid-engine","messageIndex",v->c_str());

    v = theRequest.getParameter("source");
    if (v)
      session.setAttribute("grid-engine","source",v->c_str());


    //session.print(std::cout,0,0);
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}




bool Browser::page_contentList(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse)
{
  try
  {
    ContentServer_sptr contentServer = mMainContentServer;
    std::string sourceStr = "main";
    bool mainSource = true;

    session.getAttribute("grid-engine","source",sourceStr);

    if (sourceStr == "cache")
    {
      contentServer = mCacheContentServer;
      mainSource = false;
    }

    T::ContentInfo cInfo;
    uint nextMessageIndex = 0;
    uint maxRecords = 30;

    uint producerId = 0;
    session.getAttribute("grid-engine","producerId",producerId);

    std::string producerName;
    session.getAttribute("grid-engine","producerName",producerName);

    uint generationId = 0;
    session.getAttribute("grid-engine","generationId",generationId);

    std::string generationName;
    session.getAttribute("grid-engine","generationName",generationName);

    uint fileId = 0;
    session.getAttribute("grid-engine","fileId",fileId);

    uint startMessageIndex = 0;
    session.getAttribute("grid-engine","startMessageIndex",startMessageIndex);

    uint messageIndex = 0;
    session.getAttribute("grid-engine","messageIndex",messageIndex);

    uint mode = 0;
    auto v = theRequest.getParameter("mode");
    if (v)
      mode = toUInt32(*v);

    if (producerId == 0 || generationId == 0  ||  fileId == 0  ||  messageIndex == 0xFFFFFFFF)
      mode = 0;

    if ((mFlags & Flags::contentModificationEnabled)  && mainSource  &&  mode >= 100 && (session.mUserInfo.getUserId() == 0 || session.mUserInfo.isUserGroupMember("grid-admin")))
    {
      cInfo.mProducerId = producerId;
      cInfo.mGenerationId = generationId;
      cInfo.mFileId = fileId;
      cInfo.mMessageIndex = messageIndex;

      auto v = theRequest.getParameter("filePosition");
      if (v)
        cInfo.mFilePosition = atoll(v->c_str());

      v = theRequest.getParameter("messageSize");
      if (v)
        cInfo.mMessageSize = atoi(v->c_str());

      v = theRequest.getParameter("fmiParameterName");
      if (v)
        cInfo.setFmiParameterName(v->c_str());

      v = theRequest.getParameter("fmiParameterId");
      if (v)
        cInfo.mFmiParameterId = atoi(v->c_str());

      v = theRequest.getParameter("fmiParameterLevelId");
      if (v)
        cInfo.mFmiParameterLevelId = atoi(v->c_str());

      v = theRequest.getParameter("parameterLevel");
      if (v)
        cInfo.mParameterLevel = atoi(v->c_str());

      v = theRequest.getParameter("forecastType");
      if (v)
        cInfo.mForecastType = atoi(v->c_str());

      v = theRequest.getParameter("forecastNumber");
      if (v)
        cInfo.mForecastNumber = atoi(v->c_str());

      v = theRequest.getParameter("geometryId");
      if (v)
        cInfo.mGeometryId = atoi(v->c_str());


      v = theRequest.getParameter("aggregationId");
      if (v)
        cInfo.mAggregationId = atoi(v->c_str());

      v = theRequest.getParameter("aggregationPeriod");
      if (v)
        cInfo.mAggregationPeriod = atoi(v->c_str());

      v = theRequest.getParameter("processingTypeId");
      if (v)
        cInfo.mProcessingTypeId = atoi(v->c_str());

      v = theRequest.getParameter("processingTypeValue1");
      if (v)
        cInfo.mProcessingTypeValue1 = atof(v->c_str());

      v = theRequest.getParameter("processingTypeValue2");
      if (v)
        cInfo.mProcessingTypeValue2 = atof(v->c_str());

      v = theRequest.getParameter("flags");
      if (v)
        cInfo.mFlags = atoi(v->c_str());

      v = theRequest.getParameter("sourceId");
      if (v)
        cInfo.mSourceId = atoi(v->c_str());

      v = theRequest.getParameter("forecastTime");
      if (v)
      {
        try
        {
          cInfo.setForecastTime(*v);
        }
        catch (...)
        {
          mode = 0;
        }
      }

      v = theRequest.getParameter("deletionTime");
      if (v)
      {
        try
        {
          cInfo.mDeletionTime = utcTimeToTimeT(*v);
        }
        catch (...)
        {
          mode = 0;
        }
      }

      v = theRequest.getParameter("modificationTime");
      if (v)
      {
        try
        {
          cInfo.mModificationTime = utcTimeToTimeT(*v);
        }
        catch (...)
        {
          mode = 0;
        }
      }

      switch (mode)
      {
        case MODE_ADD_CONFIRM:
        {
          int res = contentServer->addContentInfo(0,cInfo);
          if (res != 0)
          {
            // Content addition failed
          }
          else
          {
            for (uint t=0; t<20; t++)
            {
              if (mCachedFileId[t] == fileId)
                mCachedFileId[t] = 0;
            }
          }
        }
        break;

        case MODE_EDIT_CONFIRM:
        {
          int res = contentServer->setContentInfo(0,cInfo);
          if (res != 0)
          {
            // Content modification failed
          }
          else
          {
            for (uint t=0; t<20; t++)
            {
              if (mCachedFileId[t] == fileId)
                mCachedFileId[t] = 0;
            }
          }
        }
        break;

        case MODE_DELETE_CONFIRM:
        {
          int res = contentServer->deleteContentInfo(0,fileId,messageIndex);
          if (res != 0)
          {
            // Content deletion failed
          }
          else
          {
            for (uint t=0; t<20; t++)
            {
              if (mCachedFileId[t] == fileId)
              {
                mCachedContentInfoList[t].deleteContentInfoByFileIdAndMessageIndex(fileId,messageIndex);
                t = 20;
              }
            }
            messageIndex = 0xFFFFFFFF;
            session.setAttribute("grid-engine","messageIndex",messageIndex);
          }
        }
        break;
      }
      mode = MODE_NONE;
    }

    std::ostringstream output;

    output << "<HTML>\n";
    output << "<SCRIPT>\n";
    output << "function getPage(obj,frm,url)\n";
    output << "{\n";
    output << "  frm.location.href=url;\n";
    output << "}\n";
    output << "</SCRIPT>\n";
    output << "<BODY style=\"font-size:12;\">\n";

    output << "<HR>\n";
    output << "<A href=\"grid-admin?target=grid-admin&page=start\">SmartMet Server</A> / ";
    output << "<A href=\"grid-admin?target=grid-admin&page=engines\">Engines</A> / ";
    output << "<A href=\"grid-admin?page=start\">Grid Engine</A> / ";
    output << "<A href=\"grid-admin?page=contentServer\">Content Server</A> / ";
    output << "<A href=\"grid-admin?page=contentInformation\">Content Information</A> / ";
    output << "<A href=\"grid-admin?page=producers&producerId=" << producerId << "\">Producers (" << sourceStr << ")</A> / <A href=\"grid-admin?page=generations&producerId=" << producerId << "&producerName=" << producerName << "&generationId=" << generationId << "\">" << producerName << "</A> / <A href=\"grid-admin?page=files&&generationId=" << generationId << "&generationName=" << generationName << "&fileId=" << fileId << "\">" << generationName << "</A> / " << fileId;
    output << "<HR>\n";
    output << "<H2>Content</H2>\n";
    output << "<HR>\n";

    output << "<TABLE border=\"1\" width=\"100%\" style=\"font-size:12;\">\n";
    output << "<TR bgColor=\"#D0D0D0\">";
    output << "<TD>MessageIndex</TD>";
    output << "<TD>FilePosition</TD>";
    output << "<TD>MessageSize</TD>";
    output << "<TD>ForecastTime</TD>";
    output << "<TD>FmiParameterId</TD>";
    output << "<TD>FmiParameterName</TD>";
    output << "<TD>FmiParameterLevelId</TD>";
    output << "<TD>ParameterLevel</TD>";
    output << "<TD>ForecastType</TD>";
    output << "<TD>ForecastNumber</TD>";
    output << "<TD>GeometryId</TD>";
    output << "<TD>AggregationId</TD>";
    output << "<TD>AggregationPeriod</TD>";
    output << "<TD>ProcessingTypeId</TD>";
    output << "<TD>ProcessingTypeValue1</TD>";
    output << "<TD>ProcessingTypeValue2</TD>";
    output << "<TD>Flags</TD>";
    output << "<TD>SourceId</TD>";
    output << "<TD>ModificationTime</TD>";
    output << "<TD>DeletionTime</TD>";
    output << "</TR>";


    T::ContentInfoList contentInfoList;
    T::ContentInfoList *cList = &contentInfoList;

    for (uint t=0; t<20; t++)
    {
      if (mCachedFileId[t] == fileId)
      {
        cList = &mCachedContentInfoList[t];
        t = 20;
      }
    }

    if (cList == &contentInfoList || cList->getLength() == 0)
    {
      contentServer->getContentListByFileId(0,fileId,contentInfoList);
      if (contentInfoList.getLength() > 1000)
      {
        uint idx = mCachedContentCount % 20;
        mCachedContentInfoList[idx] = contentInfoList;
        mCachedFileId[idx] = fileId;
        cList = &mCachedContentInfoList[idx];
        mCachedContentCount++;
      }
    }

    uint len = cList->getLength();
    uint cnt = 0;
    for (uint t=0; t<maxRecords; t++)
    {
      T::ContentInfo *content = cList->getContentInfoByIndex(startMessageIndex+t);

      if (content != nullptr)
      {
        cnt++;
        std::string fg = "#000000";
        std::string bg = "#FFFFFF";

        if (content->mDeletionTime > 0 && content->mDeletionTime < (time(nullptr) + 120))
          fg = "#808080";

        nextMessageIndex = content->mMessageIndex+1;
        if (content->mMessageIndex == messageIndex)
        {
          if (content->mDeletionTime == 0 || content->mDeletionTime > (time(nullptr) + 120))
          {
            cInfo = *content;
            fg = "#FFFFFF";
            bg = "#FF0000";

            std::ostringstream url;

            url << "/grid-gui";
            url << "?session=";
            url << "bg=light";
            url << ";bl=1";
            url << ";cl=Grey";
            url << ";cm=None";
            url << ";f=" << content->mFileId;
            url << ";fn=" << content->mForecastNumber;
            url << ";ft=" << content->mForecastType;
            url << ";g=" << content->mGenerationId;
            url << ";gm=" << content->mGeometryId;
            url << ";hu=128";
            url << ";is=DarkGrey";
            url << ";iv=Generated";
            url << ";k=";
            url << ";l=" << content->mParameterLevel;
            url << ";lb=Grey";
            url << ";lm=LightGrey";
            url << ";lo=None";
            url << ";lt=" << toString(content->mFmiParameterLevelId);
            url << ";m=" << content->mMessageIndex;
            url << ";max=16";
            url << ";mi=Default";
            url << ";min=6";
            url << ";p=" << content->getFmiParameterName();
            url << ";pg=main";
            url << ";pi=" << content->mProducerId;
            url << ";pn=" << producerName;
            url << ";pre=Image";
            url << ";pro=" << content->mGeometryId;
            url << ";sa=60";
            url << ";sm=LightCyan";
            url << ";st=10";
            url << ";sy=None";
            url << ";t=" << content->getForecastTime();
            url << ";tg=";
            url << ";tgt=Month";
            url << ";u=";
            url << ";xx=";
            url << ";yy=";

            output << "<TR style=\"background:" << bg <<"; color:" << fg << ";\" onClick=\"window.open('" << url.str() << "','_blank');\" >\n";
          }
          else
          {
            bg = "#E0E0E0";
            output << "<TR style=\"background:" << bg <<"; color:" << fg << ";\" >\n";
          }
        }
        else
        {
          output << "<TR style=\"background:" << bg <<"; color:" << fg << ";\" onmouseout=\"this.style='background:" << bg <<"; color:" << fg << ";'\" onmouseover=\"this.style='background:#FFFF00; color:#000000;';\" onClick=\"getPage(this,parent,'/grid-admin?messageIndex=" << content->mMessageIndex << "&startMessageIndex=" << startMessageIndex <<"');\" >\n";
        }

        output << "<TD>"<< content->mMessageIndex << "</TD>";
        output << "<TD>"<< content->mFilePosition << "</TD>";
        output << "<TD>"<< content->mMessageSize << "</TD>";
        output << "<TD>"<< content->getForecastTime() << "</TD>";
        output << "<TD>"<< content->mFmiParameterId << "</TD>";
        output << "<TD>"<< content->getFmiParameterName() << "</TD>";
        output << "<TD>"<< (int)content->mFmiParameterLevelId << "</TD>";
        output << "<TD>"<< content->mParameterLevel << "</TD>";
        output << "<TD>"<< content->mForecastType << "</TD>";
        output << "<TD>"<< content->mForecastNumber << "</TD>";
        output << "<TD>"<< content->mGeometryId << "</TD>";
        output << "<TD>"<< content->mAggregationId << "</TD>";
        output << "<TD>"<< content->mAggregationPeriod << "</TD>";
        output << "<TD>"<< content->mProcessingTypeId << "</TD>";
        output << "<TD>"<< content->mProcessingTypeValue1 << "</TD>";
        output << "<TD>"<< content->mProcessingTypeValue2 << "</TD>";
        output << "<TD>"<< content->mFlags << "</TD>";
        output << "<TD>"<< content->mSourceId << "</TD>";
        if (content->mModificationTime > 0)
          output << "<TD>"<< utcTimeFromTimeT(content->mModificationTime) << "</TD>";
        else
          output << "<TD></TD>";

        if (content->mDeletionTime > 0)
          output << "<TD>"<< utcTimeFromTimeT(content->mDeletionTime) << "</TD>";
        else
          output << "<TD></TD>";

        output << "</TR>";
      }
    }

    for (uint t=cnt; t<maxRecords; t++)
    {
      output << "<TR><TD>&nbsp;</TD><TD>&nbsp;</TD><TD>&nbsp;</TD><TD>&nbsp;</TD><TD>&nbsp;</TD><TD>&nbsp;</TD><TD>&nbsp;</TD><TD>&nbsp;</TD><TD>&nbsp;</TD><TD>&nbsp;</TD><TD>&nbsp;</TD><TD>&nbsp;</TD></TD><TD>&nbsp;</TD></TD><TD>&nbsp;</TD></TD><TD>&nbsp;</TD></TD><TD>&nbsp;</TD></TD><TD>&nbsp;</TD><TD>&nbsp;</TD><TD>&nbsp;</TD><TD>&nbsp;</TD></TR>\n";
    }

    output << "</TABLE>\n";

    if (maxRecords < len)
    {
      std::string bg = "#C0C0C0";
      std::string p1 = "<TD width=\"70\" align=\"center\" style=\"background:" + bg + ";\" onmouseout=\"this.style='background:" + bg + ";'\" onmouseover=\"this.style='background:#0000FF; color:#FFFFFF;';\" onClick=\"getPage(this,parent,'/grid-admin?startMessageIndex=";
      std::string p2 = "<TD width=\"70\" style=\"background:" + bg + ";\" >&nbsp;</TD>\n";

      output << "<HR>\n";
      output << "<TABLE style=\"font-size:12;\"><TR>\n";

      if (startMessageIndex >= maxRecords)
        output << p1 << (startMessageIndex-maxRecords) << "');\" >&lt;&lt;</TD>\n";
      else
      if (startMessageIndex > 0)
        output << p1 << "0" << "');\" >&lt;&lt;</TD>\n";
      else
        output << p2;

      if ((startMessageIndex+maxRecords) < len)
        output << p1 << nextMessageIndex << "');\" >&gt;&gt;</TD>\n";
      else
        output << p2;

      if (len > 100)
      {
        output << "<TD width=\"50\"> </TD>\n";

        if (startMessageIndex > 0)
        {
          if (startMessageIndex >= 100)
            output << p1 << (startMessageIndex-100) << "');\" >&lt;&lt;&lt;</TD>\n";
          else
            output << p1 << "0');\" >&lt;&lt;&lt;</TD>\n";
        }
        else
          output << p2;

        if ((startMessageIndex+maxRecords) < len)
        {
          if ((startMessageIndex+100) < len)
            output << p1 << (startMessageIndex+100) << "');\" >&gt;&gt;&gt;</TD>\n";
          else
            output << p1 << (len-maxRecords) << "');\" >&gt;&gt;&gt;</TD>\n";
        }
        else
          output << p2;

      }

      if (len > 1000)
      {
        output << "<TD width=\"50\"> </TD>\n";

        if (startMessageIndex > 0)
        {
          if (startMessageIndex >= 1000)
            output << p1 << (startMessageIndex-1000) << "');\" >&lt;&lt;&lt;&lt;</TD>\n";
          else
            output << p1 << "0');\" >&lt;&lt;&lt;&lt;</TD>\n";
        }
        else
          output << p2;

        if ((startMessageIndex+maxRecords) < len)
        {
          if ((startMessageIndex+1000) < len)
            output << p1 << (startMessageIndex+1000) << "');\" >&gt;&gt;&gt;&gt;</TD>\n";
          else
            output << p1 << (len-maxRecords) << "');\" >&gt;&gt;&gt;&gt;</TD>\n";
        }
        else
          output << p2;
      }

      if (len > 10000)
      {
        output << "<TD width=\"50\"> </TD>\n";

        if (startMessageIndex > 0)
        {
          if (startMessageIndex >= 10000)
            output << p1 << (startMessageIndex-10000) << "');\" >&lt;&lt;&lt;&lt;&lt;</TD>\n";
          else
            output << p1 << "0');\" >&lt;&lt;&lt;&lt;&lt;</TD>\n";
        }
        else
          output << p2;

        if ((startMessageIndex+maxRecords) < len)
        {
          if ((startMessageIndex+10000) < len)
            output << p1 << (startMessageIndex+10000) << "');\" >&gt;&gt;&gt;&gt;&gt;</TD>\n";
          else
            output << p1 << (len-maxRecords) << "');\" >&gt;&gt;&gt;&gt;&gt;</TD>\n";
        }
        else
          output << p2;
      }

      for (uint t=len; t<maxRecords; t++)
      {
        output << "<TR><TD>&nbsp;</TD><TD>&nbsp;</TD><TD>&nbsp;</TD><TD>&nbsp;</TD><TD>&nbsp;</TD><TD>&nbsp;</TD></TR>\n";
      }
      output << "</TR></TABLE>\n";
    }

    if ((mFlags & Flags::contentModificationEnabled) && mainSource && (session.mUserInfo.getUserId() == 0 || session.mUserInfo.isUserGroupMember("grid-admin")))
    {
      std::string prod = "&startMessageIndex=" + toString(startMessageIndex);

      if (mode > 0  && mode < 100)
      {
        std::string ft = utcTimeFromTimeT(cInfo.mForecastTimeUTC);

        std::string dt = "21000101T000000";
        if (cInfo.mDeletionTime > 0)
          dt = utcTimeFromTimeT(cInfo.mDeletionTime);

        std::string mt = utcTimeFromTimeT(cInfo.mModificationTime);
        std::string p1 = "<TR><TD width=\"240\" bgColor=\"#D0D0D0\">";
        std::string p2 = "</TD><TD><INPUT style=\"width:100%;\" type=\"text\" ";
        std::string p3 = "\"></TD></TR>";

        output << "<HR>\n";
        output << "<H2>Content</H2>\n";
        output << "<TABLE border=\"1\" width=\"100%\" style=\"font-size:12;\">\n";
        output << p1 << "FileId</TD><TD>"<< cInfo.mFileId << "</TD></TR>";
        output << p1 << "MessageIndex</TD><TD>"<< cInfo.mMessageIndex << "</TD></TR>";
        output << p1 << "FilePosition" << p2 << "id=\"content_filePosition\" value=\"" << cInfo.mFilePosition << p3;
        output << p1 << "MessageSize" << p2 << "id=\"content_messageSize\" value=\"" << cInfo.mMessageSize << p3;
        output << p1 << "ForecastTime" << p2 << "id=\"content_forecastTime\" value=\"" << ft << p3;
        output << p1 << "FmiParameterId" << p2 << "id=\"content_fmiParameterId\" value=\"" << cInfo.mFmiParameterId << p3;
        output << p1 << "FmiParameterName" << p2 << "id=\"content_fmiParameterName\" value=\"" << cInfo.getFmiParameterName() << p3;
        output << p1 << "FmiParameterLevelId" << p2 << "id=\"content_fmiParameterLevelId\" value=\"" << cInfo.mFmiParameterLevelId << p3;
        output << p1 << "ParameterLevel" << p2 << "id=\"content_parameterLevel\" value=\"" << cInfo.mParameterLevel << p3;
        output << p1 << "ForecastType" << p2 << "id=\"content_forecastType\" value=\"" << cInfo.mForecastType << p3;
        output << p1 << "ForecastNumber" << p2 << "id=\"content_forecastNumber\" value=\"" << cInfo.mForecastNumber << p3;
        output << p1 << "GeometryId" << p2 << "id=\"content_geometryId\" value=\"" << cInfo.mGeometryId << p3;
        output << p1 << "AggregationId" << p2 << "id=\"content_aggregationId\" value=\"" << cInfo.mAggregationId << p3;
        output << p1 << "AggregationPeriod" << p2 << "id=\"content_aggregationPeriod\" value=\"" << cInfo.mAggregationPeriod << p3;
        output << p1 << "ProcessingTypeId" << p2 << "id=\"content_processingTypeId\" value=\"" << cInfo.mProcessingTypeId << p3;
        output << p1 << "ProcessingTypeValue1" << p2 << "id=\"content_processingTypeValue1\" value=\"" << cInfo.mProcessingTypeValue1 << p3;
        output << p1 << "ProcessingTypeValue2" << p2 << "id=\"content_processingTypeValue2\" value=\"" << cInfo.mProcessingTypeValue2 << p3;
        output << p1 << "Flags" << p2 << "id=\"content_flags\" value=\"" << cInfo.mFlags << p3;
        output << p1 << "SourceId" << p2 << "id=\"content_sourceId\" value=\"" << cInfo.mSourceId << p3;
        output << p1 << "ModificationTime" << p2 << "id=\"content_modificationTime\" value=\"" << mt << p3;
        output << p1 << "DeletionTime" << p2 << "id=\"content_deletionTime\" value=\"" << dt << p3;
        output << "</TABLE>\n";
      }

      std::string bg = "#C0C0C0";
      std::string p1 = "<TD width=\"150\" height=\"25\" align=\"center\" style=\"background:" + bg + ";\" onmouseout=\"this.style='background:" + bg + ";'\" onmouseover=\"this.style='background:#0000FF; color:#FFFFFF';\" onClick=\"getPage(this,parent,'/grid-admin?mode=";

      output << "<HR>\n";
      output << "<TABLE style=\"font-size:12;\">\n";
      output << "<TR>\n";

      switch (mode)
      {
        case MODE_NONE:
          output << p1 << MODE_ADD_REQUEST << prod << "&messageIndex=" << messageIndex << "');\" >New content</TD>\n";
          if (fileId !=0  &&  messageIndex != 0xFFFFFFFF)
          {
            output << p1 << MODE_EDIT_REQUEST << prod << "&messageIndex=" << messageIndex << "');\" >Edit content</TD>\n";
            output << p1 << MODE_DELETE_REQUEST << prod << "&messageIndex=" << messageIndex << "');\" >Delete content</TD>\n";
          }
          break;

        case MODE_ADD_REQUEST:
          output << p1 << MODE_ADD_CONFIRM << prod << "&messageIndex=" << messageIndex << "&fmiParameterName='+content_fmiParameterName.value+'&fmiParameterId='+content_fmiParameterId.value+'&fmiParameterLevelId='+content_fmiParameterLevelId.value+'&parameterLevel='+content_parameterLevel.value+'&flags='+content_flags.value+'&sourceId='+content_sourceId.value+'&forecastType='+content_forecastType.value+'&forecastNumber='+content_forecastNumber.value+'&geometryId='+content_geometryId.value+'&aggregationId='+content_aggregationId.value+'&aggregationPeriod='+content_aggregationPeriod.value+'&processingTypeId='+content_processingTypeId.value+'&processingTypeValue1='+content_processingTypeValue1.value+'&processingTypeValue2='+content_processingTypeValue2.value+'&modificationTime='+content_modificationTime.value+'&deletionTime='+content_deletionTime.value+'&forecastTime='+content_forecastTime.value+'&filePosition='+content_filePosition.value+'&messageSize='+content_messageSize.value);\" >Add content</TD>\n";
          output << p1 << MODE_NONE << "&messageIndex=" << messageIndex << prod << "');\" >Cancel</TD>\n";
          break;

        case MODE_EDIT_REQUEST:
          output << p1 << MODE_EDIT_CONFIRM << prod << "&messageIndex=" << messageIndex << "&fmiParameterName='+content_fmiParameterName.value+'&fmiParameterId='+content_fmiParameterId.value+'&fmiParameterLevelId='+content_fmiParameterLevelId.value+'&parameterLevel='+content_parameterLevel.value+'&flags='+content_flags.value+'&sourceId='+content_sourceId.value+'&forecastType='+content_forecastType.value+'&forecastNumber='+content_forecastNumber.value+'&geometryId='+content_geometryId.value+'&aggregationId='+content_aggregationId.value+'&aggregationPeriod='+content_aggregationPeriod.value+'&processingTypeId='+content_processingTypeId.value+'&processingTypeValue1='+content_processingTypeValue1.value+'&processingTypeValue2='+content_processingTypeValue2.value+'&modificationTime='+content_modificationTime.value+'&deletionTime='+content_deletionTime.value+'&forecastTime='+content_forecastTime.value+'&filePosition='+content_filePosition.value+'&messageSize='+content_messageSize.value);\" >Update content</TD>\n";
          output << p1 << MODE_NONE << "&messageIndex=" << messageIndex << prod << "');\" >Cancel</TD>\n";
          break;

        case MODE_DELETE_REQUEST:
          output << p1 << MODE_DELETE_CONFIRM << prod << "&messageIndex=" << messageIndex << "');\" >Delete content</TD>\n";
          output << p1 << MODE_NONE << "&messageIndex=" << messageIndex << prod << "');\" >Cancel</TD>\n";
          break;
      }

      output << "</TR>\n";
      output << "</TABLE>\n";
    }

    output << "<HR>\n";
    output << "</BODY>\n";
    output << "</HTML>\n";

    theResponse.setContent(output.str());
    theResponse.setHeader("Content-Type", "text/html; charset=UTF-8");

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}





bool Browser::page_files(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse)
{
  try
  {
    ContentServer_sptr contentServer = mMainContentServer;
    std::string sourceStr = "main";
    bool mainSource = true;

    session.getAttribute("grid-engine","source",sourceStr);

    if (sourceStr == "cache")
    {
      contentServer = mCacheContentServer;
      mainSource = false;
    }

    uint nextFileId = 0;
    uint maxRecords = 30;

    T::FileInfo fInfo;

    uint producerId = 0;
    session.getAttribute("grid-engine","producerId",producerId);

    std::string producerName;
    session.getAttribute("grid-engine","producerName",producerName);

    uint prevGenerationId = 0;
    session.getAttribute("grid-engine","prevGenerationId",prevGenerationId);

    uint generationId = 0;
    session.getAttribute("grid-engine","generationId",generationId);

    std::string generationName;
    session.getAttribute("grid-engine","generationName",generationName);

    uint fileId = 0;
    session.getAttribute("grid-engine","fileId",fileId);

    uint startFileId = 0;
    session.getAttribute("grid-engine","startFileId",startFileId);

    uint endFileId = 0;
    session.getAttribute("grid-engine","endFileId",endFileId);

    if (generationId != prevGenerationId)
    {
      startFileId = 0;
    }

    session.setAttribute("grid-engine","prevGenerationId",generationId);
    session.setAttribute("grid-engine","messageIndex",0);
    session.setAttribute("grid-engine","startMessageIndex",0);

    uint mode = 0;
    auto v = theRequest.getParameter("mode");
    if (v)
      mode = toUInt32(*v);

    if (producerId == 0 || generationId == 0)
      mode = 0;

    if ((mFlags & Flags::contentModificationEnabled) && mainSource  && mode >= 100 && (session.mUserInfo.getUserId() == 0 || session.mUserInfo.isUserGroupMember("grid-admin")))
    {
      fInfo.mProducerId = producerId;
      fInfo.mGenerationId = generationId;

      auto v = theRequest.getParameter("server");
      if (v)
        fInfo.mServer = v->c_str();

      v = theRequest.getParameter("name");
      if (v)
        fInfo.mName = v->c_str();

      v = theRequest.getParameter("protocol");
      if (v)
        fInfo.mProtocol = atoi(v->c_str());

      v = theRequest.getParameter("serverType");
      if (v)
        fInfo.mServerType = atoi(v->c_str());

      v = theRequest.getParameter("type");
      if (v)
        fInfo.mFileType = atoi(v->c_str());

      v = theRequest.getParameter("flags");
      if (v)
        fInfo.mFlags = atoi(v->c_str());

      v = theRequest.getParameter("sourceId");
      if (v)
        fInfo.mSourceId = atoi(v->c_str());

      v = theRequest.getParameter("deletionTime");
      if (v)
      {
        try
        {
          fInfo.mDeletionTime = utcTimeToTimeT(*v);
        }
        catch (...)
        {
          mode = 0;
        }
      }

      v = theRequest.getParameter("modificationTime");
      if (v)
      {
        try
        {
          fInfo.mModificationTime = utcTimeToTimeT(*v);
        }
        catch (...)
        {
          mode = 0;
        }
      }

      switch (mode)
      {
        case  MODE_ADD_CONFIRM:
        {
          int res = contentServer->addFileInfo(0,fInfo);
          if (res != 0)
          {
            // File addition failed
          }
          else
          {
            fileId = fInfo.mFileId;
          }
        }
        break;

        case MODE_EDIT_CONFIRM:
        {
          fInfo.mFileId = fileId;
          int res = contentServer->setFileInfo(0,fInfo);
          if (res != 0)
          {
            // File update failed
          }
        }
        break;

        case MODE_DELETE_CONFIRM:
        {
          if (fileId != 0)
          {
            int res = contentServer->deleteFileInfoById(0,fileId);
            if (res != 0)
            {
              // File deletion failed
            }
            else
            {
              fileId = 0;
              session.setAttribute("grid-engine","fileId",0);
            }
          }
        }
        break;

        case MODE_GENERATE_CONFIRM:
        {
          // ToDo : Generate content
          mode = 0;
        }
        break;
      }

      mode = 0;
    }

    std::ostringstream output;

    output << "<HTML>\n";
    output << "<SCRIPT>\n";
    output << "function getPage(obj,frm,url)\n";
    output << "{\n";
    output << "  frm.location.href=url;\n";
    output << "}\n";
    output << "</SCRIPT>\n";
    output << "<BODY style=\"font-size:12;\">\n";

    output << "<HR>\n";
    output << "<A href=\"grid-admin?target=grid-admin&page=start\">SmartMet Server</A> / ";
    output << "<A href=\"grid-admin?target=grid-admin&page=engines\">Engines</A> / ";
    output << "<A href=\"grid-admin?page=start\">Grid Engine</A> / ";
    output << "<A href=\"grid-admin?page=contentServer\">Content Server</A> / ";
    output << "<A href=\"grid-admin?page=contentInformation\">Content Information</A> / ";
    output << "<A href=\"grid-admin?page=producers&producerId=" << producerId << "\">Producers (" << sourceStr << ")</A> / <A href=\"grid-admin?page=generations&producerId=" << producerId << "&producerName=" << producerName << "&generationId=" << generationId << "\">" << producerName << "</A> / " + generationName;
    output << "<HR>\n";
    output << "<H2>Files</H2>\n";
    output << "<HR>\n";

    output << "<TABLE border=\"1\" width=\"100%\" style=\"font-size:12;\">\n";
    output << "<TR bgColor=\"#D0D0D0\">";
    output << "<TD>Id</TD>";
    output << "<TD>Server</TD>";
    output << "<TD>Name</TD>";
    output << "<TD>Protocol</TD>";
    output << "<TD>ServerType</TD>";
    output << "<TD>FileType</TD>";
    output << "<TD>Flags</TD>";
    output << "<TD>SourceId</TD>";
    output << "<TD>ModificationTime</TD>";
    output << "<TD>DeletionTime</TD>";
    output << "</TR>\n";

    T::FileInfoList fileInfoList;
    if (endFileId != 0)
    {
      startFileId = 0;
      mCacheContentServer->getFileInfoListByGenerationId(0,generationId,endFileId,-maxRecords,fileInfoList);
      fileInfoList.sort(1);
      uint flen = fileInfoList.getLength();
      if (flen > 0)
      {
        if (flen < maxRecords)
          startFileId = 0;
        else
        {
          T::FileInfo *f = fileInfoList.getFileInfoByIndex(0);
          if (f != nullptr)
            startFileId = f->mFileId;
        }
      }
    }

    contentServer->getFileInfoListByGenerationId(0,generationId,startFileId,maxRecords,fileInfoList);

    uint len = fileInfoList.getLength();
    for (uint t=0; t<len; t++)
    {
      T::FileInfo *file = fileInfoList.getFileInfoByIndex(t);
      nextFileId = file->mFileId+1;

      std::string fg = "#000000";
      std::string bg = "#FFFFFF";

      if (file->mDeletionTime > 0 && file->mDeletionTime < (time(nullptr) + 120))
        fg = "#808080";

      if (file->mFileId == fileId)
      {
        fInfo = *file;
        fg = "#FFFFFF";
        bg = "#FF0000";
        output << "<TR style=\"background:" << bg <<"; color:" << fg << ";\" onClick=\"getPage(this,parent,'/grid-admin?page=contentList&fileId=" << file->mFileId << "&startFileId=" << startFileId << "');\" >\n";
      }
      else
      {
        output << "<TR style=\"background:" << bg <<"; color:" << fg << ";\" onmouseout=\"this.style='background:" << bg <<"; color:" << fg << ";'\" onmouseover=\"this.style='background:#FFFF00; color:#000000;';\" onClick=\"getPage(this,parent,'/grid-admin?fileId=" << file->mFileId << "&startFileId=" << startFileId << "');\" >\n";
      }

      output << "<TD>"<< file->mFileId << "</TD>";
      output << "<TD>"<< file->mServer << "</TD>";
      output << "<TD>"<< file->mName << "</TD>";
      output << "<TD>"<< C_INT(file->mProtocol) << "</TD>";
      output << "<TD>"<< C_INT(file->mServerType) << "</TD>";
      output << "<TD>"<< C_INT(file->mFileType) << "</TD>";
      output << "<TD>"<< file->mFlags << "</TD>";
      output << "<TD>"<< file->mSourceId << "</TD>";
      if (file->mModificationTime > 0)
        output << "<TD>"<< utcTimeFromTimeT(file->mModificationTime) << "</TD>";
      else
        output << "<TD></TD>";

      if (file->mDeletionTime > 0)
        output << "<TD>"<< utcTimeFromTimeT(file->mDeletionTime) << "</TD>";
      else
        output << "<TD></TD>";
      output << "</TR>\n";
    }

    for (uint t=len; t<maxRecords; t++)
    {
      output << "<TR><TD>&nbsp;</TD><TD>&nbsp;</TD><TD>&nbsp;</TD><TD>&nbsp;</TD><TD>&nbsp;</TD><TD>&nbsp;</TD><TD>&nbsp;</TD><TD>&nbsp;</TD><TD>&nbsp;</TD><TD>&nbsp;</TD></TR>\n";
    }


    output << "</TABLE>\n";

    if (startFileId > 0 || len >= maxRecords)
    {
      std::string bg = "#C0C0C0";
      std::string p1 = "<TD width=\"70\" align=\"center\" style=\"background:" + bg + ";\" onmouseout=\"this.style='background:" + bg + ";'\" onmouseover=\"this.style='background:#0000FF; color:#FFFFFF;';\" onClick=\"getPage(this,parent,'/grid-admin?";
      std::string p2 = "<TD width=\"70\" style=\"background:" + bg + ";\" >&nbsp;</TD>\n";

      output << "<HR>\n";
      output << "<TABLE style=\"font-size:12;\"><TR>\n";

      if (startFileId > 0)
        output << p1 << "endFileId=" << (startFileId-1) << "');\" >&lt;&lt;</TD>\n";
      else
        output << p2;

      if (len >= maxRecords)
        output << p1 << "startFileId=" << nextFileId << "&endFileId=0');\" >&gt;&gt;</TD>\n";
      else
        output << p2;

      output << "</TR></TABLE>\n";
    }

    if ((mFlags & Flags::contentModificationEnabled) && mainSource && (session.mUserInfo.getUserId() == 0 || session.mUserInfo.isUserGroupMember("grid-admin")))
    {
      std::string prod = "&startFileId=" + toString(startFileId);

      if (mode > 0  && mode < 4)
      {
        std::string dt = "21000101T000000";
        if (fInfo.mDeletionTime > 0)
          dt = utcTimeFromTimeT(fInfo.mDeletionTime);

        std::string mt = utcTimeFromTimeT(fInfo.mModificationTime);

        std::string p1 = "<TR><TD width=\"240\" bgColor=\"#D0D0D0\">";
        std::string p2 = "</TD><TD><INPUT style=\"width:100%;\"  type=\"text\" ";
        std::string p3 = "\"></TD></TR>";

        output << "<HR>\n";
        output << "<H2>File</H2>\n";
        output << "<TABLE border=\"1\" width=\"100%\" style=\"font-size:12;\">\n";
        output << p1 << "Id</TD><TD>"<< fInfo.mFileId << "</TD></TR>";
        output << p1 << "Server" << p2 << "id=\"file_server\" value=\"" << fInfo.mServer << p3;
        output << p1 << "Name<" << p2 << "id=\"file_name\" value=\"" << fInfo.mName << p3;
        output << p1 << "Protocol" << p2 << "id=\"file_protocol\" value=\"" << C_INT(fInfo.mProtocol) << p3;
        output << p1 << "ServerType" << p2 << "id=\"file_serverType\" value=\"" << C_INT(fInfo.mServerType) << p3;
        output << p1 << "FileType" << p2 << "id=\"file_type\" value=\"" << C_INT(fInfo.mFileType) << p3;
        output << p1 << "Flags" << p2 << "id=\"file_flags\" value=\"" << fInfo.mFlags << p3;
        output << p1 << "SourceId" << p2 << "id=\"file_sourceId\" value=\"" << fInfo.mSourceId << p3;
        output << p1 << "ModificationTime" << p2 << "id=\"file_modificationTime\" value=\"" << mt << p3;
        output << p1 << "DeletionTime" << p2 << "id=\"file_deletionTime\" value=\"" << dt << p3;
        output << "</TABLE>\n";
      }

      std::string bg = "#C0C0C0";
      std::string p1 = "<TD width=\"150\" height=\"25\" align=\"center\" style=\"background:" + bg + ";\" onmouseout=\"this.style='background:" + bg + ";'\" onmouseover=\"this.style='background:#0000FF; color:#FFFFFF';\" onClick=\"getPage(this,parent,'/grid-admin?mode=";

      output << "<HR>\n";
      output << "<TABLE style=\"font-size:12;\">\n";
      output << "<TR>\n";
      switch (mode)
      {
        case MODE_NONE:
          output << p1 << MODE_ADD_REQUEST << prod << "');\" >New file</TD>\n";
          if (fileId != 0)
          {
            output << p1 << MODE_EDIT_REQUEST << prod << "&fileId=" << fileId << "');\" >Edit file</TD>\n";
            output << p1 << MODE_DELETE_REQUEST<< prod << "&fileId=" << fileId << "');\" >Delete file</TD>\n";
            output << p1 << MODE_GENERATE_REQUEST << prod << "&fileId=" << fileId << "');\" >Show dump</TD>\n";
          }
          break;

        case MODE_ADD_REQUEST:
          output << p1 << MODE_ADD_CONFIRM << prod << "&server='+file_server.value+'&name='+file_name.value+'&protocol='+file_protocol.value+'&serverType='+file_serverType.value+'&type='+file_type.value+'&flags='+file_flags.value+'&sourceId='+file_sourceId.value+'&modificationTime='+file_modificationTime.value+'&deletionTime='+file_deletionTime.value);\" >Add file</TD>\n";
          output << p1 << MODE_NONE << prod << "&fileId=" << fileId << "');\" >Cancel</TD>\n";
          break;

        case MODE_EDIT_REQUEST:
          output << p1 << MODE_EDIT_CONFIRM << prod << "&fileId=" << fileId << "&server='+encodeURIComponent(file_server.value)+'&name='+encodeURIComponent(file_name.value)+'&protocol='+file_protocol.value+'&serverType='+file_serverType.value+'&type='+file_type.value+'&flags='+file_flags.value+'&sourceId='+file_sourceId.value+'&modificationTime='+file_modificationTime.value+'&deletionTime='+file_deletionTime.value);\" >Update file</TD>\n";
          output << p1 << MODE_NONE << prod << "&fileId=" << fileId << "');\" >Cancel</TD>\n";
          break;

        case MODE_DELETE_REQUEST:
          output << p1 << MODE_DELETE_CONFIRM << prod << "&fileId=" << fileId << "');\" >Delete file</TD>\n";
          output << p1 << MODE_NONE << prod << "&fileId=" << fileId << "');\" >Cancel</TD>\n";
          break;

        case MODE_GENERATE_REQUEST:
          output << p1 << MODE_GENERATE_CONFIRM << prod << "&fileId=" << fileId << "');\" >Generate content</TD>\n";
          output << p1 << MODE_NONE << prod << "&fileId=" << fileId << "');\" >Cancel</TD>\n";
          break;
      }

      output << "</TR>\n";
      output << "</TABLE>\n";

      if (mode == MODE_GENERATE_REQUEST)
      {
        output << "<HR>\n";
        try
        {
          output << "<PRE style=\"background-color: #F0F0F0;\">\n";
          output << "\n";
          SmartMet::GRID::GridFile gridFile;
          gridFile.read(fInfo.mName,100);
          gridFile.print(output,0,0);
          output << "\n";
          output << "</PRE>\n";
        }
        catch (...)
        {
          Fmi::Exception exception(BCP, "Operation failed!", nullptr);
          output << exception.getHtmlStackTrace();
        }
      }
    }

    output << "<HR>\n";

    output << "</BODY>\n";
    output << "</HTML>\n";


    theResponse.setContent(output.str());
    theResponse.setHeader("Content-Type", "text/html; charset=UTF-8");

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}





bool Browser::page_generations(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse)
{
  try
  {
    ContentServer_sptr contentServer = mMainContentServer;
    std::string sourceStr = "main";
    bool mainSource = true;
    const char *statusStr[] = {"Unknown","Ready","Not Ready"};


    session.getAttribute("grid-engine","source",sourceStr);

    if (sourceStr == "cache")
    {
      contentServer = mCacheContentServer;
      mainSource = false;
    }

    uint maxRecords = 30;
    T::GenerationInfo gInfo;

    uint prevProducerId = 0;
    session.getAttribute("grid-engine","prevProducerId",prevProducerId);

    uint producerId = 0;
    session.getAttribute("grid-engine","producerId",producerId);

    std::string producerName;
    session.getAttribute("grid-engine","producerName",producerName);

    uint generationId = 0;
    session.getAttribute("grid-engine","generationId",generationId);

    uint startGenerationIndex = 0;
    session.getAttribute("grid-engine","startGenerationIndex",startGenerationIndex);

    if (producerId != prevProducerId)
    {
      startGenerationIndex = 0;
      session.setAttribute("grid-engine","startGenerationIndex",startGenerationIndex);
    }

    session.setAttribute("grid-engine","prevProducerId",producerId);

    session.setAttribute("grid-engine","fileId",0);
    session.setAttribute("grid-engine","startFileId",0);
    session.setAttribute("grid-engine","endFileId",0);

    uint mode = 0;
    auto v = theRequest.getParameter("mode");
    if (v)
      mode = toUInt32(*v);


    if (producerId == 0)
      mode = 0;

    if ((mFlags & Flags::contentModificationEnabled) &&  mainSource && mode >= 100 && (session.mUserInfo.getUserId() == 0 || session.mUserInfo.isUserGroupMember("grid-admin")))
    {
      gInfo.mProducerId = producerId;

      auto v = theRequest.getParameter("name");
      if (v)
        gInfo.mName = v->c_str();

      v = theRequest.getParameter("analysisTime");
      if (v)
        gInfo.mAnalysisTime = v->c_str();

      v = theRequest.getParameter("description");
      if (v)
        gInfo.mDescription = v->c_str();

      v = theRequest.getParameter("flags");
      if (v)
        gInfo.mFlags = atoi(v->c_str());

      v = theRequest.getParameter("sourceId");
      if (v)
        gInfo.mSourceId = atoi(v->c_str());

      v = theRequest.getParameter("status");
      if (v)
        gInfo.mStatus = atoi(v->c_str());

      v = theRequest.getParameter("deletionTime");
      if (v)
      {
        try
        {
          gInfo.mDeletionTime = utcTimeToTimeT(*v);
        }
        catch (...)
        {
          mode = 0;
        }
      }

      v = theRequest.getParameter("modificationTime");
      if (v)
      {
        try
        {
          gInfo.mModificationTime = utcTimeToTimeT(*v);
        }
        catch (...)
        {
          mode = 0;
        }
      }

      switch (mode)
      {
        case MODE_ADD_CONFIRM:
        {
          int res = contentServer->addGenerationInfo(0,gInfo);
          if (res != 0)
          {
            // Generation addition failed
          }
          else
          {
            generationId = gInfo.mGenerationId;
          }
        }
        break;

        case MODE_EDIT_CONFIRM:
        {
          gInfo.mGenerationId = generationId;
          int res = contentServer->setGenerationInfo(0,gInfo);
          if (res != 0)
          {
            // Generation update failed
          }
        }
        break;

        case MODE_DELETE_CONFIRM:
        {
          if (generationId != 0)
          {
            int res = contentServer->deleteGenerationInfoById(0,generationId);
            if (res != 0)
            {
              // Generation deletion failed
            }
            else
            {
              generationId = 0;
              session.setAttribute("grid-engine","generationId",0);
            }
          }
        }
        break;
      }

      mode = 0;
    }

    std::ostringstream output;

    output << "<HTML>\n";
    output << "<SCRIPT>\n";
    output << "function getPage(obj,frm,url)\n";
    output << "{\n";
    output << "  frm.location.href=url;\n";
    output << "}\n";
    output << "</SCRIPT>\n";
    output << "<BODY style=\"font-size:12;\">\n";

    output << "<HR>\n";
    output << "<A href=\"grid-admin?target=grid-admin&page=start\">SmartMet Server</A> / ";
    output << "<A href=\"grid-admin?target=grid-admin&page=engines\">Engines</A> / ";
    output << "<A href=\"grid-admin?page=start\">Grid Engine</A> / ";
    output << "<A href=\"grid-admin?page=contentServer\">Content Server</A> / ";
    output << "<A href=\"grid-admin?page=contentInformation\">Content Information</A> / ";
    output << "<A href=\"grid-admin?page=producers&producerId=" << producerId << "\">Producers (" << sourceStr << ")</A> / " + producerName;
    output << "<HR>\n";
    output << "<H2>Generations</H2>\n";
    output << "<HR>\n";

    output << "<TABLE border=\"1\" width=\"100%\" style=\"font-size:12;\">\n";
    output << "<TR bgColor=\"#D0D0D0\">";
    output << "<TD>Id</TD>";
    output << "<TD>Name</TD>";
    output << "<TD>Description</TD>";
    output << "<TD>AnalysisTime</TD>";
    output << "<TD>Flags</TD>";
    output << "<TD>SourceId</TD>";
    output << "<TD>Status</TD>";
    output << "<TD>ModificationTime</TD>";
    output << "<TD>DeletionTime</TD>";
    output << "</TR>";


    T::GenerationInfoList generationInfoList;
    contentServer->getGenerationInfoListByProducerId(0,producerId,generationInfoList);

    uint len = generationInfoList.getLength();
    for (uint t=0; t<maxRecords; t++)
    {
      T::GenerationInfo *generation = generationInfoList.getGenerationInfoByIndex(startGenerationIndex+t);
      if (generation != nullptr)
      {
        std::string fg = "#000000";
        std::string bg = "#FFFFFF";

        if (generation->mDeletionTime > 0 && generation->mDeletionTime < (time(nullptr) + 120))
          fg = "#808080";

        if (generation->mGenerationId == generationId)
        {
          gInfo = *generation;
          fg = "#FFFFFF";
          bg = "#FF0000";
          output << "<TR style=\"background:" << bg << "; color:" << fg << ";\" onClick=\"getPage(this,parent,'/grid-admin?page=files&generationId=" << generation->mGenerationId << "&generationName=" << generation->mName << "');\" >\n";
        }
        else
        {
          output << "<TR style=\"background:" << bg << "; color:" << fg << ";\" onmouseout=\"this.style='background:" << bg << "; color:" << fg << ";'\" onmouseover=\"this.style='background:#FFFF00; color:#000000;';\" onClick=\"getPage(this,parent,'/grid-admin?page=generations&startGenerationIndex=" << startGenerationIndex << "&generationId=" << generation->mGenerationId << "&generationName=" << generation->mName << "');\" >\n";
        }


        output << "<TD>"<< generation->mGenerationId << "</TD>";
        output << "<TD>"<< generation->mName << "</TD>";
        output << "<TD>"<< generation->mDescription << "</TD>";
        output << "<TD>"<< generation->mAnalysisTime << "</TD>";
        output << "<TD>"<< generation->mFlags << "</TD>";
        output << "<TD>"<< generation->mSourceId << "</TD>";
        output << "<TD>"<< statusStr[(int)generation->mStatus % 3] << "</TD>";
        if (generation->mModificationTime > 0)
          output << "<TD>"<< utcTimeFromTimeT(generation->mModificationTime) << "</TD>";
        else
          output << "<TD></TD>";

        if (generation->mDeletionTime > 0)
          output << "<TD>"<< utcTimeFromTimeT(generation->mDeletionTime) << "</TD>";
        else
          output << "<TD></TD>";

        output << "</TR>";
      }
      else
      {
        output << "<TR><TD>&nbsp;</TD><TD>&nbsp;</TD><TD>&nbsp;</TD><TD>&nbsp;</TD><TD>&nbsp;</TD><TD>&nbsp;</TD><TD>&nbsp;</TD><TD>&nbsp;</TD><TD>&nbsp;</TD><TD>&nbsp;</TD></TR>\n";
      }
    }

    output << "</TABLE>\n";

    if (startGenerationIndex > 0 || len >= maxRecords)
    {
      std::string bg = "#C0C0C0";
      std::string p1 = "<TD width=\"70\" align=\"center\" style=\"background:" + bg + ";\" onmouseout=\"this.style='background:" + bg + ";'\" onmouseover=\"this.style='background:#0000FF; color:#FFFFFF;';\" onClick=\"getPage(this,parent,'/grid-admin?";
      std::string p2 = "<TD width=\"70\" style=\"background:" + bg + ";\" >&nbsp;</TD>\n";

      output << "<HR>\n";
      output << "<TABLE style=\"font-size:12;\"><TR>\n";

      if (startGenerationIndex > 0)
      {
        if (startGenerationIndex > maxRecords)
          output << p1 << "startGenerationIndex=" << (startGenerationIndex-maxRecords) << "');\" >&lt;&lt;</TD>\n";
        else
          output << p1 << "startGenerationIndex=0');\" >&lt;&lt;</TD>\n";
      }
      else
        output << p2;

      if (len > (startGenerationIndex+maxRecords))
        output << p1 << "startGenerationIndex=" << (startGenerationIndex+maxRecords) << "');\" >&gt;&gt;</TD>\n";
      else
        output << p2;

      output << "</TR></TABLE>\n";
    }

    if ((mFlags & Flags::contentModificationEnabled) && mainSource && (session.mUserInfo.getUserId() == 0 || session.mUserInfo.isUserGroupMember("grid-admin")))
    {
      std::string prod = "&startGenerationIndex=" + std::to_string(startGenerationIndex);

      if (mode > 0  && mode < 100)
      {
        std::string dt = "21000101T000000";
        if (gInfo.mDeletionTime > 0)
          dt = utcTimeFromTimeT(gInfo.mDeletionTime);

        std::string mt = utcTimeFromTimeT(gInfo.mModificationTime);
        std::string p1 = "<TR><TD width=\"240\" bgColor=\"#D0D0D0\">";
        std::string p2 = "</TD><TD><INPUT style=\"width:100%;\" type=\"text\" ";
        std::string p3 = "\"></TD></TR>";

        output << "<HR>\n";
        output << "<H2>Generation</H2>\n";
        output << "<TABLE border=\"1\" width=\"100%\" style=\"font-size:12;\">\n";
        output << p1 << "Id</TD><TD>"<< gInfo.mGenerationId << "</TD></TR>";
        output << p1 << "Name" << p2 << "id=\"generation_name\" value=\"" << gInfo.mName << p3;
        output << p1 << "Description" << p2 << "id=\"generation_description\" value=\"" << gInfo.mDescription << p3;
        output << p1 << "AnalysisTime" << p2 << "id=\"generation_analysisTime\" value=\"" << gInfo.mAnalysisTime << p3;
        output << p1 << "Flags" << p2 << "id=\"generation_flags\" value=\"" << gInfo.mFlags << p3;
        output << p1 << "SourceId" << p2 << "id=\"generation_sourceId\" value=\"" << gInfo.mSourceId << p3;
        output << p1 << "Status" << p2 << "id=\"generation_status\" value=\"" << (int)gInfo.mStatus << p3;
        output << p1 << "ModificationTime" << p2 << "id=\"generation_modificationTime\" value=\"" << mt << p3;
        output << p1 << "DeletionTime" << p2 << "id=\"generation_deletionTime\" value=\"" << dt << p3;
        output << "</TABLE>\n";
      }

      std::string bg = "#C0C0C0";
      std::string p1 = "<TD width=\"150\" height=\"25\" align=\"center\" style=\"background:" + bg + ";\" onmouseout=\"this.style='background:" + bg + ";'\" onmouseover=\"this.style='background:#0000FF; color:#FFFFFF';\" onClick=\"getPage(this,parent,'/grid-admin?mode=";

      output << "<HR>\n";
      output << "<TABLE style=\"font-size:12;\">\n";
      output << "<TR>\n";
      switch (mode)
      {
        case MODE_NONE:
          output << p1 << MODE_ADD_REQUEST << prod << "');\" >New generation</TD>\n";
          if (generationId != 0)
          {
            output << p1 << MODE_EDIT_REQUEST << prod << "&generationId=" << generationId << "');\" >Edit generation</TD>\n";
            output << p1 << MODE_DELETE_REQUEST << prod << "&generationId=" << generationId << "');\" >Delete generation</TD>\n";
          }
          break;

        case MODE_ADD_REQUEST:
          output << p1 << MODE_ADD_CONFIRM << prod << "&name='+generation_name.value+'&analysisTime='+generation_analysisTime.value+'&description='+generation_description.value+'&flags='+generation_flags.value+'&sourceId='+generation_sourceId.value+'&status='+generation_status.value+'&deletionTime='+generation_deletionTime.value+'&modificationTime='+generation_modificationTime.value);\" >Add generation</TD>\n";
          output << p1 << MODE_NONE << prod << "&generationId=" << generationId << "');\" >Cancel</TD>\n";
          break;

        case MODE_EDIT_REQUEST:
          output << p1 << MODE_EDIT_CONFIRM << prod << "&generationId=" << generationId << "&name='+generation_name.value+'&analysisTime='+generation_analysisTime.value+'&description='+generation_description.value+'&flags='+generation_flags.value+'&sourceId='+generation_sourceId.value+'&status='+generation_status.value+'&deletionTime='+generation_deletionTime.value+'&modificationTime='+generation_modificationTime.value);\" >Update generation</TD>\n";
          output << p1 << MODE_NONE << prod << "&generationId=" << generationId << "');\" >Cancel</TD>\n";
          break;

        case MODE_DELETE_REQUEST:
          output << p1 << MODE_DELETE_CONFIRM << prod << "&generationId=" << generationId << "');\" >Delete generation</TD>\n";
          output << p1 << MODE_NONE << prod << "&generationId=" << generationId << "');\" >Cancel</TD>\n";
          break;
      }

      output << "</TR>\n";
      output << "</TABLE>\n";
    }

    output << "<HR>\n";
    output << "</BODY>\n";
    output << "</HTML>\n";


    theResponse.setContent(output.str());
    theResponse.setHeader("Content-Type", "text/html; charset=UTF-8");

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}





bool Browser::page_producers(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse)
{
  try
  {
    ContentServer_sptr contentServer = mMainContentServer;
    std::string sourceStr = "main";
    bool mainSource = true;

    session.getAttribute("grid-engine","source",sourceStr);

    if (sourceStr == "cache")
    {
      contentServer = mCacheContentServer;
      mainSource = false;
    }

    T::ProducerInfo pInfo;
    uint maxRecords = 30;

    uint producerId = 0;
    session.getAttribute("grid-engine","producerId",producerId);

    uint startProducerIndex = 0;
    session.getAttribute("grid-engine","startProducerIndex",startProducerIndex);

    uint mode = 0;
    auto modeStr = theRequest.getParameter("mode");
    if (modeStr)
      mode = atoi(modeStr->c_str());

    if ((mFlags & Flags::contentModificationEnabled) && mainSource  &&  mode >= 100 && (session.mUserInfo.getUserId() == 0 || session.mUserInfo.isUserGroupMember("grid-admin")))
    {
      auto v = theRequest.getParameter("name");
      if (v)
        pInfo.mName = v->c_str();

      v = theRequest.getParameter("title");
      if (v)
        pInfo.mTitle = v->c_str();

      v = theRequest.getParameter("description");
      if (v)
        pInfo.mDescription = v->c_str();

      v = theRequest.getParameter("flags");
      if (v)
        pInfo.mFlags = atoi(v->c_str());

      v = theRequest.getParameter("sourceId");
      if (v)
        pInfo.mSourceId = atoi(v->c_str());

      switch (mode)
      {
        case MODE_ADD_CONFIRM:
        {
          int res = contentServer->addProducerInfo(0,pInfo);
          if (res != 0)
          {
            // Producer add failed
          }
          else
          {
            producerId = pInfo.mProducerId;
          }
        }
        break;

        case MODE_EDIT_CONFIRM:
        {
          pInfo.mProducerId = producerId;
          int res = contentServer->setProducerInfo(0,pInfo);
          if (res != 0)
          {
            // Producer update failed
          }
        }
        break;

        case MODE_DELETE_CONFIRM:
        {
          if (producerId != 0)
          {
            int res = contentServer->deleteProducerInfoById(0,producerId);
            if (res != 0)
            {
              // Producer delete failed
            }
            else
            {
              producerId = 0;
              session.setAttribute("grid-engine","producerId",0);
            }
          }
        }
        break;
      }
      mode = 0;
    }

    std::ostringstream output;

    output << "<HTML>\n";
    output << "<SCRIPT>\n";
    output << "function getPage(obj,frm,url)\n";
    output << "{\n";
    output << "  frm.location.href=url;\n";
    output << "}\n";
    output << "</SCRIPT>\n";
    output << "<BODY style=\"font-size:12;\">\n";

    output << "<HR>\n";
    output << "<A href=\"grid-admin?target=grid-admin&page=start\">SmartMet Server</A> / ";
    output << "<A href=\"grid-admin?target=grid-admin&page=engines\">Engines</A> / ";
    output << "<A href=\"grid-admin?target=grid-engine&page=start\">Grid Engine</A> / ";
    output << "<A href=\"grid-admin?target=grid-engine&page=contentServer\">Content Server</A> / ";
    output << "<A href=\"grid-admin?target=grid-engine&page=contentInformation\">Content Information</A> / ";
    output << "<HR>\n";
    output << "<H2>Producers (" << sourceStr << " / " << contentServer->getSourceInfo()  <<  ")</H2>\n";
    output << "<HR>\n";

    output << "<TABLE border=\"1\" width=\"100%\" style=\"font-size:12;\">\n";
    output << "<TR bgColor=\"#D0D0D0\">";
    output << "<TD>Id</TD>";
    output << "<TD>Name</TD>";
    output << "<TD>Title</TD>";
    output << "<TD>Description</TD>";
    output << "<TD>Flags</TD>";
    output << "<TD>SourceId</TD>";
    output << "</TR>";

    std::string fg = "#000000";
    std::string bg = "#FFFFFF";

    T::ProducerInfoList producerInfoList;
    contentServer->getProducerInfoList(0,producerInfoList);

    uint len = producerInfoList.getLength();
    for (uint t=0; t<maxRecords; t++)
    {
      T::ProducerInfo *producer = producerInfoList.getProducerInfoByIndex(t+startProducerIndex);
      if (producer != nullptr)
      {
        if (producer->mProducerId == producerId)
        {
          fg = "#FFFFFF";
          bg = "#FF0000";
          pInfo = *producer;
          output << "<TR style=\"background:" << bg << "; color:" << fg << ";\" onClick=\"getPage(this,parent,'/grid-admin?page=generations&producerId=" << producer->mProducerId << "&producerName=" << producer->mName << "');\" >\n";
        }
        else
        {
          fg = "#000000";
          bg = "#FFFFFF";
          output << "<TR style=\"background:" << bg << "; color:" << fg << ";\" onmouseout=\"this.style='background:" << bg <<"; color:" << fg << ";'\" onmouseover=\"this.style='background:#FFFF00; color:#000000;';\" onClick=\"getPage(this,parent,'/grid-admin?startProducerIndex=" << startProducerIndex << "&producerId=" << producer->mProducerId << "&producerName=" << producer->mName << "');\" >\n";
        }

        output << "<TD>"<< producer->mProducerId << "</TD>";
        output << "<TD>"<< producer->mName << "</TD>";
        output << "<TD>"<< producer->mTitle << "</TD>";
        output << "<TD>"<< producer->mDescription << "</TD>";
        output << "<TD>"<< producer->mFlags << "</TD>";
        output << "<TD>"<< producer->mSourceId << "</TD>";
        output << "</TR>";
      }
      else
      {
        output << "<TR><TD>&nbsp;</TD><TD>&nbsp;</TD><TD>&nbsp;</TD><TD>&nbsp;</TD><TD>&nbsp;</TD><TD>&nbsp;</TD></TR>\n";
      }
    }
    output << "</TABLE>\n";

    if (startProducerIndex > 0 || len >= maxRecords)
    {
      std::string bg = "#C0C0C0";
      std::string p1 = "<TD width=\"70\" align=\"center\" style=\"background:" + bg + ";\" onmouseout=\"this.style='background:" + bg + ";'\" onmouseover=\"this.style='background:#0000FF; color:#FFFFFF;';\" onClick=\"getPage(this,parent,'/grid-admin?";
      std::string p2 = "<TD width=\"70\" style=\"background:" + bg + ";\" >&nbsp;</TD>\n";

      output << "<HR>\n";
      output << "<TABLE style=\"font-size:12;\"><TR>\n";

      if (startProducerIndex > 0)
      {
        if (startProducerIndex > maxRecords)
          output << p1 << "startProducerIndex=" << (startProducerIndex-maxRecords) << "');\" >&lt;&lt;</TD>\n";
        else
          output << p1 << "startProducerIndex=0');\" >&lt;&lt;</TD>\n";
      }
      else
        output << p2;

      if (len > (startProducerIndex+maxRecords))
        output << p1 << "startProducerIndex=" << (startProducerIndex+maxRecords) << "');\" >&gt;&gt;</TD>\n";
      else
        output << p2;

      output << "</TR></TABLE>\n";
    }

    if ((mFlags & Flags::contentModificationEnabled) && mainSource && (session.mUserInfo.getUserId() == 0 || session.mUserInfo.isUserGroupMember("grid-admin")))
    {
      if (mode > 0  && mode < 100)
      {
        std::string p1 = "<TR><TD width=\"240\" bgColor=\"#D0D0D0\">";
        std::string p2 = "</TD><TD><INPUT style=\"width:100%;\" type=\"text\" ";
        std::string p3 = "\"></TD></TR>";

        output << "<HR>\n";
        output << "<H2>Producer</H2>\n";
        output << "<TABLE border=\"1\" width=\"100%\" style=\"font-size:12;\">\n";
        output << p1 << "Id</TD><TD>"<< pInfo.mProducerId << "</TD></TR>";
        output << p1 << "Name" << p2 << "id=\"producer_name\" value=\"" << pInfo.mName << p3;
        output << p1 << "Title" << p2 << "id=\"producer_title\" value=\"" << pInfo.mTitle << p3;
        output << p1 << "Description" << p2 << "id=\"producer_description\" value=\"" << pInfo.mDescription << p3;
        output << p1 << "Flags" << p2 << "id=\"producer_flags\" value=\"" << pInfo.mFlags << p3;
        output << p1 << "SourceId" << p2 << "id=\"producer_sourceId\" value=\"" << pInfo.mSourceId << p3;
        output << "</TABLE>\n";
      }

      bg = "#C0C0C0";
      std::string p1 = "<TD width=\"150\" height=\"25\" align=\"center\" style=\"background:" + bg + ";\" onmouseout=\"this.style='background:" + bg + ";'\" onmouseover=\"this.style='background:#0000FF; color:#FFFFFF';\" onClick=\"getPage(this,parent,'/grid-admin?mode=";

      output << "<HR>\n";
      output << "<TABLE style=\"font-size:12;\">\n";
      output << "<TR>\n";
      switch (mode)
      {
        case MODE_NONE:
          output << p1 << MODE_ADD_REQUEST << ">');\" >New producer</TD>\n";
          if (producerId != 0)
          {
            output << p1 << MODE_EDIT_REQUEST << "&producerId=" << producerId << "');\" >Edit producer</TD>\n";
            output << p1 << MODE_DELETE_REQUEST << "&producerId=" << producerId << "');\" >Delete producer</TD>\n";
          }
          break;

        case MODE_ADD_REQUEST:
          output << p1 << MODE_ADD_CONFIRM << "&name='+producer_name.value+'&title='+producer_title.value+'&description='+producer_description.value+'&flags='+producer_flags.value+'&sourceId='+producer_sourceId.value);\" >Add producer</TD>\n";
          output << p1 << MODE_NONE << "&producerId=" << producerId << "');\" >Cancel</TD>\n";
          break;

        case MODE_EDIT_REQUEST:
          output << p1 << MODE_EDIT_CONFIRM << "&producerId=" << producerId << "&name='+producer_name.value+'&title='+producer_title.value+'&description='+producer_description.value+'&flags='+producer_flags.value+'&sourceId='+producer_sourceId.value);\" >Update producer</TD>\n";
          output << p1 << MODE_NONE << "&producerId=" << producerId << "');\" >Cancel</TD>\n";
          break;

        case MODE_DELETE_REQUEST:
          output << p1 << MODE_DELETE_CONFIRM << "&producerId=" << producerId << "');\" >Delete producer</TD>\n";
          output << p1 << MODE_NONE << "&producerId=" << producerId << "');\" >Cancel</TD>\n";
          break;
      }

      output << "</TR>\n";
      output << "</TABLE>\n";
    }

    output << "<HR>\n";

    output << "</BODY>\n";
    output << "</HTML>\n";


    theResponse.setContent(output.str());
    theResponse.setHeader("Content-Type", "text/html; charset=UTF-8");

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}




bool Browser::page_contentInformation(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse)
{
  try
  {
    std::ostringstream output;

    output << "<HTML>\n";
    output << "<BODY style=\"font-size:12;\">\n";
    output << "<HR>\n";
    output << "<A href=\"grid-admin?target=grid-admin&page=start\">SmartMet Server</A> / ";
    output << "<A href=\"grid-admin?target=grid-admin&page=engines\">Engines</A> / ";
    output << "<A href=\"grid-admin?target=grid-engine&page=start\">Grid Engine</A> / ";
    output << "<A href=\"grid-admin?target=grid-engine&page=contentServer\">Content Server</A> / ";
    output << "<HR>\n";
    output << "<H2>Content information</H2>\n";
    output << "<HR>\n";
    output << "<OL>\n";
    output << "  <LI>";
    output << "    <A href=\"/grid-admin?&target=grid-engine&page=producers&source=main\">Main content information (Redis)</A>";
    output << "  </LI>";
    output << "  <LI>";
    output << "     <A href=\"/grid-admin?&target=grid-engine&page=producers&source=cache\">Cached content information (Grid Engine)</A>";
    output << "  </LI>";
    output << "</OL>\n";
    output << "<HR>\n";
    output << "</BODY>\n";
    output << "</HTML>\n";


    theResponse.setContent(output.str());
    theResponse.setHeader("Content-Type", "text/html; charset=UTF-8");

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}




bool Browser::page_contentServer(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse)
{
  try
  {
    std::ostringstream output;

    output << "<HTML>\n";
    output << "<BODY style=\"font-size:12;\">\n";
    output << "<HR>\n";
    output << "<A href=\"grid-admin?target=grid-admin&page=start\">SmartMet Server</A> / ";
    output << "<A href=\"grid-admin?target=grid-admin&page=engines\">Engines</A> / ";
    output << "<A href=\"grid-admin?target=grid-engine&page=start\">Grid Engine</A> / ";
    output << "<HR>\n";
    output << "<H2>Content Server</H2>\n";
    output << "<HR>\n";
    output << "<OL>\n";
    output << "  <LI>";
    output << "    <A href=\"/grid-admin?&target=grid-engine&page=contentInformation\">Content Information</A>";
    output << "    <OL>\n";
    output << "      <LI>";
    output << "        <A href=\"/grid-admin?&target=grid-engine&page=producers&source=main\">Main content information (Redis)</A>";
    output << "      </LI>";
    output << "      <LI>";
    output << "         <A href=\"/grid-admin?&target=grid-engine&page=producers&source=cache\">Cached content information (Grid Engine)</A>";
    output << "      </LI>";
    output << "    </OL>\n";
    output << "  </LI>";
    output << "  <LI>";
    output << "    <H4>Logs</H4>";
    output << "    <OL>\n";
    output << "      <LI>";
    output << "         <A href=\"/grid-admin?&target=grid-engine&page=contentServer_processingLog\">Processing log</A>";
    output << "      </LI>";
    output << "      <LI>";
    output << "         <A href=\"/grid-admin?&target=grid-engine&page=contentServer_debugLog\">Debug log</A>";
    output << "      </LI>";
    output << "    </OL>\n";
    output << "  </LI>";
    output << "</OL>\n";
    output << "<HR>\n";
    output << "</BODY>\n";
    output << "</HTML>\n";


    theResponse.setContent(output.str());
    theResponse.setHeader("Content-Type", "text/html; charset=UTF-8");

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}




bool Browser::page_dataServer(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse)
{
  try
  {
    std::ostringstream output;

    output << "<HTML>\n";
    output << "<BODY style=\"font-size:12;\">\n";
    output << "<HR>\n";
    output << "<A href=\"grid-admin?target=grid-admin&page=start\">SmartMet Server</A> / ";
    output << "<A href=\"grid-admin?target=grid-admin&page=engines\">Engines</A> / ";
    output << "<A href=\"grid-admin?target=grid-engine&page=start\">Grid Engine</A> / ";
    output << "<HR>\n";
    output << "<H2>Data Server</H2>\n";
    output << "<HR>\n";
    output << "<OL>\n";
    output << "  <LI>";
    output << "    <H4>Logs</H4>";
    output << "    <OL>\n";
    output << "      <LI>";
    output << "         <A href=\"/grid-admin?&target=grid-engine&page=dataServer_processingLog\">Processing log</A>";
    output << "      </LI>";
    output << "      <LI>";
    output << "         <A href=\"/grid-admin?&target=grid-engine&page=dataServer_debugLog\">Debug log</A>";
    output << "      </LI>";
    output << "    </OL>\n";
    output << "  </LI>";
    output << "</OL>\n";
    output << "<HR>\n";
    output << "</BODY>\n";
    output << "</HTML>\n";


    theResponse.setContent(output.str());
    theResponse.setHeader("Content-Type", "text/html; charset=UTF-8");

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}




bool Browser::page_queryServer(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse)
{
  try
  {
    std::ostringstream output;

    output << "<HTML>\n";
    output << "<BODY style=\"font-size:12;\">\n";
    output << "<HR>\n";
    output << "<A href=\"grid-admin?target=grid-admin&page=start\">SmartMet Server</A> / ";
    output << "<A href=\"grid-admin?target=grid-admin&page=engines\">Engines</A> / ";
    output << "<A href=\"grid-admin?target=grid-engine&page=start\">Grid Engine</A> / ";
    output << "<HR>\n";
    output << "<H2>Data Server</H2>\n";
    output << "<HR>\n";
    output << "<OL>\n";
    output << "  <LI>";
    output << "    <H4>Logs</H4>";
    output << "    <OL>\n";
    output << "      <LI>";
    output << "         <A href=\"/grid-admin?&target=grid-engine&page=queryServer_processingLog\">Processing log</A>";
    output << "      </LI>";
    output << "      <LI>";
    output << "         <A href=\"/grid-admin?&target=grid-engine&page=queryServer_debugLog\">Debug log</A>";
    output << "      </LI>";
    output << "    </OL>\n";
    output << "  </LI>";
    output << "</OL>\n";
    output << "<HR>\n";
    output << "</BODY>\n";
    output << "</HTML>\n";


    theResponse.setContent(output.str());
    theResponse.setHeader("Content-Type", "text/html; charset=UTF-8");

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}




bool Browser::page_luaFile(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse)
{
  try
  {
    auto hashStr = theRequest.getParameter("filename");
    if (!hashStr)
      return false;

    auto fname = mFilenames.find(*hashStr);
    if (fname == mFilenames.end())
      return false;


    std::ostringstream output;

    output << "<HTML>\n";
    output << "<BODY style=\"font-size:12;\">\n";

    output << "<HR>\n";
    output << "<A href=\"grid-admin?target=grid-admin&page=start\">SmartMet Server</A> / ";
    output << "<A href=\"grid-admin?target=grid-admin&page=engines\">Engines</A> / ";
    output << "<A href=\"grid-admin?target=grid-engine&page=start\">Grid Engine</A> / ";
    output << "<A href=\"grid-admin?target=grid-engine&page=configuration\">Configuration</A> / ";
    output << "<A href=\"grid-admin?target=grid-engine&page=luaFiles\">LUA files</A> / ";
    output << "<HR>\n";
    output << "<H2>" << fname->second << "</H2>\n";
    output << "<HR>\n";

    output << "<PRE style=\"background-color: #F0F0F0;\">\n";


    output << "\n";
    includeFile(output,fname->second.c_str());
    output << "\n";

    output << "</PRE>\n";
    output << "<HR>\n";

    output << "</BODY>\n";
    output << "</HTML>\n";

    theResponse.setContent(output.str());
    theResponse.setHeader("Content-Type", "text/html; charset=UTF-8");

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}




bool Browser::page_luaFiles(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse)
{
  try
  {
    ConfigurationFile configurationFile;
    configurationFile.readFile(mGridEngine->getConfigurationFileName().c_str());

    string_vec luaFiles;
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.luaFiles",luaFiles);

    std::ostringstream output;

    output << "<HTML>\n";
    output << "<SCRIPT>\n";
    output << "function getPage(obj,frm,url)\n";
    output << "{\n";
    output << "  frm.location.href=url;\n";
    output << "}\n";
    output << "</SCRIPT>\n";
    output << "<BODY style=\"font-size:12;\">\n";

    output << "<HR>\n";
    output << "<A href=\"grid-admin?target=grid-admin&page=start\">SmartMet Server</A> / ";
    output << "<A href=\"grid-admin?target=grid-admin&page=engines\">Engines</A> / ";
    output << "<A href=\"grid-admin?target=grid-engine&page=start\">Grid Engine</A> / ";
    output << "<A href=\"grid-admin?target=grid-engine&page=configuration\">Configuration</A> / ";
    output << "<HR>\n";
    output << "<H2>LUA files</H2>\n";
    output << "<HR>\n";
    output << "<H3>Introduction</H3>\n";
    output << "<P>LUA files are used for the following purposes:</P> \n";
    output << "<OL>\n";
    output << "  <LI>\n";
    output << "    <B>Converting mapped parameters</B>\n";
    output << "      <P>Sometimes parameter mapping requires that the parameter values will be converted before they can be returned.\n";
    output << "      For example, the 'temperature' parameter (Celcius) is most likely mapped to a parameter that contains Kelvin values.\n";
    output << "      In this case, we have defined a conversion function (K2C or SUM) that is called in order to convert Kelvin values to \n";
    output << "      Celcius values. Conversion functions typically take only one grid parameter as input. In addition they might\n";
    output << "      take some location specific parameters (like dem, dark, daylength, etc.). The actual implementation of these \n";
    output << "      conversion functions are usually defined in LUA files.</P>\n";
    output << "  </LI>\n";
    output << "  <LI>\n";
    output << "    <B>Processing query parameters</B>\n";
    output << "      <P>LUA functions can be used for example in Timeseries queries. In this case the requested parameter values are\n";
    output << "      given as input to a LUA function (for example 'MYFUNC{Temperature;Pressure;WindSpeedMS}').</P>\n";
    output << "  </LI>\n";
    output << "</OL>\n";

    output << "<P>It is also good to realize that the most common functions (K2C,K2F,SUM,DIV,MUL,AVG,MIN,MAX,etc) are implemented \n";
    output << "with C++ for performance reasons. This means in practice that the system first checks if there is C++ implementation \n";
    output << "available, before it calls a LUA function.</P>\n";

    output << "<HR>\n";
    output << "<H3>Files</H3>\n";
    output << "<P>The current installation contains the following lua files used by the query server:</P>\n";

    output << "<TABLE border=\"1\" width=\"100%\" style=\"font-size:12;\">\n";
    output << "<TR bgColor=\"#D0D0D0\">";
    output << "<TD>Filename</TD>";
    output << "</TR>";


    std::string fg = "#000000";
    std::string bg = "#FFFFFF";

    for (auto it = luaFiles.begin(); it != luaFiles.end(); ++it)
    {
      std::size_t hash = 0;
      boost::hash_combine(hash,*it);
      std::string hashStr = std::to_string(hash);
      if (mFilenames.find(hashStr) == mFilenames.end())
        mFilenames.insert(std::pair<std::string,std::string>(hashStr,*it));

      fg = "#000000";
      bg = "#FFFFFF";
      output << "<TR style=\"background:" << bg << "; color:" << fg << ";\" onmouseout=\"this.style='background:" << bg <<"; color:" << fg << ";'\" onmouseover=\"this.style='background:#FFFF00; color:#000000;';\" onClick=\"getPage(this,parent,'/grid-admin?&target=grid-engine&page=luaFile&filename=" << hash << "');\" >\n";
      output << "<TD>"<< *it << "</TD>";
      output << "</TR>";

    }

    output << "</TABLE>\n";
    output << "<HR>\n";


    output << "</BODY>\n";
    output << "</HTML>\n";


    theResponse.setContent(output.str());
    theResponse.setHeader("Content-Type", "text/html; charset=UTF-8");

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}




bool Browser::page_parameterAliasFile(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse)
{
  try
  {
    auto hashStr = theRequest.getParameter("filename");
    if (!hashStr)
      return false;

    auto fname = mFilenames.find(*hashStr);
    if (fname == mFilenames.end())
      return false;


    std::ostringstream output;

    output << "<HTML>\n";
    output << "<BODY style=\"font-size:12;\">\n";

    output << "<HR>\n";
    output << "<A href=\"grid-admin?target=grid-admin&page=start\">SmartMet Server</A> / ";
    output << "<A href=\"grid-admin?target=grid-admin&page=engines\">Engines</A> / ";
    output << "<A href=\"grid-admin?target=grid-engine&page=start\">Grid Engine</A> / ";
    output << "<A href=\"grid-admin?target=grid-engine&page=configuration\">Configuration</A> / ";
    output << "<A href=\"grid-admin?target=grid-engine&page=parameterAliasFiles\">Parameter alias files</A> / ";
    output << "<HR>\n";
    output << "<H2>" << fname->second << "</H2>\n";
    output << "<HR>\n";

    output << "<PRE style=\"background-color: #F0F0F0;\">\n";

    output << "\n";
    includeFile(output,fname->second.c_str());
    output << "\n";

    output << "</PRE>\n";
    output << "<HR>\n";

    output << "</BODY>\n";
    output << "</HTML>\n";

    theResponse.setContent(output.str());
    theResponse.setHeader("Content-Type", "text/html; charset=UTF-8");

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}




bool Browser::page_parameterAliasFiles(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse)
{
  try
  {
    ConfigurationFile configurationFile;
    configurationFile.readFile(mGridEngine->getConfigurationFileName().c_str());

    string_vec parameterAliasFiles;
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.aliasFiles",parameterAliasFiles);


    std::ostringstream output;

    output << "<HTML>\n";
    output << "<SCRIPT>\n";
    output << "function getPage(obj,frm,url)\n";
    output << "{\n";
    output << "  frm.location.href=url;\n";
    output << "}\n";
    output << "</SCRIPT>\n";
    output << "<BODY style=\"font-size:12;\">\n";

    output << "<HR>\n";
    output << "<A href=\"grid-admin?target=grid-admin&page=start\">SmartMet Server</A> / ";
    output << "<A href=\"grid-admin?target=grid-admin&page=engines\">Engines</A> / ";
    output << "<A href=\"grid-admin?target=grid-engine&page=start\">Grid Engine</A> / ";
    output << "<A href=\"grid-admin?target=grid-engine&page=configuration\">Configuration</A> / ";
    output << "<HR>\n";
    output << "<H2>Parameter alias files</H2>\n";
    output << "<HR>\n";
    output << "<H3>Introduction</H3>\n";
    output << "<P>Parameter alias files can be used in order to create alias names for query parameters (For example,\n";
    output << "'TemperatureK' => 'T-K').Alias names are very useful when the original parameter name is long or if it \n";
    output << "contains a function (For example, 'TemperatureC' => 'SUM{T-K;-273.15}'.</P>\n";

    output << "<P>Notice that usually you should use <B>parameter mapping files</B> in order to define new  \"official\" \n";
    output << "parameter names. Alias names can be used for defining shorter names for those \"official\" names. They can be \n";
    output << "used also for translating parameter names for different languages.</P>";

    output << "<HR>\n";
    output << "<H3>Files</H3>\n";
    output << "<P>The current installation contains the following parameter alias files:</P>\n";

    output << "<TABLE border=\"1\" width=\"100%\" style=\"font-size:12;\">\n";
    output << "<TR bgColor=\"#D0D0D0\">";
    output << "<TD>Filename</TD>";
    output << "</TR>";

    std::string fg = "#000000";
    std::string bg = "#FFFFFF";

    for (auto it = parameterAliasFiles.begin(); it != parameterAliasFiles.end(); ++it)
    {
      std::size_t hash = 0;
      boost::hash_combine(hash,*it);
      std::string hashStr = std::to_string(hash);
      if (mFilenames.find(hashStr) == mFilenames.end())
        mFilenames.insert(std::pair<std::string,std::string>(hashStr,*it));

      fg = "#000000";
      bg = "#FFFFFF";
      output << "<TR style=\"background:" << bg << "; color:" << fg << ";\" onmouseout=\"this.style='background:" << bg <<"; color:" << fg << ";'\" onmouseover=\"this.style='background:#FFFF00; color:#000000;';\" onClick=\"getPage(this,parent,'/grid-admin?&target=grid-engine&page=parameterAliasFile&filename=" << hash << "');\" >\n";
      output << "<TD>"<< *it << "</TD>";
      output << "</TR>";

    }

    output << "</TABLE>\n";
    output << "<HR>\n";
    output << "</BODY>\n";
    output << "</HTML>\n";


    theResponse.setContent(output.str());
    theResponse.setHeader("Content-Type", "text/html; charset=UTF-8");

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}




bool Browser::page_producerMappingFile(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse)
{
  try
  {
    auto hashStr = theRequest.getParameter("filename");
    if (!hashStr)
      return false;

    auto fname = mFilenames.find(*hashStr);
    if (fname == mFilenames.end())
      return false;


    std::ostringstream output;

    output << "<HTML>\n";
    output << "<BODY style=\"font-size:12;\">\n";

    output << "<HR>\n";
    output << "<A href=\"grid-admin?target=grid-admin&page=start\">SmartMet Server</A> / ";
    output << "<A href=\"grid-admin?target=grid-admin&page=engines\">Engines</A> / ";
    output << "<A href=\"grid-admin?target=grid-engine&page=start\">Grid Engine</A> / ";
    output << "<A href=\"grid-admin?target=grid-engine&page=configuration\">Configuration</A> / ";
    output << "<A href=\"grid-admin?target=grid-engine&page=producerMappingFiles\">Producer mapping files</A> / ";
    output << "<HR>\n";
    output << "<H2>" << fname->second << "</H2>\n";
    output << "<HR>\n";

    output << "<PRE style=\"background-color: #F0F0F0;\">\n";

    output << "\n";
    includeFile(output,fname->second.c_str());
    output << "\n";

    output << "</PRE>\n";
    output << "<HR>\n";

    output << "</BODY>\n";
    output << "</HTML>\n";

    theResponse.setContent(output.str());
    theResponse.setHeader("Content-Type", "text/html; charset=UTF-8");

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}




bool Browser::page_producerMappingFiles(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse)
{
  try
  {
    ConfigurationFile configurationFile;
    configurationFile.readFile(mGridEngine->getConfigurationFileName().c_str());

    string_vec producerMappingFiles;
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.producerMappingFiles",producerMappingFiles);


    std::ostringstream output;

    output << "<HTML>\n";
    output << "<SCRIPT>\n";
    output << "function getPage(obj,frm,url)\n";
    output << "{\n";
    output << "  frm.location.href=url;\n";
    output << "}\n";
    output << "</SCRIPT>\n";
    output << "<BODY style=\"font-size:12;\">\n";

    output << "<HR>\n";
    output << "<A href=\"grid-admin?target=grid-admin&page=start\">SmartMet Server</A> / ";
    output << "<A href=\"grid-admin?target=grid-admin&page=engines\">Engines</A> / ";
    output << "<A href=\"grid-admin?target=grid-engine&page=start\">Grid Engine</A> / ";
    output << "<A href=\"grid-admin?target=grid-engine&page=configuration\">Configuration</A> / ";
    output << "<HR>\n";
    output << "<H2>Producer mapping files</H2>\n";
    output << "<HR>\n";
    output << "<H3>Introduction</H3>\n";
    output << "<P>Producer mapping files are needed in order to make the new queries compatible with the older (newbase) queries. We do not ";
    output << "need these files if there are no compatibility requirements in place. </P>\n";

    output << "<P>The difficulty with the older queries is that the requested grid geometry and the level type is encoded into the producer name ";
    output << "(for example,'ecmwf_europe_surface'). In the new queries, same producers usually support multiple geometries and level types.";
    output << "In addition, it is quite common that there are no one-to-one mappings between old and new producers.\n";
    output << "It is more common that we need to do one-to-many mappings instead.</P>\n";

    output << "We need producer mapping files for the following purposes:\n";
    output << "<OL>\n";
    output << "  <LI>\n";
    output << "    <B>Mapping old (newbase) producers to new (Radon) producers, geometries and level types</B>\n";
    output << "      <P>Producer level mapping is used only if there are no better parameter level mappings available.\n";
    output << "      For example, the 'pal_scandinavia' producer is mapped to the producers 'SMARTMET' and 'SMARTMETMTA'.";
    output << "      However, this does not tell which producer supports the requested parameter and that's why the both";
    output << "      producers need to be queried. This might work, but not necessary as we like. That's because it is possible ";
    output << "      that the both producers have the same parameter (which might be counted differently).</P>";
    output << "  </LI>\n";
    output << "  <LI>\n";
    output << "    <B>Mapping old (newbase) parameters to correct new (Radon) producers</B>\n";
    output << "      <P>Usually all parameters should be mapped separately, because they might be found from different produces.\n";
    output << "      For example, the 'pal_scandinavia' producer has parameters 'Temperature' and 'PrecipitationType'. The first can be \n";
    output << "      found from the 'SMARTMET' producer, but the second is found from the 'SMARTMETMTA' producer. That's why they should have\n";
    output << "      own mappings.</P>\n";
    output << "  </LI>\n";
    output << "</OL>\n";
    output << "<HR>\n";
    output << "<H3>Files</H3>\n";
    output << "<P>The current installation contains the following producer mapping files:</P>\n";

    output << "<TABLE border=\"1\" width=\"100%\" style=\"font-size:12;\">\n";
    output << "<TR bgColor=\"#D0D0D0\">";
    output << "<TD>Filename</TD>";
    output << "</TR>";

    std::string fg = "#000000";
    std::string bg = "#FFFFFF";

    for (auto it = producerMappingFiles.begin(); it != producerMappingFiles.end(); ++it)
    {
      std::size_t hash = 0;
      boost::hash_combine(hash,*it);
      std::string hashStr = std::to_string(hash);
      if (mFilenames.find(hashStr) == mFilenames.end())
        mFilenames.insert(std::pair<std::string,std::string>(hashStr,*it));

      fg = "#000000";
      bg = "#FFFFFF";
      output << "<TR style=\"background:" << bg << "; color:" << fg << ";\" onmouseout=\"this.style='background:" << bg <<"; color:" << fg << ";'\" onmouseover=\"this.style='background:#FFFF00; color:#000000;';\" onClick=\"getPage(this,parent,'/grid-admin?&target=grid-engine&page=producerMappingFile&filename=" << hash << "');\" >\n";
      output << "<TD>"<< *it << "</TD>";
      output << "</TR>";

    }

    output << "</TABLE>\n";
    output << "<HR>\n";
    output << "</BODY>\n";
    output << "</HTML>\n";


    theResponse.setContent(output.str());
    theResponse.setHeader("Content-Type", "text/html; charset=UTF-8");

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}




bool Browser::page_parameterMappingFile(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse)
{
  try
  {
    std::ostringstream output;

    auto hashStr = theRequest.getParameter("filename");
    if (!hashStr)
      return false;

    auto fname = mFilenames.find(*hashStr);
    if (fname == mFilenames.end())
      return false;


    const char* paramIdType[] = {"UNKNOWN","FMI-ID","FMI-NAME","GRIB-ID","NEWBASE-ID","NEWBASE-NAME"};
    const char* levelIdType[] = {"UNKNOWN","FMI","GRIB1","GRIB2"};
    const char* interpolationMethod[] = {"None","Linear","Nearest","Min","Max","Logarithmic"};

    std::vector<std::vector<std::string>> records;
    readCsvFile(fname->second.c_str(),records);

    output << "<HTML>\n";
    output << "<BODY style=\"font-size:12;\">\n";

    output << "<HR>\n";
    output << "<A href=\"grid-admin?target=grid-admin&page=start\">SmartMet Server</A> / ";
    output << "<A href=\"grid-admin?target=grid-admin&page=engines\">Engines</A> / ";
    output << "<A href=\"grid-admin?target=grid-engine&page=start\">Grid Engine</A> / ";
    output << "<A href=\"grid-admin?target=grid-engine&page=configuration\">Configuration</A> / ";
    output << "<A href=\"grid-admin?target=grid-engine&page=parameterMappingFiles\">Parameter mapping files</A> / ";
    output << "<HR>\n";
    output << "<H2>"<< fname->second << "</H2>\n";
    output << "<HR>\n";

    output << "<TABLE border=\"1\" width=\"100%\" style=\"font-size:12;\">\n";
    output << "<TR bgColor=\"#D0D0D0\">";
    output << "<TD>Line</TD>";
    output << "<TD>Producer</TD>";
    output << "<TD>Mapping name</TD>";
    output << "<TD>Parameter Id type</TD>";
    output << "<TD>Parameter Id</TD>";
    output << "<TD>Geometry Id</TD>";
    output << "<TD>Level Id type</TD>";
    output << "<TD>Level Id</TD>";
    output << "<TD>Level</TD>";
    output << "<TD>Area interpolation</TD>";
    output << "<TD>Time interpolation</TD>";
    output << "<TD>Level interpolation</TD>";
    output << "<TD>Group flags</TD>";
    output << "<TD>Search match</TD>";
    output << "<TD>Mapping function</TD>";
    output << "<TD>Reverse mapping function</TD>";
    output << "<TD>Default precision</TD>";
    output << "</TR>";

    uint c = 0;
    for (auto rec = records.begin(); rec != records.end(); ++rec)
    {
      c++;
      output << "<TR>";
      output << "<TD>" << c << "</TD>";

      uint f = 0;
      for (auto field = rec->begin(); field != rec->end(); ++field)
      {
        f++;
        uint i = atoi(field->c_str());
        switch (f)
        {
          case 3:
            if (i < 6)
              output << "<TD>" << paramIdType[i] << "</TD>";
            else
              output << "<TD>" << *field << "</TD>";
            break;

          case 6:
            if (i < 4)
              output << "<TD>" << levelIdType[i] << "</TD>";
            else
              output << "<TD>" << *field << "</TD>";
            break;

          case 9:
            if (i < 5)
              output << "<TD>" << interpolationMethod[i] << "</TD>";
            else
              output << "<TD>" << *field << "</TD>";
            break;

          case 10:
            if (i < 5)
              output << "<TD>" << interpolationMethod[i] << "</TD>";
            else
              output << "<TD>" << *field << "</TD>";
            break;

          case 11:
            if (i < 6)
              output << "<TD>" << interpolationMethod[i] << "</TD>";
            else
              output << "<TD>" << *field << "</TD>";
            break;

          default:
            output << "<TD>" << *field << "</TD>";
            break;
        }
      }
      output << "</TR>\n";
    }

    output << "</TABLE>\n";
    output << "<HR>\n";

    output << "</BODY>\n";
    output << "</HTML>\n";

    theResponse.setContent(output.str());
    theResponse.setHeader("Content-Type", "text/html; charset=UTF-8");

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}




bool Browser::page_parameterMappingFiles(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse)
{
  try
  {
    ConfigurationFile configurationFile;
    configurationFile.readFile(mGridEngine->getConfigurationFileName().c_str());

    string_vec parameterMappingFiles;
    configurationFile.getAttributeValue("smartmet.engine.grid.query-server.mappingFiles",parameterMappingFiles);


    std::ostringstream output;

    output << "<HTML>\n";
    output << "<SCRIPT>\n";
    output << "function getPage(obj,frm,url)\n";
    output << "{\n";
    output << "  frm.location.href=url;\n";
    output << "}\n";
    output << "</SCRIPT>\n";
    output << "<BODY style=\"font-size:12;\">\n";

    output << "<HR>\n";
    output << "<A href=\"grid-admin?target=grid-admin&page=start\">SmartMet Server</A> / ";
    output << "<A href=\"grid-admin?target=grid-admin&page=engines\">Engines</A> / ";
    output << "<A href=\"grid-admin?target=grid-engine&page=start\">Grid Engine</A> / ";
    output << "<A href=\"grid-admin?target=grid-engine&page=configuration\">Configuration</A> / ";
    output << "<HR>\n";
    output << "<H2>Parameter mapping files</H2>\n";
    output << "<HR>\n";
    output << "<H3>Introduction</H3>\n";
    output << "<P>\n";
    output << "All grid parameters are registered into the content database (= Redis) by using parameter identifiers.\n";
    output << "Usually all parameters are registered by using the same parameter identifier type (FMI-NAME, Newbase-name, Grib-Id, etc.) \n";
    output << "and the content database is optimized for using this identifier type in its queries.\n";
    output << "If we want to use other parameter identifier types in our queries, we have to define parameter mapping files for them.\n";
    output << "On the other hand if we do not need other parameter identifiers, then we do not need so many mapping files.\n";
    output << "</P>\n";

    output << "We need parameter mapping files for the following purposes:\n";
    output << "<OL>\n";
    output << "  <LI>\n";
    output << "    <B>Defining other parameter identifiers (like newbase names) that can be used in the queries</B>\n";
    output << "      <P>For example, if the content database uses FMI-NAME as the primary parameter identifier type, but we want to use newbase";
    output << "      parameter names instead, then we have to define parameter mappings for all newbase names that we want to use.</P>";
    output << "  </LI>\n";
    output << "  <LI>\n";
    output << "    <B>Defining parameter value conversions for the queries </B>\n";
    output << "      <P>It is good to realize that some parameter mappings require that parameter values are converted during";
    output << "      the query. For example, the newbase parameter 'Temperature' (Celsius) is usually mapped to FMI-parameter 'T-K' (Kelvin), ";
    output << "      which means that the parameter values need to be converted from Kelvin to Celsius. In this case the conversion function ";
    output << "      is defined in the current mapping.</P>\n";

    output << "      <P>The current mapping contains also the reverse conversion function. This function is used ";
    output << "      when the query is requesting isolines/isobands with specific values. In this case the requested Celsius values are converted ";
    output << "      to Kelvins, before isolines/isobands are counted from the grid. This is faster than converting the whole grid to Celcius.</P>";
    output << "  </LI>\n";
    output << "  <LI>\n";
    output << "    <B>Defining parameter seach orders for the queries</B>\n";
    output << "      <P>It is quite common that queries do not exactly identify parameters that they are requesting. For example, they do not";
    output << "      necessary define which geometry, level type or level should be used. In this case, the first mapping of the requested parameter ";
    output << "       which 'search type' field has value 'E' is selected. So, the order of the mappings is significant in these cases.</P>";
    output << "  </LI>\n";
    output << "  <LI>\n";
    output << "    <B>Defining additional query information</B>\n";
    output << "      <P>It is qood to realize that there is also mapping file for FMI-NAME parameters in spite of that the content database uses ";
    output << "      usually this parameter identifier type as the primary parameter identifier. In this case the mappings are needed for defining ";
    output << "      additional information for the queries. By additional information, we mean the following fields:</P>";
    output << "      <OL>\n";
    output << "        <LI>Area interpolation method</LI>\n";
    output << "        <LI>Time interpolation method</LI>\n";
    output << "        <LI>Level interpolation method</LI>\n";
    output << "        <LI>Group flags (used for identifying climatological parameters)</LI>\n";
    output << "        <LI>Default precision</LI>\n";
    output << "      </OL></P>\n";
    output << "  </LI>\n";
    output << "  <LI>\n";
    output << "    <B>Defining missing parameter mappings automatically</B>\n";
    output << "      <P>New parameters are added continuosly into the content database. However, they cannot be queried if there are no mappings ";
    output << "      in place. That's why the system creates automatically temporary mappings for such parameters. Usually these mappings can be ";
    output << "      found from '*_auto.csv' files. These mappings are not necessary correct and that's why they need to be checked manually and ";
    output << "      moved into permanent mapping files.</P>";
    output << "  </LI>\n";
    output << "</OL>\n";

    output << "<HR>\n";
    output << "<H3>Files</H3>\n";
    output << "<P>The current installation contains the following parameter mapping files:</P>\n";

    output << "<TABLE border=\"1\" width=\"100%\" style=\"font-size:12;\">\n";
    output << "<TR bgColor=\"#D0D0D0\">";
    output << "<TD>Filename</TD>";
    output << "</TR>";

    std::string fg = "#000000";
    std::string bg = "#FFFFFF";

    for (auto it = parameterMappingFiles.begin(); it != parameterMappingFiles.end(); ++it)
    {
      std::size_t hash = 0;
      boost::hash_combine(hash,*it);
      std::string hashStr = std::to_string(hash);
      if (mFilenames.find(hashStr) == mFilenames.end())
        mFilenames.insert(std::pair<std::string,std::string>(hashStr,*it));

      fg = "#000000";
      bg = "#FFFFFF";
      output << "<TR style=\"background:" << bg << "; color:" << fg << ";\" onmouseout=\"this.style='background:" << bg <<"; color:" << fg << ";'\" onmouseover=\"this.style='background:#FFFF00; color:#000000;';\" onClick=\"getPage(this,parent,'/grid-admin?&target=grid-engine&page=parameterMappingFile&filename=" << hash << "');\" >\n";
      output << "<TD>"<< *it << "</TD>";
      output << "</TR>";

    }

    output << "</TABLE>\n";
    output << "<HR>\n";
    output << "</BODY>\n";
    output << "</HTML>\n";


    theResponse.setContent(output.str());
    theResponse.setHeader("Content-Type", "text/html; charset=UTF-8");

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}




bool Browser::page_producerFile(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse)
{
  try
  {
    std::ostringstream output;

    output << "<HTML>\n";
    output << "<BODY style=\"font-size:12;\">\n";

    output << "<HR>\n";
    output << "<A href=\"grid-admin?target=grid-admin&page=start\">SmartMet Server</A> / ";
    output << "<A href=\"grid-admin?target=grid-admin&page=engines\">Engines</A> / ";
    output << "<A href=\"grid-admin?target=grid-engine&page=start\">Grid Engine</A> / ";
    output << "<A href=\"grid-admin?target=grid-engine&page=configuration\">Configuration</A> / ";
    output << "<HR>\n";
    output << "<H2>Producer search file</H2>\n";
    output << "<HR>\n";
    output << "<H3>Introduction</H3>\n";
    output << "<P>The producer search file has two purposes : </P>\n";
    output << "<OL>\n";
    output << "  <LI>\n";
    output << "    <B>Defining producers and geometries that can be searched</B>\n";
    output << "      <P>If the requested producer and geometry are not in the list, then the query returns nothing.</P>";
    output << "  </LI>\n";
    output << "  <LI>\n";
    output << "    <B>Defining the search order</B>\n";
    output << "      <P>The producer search file defines the search order in the case where no producers are given in ";
    output << "      the query. In this case, the first producer where the search location is inside the current geometry, is selected.</P>\n";
    output << "  </LI>\n";
    output << "</OL>\n";
    output << "<HR>\n";
    output << "<H3>File (" << mGridEngine->getProducerFileName() << ")</H3>\n";

    output << "<PRE style=\"background-color: #F0F0F0;\">\n";

    output << "\n";
    includeFile(output,mGridEngine->getProducerFileName().c_str());
    output << "\n";

    output << "</PRE>\n";
    output << "<HR>\n";

    output << "</BODY>\n";
    output << "</HTML>\n";

    theResponse.setContent(output.str());
    theResponse.setHeader("Content-Type", "text/html; charset=UTF-8");

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}





bool Browser::page_configurationFile(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse)
{
  try
  {
    std::ostringstream output;

    output << "<HTML>\n";
    output << "<BODY style=\"font-size:12;\">\n";

    output << "<HR>\n";
    output << "<A href=\"grid-admin?target=grid-admin&page=start\">SmartMet Server</A> / ";
    output << "<A href=\"grid-admin?target=grid-admin&page=engines\">Engines</A> / ";
    output << "<A href=\"grid-admin?target=grid-engine&page=start\">Grid Engine</A> / ";
    output << "<A href=\"grid-admin?target=grid-engine&page=configuration\">Configuration</A> / ";
    output << "<HR>\n";
    output << "<H2>Configuration file (" << mGridEngine->getConfigurationFileName() << ")</H2>\n";
    output << "<HR>\n";

    output << "<PRE style=\"background-color: #F0F0F0;\">\n";

    output << "\n";
    includeFile(output,mGridEngine->getConfigurationFileName().c_str());
    output << "\n";

    output << "</pre>\n";
    output << "<HR>\n";

    output << "</BODY>\n";
    output << "</HTML>\n";

    theResponse.setContent(output.str());
    theResponse.setHeader("Content-Type", "text/html; charset=UTF-8");

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}





bool Browser::page_contentServer_processingLog(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse)
{
  try
  {
    std::ostringstream output;

    Log *log = nullptr;
    std::string filename;

    auto cs = mGridEngine->getContentServer_sptr();
    if (cs)
    {
      log = cs->getProcessingLog();
      if (log)
        filename = log->getFileName();
    }

    uint mode = 0;
    auto modeStr = theRequest.getParameter("mode");
    if (modeStr)
      mode = atoi(modeStr->c_str());

    output << "<HTML>\n";
    output << "<SCRIPT>\n";
    output << "function getPage(obj,frm,url)\n";
    output << "{\n";
    output << "  frm.location.href=url;\n";
    output << "}\n";
    output << "</SCRIPT>\n";
    output << "<BODY style=\"font-size:12;\">\n";

    output << "<HR>\n";
    output << "<A href=\"grid-admin?target=grid-admin&page=start\">SmartMet Server</A> / ";
    output << "<A href=\"grid-admin?target=grid-admin&page=engines\">Engines</A> / ";
    output << "<A href=\"grid-admin?target=grid-engine&page=start\">Grid Engine</A> / ";
    output << "<A href=\"grid-admin?target=grid-engine&page=contentServer\">Content Server</A> / ";
    output << "<HR>\n";
    output << "<H2>Content Server: Processing log</H2>\n";

    if ((mFlags & Flags::logModificationEnabled)  && log && (session.mUserInfo.getUserId() == 0 || session.mUserInfo.isUserGroupMember("grid-admin")))
    {
      switch (mode)
      {
        case MODE_LOG_DISABLE:
          log->disable();
          break;

        case MODE_LOG_ENABLE:
          log->enable();
          break;

        case MODE_LOG_CLEAR:
          log->clear();
          break;
      }

      std::string bg = "#C0C0C0";
      std::string p1 = "<TD width=\"150\" height=\"25\" align=\"center\" style=\"background:" + bg + ";\" onmouseout=\"this.style='background:" + bg + ";'\" onmouseover=\"this.style='background:#0000FF; color:#FFFFFF';\" onClick=\"getPage(this,parent,'/grid-admin?mode=";

      output << "<HR>\n";
      output << "<TABLE style=\"font-size:12;\">\n";
      output << "<TR>\n";
      if (log->isEnabled())
      {
        output << p1 << MODE_NONE << "');\" >Refresh</TD>\n";
        output << p1 << MODE_LOG_DISABLE << "');\" >Disable</TD>\n";
      }
      else
        output << p1 << MODE_LOG_ENABLE << "');\" >Enable</TD>\n";

      output << p1 << MODE_LOG_CLEAR << "');\" >Clear</TD>\n";
      output << "</TR>\n";
      output << "</TABLE>\n";
    }


    output << "<HR>\n";
    output << "<H3>File (" << filename << ")</H3>\n";

    output << "<PRE style=\"background-color: #F0F0F0;\">\n";

    output << "\n";

    if (filename > " ")
    {
      std::vector<std::string> lines;
      try
      {
        readEofLines(filename.c_str(),100,lines);
      }
      catch (...)
      {
      }
      for (auto it=lines.begin(); it!=lines.end();++it)
        output << *it << "\n";

      //includeFile(output,filename.c_str());
    }

    output << "\n";

    output << "</PRE>\n";
    output << "<HR>\n";

    output << "</BODY>\n";
    output << "</HTML>\n";

    theResponse.setContent(output.str());
    theResponse.setHeader("Content-Type", "text/html; charset=UTF-8");

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}





bool Browser::page_contentServer_debugLog(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse)
{
  try
  {
    std::ostringstream output;

    Log *log = nullptr;
    std::string filename;

    auto cs = mGridEngine->getContentServer_sptr();
    if (cs)
    {
      log = cs->getDebugLog();
      if (log)
        filename = log->getFileName();
    }

    uint mode = 0;
    auto modeStr = theRequest.getParameter("mode");
    if (modeStr)
      mode = atoi(modeStr->c_str());

    output << "<HTML>\n";
    output << "<SCRIPT>\n";
    output << "function getPage(obj,frm,url)\n";
    output << "{\n";
    output << "  frm.location.href=url;\n";
    output << "}\n";
    output << "</SCRIPT>\n";
    output << "<BODY style=\"font-size:12;\">\n";

    output << "<HR>\n";
    output << "<A href=\"grid-admin?target=grid-admin&page=start\">SmartMet Server</A> / ";
    output << "<A href=\"grid-admin?target=grid-admin&page=engines\">Engines</A> / ";
    output << "<A href=\"grid-admin?target=grid-engine&page=start\">Grid Engine</A> / ";
    output << "<A href=\"grid-admin?target=grid-engine&page=contentServer\">Content Server</A> / ";
    output << "<HR>\n";
    output << "<H2>Content Server: Debug log</H2>\n";

    if ((mFlags & Flags::logModificationEnabled)  && log && (session.mUserInfo.getUserId() == 0 || session.mUserInfo.isUserGroupMember("grid-admin")))
    {
      switch (mode)
      {
        case MODE_LOG_DISABLE:
          log->disable();
          break;

        case MODE_LOG_ENABLE:
          log->enable();
          break;

        case MODE_LOG_CLEAR:
          log->clear();
          break;
      }

      std::string bg = "#C0C0C0";
      std::string p1 = "<TD width=\"150\" height=\"25\" align=\"center\" style=\"background:" + bg + ";\" onmouseout=\"this.style='background:" + bg + ";'\" onmouseover=\"this.style='background:#0000FF; color:#FFFFFF';\" onClick=\"getPage(this,parent,'/grid-admin?mode=";

      output << "<HR>\n";
      output << "<TABLE style=\"font-size:12;\">\n";
      output << "<TR>\n";
      if (log->isEnabled())
      {
        output << p1 << MODE_NONE << "');\" >Refresh</TD>\n";
        output << p1 << MODE_LOG_DISABLE << "');\" >Disable</TD>\n";
      }
      else
        output << p1 << MODE_LOG_ENABLE << "');\" >Enable</TD>\n";

      output << p1 << MODE_LOG_CLEAR << "');\" >Clear</TD>\n";
      output << "</TR>\n";
      output << "</TABLE>\n";
    }


    output << "<HR>\n";
    output << "<H3>File (" << filename << ")</H3>\n";

    output << "<PRE style=\"background-color: #F0F0F0;\">\n";

    output << "\n";

    if (filename > " ")
    {
      std::vector<std::string> lines;
      try
      {
        readEofLines(filename.c_str(),10000,lines);
      }
      catch (...)
      {
      }
      for (auto it=lines.rbegin(); it!=lines.rend();++it)
        output << *it << "\n";

      //includeFile(output,filename.c_str());
    }

    output << "\n";

    output << "</PRE>\n";
    output << "<HR>\n";

    output << "</BODY>\n";
    output << "</HTML>\n";

    theResponse.setContent(output.str());
    theResponse.setHeader("Content-Type", "text/html; charset=UTF-8");

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}





bool Browser::page_dataServer_processingLog(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse)
{
  try
  {
    std::ostringstream output;

    Log *log = nullptr;
    std::string filename;

    auto cs = mGridEngine->getDataServer_sptr();
    if (cs)
    {
      log = cs->getProcessingLog();
      if (log)
        filename = log->getFileName();
    }

    uint mode = 0;
    auto modeStr = theRequest.getParameter("mode");
    if (modeStr)
      mode = atoi(modeStr->c_str());

    output << "<HTML>\n";
    output << "<SCRIPT>\n";
    output << "function getPage(obj,frm,url)\n";
    output << "{\n";
    output << "  frm.location.href=url;\n";
    output << "}\n";
    output << "</SCRIPT>\n";
    output << "<BODY style=\"font-size:12;\">\n";

    output << "<HR>\n";
    output << "<A href=\"grid-admin?target=grid-admin&page=start\">SmartMet Server</A> / ";
    output << "<A href=\"grid-admin?target=grid-admin&page=engines\">Engines</A> / ";
    output << "<A href=\"grid-admin?target=grid-engine&page=start\">Grid Engine</A> / ";
    output << "<A href=\"grid-admin?target=grid-engine&page=dataServer\">Data Server</A> / ";
    output << "<HR>\n";
    output << "<H2>Data Server: Processing log</H2>\n";

    if ((mFlags & Flags::logModificationEnabled)  && log && (session.mUserInfo.getUserId() == 0 || session.mUserInfo.isUserGroupMember("grid-admin")))
    {
      switch (mode)
      {
        case MODE_LOG_DISABLE:
          log->disable();
          break;

        case MODE_LOG_ENABLE:
          log->enable();
          break;

        case MODE_LOG_CLEAR:
          log->clear();
          break;
      }

      std::string bg = "#C0C0C0";
      std::string p1 = "<TD width=\"150\" height=\"25\" align=\"center\" style=\"background:" + bg + ";\" onmouseout=\"this.style='background:" + bg + ";'\" onmouseover=\"this.style='background:#0000FF; color:#FFFFFF';\" onClick=\"getPage(this,parent,'/grid-admin?mode=";

      output << "<HR>\n";
      output << "<TABLE style=\"font-size:12;\">\n";
      output << "<TR>\n";
      if (log->isEnabled())
      {
        output << p1 << MODE_NONE << "');\" >Refresh</TD>\n";
        output << p1 << MODE_LOG_DISABLE << "');\" >Disable</TD>\n";
      }
      else
        output << p1 << MODE_LOG_ENABLE << "');\" >Enable</TD>\n";

      output << p1 << MODE_LOG_CLEAR << "');\" >Clear</TD>\n";
      output << "</TR>\n";
      output << "</TABLE>\n";
    }


    output << "<HR>\n";
    output << "<H3>File (" << filename << ")</H3>\n";

    output << "<PRE style=\"background-color: #F0F0F0;\">\n";

    output << "\n";

    if (filename > " ")
    {
      std::vector<std::string> lines;
      try
      {
        readEofLines(filename.c_str(),100,lines);
      }
      catch (...)
      {
      }
      for (auto it=lines.begin(); it!=lines.end();++it)
        output << *it << "\n";

      //includeFile(output,filename.c_str());
    }

    output << "\n";

    output << "</PRE>\n";
    output << "<HR>\n";

    output << "</BODY>\n";
    output << "</HTML>\n";

    theResponse.setContent(output.str());
    theResponse.setHeader("Content-Type", "text/html; charset=UTF-8");

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}





bool Browser::page_dataServer_debugLog(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse)
{
  try
  {
    std::ostringstream output;

    Log *log = nullptr;
    std::string filename;

    auto cs = mGridEngine->getDataServer_sptr();
    if (cs)
    {
      log = cs->getDebugLog();
      if (log)
        filename = log->getFileName();
    }

    uint mode = 0;
    auto modeStr = theRequest.getParameter("mode");
    if (modeStr)
      mode = atoi(modeStr->c_str());

    output << "<HTML>\n";
    output << "<SCRIPT>\n";
    output << "function getPage(obj,frm,url)\n";
    output << "{\n";
    output << "  frm.location.href=url;\n";
    output << "}\n";
    output << "</SCRIPT>\n";
    output << "<BODY style=\"font-size:12;\">\n";

    output << "<HR>\n";
    output << "<A href=\"grid-admin?target=grid-admin&page=start\">SmartMet Server</A> / ";
    output << "<A href=\"grid-admin?target=grid-admin&page=engines\">Engines</A> / ";
    output << "<A href=\"grid-admin?target=grid-engine&page=start\">Grid Engine</A> / ";
    output << "<A href=\"grid-admin?target=grid-engine&page=dataServer\">Data Server</A> / ";
    output << "<HR>\n";
    output << "<H2>Data Server: Debug log</H2>\n";

    if ((mFlags & Flags::logModificationEnabled)  && log && (session.mUserInfo.getUserId() == 0 || session.mUserInfo.isUserGroupMember("grid-admin")))
    {
      switch (mode)
      {
        case MODE_LOG_DISABLE:
          log->disable();
          break;

        case MODE_LOG_ENABLE:
          log->enable();
          break;

        case MODE_LOG_CLEAR:
          log->clear();
          break;
      }

      std::string bg = "#C0C0C0";
      std::string p1 = "<TD width=\"150\" height=\"25\" align=\"center\" style=\"background:" + bg + ";\" onmouseout=\"this.style='background:" + bg + ";'\" onmouseover=\"this.style='background:#0000FF; color:#FFFFFF';\" onClick=\"getPage(this,parent,'/grid-admin?mode=";

      output << "<HR>\n";
      output << "<TABLE style=\"font-size:12;\">\n";
      output << "<TR>\n";
      if (log->isEnabled())
      {
        output << p1 << MODE_NONE << "');\" >Refresh</TD>\n";
        output << p1 << MODE_LOG_DISABLE << "');\" >Disable</TD>\n";
      }
      else
        output << p1 << MODE_LOG_ENABLE << "');\" >Enable</TD>\n";

      output << p1 << MODE_LOG_CLEAR << "');\" >Clear</TD>\n";
      output << "</TR>\n";
      output << "</TABLE>\n";
    }


    output << "<HR>\n";
    output << "<H3>File (" << filename << ")</H3>\n";

    output << "<PRE style=\"background-color: #F0F0F0;\">\n";

    output << "\n";

    if (filename > " ")
    {
      std::vector<std::string> lines;
      try
      {
        readEofLines(filename.c_str(),10000,lines);
      }
      catch (...)
      {
      }
      for (auto it=lines.rbegin(); it!=lines.rend();++it)
        output << *it << "\n";

      //includeFile(output,filename.c_str());
    }

    output << "\n";

    output << "</PRE>\n";
    output << "<HR>\n";

    output << "</BODY>\n";
    output << "</HTML>\n";

    theResponse.setContent(output.str());
    theResponse.setHeader("Content-Type", "text/html; charset=UTF-8");

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}





bool Browser::page_queryServer_processingLog(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse)
{
  try
  {
    std::ostringstream output;

    Log *log = nullptr;
    std::string filename;

    auto cs = mGridEngine->getQueryServer_sptr();
    if (cs)
    {
      log = cs->getProcessingLog();
      if (log)
        filename = log->getFileName();
    }

    uint mode = 0;
    auto modeStr = theRequest.getParameter("mode");
    if (modeStr)
      mode = atoi(modeStr->c_str());

    output << "<HTML>\n";
    output << "<SCRIPT>\n";
    output << "function getPage(obj,frm,url)\n";
    output << "{\n";
    output << "  frm.location.href=url;\n";
    output << "}\n";
    output << "</SCRIPT>\n";
    output << "<BODY style=\"font-size:12;\">\n";

    output << "<HR>\n";
    output << "<A href=\"grid-admin?target=grid-admin&page=start\">SmartMet Server</A> / ";
    output << "<A href=\"grid-admin?target=grid-admin&page=engines\">Engines</A> / ";
    output << "<A href=\"grid-admin?target=grid-engine&page=start\">Grid Engine</A> / ";
    output << "<A href=\"grid-admin?target=grid-engine&page=queryServer\">Query Server</A> / ";
    output << "<HR>\n";
    output << "<H2>Query Server: Processing log</H2>\n";

    if ((mFlags & Flags::logModificationEnabled)  && log && (session.mUserInfo.getUserId() == 0 || session.mUserInfo.isUserGroupMember("grid-admin")))
    {
      switch (mode)
      {
        case MODE_LOG_DISABLE:
          log->disable();
          break;

        case MODE_LOG_ENABLE:
          log->enable();
          break;

        case MODE_LOG_CLEAR:
          log->clear();
          break;
      }

      std::string bg = "#C0C0C0";
      std::string p1 = "<TD width=\"150\" height=\"25\" align=\"center\" style=\"background:" + bg + ";\" onmouseout=\"this.style='background:" + bg + ";'\" onmouseover=\"this.style='background:#0000FF; color:#FFFFFF';\" onClick=\"getPage(this,parent,'/grid-admin?mode=";

      output << "<HR>\n";
      output << "<TABLE style=\"font-size:12;\">\n";
      output << "<TR>\n";
      if (log->isEnabled())
      {
        output << p1 << MODE_NONE << "');\" >Refresh</TD>\n";
        output << p1 << MODE_LOG_DISABLE << "');\" >Disable</TD>\n";
      }
      else
        output << p1 << MODE_LOG_ENABLE << "');\" >Enable</TD>\n";

      output << p1 << MODE_LOG_CLEAR << "');\" >Clear</TD>\n";
      output << "</TR>\n";
      output << "</TABLE>\n";
    }


    output << "<HR>\n";
    output << "<H3>File (" << filename << ")</H3>\n";

    output << "<PRE style=\"background-color: #F0F0F0;\">\n";

    output << "\n";

    if (filename > " ")
    {
      std::vector<std::string> lines;
      try
      {
        readEofLines(filename.c_str(),100,lines);
      }
      catch (...)
      {
      }
      for (auto it=lines.begin(); it!=lines.end();++it)
        output << *it << "\n";

      //includeFile(output,filename.c_str());
    }

    output << "\n";

    output << "</PRE>\n";
    output << "<HR>\n";

    output << "</BODY>\n";
    output << "</HTML>\n";

    theResponse.setContent(output.str());
    theResponse.setHeader("Content-Type", "text/html; charset=UTF-8");

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}





bool Browser::page_queryServer_debugLog(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse)
{
  try
  {
    std::ostringstream output;

    Log *log = nullptr;
    std::string filename;

    auto cs = mGridEngine->getQueryServer_sptr();
    if (cs)
    {
      log = cs->getDebugLog();
      if (log)
        filename = log->getFileName();
    }

    uint mode = 0;
    auto modeStr = theRequest.getParameter("mode");
    if (modeStr)
      mode = atoi(modeStr->c_str());

    output << "<HTML>\n";
    output << "<SCRIPT>\n";
    output << "function getPage(obj,frm,url)\n";
    output << "{\n";
    output << "  frm.location.href=url;\n";
    output << "}\n";
    output << "</SCRIPT>\n";
    output << "<BODY style=\"font-size:12;\">\n";

    output << "<HR>\n";
    output << "<A href=\"grid-admin?target=grid-admin&page=start\">SmartMet Server</A> / ";
    output << "<A href=\"grid-admin?target=grid-admin&page=engines\">Engines</A> / ";
    output << "<A href=\"grid-admin?target=grid-engine&page=start\">Grid Engine</A> / ";
    output << "<A href=\"grid-admin?target=grid-engine&page=queryServer\">Query Server</A> / ";
    output << "<HR>\n";
    output << "<H2>Query Server: Debug log</H2>\n";

    if ((mFlags & Flags::logModificationEnabled)  && log && (session.mUserInfo.getUserId() == 0 || session.mUserInfo.isUserGroupMember("grid-admin")))
    {
      switch (mode)
      {
        case MODE_LOG_DISABLE:
          log->disable();
          break;

        case MODE_LOG_ENABLE:
          log->enable();
          break;

        case MODE_LOG_CLEAR:
          log->clear();
          break;
      }

      std::string bg = "#C0C0C0";
      std::string p1 = "<TD width=\"150\" height=\"25\" align=\"center\" style=\"background:" + bg + ";\" onmouseout=\"this.style='background:" + bg + ";'\" onmouseover=\"this.style='background:#0000FF; color:#FFFFFF';\" onClick=\"getPage(this,parent,'/grid-admin?mode=";

      output << "<HR>\n";
      output << "<TABLE style=\"font-size:12;\">\n";
      output << "<TR>\n";
      if (log->isEnabled())
      {
        output << p1 << MODE_NONE << "');\" >Refresh</TD>\n";
        output << p1 << MODE_LOG_DISABLE << "');\" >Disable</TD>\n";
      }
      else
        output << p1 << MODE_LOG_ENABLE << "');\" >Enable</TD>\n";

      output << p1 << MODE_LOG_CLEAR << "');\" >Clear</TD>\n";
      output << "</TR>\n";
      output << "</TABLE>\n";
    }


    output << "<HR>\n";
    output << "<H3>File (" << filename << ")</H3>\n";

    output << "<PRE style=\"background-color: #F0F0F0;\">\n";

    output << "\n";

    if (filename > " ")
    {
      std::vector<std::string> lines;
      try
      {
        readEofLines(filename.c_str(),10000,lines);
      }
      catch (...)
      {
      }
      for (auto it=lines.rbegin(); it!=lines.rend();++it)
        output << *it << "\n";

      //includeFile(output,filename.c_str());
    }

    output << "\n";

    output << "</PRE>\n";
    output << "<HR>\n";

    output << "</BODY>\n";
    output << "</HTML>\n";

    theResponse.setContent(output.str());
    theResponse.setHeader("Content-Type", "text/html; charset=UTF-8");

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}





bool Browser::page_configuration(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse)
{
  try
  {
    std::ostringstream output;

    output << "<HTML>\n";
    output << "<BODY style=\"font-size:12;\">\n";
    output << "<HR>\n";
    output << "<A href=\"grid-admin?target=grid-admin&page=start\">SmartMet Server</A> / ";
    output << "<A href=\"grid-admin?target=grid-admin&page=engines\">Engines</A> / ";
    output << "<A href=\"grid-admin?target=grid-engine&page=start\">Grid Engine</A> / ";
    output << "<HR>\n";
    output << "<H2>Configuration</H2>\n";
    output << "<HR>\n";
    output << "            <OL>\n";
    output << "              <LI>";
    output << "                <A href=\"/grid-admin?&target=grid-engine&page=configurationFile\">Configuration file</A>";
    output << "              </LI>";
    output << "              <LI>";
    output << "                <A href=\"/grid-admin?&target=grid-engine&page=parameterMappingFiles\">Parameter mapping files</A>";
    output << "              </LI>";
    output << "              <LI>";
    output << "                <A href=\"/grid-admin?&target=grid-engine&page=producerMappingFiles\">Producer mapping files</A>";
    output << "              </LI>";
    output << "              <LI>";
    output << "                <A href=\"/grid-admin?&target=grid-engine&page=parameterAliasFiles\">Parameter alias files</A>";
    output << "              </LI>";
    output << "              <LI>";
    output << "                <A href=\"/grid-admin?&target=grid-engine&page=luaFiles\">LUA files</A>";
    output << "              </LI>";
    output << "              <LI>";
    output << "                <A href=\"/grid-admin?&target=grid-engine&page=producerFile\">Producer search file</A>";
    output << "              </LI>";
    output << "            </OL>\n";
    output << "<HR>\n";
    output << "</BODY>\n";
    output << "</HTML>\n";


    theResponse.setContent(output.str());
    theResponse.setHeader("Content-Type", "text/html; charset=UTF-8");

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}




bool Browser::page_stateInformation(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse)
{
  try
  {

    std::shared_ptr<T::AttributeNode> gridEngineAttributes(new T::AttributeNode("Grid-Engine"));
    mGridEngine->getStateAttributes(gridEngineAttributes);

    std::ostringstream output;

    output << "<HTML>\n";
    output << "<BODY style=\"font-size:12;\">\n";
    output << "<HR>\n";
    output << "<A href=\"grid-admin?target=grid-admin&page=start\">SmartMet Server</A> / ";
    output << "<A href=\"grid-admin?target=grid-admin&page=engines\">Engines</A> / ";
    output << "<A href=\"grid-admin?target=grid-engine&page=start\">Grid Engine</A> / ";
    output << "<HR>\n";
    output << "<H2>State Information</H2>\n";
    output << "<HR>\n";
    output << "<PRE>\n";

    gridEngineAttributes->print(output,0,0);

    output << "</PRE>\n";
    output << "<HR>\n";
    output << "</BODY>\n";
    output << "</HTML>\n";


    theResponse.setContent(output.str());
    theResponse.setHeader("Content-Type", "text/html; charset=UTF-8");

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}




bool Browser::page_start(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse)
{
  try
  {
    std::ostringstream output;

    output << "<HTML>\n";
    output << "<BODY style=\"font-size:12;\">\n";
    output << "<HR>\n";
    output << "<A href=\"grid-admin?target=grid-admin&page=start\">SmartMet Server</A> / ";
    output << "<A href=\"grid-admin?target=grid-admin&page=engines\">Engines</A> / ";
    output << "<HR>\n";
    output << "<H2>Grid Engine</H2>\n";
    output << "<HR>\n";
    browserContent(session,output);
    output << "<HR>\n";
    output << "</BODY>\n";
    output << "</HTML>\n";


    theResponse.setContent(output.str());
    theResponse.setHeader("Content-Type", "text/html; charset=UTF-8");

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}





void Browser::browserContent(SessionManagement::SessionInfo& session,std::ostringstream& output)
{
  try
  {
    output << "        <OL>\n";
    output << "          <LI>";
    output << "            <H4><A href=\"/grid-admin?&target=grid-engine&page=configuration\">Configuration</A></H4>";
    output << "            <OL>\n";
    output << "              <LI>";
    output << "                <A href=\"/grid-admin?&target=grid-engine&page=configurationFile\">Configuration file</A>";
    output << "              </LI>";
    output << "              <LI>";
    output << "                <A href=\"/grid-admin?&target=grid-engine&page=parameterMappingFiles\">Parameter mapping files</A>";
    output << "              </LI>";
    output << "              <LI>";
    output << "                <A href=\"/grid-admin?&target=grid-engine&page=producerMappingFiles\">Producer mapping files</A>";
    output << "              </LI>";
    output << "              <LI>";
    output << "                <A href=\"/grid-admin?&target=grid-engine&page=parameterAliasFiles\">Parameter alias files</A>";
    output << "              </LI>";
    output << "              <LI>";
    output << "                <A href=\"/grid-admin?&target=grid-engine&page=luaFiles\">LUA files</A>";
    output << "              </LI>";
    output << "              <LI>";
    output << "                <A href=\"/grid-admin?&target=grid-engine&page=producerFile\">Producer search file</A>";
    output << "              </LI>";
    output << "            </OL>\n";
    output << "          </LI>";
    output << "          <LI>";
    output << "            <H4><A href=\"/grid-admin?&target=grid-engine&page=stateInformation\">StateInformation</A></H4>";
    output << "          </LI>";
    output << "          <LI>";
    output << "            <H4><A href=\"/grid-admin?&target=grid-engine&page=contentServer\">Content Server</A></H4>";
    output << "            <OL>\n";
    output << "              <LI>";
    output << "                <H4><A href=\"/grid-admin?&target=grid-engine&page=contentInformation\">Content Information</A></H4>";
    output << "                <OL>\n";
    output << "                  <LI>";
    output << "                    <A href=\"/grid-admin?&target=grid-engine&page=producers&source=main\">Main content information (Redis)</A>";
    output << "                  </LI>";
    output << "                  <LI>";
    output << "                    <A href=\"/grid-admin?&target=grid-engine&page=producers&source=cache\">Cached content information (Grid Engine)</A>";
    output << "                  </LI>";
    output << "                </OL>\n";
    output << "              </LI>";
    output << "              <LI>";
    output << "                <H4>Logs</H4>";
    output << "                <OL>\n";
    output << "                  <LI>";
    output << "                    <A href=\"/grid-admin?&target=grid-engine&page=contentServer_processingLog\">Processing log</A>";
    output << "                  </LI>";
    output << "                  <LI>";
    output << "                    <A href=\"/grid-admin?&target=grid-engine&page=contentServer_debugLog\">Debug log</A>";
    output << "                  </LI>";
    output << "                </OL>\n";
    output << "              </LI>";
    output << "            </OL>\n";
    output << "          </LI>";
    output << "          <LI>";
    output << "            <H4><A href=\"/grid-admin?&target=grid-engine&page=dataServer\">Data Server</A></H4>";
    output << "            <OL>\n";
    output << "              <LI>";
    output << "                <H4>Logs</H4>";
    output << "                <OL>\n";
    output << "                  <LI>";
    output << "                    <A href=\"/grid-admin?&target=grid-engine&page=dataServer_processingLog\">Processing log</A>";
    output << "                  </LI>";
    output << "                  <LI>";
    output << "                    <A href=\"/grid-admin?&target=grid-engine&page=dataServer_debugLog\">Debug log</A>";
    output << "                  </LI>";
    output << "                 </OL>\n";
    output << "              </LI>";
    output << "            </OL>\n";
    output << "          </LI>";
    output << "          <LI>";
    output << "            <H4><A href=\"/grid-admin?&target=grid-engine&page=queryServer\">Query Server</A></H4>";
    output << "            <OL>\n";
    output << "              <LI>";
    output << "                <H4>Logs</H4>";
    output << "                <OL>\n";
    output << "                  <LI>";
    output << "                    <A href=\"/grid-admin?&target=grid-engine&page=queryServer_processingLog\">Processing log</A>";
    output << "                  </LI>";
    output << "                  <LI>";
    output << "                    <A href=\"/grid-admin?&target=grid-engine&page=queryServer_debugLog\">Debug log</A>";
    output << "                  </LI>";
    output << "                </OL>\n";
    output << "              </LI>";
    output << "            </OL>\n";
    output << "          </LI>";
    output << "        </OL>\n";
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}





bool Browser::requestHandler(SessionManagement::SessionInfo& session,const Spine::HTTP::Request& theRequest,Spine::HTTP::Response& theResponse)
{
  try
  {
    updateSessionParameters(session,theRequest,theResponse);

    std::string page = "start";
    session.getAttribute("grid-admin","page",page);


    if (page == "start")
      return page_start(session,theRequest,theResponse);

    if (page == "configuration")
      return page_configuration(session,theRequest,theResponse);

    if (page == "configurationFile")
      return page_configurationFile(session,theRequest,theResponse);

    if (page == "producerFile")
      return page_producerFile(session,theRequest,theResponse);

    if (page == "parameterMappingFiles")
      return page_parameterMappingFiles(session,theRequest,theResponse);

    if (page == "parameterMappingFile")
      return page_parameterMappingFile(session,theRequest,theResponse);

    if (page == "parameterAliasFiles")
      return page_parameterAliasFiles(session,theRequest,theResponse);

    if (page == "parameterAliasFile")
      return page_parameterAliasFile(session,theRequest,theResponse);

    if (page == "producerMappingFiles")
      return page_producerMappingFiles(session,theRequest,theResponse);

    if (page == "producerMappingFile")
      return page_producerMappingFile(session,theRequest,theResponse);

    if (page == "luaFiles")
      return page_luaFiles(session,theRequest,theResponse);

    if (page == "luaFile")
      return page_luaFile(session,theRequest,theResponse);

    if (page == "contentServer")
      return page_contentServer(session,theRequest,theResponse);

    if (page == "contentInformation")
      return page_contentInformation(session,theRequest,theResponse);

    if (page == "producers")
      return page_producers(session,theRequest,theResponse);

    if (page == "generations")
      return page_generations(session,theRequest,theResponse);

    if (page == "files")
      return page_files(session,theRequest,theResponse);

    if (page == "stateInformation")
      return page_stateInformation(session,theRequest,theResponse);

    if (page == "contentList")
      return page_contentList(session,theRequest,theResponse);

    if (page == "contentServer_processingLog")
      return page_contentServer_processingLog(session,theRequest,theResponse);

    if (page == "contentServer_debugLog")
      return page_contentServer_debugLog(session,theRequest,theResponse);

    if (page == "dataServer")
      return page_dataServer(session,theRequest,theResponse);

    if (page == "dataServer_processingLog")
      return page_dataServer_processingLog(session,theRequest,theResponse);

    if (page == "dataServer_debugLog")
      return page_dataServer_debugLog(session,theRequest,theResponse);

    if (page == "queryServer")
      return page_queryServer(session,theRequest,theResponse);

    if (page == "queryServer_processingLog")
      return page_queryServer_processingLog(session,theRequest,theResponse);

    if (page == "queryServer_debugLog")
      return page_queryServer_debugLog(session,theRequest,theResponse);

    std::ostringstream output;

    output << "<HTML>\n";
    output << "<BODY style=\"font-size:12;\">\n";

    output << "Unknown page : (" << page << ")\n";
    output << "</BODY>\n";
    output << "</HTML>\n";


    theResponse.setContent(output.str());
    theResponse.setHeader("Content-Type", "text/html; charset=UTF-8");

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}



}  // namespace Grid
}  // namespace Engine
}  // namespace SmartMet

// ======================================================================
