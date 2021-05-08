#ifndef DELIVERY_HPP_
#define DELIVERY_HPP_

#include <vector>
#include <pthread.h>

namespace Delivery
{
  std::vector<pthread_t> initDelivery(int nDelivery, int nCustomers);

  bool deliverOrder();

  void *Customer(void *args);
}

#endif /* DELIVERY_HPP_ */
