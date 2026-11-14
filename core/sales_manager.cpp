#include "sales_manager.h"
#include "order_manager.h"

bool processPayment(Order& o, RestaurantData& d, PaymentMethod method, double paidAmount) {
    if (!hasSufficientStock(o, d)) {
        return false;
    }

    o.bill.payment = method;

    if (method == PaymentMethod::CreditCard || method == PaymentMethod::Wallet) {
        o.bill.paid_amount = o.final_total;
    } else {
        if (paidAmount < o.final_total) {
            return false; // Insufficient cash payment
        }
        o.bill.paid_amount = paidAmount;
    }

    // Deduct inventory only after successful payment validation
    deductInventory(o, d);

    d.total_orders++;
    d.total_sales += o.final_total;

    if (method == PaymentMethod::Cash)
        d.cash_orders++;
    else if (method == PaymentMethod::CreditCard)
        d.card_orders++;
    else
        d.wallet_orders++;

    return true;
}

double getChangeAmount(const Order& o) {
    return o.bill.paid_amount - o.final_total;
}

double getAverageOrderSales(const RestaurantData& d) {
    return d.total_orders > 0 ? d.total_sales / d.total_orders : 0.00;
}
