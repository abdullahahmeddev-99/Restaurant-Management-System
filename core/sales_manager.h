#ifndef SALES_MANAGER_H
#define SALES_MANAGER_H

#include "models.h"

bool processPayment(Order& o, RestaurantData& d, PaymentMethod method, double paidAmount);
double getChangeAmount(const Order& o);
double getAverageOrderSales(const RestaurantData& d);

#endif // SALES_MANAGER_H
