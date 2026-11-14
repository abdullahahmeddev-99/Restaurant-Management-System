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

static int g_failures = 0;

#define CHECK(cond)                                                          \
    do {                                                                     \
        if (!(cond)) {                                                       \
            std::cerr << "  FAILED: " << #cond << " (line " << __LINE__       \
                      << ")" << std::endl;                                   \
            ++g_failures;                                                    \
        }                                                                    \
    } while (0)

bool nearEqual(double a, double b, double epsilon = 0.01) {
    return std::fabs(a - b) < epsilon;
}

void testValidation() {
    cout << "[TEST] Validation functions..." << endl;
    CHECK(validName("John Doe") == true);
    CHECK(validName("John123") == false);
    CHECK(validName("") == false);

    CHECK(validPhone("01012345678") == true);
    CHECK(validPhone("02012345678") == false); // doesn't start with 01
    CHECK(validPhone("0101234567") == false);  // 10 digits
    CHECK(validPhone("010123456789") == false); // 12 digits
    cout << " -> Validation tests PASSED!" << endl;
}

void testMealsAndOffers() {
    cout << "[TEST] Meals & Discount Offers..." << endl;
    RestaurantData data;
    Order o;
    o.order_id = generateID();
    CHECK(o.order_id == 1001);

    // Pizza Test: 3 pizzas (Large $250, Medium $150, Small $100) -> Buy 2 Get 1 Free (cheapest $100 free) -> Total = 250 + 150 = 400
    vector<int> pizzaSizes = {1, 2, 3};
    bool pizzaOk = addPizzaToOrder(o, data, pizzaSizes);
    CHECK(pizzaOk == true);
    CHECK(data.pizza_stock == 50);  // Stock NOT deducted during add
    CHECK(nearEqual(o.items[0].total_price, 400.00));

    // Burger Test: 2 Beef Burgers ($200 each) -> $30 offer applied -> Total = 400 - 30 = 370
    bool burgerOk = addBurgerToOrder(o, data, 2, 1);
    CHECK(burgerOk == true);
    CHECK(data.burger_stock == 50);  // Stock NOT deducted during add
    CHECK(nearEqual(o.items[1].total_price, 370.00));

    // Potatoes Test: 4 Potatoes ($120 each) -> Buy 2 Get 1 Free (pay 3) = 360 -> 5% bulk discount = 360 * 0.95 = 342
    bool potatoesOk = addPotatoesToOrder(o, data, 4);
    CHECK(potatoesOk == true);
    CHECK(data.potatoes_stock == 50);  // Stock NOT deducted during add
    CHECK(nearEqual(o.items[2].total_price, 342.00));

    // Chicken Test: 1 Full Chicken ($600) -> Auto includes 1 Free Potatoes item
    vector<int> chickenPortions = {1};
    bool chickenOk = addChickenToOrder(o, data, chickenPortions);
    CHECK(chickenOk == true);
    CHECK(data.chicken_stock == 30);  // Stock NOT deducted during add
    CHECK(data.potatoes_stock == 50);  // Free potatoes stock NOT deducted during add
    CHECK(o.item_count == 5);  // Pizza, Burger, Potatoes, Chicken, Free Potatoes
    CHECK(o.items[4].name == "Potatoes");
    CHECK(nearEqual(o.items[4].total_price, 0.00));

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
    CHECK(nearEqual(o.subtotal, 490.00));

    // Discount:
    // 1) Discount card: 5% of 490 = 24.50
    // 2) Burger + Potatoes combo: 10% of (490 - 24.50) = 46.55
    // Total discount = 71.05
    CHECK(nearEqual(o.discount, 71.05));

    // Delivery fee = 50.00
    CHECK(nearEqual(o.delivery_fee, 50.00));

    // Taxable = 490 - 71.05 + 50 = 468.95
    // Tax (14%) = 65.653
    CHECK(nearEqual(o.tax, 65.653));

    // Final total = 468.95 + 65.653 = 534.603
    CHECK(nearEqual(o.final_total, 534.603));

    // Test Payment
    bool payFail = processPayment(o, data, PaymentMethod::Cash, 500.00);
    CHECK(payFail == false); // Insufficient cash

    bool payOk = processPayment(o, data, PaymentMethod::Cash, 600.00);
    CHECK(payOk == true);
    CHECK(nearEqual(getChangeAmount(o), 65.397));
    CHECK(data.total_orders == 1);
    CHECK(data.cash_orders == 1);
    CHECK(nearEqual(data.total_sales, 534.603));
    CHECK(nearEqual(getAverageOrderSales(data), 534.603));

    cout << " -> Calculate & Payment tests PASSED!" << endl;
}

