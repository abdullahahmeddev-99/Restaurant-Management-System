#include <QApplication>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QTableWidget>
#include <QTimer>
#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "core/meal_logic.h"
#include "core/order_manager.h"
#include "core/sales_manager.h"
#include "gui/dialogs/payment_dialog.h"
#include "gui/dialogs/card_details_dialog.h"
#include "gui/dialogs/otp_verification_dialog.h"
#include "gui/dialogs/invoice_dialog.h"

namespace {
int g_failures = 0;

void check(bool condition, const char* message) {
    if (condition) {
        std::printf("  ok   %s\n", message);
    } else {
        std::printf("  FAIL %s\n", message);
        ++g_failures;
    }
}

bool near(double lhs, double rhs) { return std::fabs(lhs - rhs) < 0.01; }

Order makePizzaOrder(RestaurantData& data) {
    Order order;
    order.customer.order_type = OrderType::Takeaway;
    // Distinct sizes prove the invoice/cart detail carries actual menu prices.
    check(addPizzaToOrder(order, data, {1, 2}), "add Large + Medium pizza");
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

QWidget* activeModalDialog() {
    QWidget* modal = QApplication::activeModalWidget();
    if (modal) return modal;
    for (QWidget* widget : QApplication::topLevelWidgets())
        if (widget->isVisible() && qobject_cast<QDialog*>(widget)) return widget;
    return nullptr;
}

// The credit-card flow opens modal dialogs with exec(), which blocks the
// caller. This handler runs on a timer inside those nested event loops and
// fills/confirms each dialog as soon as it appears.
void driveModalDialogs() {
    QWidget* modal = activeModalDialog();
    if (auto* card = qobject_cast<CardDetailsDialog*>(modal)) {
        auto* number = card->findChild<QLineEdit*>("cardNumberEdit");
        if (number && number->text().isEmpty()) {
            number->setText("4532123456789012");
            card->findChild<QLineEdit*>("cardholderNameEdit")->setText("Test User");
            card->findChild<QLineEdit*>("expiryDateEdit")->setText("12/30");
            card->findChild<QLineEdit*>("cvvEdit")->setText("123");
            if (QPushButton* verify = buttonWithText(*card, "Verify Card"))
                verify->click();
        }
    } else if (auto* otp = qobject_cast<OtpVerificationDialog*>(modal)) {
        auto* input = otp->findChild<QLineEdit*>("otpInputEdit");
        if (input && input->text().isEmpty()) {
            input->setText(otp->getGeneratedOtp());
            if (QPushButton* verify = buttonWithText(*otp, "Verify Payment"))
                verify->click();
        }
    }
}
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    RestaurantData data;
    constexpr double total = 456.00; // Pizza L + M = $400; 14% tax = $56.

    std::printf("RUNNING GUI CHECKOUT TESTS\n");

    // 1. Exact cash payment.
    Order exact = makePizzaOrder(data);
    check(data.pizza_stock == 50, "stock not deducted during add");
    PaymentDialog exactDialog(exact, data);
    buttonWithText(exactDialog, "Confirm Order")->click();
    check(exactDialog.paymentConfirmed() && near(exact.bill.paid_amount, total),
          "exact cash confirms payment");
    check(data.pizza_stock == 48, "stock deducted after payment (2 pizzas)");
    check(data.total_orders == 1 && data.cash_orders == 1 && near(data.total_sales, total),
          "cash counters updated");

    // 2. Overpayment produces change.
    Order over = makePizzaOrder(data);
    check(data.pizza_stock == 48, "stock unchanged by add");
    PaymentDialog overDialog(over, data);
    overDialog.findChild<QDoubleSpinBox*>()->setValue(500.00);
    buttonWithText(overDialog, "Confirm Order")->click();
    check(overDialog.paymentConfirmed() && near(getChangeAmount(over), 44.00),
          "overpayment yields $44 change");
    check(data.pizza_stock == 46, "stock deducted after payment");
    check(data.total_orders == 2 && data.cash_orders == 2 && near(data.total_sales, total * 2),
          "cash counters updated");

    // 3. Insufficient cash does not complete or alter counters.
    Order insufficient = makePizzaOrder(data);
    check(data.pizza_stock == 46, "stock unchanged by add");
    PaymentDialog insufficientDialog(insufficient, data);
    insufficientDialog.findChild<QDoubleSpinBox*>()->setValue(455.99);
    QApplication::processEvents();
    check(!buttonWithText(insufficientDialog, "Confirm Order")->isEnabled(),
          "confirm disabled for insufficient cash");
    buttonWithText(insufficientDialog, "Confirm Order")->click();
    check(!insufficientDialog.paymentConfirmed(), "insufficient cash not confirmed");
    check(data.pizza_stock == 46, "stock unchanged after failed payment");
    check(data.total_orders == 2 && data.cash_orders == 2 && near(data.total_sales, total * 2),
          "counters unchanged after failed payment");

    // 4. Credit card flow (card details + OTP are modal).
    Order card = makePizzaOrder(data);
    check(data.pizza_stock == 46, "stock unchanged by add");
    PaymentDialog cardDialog(card, data);
    radioWithText(cardDialog, "Credit Card")->click();

    QTimer driver;
    driver.setInterval(10);
    QObject::connect(&driver, &QTimer::timeout, [] { driveModalDialogs(); });
    driver.start();
    buttonWithText(cardDialog, "Confirm Order")->click();
    driver.stop();

    check(cardDialog.paymentConfirmed() && card.bill.payment == PaymentMethod::CreditCard &&
              near(card.bill.paid_amount, total),
          "credit card confirms payment");
    check(data.pizza_stock == 44, "stock deducted after card payment");
    check(data.total_orders == 3 && data.card_orders == 1 && near(data.total_sales, total * 3),
          "card counters updated");

    // 5. Wallet uses the full Core total.
    Order wallet = makePizzaOrder(data);
    check(data.pizza_stock == 44, "stock unchanged by add");
    PaymentDialog walletDialog(wallet, data);
    radioWithText(walletDialog, "Wallet")->click();
    buttonWithText(walletDialog, "Confirm Order")->click();
    check(walletDialog.paymentConfirmed() && wallet.bill.payment == PaymentMethod::Wallet &&
              near(wallet.bill.paid_amount, total),
          "wallet confirms payment");
    check(data.pizza_stock == 42, "stock deducted after wallet payment");
    check(data.total_orders == 4 && data.wallet_orders == 1 && near(data.total_sales, total * 4),
          "wallet counters updated");

    // 6. Two UI clicks still produce one completed payment.
    Order rapid = makePizzaOrder(data);
    check(data.pizza_stock == 42, "stock unchanged by add");
    PaymentDialog rapidDialog(rapid, data);
    QPushButton* rapidConfirm = buttonWithText(rapidDialog, "Confirm Order");
    rapidConfirm->click();
    rapidConfirm->click();
    check(rapidDialog.paymentConfirmed(), "rapid clicks still confirm once");
    check(data.pizza_stock == 40, "stock deducted only once");
    check(data.total_orders == 5 && data.cash_orders == 3 && near(data.total_sales, total * 5),
          "rapid click counters correct");

    // 7. Cancel does not complete or change counters.
    Order cancelled = makePizzaOrder(data);
    check(data.pizza_stock == 40, "stock unchanged by add");
    PaymentDialog cancelDialog(cancelled, data);
    buttonWithText(cancelDialog, "Cancel")->click();
    check(!cancelDialog.paymentConfirmed(), "cancel does not confirm payment");
    check(data.pizza_stock == 40, "stock unchanged after cancel");
    check(data.total_orders == 5 && near(data.total_sales, total * 5),
          "counters unchanged after cancel");

    // 8. Invoice shows Core totals plus distinct pizza-size details.
    InvoiceDialog invoice(exact);
    QTableWidget* items = invoice.findChild<QTableWidget*>();
    check(items && items->rowCount() == 1, "invoice has one row");
    if (items && items->rowCount() == 1) {
        check(items->item(0, 0)->text().contains("Large x1 @ $250.00"),
              "invoice shows Large pizza price");
        check(items->item(0, 0)->text().contains("Medium x1 @ $150.00"),
              "invoice shows Medium pizza price");
        check(items->item(0, 2)->text() == "$400.00", "invoice line total is $400.00");
    }

    if (g_failures != 0) {
        std::printf("GUI CHECKOUT TESTS FAILED (%d)\n", g_failures);
        return 1;
    }
    std::printf("ALL GUI CHECKOUT TESTS PASSED\n");
    return 0;
}
