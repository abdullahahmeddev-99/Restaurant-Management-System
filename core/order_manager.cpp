#include "order_manager.h"

#include <iomanip>
#include <sstream>

namespace {
double pizzaPrice(int size, const RestaurantData& data) {
    return size == 1 ? data.pizza_l : size == 2 ? data.pizza_m : data.pizza_s;
}

double chickenPrice(int portion, const RestaurantData& data) {
    return portion == 1 ? data.chicken : portion == 2 ? data.chicken / 2.0 : data.chicken / 4.0;
}

void refreshPizzaPresentationAndPrice(OrderItem& item, const RestaurantData& data) {
    double total = 0.0;
    double cheapest = 0.0;
    int large = 0, medium = 0, small = 0;
    for (int i = 0; i < item.configuration_count; ++i) {
        const int size = item.configurations[i];
        const double price = pizzaPrice(size, data);
        total += price;
        if (i == 0 || price < cheapest) cheapest = price;
        if (size == 1) ++large; else if (size == 2) ++medium; else ++small;
    }
    if (item.quantity / 3 > 0) total -= cheapest * (item.quantity / 3);
    if (item.quantity >= 4) total *= 0.90;

    std::ostringstream details;
    details << std::fixed << std::setprecision(2);
    bool first = true;
    auto append = [&details, &first](const char* name, int count, double price) {
        if (!count) return;
        if (!first) details << "; ";
        details << name << " x" << count << " @ $" << price;
        first = false;
    };
    append("Large", large, data.pizza_l);
    append("Medium", medium, data.pizza_m);
    append("Small", small, data.pizza_s);
    item.details = details.str();
    item.total_price = total;
    item.unit_price = total / item.quantity;
}

void refreshChickenPresentationAndPrice(OrderItem& item, const RestaurantData& data) {
    double total = 0.0;
    int full = 0, half = 0, quarter = 0;
    for (int i = 0; i < item.configuration_count; ++i) {
        const int portion = item.configurations[i];
        total += chickenPrice(portion, data);
        if (portion == 1) ++full; else if (portion == 2) ++half; else ++quarter;
    }
    if (item.quantity >= 2) total *= 0.95;

    std::ostringstream details;
    details << std::fixed << std::setprecision(2);
    bool first = true;
    auto append = [&details, &first](const char* name, int count, double price) {
        if (!count) return;
        if (!first) details << "; ";
        details << name << " x" << count << " @ $" << price;
        first = false;
    };
    append("Full", full, data.chicken);
    append("Half", half, data.chicken / 2.0);
    append("Quarter", quarter, data.chicken / 4.0);
    item.details = details.str();
    item.total_price = total;
    item.unit_price = total / item.quantity;
}

void refreshStandardItemPrice(OrderItem& item, const RestaurantData& data) {
    if (item.type == MealType::Burger) {
        item.total_price = item.unit_price * item.quantity;
        if (item.quantity == 2)
            item.total_price -= item.details == "Beef" ? 30.0 : 40.0;
    } else if (item.type == MealType::Potatoes) {
        const int paid = item.quantity - item.quantity / 3;
        item.total_price = paid * data.potatoes;
        if (item.quantity >= 4) item.total_price *= 0.95;
    }
}
}

int generateID() {
    static int id = 1000;
    return ++id;
}

bool mealAlreadySelected(const Order& o, MealType meal) {
    for (int i = 0; i < o.item_count; i++)
        if (o.items[i].type == meal)
            return true;
    return false;
}

bool addItem(Order& o, const OrderItem& item) {
    if (o.item_count >= 20) {
        return false;
    }
    o.items[o.item_count++] = item;
    return true;
}

bool removeItem(Order& o, RestaurantData& d, int index) {
    if (index < 0 || index >= o.item_count) {
        return false;
    }

    for (int i = index; i < o.item_count - 1; i++) {
        o.items[i] = o.items[i + 1];
    }
    o.item_count--;

    calculate(o, d);
    return true;
}

