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

    uint          parameterId;
    std::string   parameterName;
    std::string   parameterUnits;
    std::string   parameterDescription;
};


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


  uint                    producerId;
  std::string             producerName;
  std::string             producerDescription;
  uint                    generationId;
  std::string             analysisTime;     // All times are UTC time string (format "YYYYMMDDTHHMISS")

  int                     geometryId;
  int                     projectionId;
  std::string             projectionName;
  std::string             wkt;
  std::string             proj4;
  T::Coordinate           latlon_topLeft;           // Latlon koordinates
  T::Coordinate           latlon_topRight;
  T::Coordinate           latlon_bottomLeft;
  T::Coordinate           latlon_bottomRight;
  int                     xNumber;           // Horizontal grid points
  int                     yNumber;           // Vertical grid points

  T::ParamLevelId         levelId;
  std::string             levelName;
  std::string             levelDescription;
  std::set<T::ParamLevel> levels;     // Level values

  std::set<std::string>   times;
  std::vector<Parameter>  parameters;
};

}  // namespace Grid
}  // namespace Engine
}  // namespace SmartMet
