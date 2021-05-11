#ifndef WORKER_HPP_
#define WORKER_HPP_

#include <vector>
#include <pthread.h>

namespace Worker
{
  // Initializes Worker threads and returns all threads created.
  std::vector<pthread_t> initWorkers(int nWorkers);

  // Signal all Worker threads that there are tasks available.
  void broadcastAvailableTasks();

  // Attempts to do tasks. If not possible, waits for a task to do.
  void *Worker(void *args);
}

#endif /* WORKER_HPP_ */
