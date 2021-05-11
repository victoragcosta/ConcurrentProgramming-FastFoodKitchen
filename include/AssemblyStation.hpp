#ifndef ASSEMBLY_STATION_HPP_
#define ASSEMBLY_STATION_HPP_

#include "StatusDisplayer.hpp"

#include <pthread.h>
namespace AssemblyStation
{
  extern const int assemblingTime;
  extern const int burgersPerBatch;

  extern pthread_mutex_t mutex;
  extern int burgers;

  // Configure assembly stations for the simulation
  void initAssemblyStations(int nAssemblyStations);

  // Attempt to assemble burgers. If possible, it assembles taking assemblingTime
  // to do so and returns true. If not, it just returns false.
  bool makeBurgers();

  // Attempt to take completed burgers from the station. If possible, it subtracts
  // the amount and returns true. If not, it just returns false.
  bool getBurgers(int n);
}

#endif /* ASSEMBLY_STATION_HPP_ */
