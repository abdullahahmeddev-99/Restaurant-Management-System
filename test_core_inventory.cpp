#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

#include "core/meal_logic.h"
#include "core/order_manager.h"
#include "core/sales_manager.h"

using namespace std;

bool nearEqual(double a, double b, double epsilon = 0.01) {
    return fabs(a - b) < epsilon;
}

void testStockNotDeductedOnAdd() {
    // Stock should NOT be deducted when adding items to cart
    RestaurantData data;
    Order o;

    assert(data.pizza_stock == 50);
    assert(addPizzaToOrder(o, data, {1, 2, 3}));
    assert(data.pizza_stock == 50);  // Stock unchanged

    assert(addBurgerToOrder(o, data, 2, 1));
    assert(data.burger_stock == 50);  // Stock unchanged

    assert(addPotatoesToOrder(o, data, 4));
    assert(data.potatoes_stock == 50);  // Stock unchanged

    assert(addChickenToOrder(o, data, {2, 3}));
    assert(data.chicken_stock == 30);  // Stock unchanged (free potatoes check happens first)
    assert(data.potatoes_stock == 50);  // Stock still unchanged

    cout << " -> Stock NOT deducted on add (as expected)\n";
}

void testStockNotChangedDuringQuantityModification() {
    // Stock should NOT change when modifying quantities in cart
    RestaurantData data;
    Order o;

    assert(addBurgerToOrder(o, data, 2, 1));
    assert(data.burger_stock == 50);  // Initial stock

    assert(changeItemQuantity(o, data, 0, 1));
    assert(o.items[0].quantity == 3);
    assert(data.burger_stock == 50);  // Stock unchanged

    assert(changeItemQuantity(o, data, 0, -1));
    assert(o.items[0].quantity == 2);
    assert(data.burger_stock == 50);  // Stock unchanged

    cout << " -> Stock unchanged during quantity modification\n";
}

void testInsufficientStockBlocksCheckout() {
    // Insufficient stock should block payment
    RestaurantData data;
    Order o;
    o.customer.order_type = OrderType::Takeaway;

    // Try to add more than available
    vector<int> tooMany(51, 1);
    assert(!addPizzaToOrder(o, data, tooMany));
    assert(o.item_count == 0);
    assert(data.pizza_stock == 50);

    // Add valid items
    assert(addPizzaToOrder(o, data, {1, 2, 3}));
    assert(o.item_count == 1);
    calculate(o, data);

    // Manually reduce stock to simulate low inventory
    data.pizza_stock = 1;  // Not enough for 3 pizzas in order
    assert(!hasSufficientStock(o, data));

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
    assert(addBurgerToOrder(o, data, 2, 1));
    assert(addPotatoesToOrder(o, data, 1));
    assert(data.burger_stock == 50);
    assert(data.potatoes_stock == 50);

    calculate(o, data);

    // Process payment
    bool ok = processPayment(o, data, PaymentMethod::Cash, o.final_total);
    assert(ok);

    // Now stock SHOULD be deducted
    assert(data.burger_stock == 48);  // 50 - 2
    assert(data.potatoes_stock == 49);  // 50 - 1
    assert(o.inventory_deducted == true);

    cout << " -> Stock deducted after successful payment\n";
}

void testNoDuplicateInventoryDeduction() {
    // Inventory should not be deducted twice if payment is confirmed multiple times
    RestaurantData data;
    Order o;
    o.customer.order_type = OrderType::Takeaway;
    o.customer.name = "Jane";
    o.customer.phone = "01087654321";

    assert(addPizzaToOrder(o, data, {1}));
    calculate(o, data);

    assert(data.pizza_stock == 50);

    // First payment
    bool ok1 = processPayment(o, data, PaymentMethod::Cash, o.final_total);
    assert(ok1);
    assert(data.pizza_stock == 49);

    // Try to process payment again (should be prevented)
    bool ok2 = processPayment(o, data, PaymentMethod::Cash, o.final_total);
    assert(ok2);  // Returns true but doesn't deduct again
    assert(data.pizza_stock == 49);  // Stock unchanged

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
    assert(addPizzaToOrder(o, data, pizzaSizes));
    assert(data.pizza_stock == 50);

    assert(changeItemQuantity(o, data, 0, 1));  // 6 pizzas now
    assert(data.pizza_stock == 50);  // Still 50, not deducted

    calculate(o, data);

    // Process payment
    bool ok = processPayment(o, data, PaymentMethod::Cash, o.final_total);
    assert(ok);

    // Now 6 pizzas should be deducted
    assert(data.pizza_stock == 44);
    assert(o.items[0].quantity == 6);

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
    assert(addChickenToOrder(o, data, {1}));  // 1 full chicken
    assert(o.item_count == 2);  // Chicken + free potatoes

    assert(data.chicken_stock == 30);
    assert(data.potatoes_stock == 50);  // Free potatoes not deducted yet

    calculate(o, data);

    // Process payment
    bool ok = processPayment(o, data, PaymentMethod::Cash, o.final_total);
    assert(ok);

    // Now inventory should be deducted for both chicken and free potatoes
    assert(data.chicken_stock == 29);  // 30 - 1
    assert(data.potatoes_stock == 49);  // 50 - 1 (free potatoes)

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
    assert(addPizzaToOrder(o, data, pizzaSizes));
    assert(data.pizza_stock == 50);

    calculate(o, data);

    // Process payment
    bool ok = processPayment(o, data, PaymentMethod::Cash, o.final_total);
    assert(ok);

    // Stock should be exactly 0, not negative
    assert(data.pizza_stock == 0);
    assert(data.pizza_stock >= 0);

    cout << " -> Negative inventory prevented\n";
}

void testCheckoutStockGuard() {
    // Payment should be blocked if stock is insufficient
    RestaurantData data;
    Order o;
    o.customer.order_type = OrderType::Takeaway;

    assert(addBurgerToOrder(o, data, 2, 1));
    assert(addPotatoesToOrder(o, data, 1));
    calculate(o, data);

    assert(hasSufficientStock(o, data));

    // Simulate low stock
    data.burger_stock = -1;
    assert(!hasSufficientStock(o, data));
    assert(!processPayment(o, data, PaymentMethod::Cash, o.final_total));

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

    cout << "\n===== ALL INVENTORY TESTS PASSED! =====\n";
    return 0;
}
