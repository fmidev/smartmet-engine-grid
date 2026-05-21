#pragma once

#include <grid-files/identification/GridDef.h>
#include <grid-content/contentServer/definition/ProducerInfo.h>
#include <set>

namespace SmartMet
{
namespace Engine
{
namespace Grid
{


/*! \brief Minimal descriptor for one meteorological parameter. */
struct Parameter
{
  public:
    Parameter()
    {
      parameterId = 0;
    }

    Parameter(const Parameter &param) = default;
    Parameter &operator=(const Parameter &param) = default;

    void print(std::ostream& stream,uint level,uint optionFlags)
    {
      stream << space(level) << "Parameter\n";
      stream << space(level) << "- parameterId          = " << parameterId << "\n";
      stream << space(level) << "- parameterName        = " << parameterName << "\n";
      stream << space(level) << "- parameterUnits       = " << parameterUnits << "\n";
      stream << space(level) << "- parameterDescription = " << parameterDescription << "\n";
    }

    uint          parameterId;         //!< FMI parameter identifier.
    std::string   parameterName;       //!< Short parameter name.
    std::string   parameterUnits;      //!< Unit string (e.g. "K", "m/s").
    std::string   parameterDescription;//!< Human-readable description.
};


/*! \brief Full metadata record for one producer/generation/geometry/level slice as
 *  returned by Engine::getEngineMetadata().
 *
 *  Aggregates producer identity, grid geometry and projection, bounding box, level
 *  type, all available level values, all available forecast times, and the list of
 *  parameters that exist in this slice. */
struct MetaData
{
  MetaData()
  {
    geometryId = 0;
    levelId = 0;
    xNumber = 0;
    yNumber = 0;
    projectionId = 0;
  }

  MetaData(const MetaData &md) = default;
  MetaData &operator=(const MetaData &md) = default;

  void print(std::ostream& stream,uint level,uint optionFlags)
  {
    stream << space(level) << "MetaData\n";
    stream << space(level) << "- producerId          = " << producerId << "\n";
    stream << space(level) << "- producerName        = " << producerName << "\n";
    stream << space(level) << "- producerDescription = " << producerDescription << "\n";
    stream << space(level) << "- generationId        = " << generationId << "\n";
    stream << space(level) << "- analysisTime        = " << analysisTime << "\n";
    stream << space(level) << "- geometryId          = " << geometryId << "\n";
    stream << space(level) << "- projectionId        = " << projectionId << "\n";
    stream << space(level) << "- projectionName      = " << projectionName << "\n";
    stream << space(level) << "- wkt                 = " << wkt << "\n";
    stream << space(level) << "- proj4               = " << proj4 << "\n";
    stream << space(level) << "- xNumber             = " << xNumber << "\n";
    stream << space(level) << "- yNumber             = " << yNumber << "\n";
    stream << space(level) << "- latlon_topLeft      = " << latlon_topLeft.x() << "," << latlon_topLeft.y() << "\n";
    stream << space(level) << "- latlon_topRight     = " << latlon_topRight.x() << "," << latlon_topRight.y() << "\n";
    stream << space(level) << "- latlon_bottomLeft   = " << latlon_bottomLeft.x() << "," << latlon_bottomLeft.y() << "\n";
    stream << space(level) << "- latlon_bottomRight  = " << latlon_bottomRight.x() << "," << latlon_bottomRight.y() << "\n";
    stream << space(level) << "- levelId             = " << levelId << "\n";
    stream << space(level) << "- levelName           = " << levelName << "\n";
    stream << space(level) << "- levelDescription    = " << levelDescription << "\n";
    stream << space(level) << "- levels              = " ;

    for (auto a = levels.begin(); a != levels.end(); ++a)
      stream << *a << " ";

    stream << "\n";

    stream << space(level) << "- times               = \n";
    for (auto t = times.begin(); t != times.end(); ++t)
      stream << space(level+2) << "* " << *t << "\n";

    stream << space(level) << "- parameters          = \n";
    for (auto p = parameters.begin(); p != parameters.end(); ++p)
      p->print(stream,level+2,optionFlags);
  }


  T::ProducerId           producerId;           //!< FMI producer identifier.
  std::string             producerName;         //!< Producer short name.
  std::string             producerDescription;  //!< Producer human-readable description.
  T::GenerationId         generationId;         //!< Generation (model run) identifier.
  std::string             analysisTime;         //!< Analysis time in UTC (format "YYYYMMDDTHHMISS").

  T::GeometryId           geometryId;           //!< FMI grid geometry identifier.
  int                     projectionId;         //!< Projection type code.
  std::string             projectionName;       //!< Projection short name.
  std::string             wkt;                  //!< Well-known text projection string.
  std::string             proj4;                //!< Proj4 projection string.
  T::Coordinate           latlon_topLeft;       //!< Bounding box top-left corner in lat/lon.
  T::Coordinate           latlon_topRight;      //!< Bounding box top-right corner in lat/lon.
  T::Coordinate           latlon_bottomLeft;    //!< Bounding box bottom-left corner in lat/lon.
  T::Coordinate           latlon_bottomRight;   //!< Bounding box bottom-right corner in lat/lon.
  int                     xNumber;              //!< Number of grid columns.
  int                     yNumber;              //!< Number of grid rows.

  T::ParamLevelId         levelId;              //!< FMI level type identifier.
  std::string             levelName;            //!< Level type short name.
  std::string             levelDescription;     //!< Level type human-readable description.
  std::set<T::ParamLevel> levels;               //!< Available level values in this geometry slice.

  std::set<std::string>   times;                //!< Available forecast times (UTC "YYYYMMDDTHHMISS").
  std::vector<Parameter>  parameters;           //!< Parameters present in this geometry/level slice.
};

}  // namespace Grid
}  // namespace Engine
}  // namespace SmartMet
