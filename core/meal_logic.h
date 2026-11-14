#ifndef MEAL_LOGIC_H
#define MEAL_LOGIC_H

#include "models.h"
#include "order_manager.h"
#include <vector>

bool addPizzaToOrder(Order& o, RestaurantData& d, const std::vector<int>& sizes);
bool addBurgerToOrder(Order& o, RestaurantData& d, int q, int type);
bool addPotatoesToOrder(Order& o, RestaurantData& d, int q);
bool addChickenToOrder(Order& o, RestaurantData& d, const std::vector<int>& portions);

#endif // MEAL_LOGIC_H
