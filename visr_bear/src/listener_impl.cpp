#include "listener_impl.hpp"

#include <cmath>

namespace bear {

bool listeners_approx_equal(const ListenerImpl &a, const ListenerImpl &b)
{
  for (size_t i = 0; i < 3; i++)
    if (std::abs(a.position[i] - b.position[i]) > 1e-6) return false;

  for (size_t i = 0; i < 4; i++)
    if (std::abs(a.orientation.coeffs()[i] - b.orientation.coeffs()[i]) > 1e-6) return false;
  return true;
}

}  // namespace bear
