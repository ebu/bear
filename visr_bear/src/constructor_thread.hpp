#pragma once
#include <condition_variable>
#include <exception>
#include <mutex>
#include <thread>

#include "bear/api.hpp"

namespace bear {

// design notes:
// - we need to notify the thread, and therefore have to use a CV (can't wait on atomics)
// - CV requires a mutex, so we might as well protect the rest of the state with it
// - std::future<T> doesn't allow us to check if it's been resolved yet, only to
//   wait
// - mut remains locked while constructing -- we can't do anything useful in
//   this state anyway, and it simplifies the thread code as next_config does not
//   need to be copied

/// Utility to construct Renderer instances on a background thread without
/// waiting.
class ConstructorThread {
 public:
  ConstructorThread();

  ~ConstructorThread();

  /// Start the construction of a Renderer from the given Config on the thread.
  ///
  /// This does not block -- it will return true if it was successful, and you
  /// should try again later if it returns false.
  bool start(const Config &config);
  bool start(const ConfigImpl &config);

  /// Get the result of a previous call to start.
  ///
  /// This does not block -- it returns boost::none if construction is
  /// currently in progress, or if start has not yet been called
  /// since the last result was retrieved.
  boost::optional<Renderer> get_result();

  /// Get an exception possibly thrown when constructing the renderer,
  /// corresponding to the last successful call to start (null otherwise).
  ///
  /// This will clear the error, so it will only be returned once.
  std::exception_ptr get_construction_error();

  /// Swap the contents of to_swap with the result of the previous call to
  /// start. This is useful because it allows you to get rid of the old
  /// renderer (which will be destructed on a background thread) and accept a
  /// new one atomically.
  ///
  /// This does not block -- if returns false and has no effect on to_swap if
  /// there's no result, or there's nowhere to store the old contents of
  /// to_swap.
  bool try_swap_result(boost::optional<Renderer> &to_swap);

 private:
  /// function to be ran in thread
  void thread_fn();

  std::mutex mut;  // everything below is protected by this
  std::condition_variable cv;

  bool next_config_set = false;
  Config next_config;

  bool result_set = false;
  Renderer result;
  std::exception_ptr construction_error;

  bool to_destroy_set = false;
  Renderer to_destroy;

  bool should_exit = false;

  std::thread thread;
};

}  // namespace bear
