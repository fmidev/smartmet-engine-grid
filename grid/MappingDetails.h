#pragma once

#include <grid-files/grid/Typedefs.h>
#include <grid-content/queryServer/definition/ParameterMapping.h>


namespace SmartMet
{
namespace Engine
{
namespace Grid
{

typedef std::map<std::string,std::set<std::string>> Times;

class MappingDetails
{
  public:
                                  MappingDetails();
                                  MappingDetails(const MappingDetails& mappingDetails);
    virtual                       ~MappingDetails();

    void                          print(std::ostream& stream,uint level,uint optionFlags);

    QueryServer::ParameterMapping mMapping;
    Times                         mTimes;
};



typedef std::vector<MappingDetails> MappingDetails_vec;


}  // namespace Grid
}  // namespace Engine
}  // namespace SmartMet
