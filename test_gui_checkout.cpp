#include <QApplication>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QRadioButton>
#include <QTableWidget>
#include <QLineEdit>
#include <cassert>
#include <cmath>

#include "core/meal_logic.h"
#include "core/order_manager.h"
#include "core/sales_manager.h"
#include "gui/dialogs/payment_dialog.h"
#include "gui/dialogs/card_details_dialog.h"
#include "gui/dialogs/otp_verification_dialog.h"
#include "gui/dialogs/invoice_dialog.h"

namespace {
bool near(double lhs, double rhs) { return std::fabs(lhs - rhs) < 0.01; }

Order makePizzaOrder(RestaurantData& data) {
    Order order;
    order.customer.order_type = OrderType::Takeaway;
    // Distinct sizes prove the invoice/cart detail carries actual menu prices.
    assert(addPizzaToOrder(order, data, {1, 2}));
    calculate(order, data);
    return order;
}

QPushButton* buttonWithText(QWidget& widget, const QString& text) {
    for (QPushButton* button : widget.findChildren<QPushButton*>())
        if (button->text().contains(text)) return button;
    return nullptr;
}

QRadioButton* radioWithText(QWidget& widget, const QString& text) {
    for (QRadioButton* radio : widget.findChildren<QRadioButton*>())
        if (radio->text().contains(text)) return radio;
    return nullptr;
}
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    RestaurantData data;
    constexpr double total = 456.00; // Pizza L + M = $400; 14% tax = $56.

    // 1. Exact cash and 2. overpayment/change.
    Order exact = makePizzaOrder(data);
    assert(data.pizza_stock == 50);  // Stock not deducted during add
    PaymentDialog exactDialog(exact, data);
    buttonWithText(exactDialog, "Confirm Order")->click();
    assert(exactDialog.paymentConfirmed() && near(exact.bill.paid_amount, total));
    assert(data.pizza_stock == 48);  // Stock deducted after payment (2 pizzas)
    assert(data.total_orders == 1 && data.cash_orders == 1 && near(data.total_sales, total));

    Order over = makePizzaOrder(data);
    assert(data.pizza_stock == 48);  // Stock unchanged by add
    PaymentDialog overDialog(over, data);
    overDialog.findChild<QDoubleSpinBox*>()->setValue(500.00);
    buttonWithText(overDialog, "Confirm Order")->click();
    assert(overDialog.paymentConfirmed() && near(getChangeAmount(over), 44.00));
    assert(data.pizza_stock == 46);  // Stock deducted after payment (2 more pizzas)
    assert(data.total_orders == 2 && data.cash_orders == 2 && near(data.total_sales, total * 2));

    // 3. Insufficient cash does not complete or alter counters.
    Order insufficient = makePizzaOrder(data);
    assert(data.pizza_stock == 46);  // Stock unchanged by add
    PaymentDialog insufficientDialog(insufficient, data);
    insufficientDialog.findChild<QDoubleSpinBox*>()->setValue(455.99);
    QApplication::processEvents();
    assert(!buttonWithText(insufficientDialog, "Confirm Order")->isEnabled());
    buttonWithText(insufficientDialog, "Confirm Order")->click();
    assert(!insufficientDialog.paymentConfirmed());
    assert(data.pizza_stock == 46);  // Stock unchanged (payment failed)
    assert(data.total_orders == 2 && data.cash_orders == 2 && near(data.total_sales, total * 2));

    // 4. Credit card and 5. wallet use the full Core total.
    Order card = makePizzaOrder(data);
    assert(data.pizza_stock == 46);  // Stock unchanged by add
    PaymentDialog cardDialog(card, data);
    radioWithText(cardDialog, "Credit Card")->click();
    buttonWithText(cardDialog, "Confirm Order")->click();
    
    // Simulate Card Details Dialog
    CardDetailsDialog* cardDetailsDlg = cardDialog.findChild<CardDetailsDialog*>();
    if (cardDetailsDlg) {
        cardDetailsDlg->findChild<QLineEdit*>("cardNumberEdit")->setText("4532123456789012");
        cardDetailsDlg->findChild<QLineEdit*>("cardholderNameEdit")->setText("Test User");
        cardDetailsDlg->findChild<QLineEdit*>("expiryDateEdit")->setText("12/25");
        cardDetailsDlg->findChild<QLineEdit*>("cvvEdit")->setText("123");
        buttonWithText(*cardDetailsDlg, "Verify Card")->click();
        QApplication::processEvents();
    }
    
    // Simulate OTP Verification Dialog
    OtpVerificationDialog* otpDlg = cardDialog.findChild<OtpVerificationDialog*>();
    if (otpDlg) {
        QString otp = otpDlg->getGeneratedOtp();
        otpDlg->findChild<QLineEdit*>("otpInputEdit")->setText(otp);
        buttonWithText(*otpDlg, "Verify Payment")->click();
        QApplication::processEvents();
    }
    
    assert(cardDialog.paymentConfirmed() && card.bill.payment == PaymentMethod::CreditCard && near(card.bill.paid_amount, total));
    assert(data.pizza_stock == 44);  // Stock deducted after payment
    assert(data.total_orders == 3 && data.card_orders == 1 && near(data.total_sales, total * 3));

    Order wallet = makePizzaOrder(data);
    assert(data.pizza_stock == 44);  // Stock unchanged by add
    PaymentDialog walletDialog(wallet, data);
    radioWithText(walletDialog, "Wallet")->click();
    buttonWithText(walletDialog, "Confirm Order")->click();
    assert(walletDialog.paymentConfirmed() && wallet.bill.payment == PaymentMethod::Wallet && near(wallet.bill.paid_amount, total));
    assert(data.pizza_stock == 42);  // Stock deducted after payment
    assert(data.total_orders == 4 && data.wallet_orders == 1 && near(data.total_sales, total * 4));

    // 6. Two UI clicks still produce one completed payment.
    Order rapid = makePizzaOrder(data);
    assert(data.pizza_stock == 42);  // Stock unchanged by add
    PaymentDialog rapidDialog(rapid, data);
    QPushButton* rapidConfirm = buttonWithText(rapidDialog, "Confirm Order");
    rapidConfirm->click();
    rapidConfirm->click();
    assert(rapidDialog.paymentConfirmed());
    assert(data.pizza_stock == 40);  // Stock deducted only once
    assert(data.total_orders == 5 && data.cash_orders == 3 && near(data.total_sales, total * 5));

    // 7. Cancel does not complete or change counters.
    Order cancelled = makePizzaOrder(data);
    assert(data.pizza_stock == 40);  // Stock unchanged by add
    PaymentDialog cancelDialog(cancelled, data);
    buttonWithText(cancelDialog, "Cancel")->click();
    assert(!cancelDialog.paymentConfirmed());
    assert(data.pizza_stock == 40);  // Stock unchanged (payment canceled)
    assert(data.total_orders == 5 && near(data.total_sales, total * 5));

    // 8. Invoice shows Core totals plus distinct pizza-size details, not an average unit price.
    InvoiceDialog invoice(exact);
    QTableWidget* items = invoice.findChild<QTableWidget*>();
    assert(items && items->rowCount() == 1);
    assert(items->item(0, 0)->text().contains("Large x1 @ $250.00"));
    assert(items->item(0, 0)->text().contains("Medium x1 @ $150.00"));
    assert(items->item(0, 2)->text() == "$400.00");

    return 0;
}
