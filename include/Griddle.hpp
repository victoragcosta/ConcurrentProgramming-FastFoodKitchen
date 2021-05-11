#ifndef GRIDDLE_HPP_
#define GRIDDLE_HPP_

namespace Griddle
{
  extern const int griddlingTime;
  extern const int burgersPerBatch;

  // Setups some parameters for the Griddle, preparing for the simulation.
  void initGriddle(int nGriddles);

  // Attempts to griddle some burgers' meats. If possible, sleeps for griddlingTime,
  // then adds burgersPerBatch to burgerMeats and return true.
  bool makeBurgerMeats();

  // Attempts to get some burgers' meats. If possible, subtracts n and return true.
  // If not possible, returns false.
  bool getBurgerMeats(int n);
}

#endif /* GRIDDLE_HPP_ */
