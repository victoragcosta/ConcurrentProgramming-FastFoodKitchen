#ifndef STATUS_DISPLAYER_HPP_
#define STATUS_DISPLAYER_HPP_

#include "Delivery.hpp"

#include <queue>
#include <pthread.h>

class StatusDisplayer
{
private:
  int nWorkers, nCustomers, nDeepFriers;
  int nDelivery, nSalters, nAssemblyStations, nGriddles;

  int assemblingTimeAssemblyStation, burgersPerBatchAssemblyStation;
  int griddlingTimeGriddle, burgersPerBatchGriddle;
  int setupTimeDeepFriers, fryTimeDeepFriers, friesPerBatchDeepFriers;
  int saltingTimeSalting, friesPerPortionSalting;
  int assemblingTimeDelivery;

  int burgers, workersAssemblyStation;
  int unsaltedFries, availableDeepFrier;
  int fries, workersSalting;
  int workersDelivery;
  Delivery::order_t ordered, firstOrder;
  int burgersMeat, workersGriddle;

  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

  void displayStatus();

public:
  StatusDisplayer(int nWorkers, int nCustomers, int nDelivery, int nDeepFriers, int nSalters, int nAssemblyStations, int nGriddles);
  // ~StatusDisplayer();

  void updateAssemblyStationBurgers(int n);
  void updateAssemblyStationWorkers(int n);
  void updateDeepFrierAvailable(int n);
  void updateDeepFrierUnsaltedFries(int n);
  void updateSaltingFries(int n);
  void updateSaltingWorkers(int n);
  void updateDeliveryOrdered(int fries, int burgers);
  void updateDeliveryFirstOrder(std::queue<Delivery::order_t> ordersQueue);
  void updateDeliveryWorkers(int n);
  void updateGriddleBurgersMeat(int n);
  void updateGriddleWorkers(int n);
};

extern StatusDisplayer *statusDisplayer;

#endif /* STATUS_DISPLAYER_HPP_ */
