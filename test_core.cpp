#include <iostream>
#include <cassert>
#include <vector>
#include <cmath>
#include "core/models.h"
#include "core/validation.h"
#include "core/order_manager.h"
#include "core/meal_logic.h"
#include "core/sales_manager.h"

using namespace std;

bool nearEqual(double a, double b, double epsilon = 0.01) {
    return std::fabs(a - b) < epsilon;
}

void testValidation() {
    cout << "[TEST] Validation functions..." << endl;
    assert(validName("John Doe") == true);
    assert(validName("John123") == false);
    assert(validName("") == false);

    assert(validPhone("01012345678") == true);
    assert(validPhone("02012345678") == false); // doesn't start with 01
    assert(validPhone("0101234567") == false);  // 10 digits
    assert(validPhone("010123456789") == false); // 12 digits
    cout << " -> Validation tests PASSED!" << endl;
}

void testMealsAndOffers() {
    cout << "[TEST] Meals & Discount Offers..." << endl;
    RestaurantData data;
    Order o;
    o.order_id = generateID();
    assert(o.order_id == 1001);

    // Pizza Test: 3 pizzas (Large $250, Medium $150, Small $100) -> Buy 2 Get 1 Free (cheapest $100 free) -> Total = 250 + 150 = 400
    vector<int> pizzaSizes = {1, 2, 3};
    bool pizzaOk = addPizzaToOrder(o, data, pizzaSizes);
    assert(pizzaOk == true);
    assert(data.pizza_stock == 50);  // Stock NOT deducted during add
    assert(nearEqual(o.items[0].total_price, 400.00));

    // Burger Test: 2 Beef Burgers ($200 each) -> $30 offer applied -> Total = 400 - 30 = 370
    bool burgerOk = addBurgerToOrder(o, data, 2, 1);
    assert(burgerOk == true);
    assert(data.burger_stock == 50);  // Stock NOT deducted during add
    assert(nearEqual(o.items[1].total_price, 370.00));

    // Potatoes Test: 4 Potatoes ($120 each) -> Buy 2 Get 1 Free (pay 3) = 360 -> 5% bulk discount = 360 * 0.95 = 342
    bool potatoesOk = addPotatoesToOrder(o, data, 4);
    assert(potatoesOk == true);
    assert(data.potatoes_stock == 50);  // Stock NOT deducted during add
    assert(nearEqual(o.items[2].total_price, 342.00));

    // Chicken Test: 1 Full Chicken ($600) -> Auto includes 1 Free Potatoes item
    vector<int> chickenPortions = {1};
    bool chickenOk = addChickenToOrder(o, data, chickenPortions);
    assert(chickenOk == true);
    assert(data.chicken_stock == 30);  // Stock NOT deducted during add
    assert(data.potatoes_stock == 50);  // Free potatoes stock NOT deducted during add
    assert(o.item_count == 5);  // Pizza, Burger, Potatoes, Chicken, Free Potatoes
    assert(o.items[4].name == "Potatoes");
    assert(nearEqual(o.items[4].total_price, 0.00));

    cout << " -> Meal ordering & discount offer tests PASSED!" << endl;
}

void testCalculateAndPayment() {
    cout << "[TEST] Order Calculation & Payment..." << endl;
    RestaurantData data;
    Order o;
    o.order_id = generateID();
    o.customer.name = "Ahmed";
    o.customer.phone = "01123456789";
    o.customer.order_type = OrderType::Delivery; // Delivery fee = $50
    o.customer.discount_card = true; // 5% discount card

    // Add 2 Beef Burgers ($370) and 1 Potato ($120) -> Combo discount applies (10%)
    addBurgerToOrder(o, data, 2, 1);
    addPotatoesToOrder(o, data, 1);

    // Calculate order
    calculate(o, data);

    // Subtotal = 370 + 120 = 490
    assert(nearEqual(o.subtotal, 490.00));

    // Discount:
    // 1) Discount card: 5% of 490 = 24.50
    // 2) Burger + Potatoes combo: 10% of (490 - 24.50) = 46.55
    // Total discount = 71.05
    assert(nearEqual(o.discount, 71.05));

    // Delivery fee = 50.00
    assert(nearEqual(o.delivery_fee, 50.00));

    // Taxable = 490 - 71.05 + 50 = 468.95
    // Tax (14%) = 65.653
    assert(nearEqual(o.tax, 65.653));

    // Final total = 468.95 + 65.653 = 534.603
    assert(nearEqual(o.final_total, 534.603));

    // Test Payment
    bool payFail = processPayment(o, data, PaymentMethod::Cash, 500.00);
    assert(payFail == false); // Insufficient cash

    bool payOk = processPayment(o, data, PaymentMethod::Cash, 600.00);
    assert(payOk == true);
    assert(nearEqual(getChangeAmount(o), 65.397));
    assert(data.total_orders == 1);
    assert(data.cash_orders == 1);
    assert(nearEqual(data.total_sales, 534.603));
    assert(nearEqual(getAverageOrderSales(data), 534.603));

    cout << " -> Calculate & Payment tests PASSED!" << endl;
}

