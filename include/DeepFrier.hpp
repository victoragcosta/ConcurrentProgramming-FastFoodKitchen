#ifndef DEEP_FRIER_HPP_
#define DEEP_FRIER_HPP_

#include <pthread.h>
#include <vector>

  namespace DeepFriers
  {
    extern const int setupTime, fryTime, friesPerBatch;

    extern pthread_cond_t condition;

    // Initializes DeepFrier threads preparing for the simulation. Returns a
    // vector with all threads created.
    std::vector<pthread_t> initDeepFriers(int nDeepFriers);

    // A thread to simulate the independent behavior of a deep frier.
    // Simply takes fryTime to add friesPerBatch to unsaltedFries.
    void *DeepFrier(void *args);

    // Attempts to start a DeepFrier. If possible, signal a DeepFrier to fry some
    // fries and return true after sleeping for setupTime. If not possible, returns false.
    bool setupDeepFrier();

    // Attempts to get n unsalted fries. If possible, subtracts n from unsaltedFries
    // and returns true. If not possible, returns false.
    bool getUnsaltedFries(int n);
  }

#endif /* DEEP_FRIER_HPP_ */
