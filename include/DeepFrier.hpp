#ifndef DEEP_FRIER_HPP_
#define DEEP_FRIER_HPP_

#include <pthread.h>
#include <vector>

  namespace DeepFriers
  {
    std::vector<pthread_t> initDeepFriers(int nDeepFriers);
    extern const int setupTime, fryTime, friesPerBatch;

    void *DeepFrier(void *args);

    bool setupDeepFrier();

    bool getUnsaltedFries(int n);
  }

#endif /* DEEP_FRIER_HPP_ */
