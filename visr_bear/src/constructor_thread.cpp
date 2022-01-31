#include "constructor_thread.hpp"

#include "config_impl.hpp"

namespace bear {

ConstructorThread::ConstructorThread() : thread(&ConstructorThread::thread_fn, this) {}

ConstructorThread::~ConstructorThread()
{
  {
    std::lock_guard<std::mutex> lk(mut);
    should_exit = true;
    cv.notify_one();
  }

  thread.join();
}

/// Start the construction of a Renderer from the given Config on the thread.
///
/// This does not block -- it will return true if it was successful, and you
/// should try again later if it returns false.
bool ConstructorThread::start(const ConfigImpl &config)
{
  std::unique_lock<std::mutex> lk(mut, std::try_to_lock);

  if (lk) {
    next_config.get_impl() = config;
    next_config_set = true;
    // ensure that a call to get_result or get_construction_error immediately
    // after doesn't retrieve an old result
    result_set = false;
    construction_error = nullptr;

    cv.notify_one();
    return true;
  } else
    return false;
}

bool ConstructorThread::start(const Config &config) { return start(config.get_impl()); }

/// Get the result of a previous call to start.
///
/// This does not block -- it returns boost::none if construction is
/// currently in progress, or if start has not yet been called
/// since the last result was retrieved.
boost::optional<Renderer> ConstructorThread::get_result()
{
  std::unique_lock<std::mutex> lk(mut, std::try_to_lock);

  if (lk && result_set) {
    result_set = false;
    return std::move(result);
  } else
    return boost::none;
}

bool ConstructorThread::try_swap_result(boost::optional<Renderer> &to_swap)
{
  std::unique_lock<std::mutex> lk(mut, std::try_to_lock);

  if (lk && result_set && (!to_swap || !to_destroy_set)) {
    if (to_swap) {
      to_destroy = std::move(*to_swap);
      to_destroy_set = true;
    }

    to_swap = std::move(result);
    result_set = false;

    cv.notify_one();
    return true;
  } else
    return false;
}

std::exception_ptr ConstructorThread::get_construction_error()
{
  std::unique_lock<std::mutex> lk(mut, std::try_to_lock);

  std::exception_ptr tmp = nullptr;
  if (lk) std::swap(tmp, construction_error);

  return tmp;
}

void ConstructorThread::thread_fn()
{
  while (true) {
    std::unique_lock<std::mutex> lk(mut);

    cv.wait(lk, [&]() { return next_config_set || to_destroy_set || should_exit; });

    if (should_exit) return;

    if (to_destroy_set) {
      to_destroy = Renderer();
      to_destroy_set = false;
    }

    if (next_config_set) {
      try {
        result = Renderer(next_config);
        result_set = true;
      } catch (std::exception &e) {
        construction_error = std::current_exception();
      }

      next_config_set = false;
    }
  }
}

}  // namespace bear
