#ifndef SALTING_STATION_HPP_
#define SALTING_STATION_HPP_

#include <pthread.h>
#include <vector>

namespace SaltingStation
{
  void initSaltingStations(int nSalters);

  extern const int saltingTime, friesPerPortion;

  bool saltFries();

  extern pthread_mutex_t mutex;
  extern int fries;

  bool getPortions(int n);
}

#endif /* SALTING_STATION_HPP_ */
