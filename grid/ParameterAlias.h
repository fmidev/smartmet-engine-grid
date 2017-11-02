#pragma once

#include <grid-files/grid/Typedefs.h>


namespace SmartMet
{
namespace Engine
{
namespace Grid
{


class ParameterAlias
{
  public:
                   ParameterAlias();
                   ParameterAlias(const ParameterAlias& alias);
    virtual        ~ParameterAlias();

    void           print(std::ostream& stream,uint level,uint optionFlags);

    std::string    mName;   // Alias name (must be unique)
    std::string    mTitle;
    std::string    mParameterString;
};

}  // namespace Grid
}  // namespace Engine
}  // namespace SmartMet
