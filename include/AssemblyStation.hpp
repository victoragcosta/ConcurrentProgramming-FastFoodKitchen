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

  void initAssemblyStations(int nAssemblyStations);

  bool makeBurgers();

  bool getBurgers(int n);
}

#endif /* ASSEMBLY_STATION_HPP_ */
