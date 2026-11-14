#ifndef ORDER_MANAGER_H
#define ORDER_MANAGER_H

#include "models.h"

int generateID();
bool mealAlreadySelected(const Order& o, MealType meal);
bool addItem(Order& o, const OrderItem& item);
bool removeItem(Order& o, RestaurantData& d, int index);
bool changeItemQuantity(Order& o, RestaurantData& d, int index, int delta);
bool hasSufficientStock(const Order& o, const RestaurantData& d);
void calculate(Order& o, const RestaurantData& d);
void restoreStock(const Order& o, RestaurantData& d);
void deductInventory(Order& o, RestaurantData& d);

#endif // ORDER_MANAGER_H
