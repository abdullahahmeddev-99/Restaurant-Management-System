#ifndef MODELS_H
#define MODELS_H

#include <string>

enum class PaymentMethod { Cash = 1, CreditCard, Wallet };
enum class OrderType { DineIn = 1, Takeaway, Delivery };
enum class MealType { Pizza = 1, Burger, Potatoes, Chicken };

struct RestaurantData {
    double pizza_l = 250.00;
    double pizza_m = 150.00;
    double pizza_s = 100.00;
    double burger_beef = 200.00;
    double burger_chicken = 220.00;
    double potatoes = 120.00;
    double chicken = 600.00;

    int pizza_stock = 50;
    int burger_stock = 50;
    int potatoes_stock = 50;
    int chicken_stock = 30;

    double tax_rate = 0.14;
    double delivery_fee = 50.00;

    int total_orders = 0;
    int cash_orders = 0;
    int card_orders = 0;
    int wallet_orders = 0;

    double total_sales = 0.00;
};

struct RestaurantInfo {
    std::string name;
    std::string phone;
    int table_number = 0;
    bool discount_card = false;
    OrderType order_type = OrderType::Takeaway;
};

struct OrderItem {
    MealType type = MealType::Pizza;
    std::string name;
    std::string details;
    int quantity = 0;
    double unit_price = 0.00;
    double total_price = 0.00;
    // Core-owned selections for configurable meals (pizza size or chicken portion).
    // They let a cart quantity edit preserve the customer's original choices.
    int configurations[50] = {};
    int configuration_count = 0;
};

struct BillInfo {
    PaymentMethod payment = PaymentMethod::Cash;
    double paid_amount = 0.00;
};

struct Order {
    int order_id = 0;
    RestaurantInfo customer;
    OrderItem items[20];
    int item_count = 0;

    double subtotal = 0.00;
    double discount = 0.00;
    double tax = 0.00;
    double delivery_fee = 0.00;
    double final_total = 0.00;

    BillInfo bill;
    
    // Inventory tracking: set to true after successful payment to prevent duplicate deduction
    bool inventory_deducted = false;
};

#endif // MODELS_H