bool changeItemQuantity(Order& o, RestaurantData& d, int index, int delta) {
    if (index < 0 || index >= o.item_count || (delta != 1 && delta != -1)) return false;
    OrderItem& item = o.items[index];
    if (delta < 0 && item.quantity <= 1) return false;

    if (delta > 0) {
        if ((item.type == MealType::Pizza || item.type == MealType::Chicken) &&
            item.configuration_count > 0 && item.configuration_count < 50) {
            // Repeat the last explicit selection; never invent or merge a configuration.
            item.configurations[item.configuration_count] = item.configurations[item.configuration_count - 1];
            ++item.configuration_count;
        }
        ++item.quantity;
    } else {
        if ((item.type == MealType::Pizza || item.type == MealType::Chicken) && item.configuration_count > 1)
            --item.configuration_count;
        --item.quantity;
    }

    if (item.type == MealType::Pizza) refreshPizzaPresentationAndPrice(item, d);
    else if (item.type == MealType::Chicken) refreshChickenPresentationAndPrice(item, d);
    else refreshStandardItemPrice(item, d);

    calculate(o, d);
    return true;
}

void calculate(Order& o, const RestaurantData& d) {
    o.subtotal = 0.00;

    bool burger = false;
    bool potatoes = false;

    for (int i = 0; i < o.item_count; i++) {
        o.subtotal += o.items[i].total_price;

        if (o.items[i].type == MealType::Burger)
            burger = true;

        if (o.items[i].type == MealType::Potatoes)
            potatoes = true;
    }

    o.discount =
        o.customer.discount_card
        ? o.subtotal * 0.05
        : 0.00;

    if (burger && potatoes)
        o.discount +=
            (o.subtotal - o.discount) * 0.10;

    o.delivery_fee =
        o.customer.order_type == OrderType::Delivery
        ? d.delivery_fee
        : 0.00;

    o.tax =
        (o.subtotal - o.discount + o.delivery_fee)
        * d.tax_rate;

    o.final_total =
        o.subtotal
        - o.discount
        + o.delivery_fee
        + o.tax;
}

bool hasSufficientStock(const Order& o, const RestaurantData& d) {
    int pizzaQty = 0;
    int burgerQty = 0;
    int potatoesQty = 0;
    int chickenQty = 0;

    for (int i = 0; i < o.item_count; i++) {
        const OrderItem& item = o.items[i];
        if (item.type == MealType::Pizza)
            pizzaQty += item.quantity;
        else if (item.type == MealType::Burger)
            burgerQty += item.quantity;
        else if (item.type == MealType::Potatoes)
            potatoesQty += item.quantity;
        else if (item.type == MealType::Chicken)
            chickenQty += item.quantity;
    }

    // Check if current stock is sufficient for each item type
    const bool pizzaOk = pizzaQty == 0 || d.pizza_stock >= pizzaQty;
    const bool burgerOk = burgerQty == 0 || d.burger_stock >= burgerQty;
    const bool potatoesOk = potatoesQty == 0 || d.potatoes_stock >= potatoesQty;
    const bool chickenOk = chickenQty == 0 || d.chicken_stock >= chickenQty;

    return pizzaOk && burgerOk && potatoesOk && chickenOk;
}

void restoreStock(const Order& o, RestaurantData& d) {
    for (int i = 0; i < o.item_count; i++) {
        if (o.items[i].type == MealType::Pizza)
            d.pizza_stock += o.items[i].quantity;
        else if (o.items[i].type == MealType::Burger)
            d.burger_stock += o.items[i].quantity;
        else if (o.items[i].type == MealType::Potatoes)
            d.potatoes_stock += o.items[i].quantity;
        else if (o.items[i].type == MealType::Chicken)
            d.chicken_stock += o.items[i].quantity;
    }
}

void deductInventory(Order& o, RestaurantData& d) {
    // Prevent duplicate deduction if payment is confirmed more than once
    if (o.inventory_deducted) {
        return;
    }

    // Deduct stock for each item in the order
    for (int i = 0; i < o.item_count; i++) {
        const OrderItem& item = o.items[i];
        if (item.type == MealType::Pizza)
            d.pizza_stock -= item.quantity;
        else if (item.type == MealType::Burger)
            d.burger_stock -= item.quantity;
        else if (item.type == MealType::Potatoes)
            d.potatoes_stock -= item.quantity;
        else if (item.type == MealType::Chicken)
            d.chicken_stock -= item.quantity;
    }

    // Mark that inventory has been deducted for this order
    o.inventory_deducted = true;
}
