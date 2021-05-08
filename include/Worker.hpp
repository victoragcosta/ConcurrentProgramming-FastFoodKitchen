#ifndef WORKER_HPP_
#define WORKER_HPP_

#include <vector>
#include <pthread.h>

namespace Worker
{
  std::vector<pthread_t> initWorkers(int nWorkers);

  void broadcastAvailableTasks();

  void *Worker(void *args);
}

#endif /* WORKER_HPP_ */
