// Tests for the grid engine without Redis.
//
// The engine supports a "file" content source: an in-memory content server loaded from CSV files.
// The test registers smartmet-test-data GRIB files in such CSV files (one content record per
// message, as filesys2smartmet does), writes an engine configuration based on the
// smartmet-library-grid-files-test configuration, initializes the engine and checks its services
// against values read from the GRIB files directly.

#define BOOST_TEST_MODULE EngineTest
#include <boost/test/included/unit_test.hpp>

#include "TestCommon.h"
#include "grid/Engine.h"

#include <grid-content/contentServer/definition/ContentInfo.h>
#include <grid-content/contentServer/definition/FileInfo.h>
#include <grid-content/contentServer/definition/GenerationInfo.h>
#include <grid-content/contentServer/definition/ProducerInfo.h>
#include <grid-content/queryServer/definition/QueryConfigurator.h>
#include <grid-files/common/GeneralFunctions.h>
#include <grid-files/grid/GridFile.h>
#include <grid-files/grid/Message.h>
#include <grid-files/identification/GridDef.h>

#include <boost/algorithm/string/replace.hpp>
#include <filesystem>
#include <fstream>
#include <memory>
#include <set>
#include <sstream>

using namespace SmartMet;
using namespace GridTest;

namespace
{
const std::string PAL = testData("grib/pal/200808050729_pal_skandinavia_pinta.grib");
const std::string PRODUCER = "pal_skandinavia";
const double LON = 24.94;
const double LAT = 60.17;

using GridEngine = SmartMet::Engine::Grid::Engine;

class TestEngine : public GridEngine
{
 public:
  explicit TestEngine(const char *configFile) : GridEngine(configFile) {}
  using GridEngine::init;
  using GridEngine::shutdown;
};

void writeFile(const std::string &path, const std::string &contents)
{
  std::ofstream out(path);
  out << contents;
  BOOST_TEST_REQUIRE(out.good(), "cannot write " << path);
}

// The engine and its content directory, shared by all tests
struct World
{
  std::string dir;
  std::unique_ptr<TestEngine> engine;
  GRID::GridFile file;