void testSingleItemRemoval() {
    cout << "[TEST] Single Item Removal & Cart Consistency..." << endl;
    RestaurantData data;
    Order o;

    // Initial stocks: pizza_stock = 50, burger_stock = 50
    addBurgerToOrder(o, data, 3, 1);
    CHECK(data.burger_stock == 50);  // Stock NOT deducted when adding

    vector<int> pizzaSizes = {1, 2};
    addPizzaToOrder(o, data, pizzaSizes);
    CHECK(data.pizza_stock == 50);  // Stock NOT deducted when adding
    CHECK(o.item_count == 2);

    // Remove Burger item (index 0)
    bool removeOk = removeItem(o, data, 0);
    CHECK(removeOk == true);
    CHECK(data.burger_stock == 50);  // Stock unchanged (was never deducted)
    CHECK(data.pizza_stock == 50);   // Pizza stock unchanged
    CHECK(o.item_count == 1);
    CHECK(o.items[0].type == MealType::Pizza);

    cout << " -> Single Item Removal tests PASSED!" << endl;
}

void testCartQuantityChanges() {
    cout << "[TEST] Cart quantity controls Core flow..." << endl;
    RestaurantData data;
    Order o;
    o.customer.order_type = OrderType::Takeaway;

    // Pizza retains the configured sizes, then repeats the last explicit size on +.
    CHECK(addPizzaToOrder(o, data, {1, 2}));
    CHECK(changeItemQuantity(o, data, 0, 1));
    CHECK(o.items[0].quantity == 3);
    CHECK(o.items[0].configuration_count == 3);
    CHECK(o.items[0].configurations[0] == 1);
    CHECK(o.items[0].configurations[1] == 2);
    CHECK(o.items[0].configurations[2] == 2);
    CHECK(nearEqual(o.items[0].total_price, 400.00)); // 250 + 150 + 150, cheapest free
    CHECK(nearEqual(o.subtotal, 400.00));
    CHECK(nearEqual(o.tax, 56.00));
    CHECK(nearEqual(o.final_total, 456.00));
    CHECK(changeItemQuantity(o, data, 0, -1));
    CHECK(o.items[0].quantity == 2);
    CHECK(nearEqual(o.items[0].total_price, 400.00));
    CHECK(changeItemQuantity(o, data, 0, -1));
    CHECK(o.items[0].quantity == 1);
    CHECK(!changeItemQuantity(o, data, 0, -1)); // quantity one removal is never implicit

    // Chicken portion configuration is retained and is safe to edit.
    Order chickenOrder;
    CHECK(addChickenToOrder(chickenOrder, data, {3}));
    CHECK(changeItemQuantity(chickenOrder, data, 0, 1));
    CHECK(chickenOrder.items[0].quantity == 2);
    CHECK(chickenOrder.items[0].configurations[0] == 3);
    CHECK(chickenOrder.items[0].configurations[1] == 3);
    CHECK(changeItemQuantity(chickenOrder, data, 0, -1));
    CHECK(chickenOrder.items[0].quantity == 1);

    // A full chicken still has its Core-created free-potatoes item and can be edited safely.
    Order fullChickenOrder;
    CHECK(addChickenToOrder(fullChickenOrder, data, {1}));
    CHECK(fullChickenOrder.item_count == 2);
    CHECK(changeItemQuantity(fullChickenOrder, data, 0, 1));
    CHECK(fullChickenOrder.items[0].quantity == 2);
    CHECK(fullChickenOrder.items[0].configurations[1] == 1);

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
    if (g_failures == 0) {
        cout << "    ALL CORE TESTS PASSED SUCCESSFULLY!   " << endl;
    } else {
        cout << "    CORE TESTS FAILED: " << g_failures << " check(s)     " << endl;
    }
    cout << "==========================================" << endl;
    return g_failures == 0 ? 0 : 1;
}
