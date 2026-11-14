#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

#include "core/meal_logic.h"
#include "core/order_manager.h"
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
    return fabs(a - b) < epsilon;
}

void testStockNotDeductedOnAdd() {
    // Stock should NOT be deducted when adding items to cart
    RestaurantData data;
    Order o;

    CHECK(data.pizza_stock == 50);
    CHECK(addPizzaToOrder(o, data, {1, 2, 3}));
    CHECK(data.pizza_stock == 50);  // Stock unchanged

    CHECK(addBurgerToOrder(o, data, 2, 1));
    CHECK(data.burger_stock == 50);  // Stock unchanged

    CHECK(addPotatoesToOrder(o, data, 4));
    CHECK(data.potatoes_stock == 50);  // Stock unchanged

    CHECK(addChickenToOrder(o, data, {2, 3}));
    CHECK(data.chicken_stock == 30);  // Stock unchanged (free potatoes check happens first)
    CHECK(data.potatoes_stock == 50);  // Stock still unchanged

    cout << " -> Stock NOT deducted on add (as expected)\n";
}

void testStockNotChangedDuringQuantityModification() {
    // Stock should NOT change when modifying quantities in cart
    RestaurantData data;
    Order o;

    CHECK(addBurgerToOrder(o, data, 2, 1));
    CHECK(data.burger_stock == 50);  // Initial stock

    CHECK(changeItemQuantity(o, data, 0, 1));
    CHECK(o.items[0].quantity == 3);
    CHECK(data.burger_stock == 50);  // Stock unchanged

    CHECK(changeItemQuantity(o, data, 0, -1));
    CHECK(o.items[0].quantity == 2);
    CHECK(data.burger_stock == 50);  // Stock unchanged

    cout << " -> Stock unchanged during quantity modification\n";
}

void testInsufficientStockBlocksCheckout() {
    // Insufficient stock should block payment
    RestaurantData data;
    Order o;
    o.customer.order_type = OrderType::Takeaway;

    // Try to add more than available
    vector<int> tooMany(51, 1);
    CHECK(!addPizzaToOrder(o, data, tooMany));
    CHECK(o.item_count == 0);
    CHECK(data.pizza_stock == 50);

    // Add valid items
    CHECK(addPizzaToOrder(o, data, {1, 2, 3}));
    CHECK(o.item_count == 1);
    calculate(o, data);

    // Manually reduce stock to simulate low inventory
    data.pizza_stock = 1;  // Not enough for 3 pizzas in order
    CHECK(!hasSufficientStock(o, data));

    cout << " -> Insufficient stock blocks checkout\n";
}

void testStockDeductedAfterSuccessfulPayment() {
    // Stock SHOULD be deducted only after successful payment
    RestaurantData data;
    Order o;
    o.customer.order_type = OrderType::Takeaway;
    o.customer.name = "John";
    o.customer.phone = "01012345678";

    // Add items
    CHECK(addBurgerToOrder(o, data, 2, 1));
    CHECK(addPotatoesToOrder(o, data, 1));
    CHECK(data.burger_stock == 50);
    CHECK(data.potatoes_stock == 50);

    calculate(o, data);

    // Process payment
    bool ok = processPayment(o, data, PaymentMethod::Cash, o.final_total);
    CHECK(ok);

    // Now stock SHOULD be deducted
    CHECK(data.burger_stock == 48);  // 50 - 2
    CHECK(data.potatoes_stock == 49);  // 50 - 1
    CHECK(o.inventory_deducted == true);

    cout << " -> Stock deducted after successful payment\n";
}

void testNoDuplicateInventoryDeduction() {
    // Inventory should not be deducted twice if payment is confirmed multiple times
    RestaurantData data;
    Order o;
    o.customer.order_type = OrderType::Takeaway;
    o.customer.name = "Jane";
    o.customer.phone = "01087654321";

    CHECK(addPizzaToOrder(o, data, {1}));
    calculate(o, data);

    CHECK(data.pizza_stock == 50);

    // First payment
    bool ok1 = processPayment(o, data, PaymentMethod::Cash, o.final_total);
    CHECK(ok1);
    CHECK(data.pizza_stock == 49);

    // Try to process payment again (should be prevented)
    bool ok2 = processPayment(o, data, PaymentMethod::Cash, o.final_total);
    CHECK(ok2);  // Returns true but doesn't deduct again
    CHECK(data.pizza_stock == 49);  // Stock unchanged

    cout << " -> Duplicate inventory deduction prevented\n";
}

