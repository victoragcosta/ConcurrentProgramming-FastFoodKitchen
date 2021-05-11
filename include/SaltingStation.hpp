#ifndef SALTING_STATION_HPP_
#define SALTING_STATION_HPP_

#include <pthread.h>
#include <vector>

namespace SaltingStation
{

  extern const int saltingTime, friesPerPortion;
  extern pthread_mutex_t mutex;
  extern int fries;

  // Setups some parameters for the salting station, preparing for the simulation.
  void initSaltingStations(int nSalters);

  // Attempts to salt fries and create portions. If possible, waits saltingTime
  // before adding and then returns true. If not possible, returns false.
  bool saltFries();

  // Attempts to get fries. If possible, subtracts n and returns true. If not
  // possible, returns false.
  bool getPortions(int n);
}

#endif /* SALTING_STATION_HPP_ */
