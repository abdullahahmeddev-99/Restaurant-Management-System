#include "meal_logic.h"

#include <sstream>
#include <iomanip>

bool addPizzaToOrder(Order& o, RestaurantData& d, const std::vector<int>& sizes) {
    int q = static_cast<int>(sizes.size());
    if (q <= 0) {
        return false;
    }

    // Check if we have sufficient stock
    if (d.pizza_stock < q) {
        return false;
    }

    OrderItem item;
    item.type = MealType::Pizza;
    item.name = "Pizza";
    item.quantity = q;
    item.configuration_count = q;

    double total = 0.00;
    double cheapest = 0.00;

    int largeCount = 0;
    int mediumCount = 0;
    int smallCount = 0;

    for (int i = 0; i < q; i++) {
        int size = sizes[i];
        item.configurations[i] = size;
        double price =
            size == 1 ? d.pizza_l :
            size == 2 ? d.pizza_m :
            d.pizza_s;

        if (size == 1) ++largeCount;
        else if (size == 2) ++mediumCount;
        else ++smallCount;

        total += price;

        if (i == 0 || price < cheapest)
            cheapest = price;
    }

    int freePizza = q / 3;

    if (freePizza > 0) {
        total -= cheapest * freePizza;
    }

    if (q >= 4) {
        total *= 0.90;
    }

    // Retain the selected sizes as display data so the GUI can show their
    // actual menu prices instead of presenting a misleading averaged price.
    std::ostringstream details;
    details << std::fixed << std::setprecision(2);
    auto appendSize = [&details](const char* sizeName, int count, double price, bool& first) {
        if (count == 0) return;
        if (!first) details << "; ";
        details << sizeName << " x" << count << " @ $" << price;
        first = false;
    };
    bool firstSize = true;
    appendSize("Large", largeCount, d.pizza_l, firstSize);
    appendSize("Medium", mediumCount, d.pizza_m, firstSize);
    appendSize("Small", smallCount, d.pizza_s, firstSize);
    item.details = details.str();

    item.total_price = total;
    item.unit_price = total / q;

    if (addItem(o, item)) {
        // NOTE: Stock is NOT deducted here. It will be deducted after successful payment.
        return true;
    }
    return false;
}

bool addBurgerToOrder(Order& o, RestaurantData& d, int q, int type) {
    if (q <= 0) {
        return false;
    }

    // Check if we have sufficient stock
    if (d.burger_stock < q) {
        return false;
    }

    OrderItem item;
    item.type = MealType::Burger;
    item.name = "Burger";
    item.details = type == 1 ? "Beef" : "Chicken";
    item.quantity = q;
    item.unit_price = type == 1 ? d.burger_beef : d.burger_chicken;
    item.total_price = item.unit_price * q;

    if (q == 2) {
        item.total_price -= type == 1 ? 30.00 : 40.00;
    }

    if (addItem(o, item)) {
        // NOTE: Stock is NOT deducted here. It will be deducted after successful payment.
        return true;
    }
    return false;
}

bool addPotatoesToOrder(Order& o, RestaurantData& d, int q) {
    if (q <= 0) {
        return false;
    }

    // Check if we have sufficient stock
    if (d.potatoes_stock < q) {
        return false;
    }

    OrderItem item;
    item.type = MealType::Potatoes;
    item.name = "Potatoes";
    item.details = "Regular";
    item.quantity = q;
    item.unit_price = d.potatoes;

    int freePotatoes = q / 3;
    int paid = q - freePotatoes;

    item.total_price = paid * d.potatoes;

    if (q >= 4) {
        item.total_price *= 0.95;
    }

    if (addItem(o, item)) {
        // NOTE: Stock is NOT deducted here. It will be deducted after successful payment.
        return true;
    }
    return false;
}

bool addChickenToOrder(Order& o, RestaurantData& d, const std::vector<int>& portions) {
    int q = static_cast<int>(portions.size());
    if (q <= 0) {
        return false;
    }

    // Check if we have sufficient stock
    if (d.chicken_stock < q) {
        return false;
    }

    OrderItem item;
    item.type = MealType::Chicken;
    item.name = "Chicken";
    item.details = "Different Portions";
    item.quantity = q;
    item.configuration_count = q;

    double total = 0.00;
    int fullChickenCount = 0;

    for (int i = 0; i < q; i++) {
        int type = portions[i];
        item.configurations[i] = type;
        if (type == 1) {
            total += d.chicken;
            fullChickenCount++;
        }
        else if (type == 2)
            total += d.chicken / 2;
        else
            total += d.chicken / 4;
    }

    if (q >= 2) {
        total *= 0.95;
    }

    item.total_price = total;
    item.unit_price = total / q;

    if (!addItem(o, item))
        return false;

    // NOTE: Chicken stock is NOT deducted here. It will be deducted after successful payment.

    // Add free potatoes if we have full chicken portions and sufficient potato stock
    if (fullChickenCount > 0) {
        if (d.potatoes_stock >= fullChickenCount) {
            OrderItem freePotatoes;
            freePotatoes.type = MealType::Potatoes;
            freePotatoes.name = "Potatoes";
            freePotatoes.details = "Free with Full Chicken";
            freePotatoes.quantity = fullChickenCount;
            freePotatoes.unit_price = 0.00;
            freePotatoes.total_price = 0.00;

            if (addItem(o, freePotatoes)) {
                // NOTE: Potatoes stock is NOT deducted here. It will be deducted after successful payment.
            }
        }
    }
    return true;
}
