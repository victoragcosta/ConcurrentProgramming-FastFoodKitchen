#ifndef FRIES_HPP_
#define FRIES_HPP_

#include <pthread.h>
#include <vector>

namespace Fries
{
  std::vector<pthread_t> initFries(int nDeepFriers, int nSalters);

  namespace DeepFriers
  {
    extern const int setupTime, fryTime, friesPerBatch;

    void *DeepFrier(void *args);

    bool setupDeepFrier();

    bool getUnsaltedFries(int n);
  }

  namespace Salting
  {
    extern const int saltingTime, friesPerPortion;

    bool saltFries();

    extern pthread_mutex_t mutex;
    extern int fries;

    bool getPortions(int n);
  }
}

#endif /* FRIES_HPP_ */
