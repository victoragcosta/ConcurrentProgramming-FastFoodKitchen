#ifndef FRIES_HPP_
#define FRIES_HPP_

#include <pthread.h>
#include <vector>

namespace Fries
{
  std::vector<pthread_t> initFries(int nDeepFriers, int nSalters);

  namespace DeepFriers
  {
    void* DeepFrier(void* args);

    bool setupDeepFrier();

    bool getUnsaltedFries(int n);
  }

  namespace Salting
  {
    bool saltFries();

    bool getPortions(int n);
  }
}

#endif /* FRIES_HPP_ */
