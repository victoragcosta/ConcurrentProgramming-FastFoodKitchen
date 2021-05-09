#include "StatusDisplayer.hpp"

#include "AssemblyStation.hpp"
#include "Griddle.hpp"
#include "Fries.hpp"
#include "Delivery.hpp"

#include <queue>
#include <sstream>
#include <iostream>
#include <iomanip>

StatusDisplayer::StatusDisplayer(int nWorkers, int nCustomers, int nDelivery, int nDeepFriers, int nSalters, int nAssemblyStations, int nGriddles)
{
  this->nWorkers = nWorkers;
  this->nCustomers = nCustomers;
  this->nDelivery = nDelivery;
  this->nDeepFriers = nDeepFriers;
  this->nSalters = nSalters;
  this->nAssemblyStations = nAssemblyStations;
  this->nGriddles = nGriddles;

  assemblingTimeAssemblyStation = AssemblyStation::assemblingTime;
  burgersPerBatchAssemblyStation = AssemblyStation::burgersPerBatch;

  griddlingTimeGriddle = Griddle::griddlingTime;
  burgersPerBatchGriddle = Griddle::burgersPerBatch;

  setupTimeDeepFriers = Fries::DeepFriers::setupTime;
  fryTimeDeepFriers = Fries::DeepFriers::fryTime;
  friesPerBatchDeepFriers = Fries::DeepFriers::friesPerBatch;

  saltingTimeSalting = Fries::Salting::saltingTime;
  friesPerPortionSalting = Fries::Salting::friesPerPortion;

  assemblingTimeDelivery = Delivery::assemblingTime;
}

void StatusDisplayer::displayStatus()
{
  std::ostringstream out;

  out << "===============================================================\n";

  out << " " << std::setw(2) << assemblingTimeAssemblyStation << "s para montar hambúrgueres | ";
  out << std::setw(2) << burgersPerBatchAssemblyStation << " hambúrgueres por montagem \n";

  out << " " << std::setw(2) << griddlingTimeGriddle << "s para grelhar hambúrgueres | ";
  out << std::setw(2) << burgersPerBatchGriddle << " hambúrgueres por grelhada \n";

  out << " " << std::setw(2) << setupTimeDeepFriers << "s ligar a fritadeira | ";
  out << std::setw(2) << fryTimeDeepFriers << "s para fritar | ";
  out << std::setw(2) << friesPerBatchDeepFriers << " fritas por vez \n";

  out << " " << std::setw(2) << saltingTimeSalting << "s para salgar fritas | ";
  out << std::setw(2) << friesPerPortionSalting << " porções por vez \n";

  out << " " << std::setw(2) << assemblingTimeDelivery << "s para montar e entregar pedidos \n";

  out << "===============================================================\n";

  out << " " << std::setw(2) << nWorkers << " funcionários | ";
  out << std::setw(2) << nCustomers << " fregueses | ";
  out << std::setw(2) << nDelivery << " estações de entrega \n";

  out << " " << std::setw(2) << nDeepFriers << " fritadeiras | ";
  out << std::setw(2) << nSalters << " saleiros \n";

  out << " " << std::setw(2) << nAssemblyStations << " estações de montagem de hambúrguer | ";
  out << std::setw(2) << nGriddles << " chapas \n";

  out << "===============================================================\n\n";

  out << " Grelhas\n";
  out << " carnes: " << burgersMeat << " | trabalhando: " << workersGriddle << "\n\n";

  out << " Montagem de hambúrgueres\n";
  out << " hambúrgueres: " << burgers << " | trabalhando: " << workersAssemblyStation << "\n\n";

  out << " Fritadeiras\n";
  out << " fritas sem sal: " << unsaltedFries << " | fritando: " << (nDeepFriers - availableDeepFrier) << "\n\n";

  out << " Salgagem de fritas\n";
  out << " fritas: " << burgers << " | trabalhando: " << workersSalting << "\n\n";

  out << " Entrega de pedidos\n";
  out << " trabalhando: " << workersDelivery << "\n";
  out << " fritas pedidas: " << ordered.fries << " | hambúrgueres pedidos: " << ordered.burgers << "\n";
  out << " próximo pedido - fritas: " << firstOrder.fries << " | hambúrgueres: " << firstOrder.burgers << "\n\n";

  out << "===============================================================\n";

  std::cout << out.str();
}

void StatusDisplayer::updateAssemblyStationBurgers(int n)
{
  pthread_mutex_lock(&mutex);
  burgers = n;
  displayStatus();
  pthread_mutex_unlock(&mutex);
}
void StatusDisplayer::updateAssemblyStationWorkers(int n)
{
  pthread_mutex_lock(&mutex);
  workersAssemblyStation = n;
  displayStatus();
  pthread_mutex_unlock(&mutex);
}
void StatusDisplayer::updateDeepFrierAvailable(int n)
{
  pthread_mutex_lock(&mutex);
  availableDeepFrier = n;
  displayStatus();
  pthread_mutex_unlock(&mutex);
}
void StatusDisplayer::updateDeepFrierUnsaltedFries(int n)
{
  pthread_mutex_lock(&mutex);
  unsaltedFries = n;
  displayStatus();
  pthread_mutex_unlock(&mutex);
}
void StatusDisplayer::updateSaltingFries(int n)
{
  pthread_mutex_lock(&mutex);
  fries = n;
  displayStatus();
  pthread_mutex_unlock(&mutex);
}
void StatusDisplayer::updateSaltingWorkers(int n)
{
  pthread_mutex_lock(&mutex);
  workersSalting = n;
  displayStatus();
  pthread_mutex_unlock(&mutex);
}
void StatusDisplayer::updateDeliveryOrdered(int fries, int burgers)
{
  pthread_mutex_lock(&mutex);
  ordered.fries = fries;
  ordered.burgers = burgers;
  displayStatus();
  pthread_mutex_unlock(&mutex);
}
void StatusDisplayer::updateDeliveryFirstOrder(std::queue<Delivery::order_t> ordersQueue)
{
  pthread_mutex_lock(&mutex);
  if (ordersQueue.size() == 0)
  {
    firstOrder.burgers = 0;
    firstOrder.fries = 0;
  }
  else
    firstOrder = ordersQueue.front();
  displayStatus();
  pthread_mutex_unlock(&mutex);
}
void StatusDisplayer::updateDeliveryWorkers(int n)
{
  pthread_mutex_lock(&mutex);
  workersDelivery = n;
  displayStatus();
  pthread_mutex_unlock(&mutex);
}
void StatusDisplayer::updateGriddleBurgersMeat(int n)
{
  pthread_mutex_lock(&mutex);
  burgersMeat = n;
  displayStatus();
  pthread_mutex_unlock(&mutex);
}
void StatusDisplayer::updateGriddleWorkers(int n)
{
  pthread_mutex_lock(&mutex);
  workersGriddle = n;
  displayStatus();
  pthread_mutex_unlock(&mutex);
}
