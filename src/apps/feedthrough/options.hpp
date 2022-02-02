/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_APPS_FEEDTHROUGH_OPTIONS_HPP_INCLUDED
#define VISR_APPS_FEEDTHROUGH_OPTIONS_HPP_INCLUDED

#include <libapputilities/options.hpp>

namespace visr
{
namespace apps
{
namespace feedthrough
{

class Options: public apputilities::Options
{
public:
  Options();

  ~Options();
};

} // namespace feedthrough
} // namespace apps
} // namespace visr

#endif // #ifndef VISR_APPS_FEEDTHROUGH_OPTIONS_HPP_INCLUDED
