#include "payment_dialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QGridLayout>
#include <algorithm>

#include "../../core/sales_manager.h"
#include "card_details_dialog.h"
#include "otp_verification_dialog.h"

PaymentDialog::PaymentDialog(Order& order, RestaurantData& data, QWidget *parent)
    : QDialog(parent), m_order(order), m_data(data)
{
    setWindowTitle("Checkout & Payment");
    setMinimumWidth(520);
    setMinimumHeight(520);
    resize(540, 560);
    setModal(true);
    setupUi();
    applyStylesheet();
}

void PaymentDialog::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(28, 24, 28, 24);
    mainLayout->setSpacing(18);

    // ── Header ──────────────────────────────────────────────────────────────
    QFrame *headerFrame = new QFrame(this);
    headerFrame->setObjectName("payHeader");
    QVBoxLayout *headerLayout = new QVBoxLayout(headerFrame);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(4);

    QLabel *titleLbl = new QLabel("💳 Checkout & Payment", headerFrame);
    titleLbl->setStyleSheet("font-size: 20px; font-weight: bold; color: #1E1E24;");

    QLabel *subLbl = new QLabel(QString("Order #%1  ·  %2 item(s)")
        .arg(m_order.order_id)
        .arg(m_order.item_count), headerFrame);
    subLbl->setStyleSheet("font-size: 13px; color: #666666;");

    headerLayout->addWidget(titleLbl);
    headerLayout->addWidget(subLbl);
    mainLayout->addWidget(headerFrame);

    QFrame *divider = new QFrame(this);
    divider->setFrameShape(QFrame::HLine);
    divider->setStyleSheet("background-color: #E5E7EB; max-height: 1px;");
    mainLayout->addWidget(divider);

    // ── Order Total Summary ──────────────────────────────────────────────────
    QFrame *totalCard = new QFrame(this);
    totalCard->setObjectName("totalCard");
    QHBoxLayout *totalLayout = new QHBoxLayout(totalCard);
    totalLayout->setContentsMargins(20, 14, 20, 14);

    QLabel *totalLbl = new QLabel("Order Total (incl. tax & fees):", totalCard);
    totalLbl->setStyleSheet("font-size: 14px; font-weight: 600; color: #374151;");

    totalValueLabel = new QLabel(QString("$%1").arg(m_order.final_total, 0, 'f', 2), totalCard);
    totalValueLabel->setStyleSheet("font-size: 22px; font-weight: bold; color: #D32F2F;");
    totalValueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    totalLayout->addWidget(totalLbl);
    totalLayout->addStretch();
    totalLayout->addWidget(totalValueLabel);
    mainLayout->addWidget(totalCard);

    // ── Payment Method Selection ─────────────────────────────────────────────
    QFrame *methodCard = new QFrame(this);
    methodCard->setObjectName("methodCard");
    QVBoxLayout *methodLayout = new QVBoxLayout(methodCard);
    methodLayout->setContentsMargins(20, 16, 20, 16);
    methodLayout->setSpacing(12);

    QLabel *methodTitle = new QLabel("Select Payment Method:", methodCard);
    methodTitle->setStyleSheet("font-size: 14px; font-weight: 600; color: #1E1E24;");
    methodLayout->addWidget(methodTitle);

    methodGroup = new QButtonGroup(this);

    QHBoxLayout *radioRow = new QHBoxLayout();
    radioRow->setSpacing(20);

    cashRadio   = new QRadioButton("💵  Cash",        methodCard);
    cardRadio   = new QRadioButton("💳  Credit Card", methodCard);
    walletRadio = new QRadioButton("👛  Wallet",       methodCard);

    cashRadio->setChecked(true);
    for (auto *r : {cashRadio, cardRadio, walletRadio}) {
        r->setCursor(Qt::PointingHandCursor);
        r->setStyleSheet("font-size: 14px; font-weight: 600; color: #1E1E24; spacing: 8px;");
    }

    methodGroup->addButton(cashRadio,   static_cast<int>(PaymentMethod::Cash));
    methodGroup->addButton(cardRadio,   static_cast<int>(PaymentMethod::CreditCard));
    methodGroup->addButton(walletRadio, static_cast<int>(PaymentMethod::Wallet));

    radioRow->addWidget(cashRadio);
    radioRow->addWidget(cardRadio);
    radioRow->addWidget(walletRadio);
    radioRow->addStretch();
    methodLayout->addLayout(radioRow);
    mainLayout->addWidget(methodCard);

    // ── Cash Input (shown only for Cash) ────────────────────────────────────
    cashInputWidget = new QWidget(this);
    QVBoxLayout *cashLayout = new QVBoxLayout(cashInputWidget);
    cashLayout->setContentsMargins(0, 0, 0, 0);
    cashLayout->setSpacing(10);

    // Paid amount row
    QHBoxLayout *paidRow = new QHBoxLayout();
    QLabel *paidLbl = new QLabel("Paid Amount ($):", cashInputWidget);
    paidLbl->setStyleSheet("font-size: 13px; font-weight: 600; color: #374151;");

    paidAmountSpin = new QDoubleSpinBox(cashInputWidget);
    paidAmountSpin->setRange(0.00, 999999.00);
    paidAmountSpin->setDecimals(2);
    paidAmountSpin->setSingleStep(10.00);
    paidAmountSpin->setValue(m_order.final_total);   // default to exact total
    paidAmountSpin->setFixedHeight(40);
    paidAmountSpin->setFixedWidth(160);
    paidAmountSpin->setStyleSheet(
        "QDoubleSpinBox { background-color: #FFFFFF; color: #1E1E24; border: 1px solid #D1D5DB; border-radius: 6px; padding: 4px 8px; font-size: 14px; font-weight: bold; }"
        "QDoubleSpinBox:focus { border: 2px solid #D32F2F; }"
    );

    paidRow->addWidget(paidLbl);
    paidRow->addStretch();
    paidRow->addWidget(paidAmountSpin);
    cashLayout->addLayout(paidRow);

    // Change row
    QHBoxLayout *changeRow = new QHBoxLayout();
    changeLabel = new QLabel("Change:", cashInputWidget);
    changeLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #374151;");

    changeValueLabel = new QLabel("$0.00", cashInputWidget);
    changeValueLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #2E7D32;");
    changeValueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    changeRow->addWidget(changeLabel);
    changeRow->addStretch();
    changeRow->addWidget(changeValueLabel);
    cashLayout->addLayout(changeRow);

    // Insufficient warning
    insufficientLabel = new QLabel("⚠  Insufficient payment — please enter at least the order total.", cashInputWidget);
    insufficientLabel->setStyleSheet("font-size: 12px; color: #C62828; font-weight: 600;");
    insufficientLabel->setWordWrap(true);
    insufficientLabel->setVisible(false);
    cashLayout->addWidget(insufficientLabel);

    mainLayout->addWidget(cashInputWidget);

    // ── Action Buttons ───────────────────────────────────────────────────────
    mainLayout->addStretch();

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(14);

    cancelBtn = new QPushButton("✖  Cancel", this);
    cancelBtn->setObjectName("btnPayCancel");
    cancelBtn->setFixedHeight(46);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    confirmBtn = new QPushButton("✔  Confirm Order", this);
    confirmBtn->setObjectName("btnPayConfirm");
    confirmBtn->setFixedHeight(46);
    confirmBtn->setCursor(Qt::PointingHandCursor);
    connect(confirmBtn, &QPushButton::clicked, this, &PaymentDialog::onConfirmClicked);

    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(confirmBtn, 1);
    mainLayout->addLayout(btnLayout);

    // ── Signal connections ───────────────────────────────────────────────────
    connect(methodGroup, QOverload<int>::of(&QButtonGroup::idClicked),
            this, &PaymentDialog::onPaymentMethodChanged);

    connect(paidAmountSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PaymentDialog::onPaidAmountChanged);

    updateChangeDisplay();
}