  World()
  {
    if (!exists(CONFIG) || !exists(PAL))
      return;  // the tests fail (or are skipped) on the missing fixtures

    char tmpl[] = "/tmp/gridenginetest-XXXXXX";
    dir = mkdtemp(tmpl);

    Identification::gridDef.init(CONFIG);
    file.read(PAL);

    // ---- Content CSV files ----

    T::ProducerInfo producer;
    producer.mProducerId = 1;
    producer.mName = PRODUCER;
    producer.mTitle = PRODUCER;
    producer.mSourceId = 100;

    T::GenerationInfo generation;
    generation.mGenerationId = 1;
    generation.mProducerId = 1;
    generation.mName = PRODUCER + ":20080805T050000";
    generation.mAnalysisTime = "20080805T050000";
    generation.mStatus = T::GenerationInfo::Status::Ready;
    generation.mSourceId = 100;

    T::FileInfo fileInfo;
    fileInfo.mFileId = 1;
    fileInfo.mProducerId = 1;
    fileInfo.mGenerationId = 1;
    fileInfo.mName = PAL;
    fileInfo.mFileType = file.getFileType();
    fileInfo.mServerType = T::FileInfo::ServerType::Filesys;
    fileInfo.mSourceId = 100;

    std::string content;
    std::set<std::string> mappings;
    for (uint i = 0; i < file.getNumberOfMessages(); i++)
    {
      GRID::Message *m = file.getMessageByIndex(i);
      T::ContentInfo c;
      c.mFileId = 1;
      c.mMessageIndex = i;
      c.mProducerId = 1;
      c.mGenerationId = 1;
      c.mFileType = m->getMessageType();
      c.mFilePosition = m->getFilePosition();
      c.mMessageSize = m->getMessageSize();
      c.setForecastTime(m->getForecastTime());
      c.mFmiParameterId = m->getFmiParameterId();
      c.setFmiParameterName(m->getFmiParameterName());
      c.mFmiParameterLevelId = m->getFmiParameterLevelId();
      c.mParameterLevel = m->getGridParameterLevel();
      c.mForecastType = m->getForecastType();
      c.mForecastNumber = m->getForecastNumber();
      c.mGeometryId = m->getGridGeometryId();
      c.mSourceId = 100;
      content += c.getCsv() + "\n";
      mappings.insert(PRODUCER + ";" + m->getFmiParameterName() + ";2;" + m->getFmiParameterName() + ";" +
                      std::to_string(c.mGeometryId) + ";1;" + std::to_string(c.mFmiParameterLevelId) +
                      ";" + std::to_string(c.mParameterLevel) + ";1;1;1;0;E;;;1;");
    }

    writeFile(dir + "/producers.csv", producer.getCsv() + "\n");
    writeFile(dir + "/generations.csv", generation.getCsv() + "\n");
    writeFile(dir + "/geometries.csv", "");
    writeFile(dir + "/files.csv", fileInfo.getCsv() + "\n");
    writeFile(dir + "/content.csv", content);

    std::string mappingText;
    for (const auto &m : mappings)
      mappingText += m + "\n";
    writeFile(dir + "/mapping_generated.csv", mappingText);

    // ---- Engine configuration from the test configuration ----

    std::ifstream in(std::string(GRID_TEST_DIR) + "/engine/grid-engine.conf");
    std::stringstream buf;
    buf << in.rdbuf();
    std::string conf = buf.str();
    boost::replace_all(conf, "%(DIR)", std::string(GRID_TEST_DIR) + "/engine");
    boost::replace_all(conf, "type = \"redis\"", "type = \"file\"");
    boost::replace_all(conf, "contentDir          = \"$(HOME)/Data\"", "contentDir = \"" + dir + "\"");
    boost::replace_all(conf, "directory = \"/usr/share/smartmet/test/data\"", "directory = \"\"");
    boost::replace_all(conf,
                       "\"" + std::string(GRID_TEST_DIR) + "/engine/mapping_fmi_test.csv\",",
                       "\"" + dir + "/mapping_generated.csv\",\n    \"" + std::string(GRID_TEST_DIR) +
                           "/engine/mapping_fmi_test.csv\",");
    // No content cache: the file content source has no events
    boost::replace_all(conf, "enabled = true\n    requestForwardEnabled", "enabled = false\n    requestForwardEnabled");
    writeFile(dir + "/grid-engine.conf", conf);

    engine = std::make_unique<TestEngine>((dir + "/grid-engine.conf").c_str());
    engine->init();
  }

  ~World()
  {
    if (engine)
      engine->shutdown();
    engine.reset();
    if (!dir.empty())
      std::filesystem::remove_all(dir);
  }

  double direct(const std::string &param, const std::string &time, double lat, double lon)
  {
    for (uint i = 0; i < file.getNumberOfMessages(); i++)
    {
      GRID::Message *m = file.getMessageByIndex(i);
      if (m->getFmiParameterName() == param && m->getForecastTime() == time)
        return m->getGridValueByLatLonCoordinate(lat, lon, T::AreaInterpolationMethod::Linear);
    }
    return ParamValueMissing;
  }
};

World *world = nullptr;

struct GlobalFixture
{
  GlobalFixture() {}
  ~GlobalFixture()
  {
    delete world;
    world = nullptr;
  }
};

World &theWorld()
{
  if (world == nullptr)
    world = new World();
  BOOST_TEST_REQUIRE(world->engine.get() != nullptr, "the engine was not initialized");
  return *world;
}

bool close(double a, double b) { return std::fabs(a - b) <= 1e-3 * std::max(1.0, std::fabs(b)); }

}  // namespace

BOOST_TEST_GLOBAL_FIXTURE(GlobalFixture);

BOOST_AUTO_TEST_SUITE(engine, *fixtures({CONFIG, PAL}))