void testSingleItemRemoval() {
    cout << "[TEST] Single Item Removal & Cart Consistency..." << endl;
    RestaurantData data;
    Order o;

    // Initial stocks: pizza_stock = 50, burger_stock = 50
    addBurgerToOrder(o, data, 3, 1);
    assert(data.burger_stock == 50);  // Stock NOT deducted when adding

    vector<int> pizzaSizes = {1, 2};
    addPizzaToOrder(o, data, pizzaSizes);
    assert(data.pizza_stock == 50);  // Stock NOT deducted when adding
    assert(o.item_count == 2);

    // Remove Burger item (index 0)
    bool removeOk = removeItem(o, data, 0);
    assert(removeOk == true);
    assert(data.burger_stock == 50);  // Stock unchanged (was never deducted)
    assert(data.pizza_stock == 50);   // Pizza stock unchanged
    assert(o.item_count == 1);
    assert(o.items[0].type == MealType::Pizza);

    cout << " -> Single Item Removal tests PASSED!" << endl;
}

void testCartQuantityChanges() {
    cout << "[TEST] Cart quantity controls Core flow..." << endl;
    RestaurantData data;
    Order o;
    o.customer.order_type = OrderType::Takeaway;

    // Pizza retains the configured sizes, then repeats the last explicit size on +.
    assert(addPizzaToOrder(o, data, {1, 2}));
    assert(changeItemQuantity(o, data, 0, 1));
    assert(o.items[0].quantity == 3);
    assert(o.items[0].configuration_count == 3);
    assert(o.items[0].configurations[0] == 1);
    assert(o.items[0].configurations[1] == 2);
    assert(o.items[0].configurations[2] == 2);
    assert(nearEqual(o.items[0].total_price, 400.00)); // 250 + 150 + 150, cheapest free
    assert(nearEqual(o.subtotal, 400.00));
    assert(nearEqual(o.tax, 56.00));
    assert(nearEqual(o.final_total, 456.00));
    assert(changeItemQuantity(o, data, 0, -1));
    assert(o.items[0].quantity == 2);
    assert(nearEqual(o.items[0].total_price, 400.00));
    assert(changeItemQuantity(o, data, 0, -1));
    assert(o.items[0].quantity == 1);
    assert(!changeItemQuantity(o, data, 0, -1)); // quantity one removal is never implicit

    // Chicken portion configuration is retained and is safe to edit.
    Order chickenOrder;
    assert(addChickenToOrder(chickenOrder, data, {3}));
    assert(changeItemQuantity(chickenOrder, data, 0, 1));
    assert(chickenOrder.items[0].quantity == 2);
    assert(chickenOrder.items[0].configurations[0] == 3);
    assert(chickenOrder.items[0].configurations[1] == 3);
    assert(changeItemQuantity(chickenOrder, data, 0, -1));
    assert(chickenOrder.items[0].quantity == 1);

    // A full chicken still has its Core-created free-potatoes item and can be edited safely.
    Order fullChickenOrder;
    assert(addChickenToOrder(fullChickenOrder, data, {1}));
    assert(fullChickenOrder.item_count == 2);
    assert(changeItemQuantity(fullChickenOrder, data, 0, 1));
    assert(fullChickenOrder.items[0].quantity == 2);
    assert(fullChickenOrder.items[0].configurations[1] == 1);

    cout << " -> Cart quantity Core tests PASSED!" << endl;
}

int main() {
    cout << "==========================================" << endl;
    cout << "    RUNNING CORE BUSINESS LOGIC TESTS     " << endl;
    cout << "==========================================" << endl;

    testValidation();
    testMealsAndOffers();
    testCalculateAndPayment();
    testSingleItemRemoval();
    testCartQuantityChanges();

    cout << "==========================================" << endl;
    cout << "    ALL CORE TESTS PASSED SUCCESSFULLY!   " << endl;
    cout << "==========================================" << endl;
    return 0;
}