void PaymentDialog::onPaymentMethodChanged(int id) {
    bool isCash = (id == static_cast<int>(PaymentMethod::Cash));
    cashInputWidget->setVisible(isCash);
    insufficientLabel->setVisible(false);
    updateChangeDisplay();
}

void PaymentDialog::onPaidAmountChanged(double /*value*/) {
    updateChangeDisplay();
}

void PaymentDialog::updateChangeDisplay() {
    bool isCash = (methodGroup->checkedId() == static_cast<int>(PaymentMethod::Cash));
    if (!isCash) return;

    double paid   = paidAmountSpin->value();
    double total  = m_order.final_total;
    double change = paid - total;

    bool insufficient = (paid < total - 0.001);
    insufficientLabel->setVisible(insufficient);

    if (insufficient) {
        changeValueLabel->setText("—");
        changeValueLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #C62828;");
        confirmBtn->setEnabled(false);
    } else {
        changeValueLabel->setText(QString("$%1").arg(change, 0, 'f', 2));
        changeValueLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #2E7D32;");
        confirmBtn->setEnabled(true);
    }
}

void PaymentDialog::onConfirmClicked() {
    if (m_confirmed) return;   // prevent double-click / double-submission

    PaymentMethod method = static_cast<PaymentMethod>(methodGroup->checkedId());

    // For Credit Card, show the card details and OTP flow
    if (method == PaymentMethod::CreditCard) {
        if (!processCreditCardPayment()) {
            return;
        }
    } else {
        // For Cash and Wallet, process normally
        double paidAmount = (method == PaymentMethod::Cash) ? paidAmountSpin->value() : 0.0;

        bool ok = processPayment(m_order, m_data, method, paidAmount);

        if (!ok) {
            // processPayment returns false only when cash is insufficient
            insufficientLabel->setVisible(true);
            confirmBtn->setEnabled(false);
            return;
        }
    }

    m_confirmed = true;
    confirmBtn->setEnabled(false);    // prevent any further clicks
    accept();
}