BOOST_AUTO_TEST_CASE(producers)
{
  requireFixture(CONFIG);
  requireFixture(PAL);
  withFmiErrors(
      []
      {
        auto &w = theWorld();
        string_vec producers;
        w.engine->getProducerList(producers);
        // The engine reports the producer names in upper case
        BOOST_TEST((std::find(producers.begin(), producers.end(), "PAL_SKANDINAVIA") != producers.end()));

        T::ProducerInfo info;
        BOOST_TEST(w.engine->getProducerInfoByName(PRODUCER, info));
        BOOST_TEST(info.mProducerId == 1U);
        BOOST_TEST(!w.engine->getProducerInfoByName("no_such_producer", info));
      });
}

BOOST_AUTO_TEST_CASE(parameter_mappings)
{
  withFmiErrors(
      []
      {
        auto &w = theWorld();
        QueryServer::ParameterMapping_vec mappings;
        w.engine->getParameterMappings(PRODUCER, "Temperature", false, mappings);
        BOOST_TEST_REQUIRE(!mappings.empty());
        BOOST_TEST(mappings[0].mProducerName == PRODUCER);
        BOOST_TEST(mappings[0].mGeometryId == 5900);

        QueryServer::ParameterMapping_vec none;
        w.engine->getParameterMappings(PRODUCER, "NoSuchParameter", false, none);
        BOOST_TEST(none.empty());

        std::set<T::ParamLevelId> levelIds;
        w.engine->getProducerParameterLevelIdList(PRODUCER, levelIds);
        BOOST_TEST(levelIds.count(1) == 1U);
      });
}

BOOST_AUTO_TEST_CASE(point_time_series)
{
  withFmiErrors(
      []
      {
        auto &w = theWorld();
        T::AttributeList attributes;
        attributes.addAttribute("starttime", "20080805T050000");
        attributes.addAttribute("endtime", "20080806T040000");
        attributes.addAttribute("timestep", "data");
        attributes.addAttribute("producer", PRODUCER);
        attributes.addAttribute("param", "Temperature,WindSpeedMS");
        attributes.addAttribute("areaInterpolationMethod", "1");

        QueryServer::Query query;
        QueryServer::QueryConfigurator configurator;
        configurator.configure(query, attributes);
        query.mCoordinateType = T::CoordinateTypeValue::LATLON_COORDINATES;
        T::Coordinate_vec point;
        point.emplace_back(LON, LAT);
        query.mAreaCoordinates.push_back(point);

        BOOST_TEST_REQUIRE(w.engine->executeQuery(query) == 0);
        BOOST_TEST_REQUIRE(query.mQueryParameterList.size() == 2U);
        for (const auto &param : query.mQueryParameterList)
        {
          BOOST_TEST_CONTEXT(param.mParam)
          {
            BOOST_TEST_REQUIRE(param.mValueList.size() == 24U);
            for (const auto &pv : param.mValueList)
            {
              T::GridValue gv;
              BOOST_TEST_REQUIRE(pv->mValueList.getLength() == 1U);
              pv->mValueList.getGridValueByIndex(0, gv);
              const std::string time = utcTimeFromTimeT(pv->mForecastTimeUTC);
              const double expected = w.direct(param.mParam, time, LAT, LON);
              BOOST_TEST(close(gv.mValue, expected), time << ": engine " << gv.mValue << ", grib " << expected);
            }
          }
        }
      });
}

BOOST_AUTO_TEST_CASE(content_tables)
{
  withFmiErrors(
      []
      {
        auto &w = theWorld();
        auto producers = w.engine->getProducerInfo(std::optional<std::string>(PRODUCER), "iso");
        BOOST_TEST(producers.get() != nullptr);
        auto generations = w.engine->getGenerationInfo(std::optional<std::string>(PRODUCER), "iso");
        BOOST_TEST(generations.get() != nullptr);
      });
}

BOOST_AUTO_TEST_SUITE_END()