void testMultipleQuantitiesWithInventoryDeduction() {
    // Test correct inventory deduction for quantities > 1
    RestaurantData data;
    Order o;
    o.customer.order_type = OrderType::Takeaway;
    o.customer.name = "Ahmed";
    o.customer.phone = "01123456789";

    // Add multiple quantities
    vector<int> pizzaSizes(5, 1);  // 5 large pizzas
    CHECK(addPizzaToOrder(o, data, pizzaSizes));
    CHECK(data.pizza_stock == 50);

    CHECK(changeItemQuantity(o, data, 0, 1));  // 6 pizzas now
    CHECK(data.pizza_stock == 50);  // Still 50, not deducted

    calculate(o, data);

    // Process payment
    bool ok = processPayment(o, data, PaymentMethod::Cash, o.final_total);
    CHECK(ok);

    // Now 6 pizzas should be deducted
    CHECK(data.pizza_stock == 44);
    CHECK(o.items[0].quantity == 6);

    cout << " -> Correct inventory deduction for quantities > 1\n";
}

void testFreeItemsInventoryDeduction() {
    // Test inventory deduction for free items (free potatoes with chicken, free pizza, etc.)
    RestaurantData data;
    Order o;
    o.customer.order_type = OrderType::Takeaway;
    o.customer.name = "Sara";
    o.customer.phone = "01234567890";

    // Add chicken (should auto-add free potatoes)
    CHECK(addChickenToOrder(o, data, {1}));  // 1 full chicken
    CHECK(o.item_count == 2);  // Chicken + free potatoes

    CHECK(data.chicken_stock == 30);
    CHECK(data.potatoes_stock == 50);  // Free potatoes not deducted yet

    calculate(o, data);

    // Process payment
    bool ok = processPayment(o, data, PaymentMethod::Cash, o.final_total);
    CHECK(ok);

    // Now inventory should be deducted for both chicken and free potatoes
    CHECK(data.chicken_stock == 29);  // 30 - 1
    CHECK(data.potatoes_stock == 49);  // 50 - 1 (free potatoes)

    cout << " -> Free items inventory deducted correctly\n";
}

void testNegativeInventoryPrevented() {
    // Stock should never go negative
    RestaurantData data;
    Order o;
    o.customer.order_type = OrderType::Takeaway;
    o.customer.name = "Ali";
    o.customer.phone = "01545678901";

    // Add items up to stock limit
    vector<int> pizzaSizes(50, 1);  // All 50 pizzas
    CHECK(addPizzaToOrder(o, data, pizzaSizes));
    CHECK(data.pizza_stock == 50);

    calculate(o, data);

    // Process payment
    bool ok = processPayment(o, data, PaymentMethod::Cash, o.final_total);
    CHECK(ok);

    // Stock should be exactly 0, not negative
    CHECK(data.pizza_stock == 0);
    CHECK(data.pizza_stock >= 0);

    cout << " -> Negative inventory prevented\n";
}

void testCheckoutStockGuard() {
    // Payment should be blocked if stock is insufficient
    RestaurantData data;
    Order o;
    o.customer.order_type = OrderType::Takeaway;

    CHECK(addBurgerToOrder(o, data, 2, 1));
    CHECK(addPotatoesToOrder(o, data, 1));
    calculate(o, data);

    CHECK(hasSufficientStock(o, data));

    // Simulate low stock
    data.burger_stock = -1;
    CHECK(!hasSufficientStock(o, data));
    CHECK(!processPayment(o, data, PaymentMethod::Cash, o.final_total));

    cout << " -> Checkout stock guard prevents negative stock\n";
}

int main() {
    cout << "\n===== INVENTORY TESTS =====\n";

    testStockNotDeductedOnAdd();
    testStockNotChangedDuringQuantityModification();
    testInsufficientStockBlocksCheckout();
    testStockDeductedAfterSuccessfulPayment();
    testNoDuplicateInventoryDeduction();
    testMultipleQuantitiesWithInventoryDeduction();
    testFreeItemsInventoryDeduction();
    testNegativeInventoryPrevented();
    testCheckoutStockGuard();

    if (g_failures == 0) {
        cout << "\n===== ALL INVENTORY TESTS PASSED! =====\n";
    } else {
        cout << "\n===== INVENTORY TESTS FAILED: " << g_failures
             << " check(s) =====\n";
    }
    return g_failures == 0 ? 0 : 1;
}