bool PaymentDialog::processCreditCardPayment() {
    // Step 1: Show Card Details Dialog
    CardDetailsDialog cardDialog(m_order.final_total, this);
    if (cardDialog.exec() != QDialog::Accepted) {
        // User cancelled the card details entry
        return false;
    }

    // Step 2: Show OTP Verification Dialog
    OtpVerificationDialog otpDialog(m_order.final_total, this);
    if (otpDialog.exec() != QDialog::Accepted) {
        // User cancelled or entered wrong OTP
        return false;
    }

    if (!otpDialog.otpVerified()) {
        // OTP verification failed
        return false;
    }

    // Step 3: Process payment through core (inventory deduction happens here)
    bool ok = processPayment(m_order, m_data, PaymentMethod::CreditCard, 0.0);

    if (!ok) {
        return false;
    }

    return true;
}

void PaymentDialog::applyStylesheet() {
    setStyleSheet(R"(
        QDialog {
            background-color: #FFFFFF;
        }

        QLabel {
            color: #1E1E24;
            background-color: transparent;
        }

        #payHeader {
            background-color: transparent;
        }

        #totalCard {
            background-color: #FEF2F2;
            border: 1px solid #FCA5A5;
            border-radius: 10px;
        }

        #methodCard {
            background-color: #F8F9FA;
            border: 1px solid #E5E7EB;
            border-radius: 10px;
        }

        QRadioButton::indicator {
            width: 18px;
            height: 18px;
            border-radius: 9px;
            border: 2px solid #9CA3AF;
            background-color: #FFFFFF;
        }

        QRadioButton::indicator:checked {
            border: 2px solid #D32F2F;
            background-color: #D32F2F;
        }

        #btnPayCancel {
            background-color: #F3F4F6;
            color: #374151;
            border: 1px solid #D1D5DB;
            border-radius: 8px;
            font-size: 14px;
            font-weight: 600;
        }

        #btnPayCancel:hover {
            background-color: #E5E7EB;
        }

        #btnPayConfirm {
            background-color: #D32F2F;
            color: #FFFFFF;
            border: none;
            border-radius: 8px;
            font-size: 15px;
            font-weight: bold;
        }

        #btnPayConfirm:hover {
            background-color: #B71C1C;
        }

        #btnPayConfirm:disabled {
            background-color: #CCCCCC;
            color: #888888;
        }
    )");
}
