#include "new_order_view.h"
#include "../core/order_manager.h"
#include "../core/validation.h"
#include "../core/sales_manager.h"
#include "dialogs/payment_dialog.h"
#include "dialogs/invoice_dialog.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QHeaderView>
#include <QMessageBox>
#include <QScrollArea>

NewOrderView::NewOrderView(RestaurantData& data, Order& activeOrder, QWidget *parent)
    : QWidget(parent), m_data(data), m_order(activeOrder)
{
    setupUi();
    applyViewStylesheet();
    refreshUi();
}

void NewOrderView::setupUi() {
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 24);
    mainLayout->setSpacing(24);

    // Left Column: Customer Form Panel
    QWidget *formPanel = createCustomerFormPanel();
    mainLayout->addWidget(formPanel, 5);

    // Right Column: Cart & Summary Panel
    QWidget *cartPanel = createCartSummaryPanel();
    mainLayout->addWidget(cartPanel, 7);
}

QWidget* NewOrderView::createCustomerFormPanel() {
    QFrame *panel = new QFrame(this);
    panel->setObjectName("customerPanel");
    panel->setStyleSheet("QFrame#customerPanel { background-color: #FFFFFF; border: 1px solid #E4E4E7; border-radius: 12px; }");

    QVBoxLayout *layout = new QVBoxLayout(panel);
    layout->setContentsMargins(24, 20, 24, 24);
    layout->setSpacing(18);

    // Header Title
    QLabel *header = new QLabel("👤 Customer & Order Details", panel);
    header->setStyleSheet("font-size: 18px; font-weight: bold; color: #1E1E24;");
    layout->addWidget(header);

    QFrame *div = new QFrame(panel);
    div->setFrameShape(QFrame::HLine);
    div->setStyleSheet("background-color: #E5E7EB; max-height: 1px; min-height: 1px;");
    layout->addWidget(div);

    // 1. Customer Name Input
    QVBoxLayout *nameLayout = new QVBoxLayout();
    nameLayout->setSpacing(4);

    QHBoxLayout *nameHeader = new QHBoxLayout();
    QLabel *nameLabel = new QLabel("Customer Name *", panel);
    nameLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #1E1E24;");

    nameValidationLabel = new QLabel(panel);
    nameValidationLabel->setStyleSheet("font-size: 11px; font-weight: 600; color: #C62828;");
    nameValidationLabel->setAlignment(Qt::AlignRight);

    nameHeader->addWidget(nameLabel);
    nameHeader->addWidget(nameValidationLabel);
    nameLayout->addLayout(nameHeader);

    nameEdit = new QLineEdit(panel);
    nameEdit->setPlaceholderText("e.g. John Doe");
    nameEdit->setFixedHeight(40);
    connect(nameEdit, &QLineEdit::textChanged, this, &NewOrderView::onNameChanged);
    nameLayout->addWidget(nameEdit);
    layout->addLayout(nameLayout);

    // 2. Customer Phone Input
    QVBoxLayout *phoneLayout = new QVBoxLayout();
    phoneLayout->setSpacing(4);

    QHBoxLayout *phoneHeader = new QHBoxLayout();
    QLabel *phoneLabel = new QLabel("Phone Number *", panel);
    phoneLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #1E1E24;");

    phoneValidationLabel = new QLabel(panel);
    phoneValidationLabel->setStyleSheet("font-size: 11px; font-weight: 600; color: #C62828;");
    phoneValidationLabel->setAlignment(Qt::AlignRight);

    phoneHeader->addWidget(phoneLabel);
    phoneHeader->addWidget(phoneValidationLabel);
    phoneLayout->addLayout(phoneHeader);

    phoneEdit = new QLineEdit(panel);
    phoneEdit->setPlaceholderText("e.g. 01012345678");
    phoneEdit->setFixedHeight(40);
    connect(phoneEdit, &QLineEdit::textChanged, this, &NewOrderView::onPhoneChanged);
    phoneLayout->addWidget(phoneEdit);
    layout->addLayout(phoneLayout);

    // 3. Order Type Radio Selection
    QVBoxLayout *typeLayout = new QVBoxLayout();
    typeLayout->setSpacing(8);

    QLabel *typeLabel = new QLabel("Order Type *", panel);
    typeLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #1E1E24;");
    typeLayout->addWidget(typeLabel);

    orderTypeGroup = new QButtonGroup(this);

    QHBoxLayout *radioRow = new QHBoxLayout();
    radioRow->setSpacing(12);

    dineInRadio = new QRadioButton("Dine-in 🍽️", panel);
    takeawayRadio = new QRadioButton("Takeaway 🛍️", panel);
    deliveryRadio = new QRadioButton("Delivery 🛵", panel);

    dineInRadio->setCursor(Qt::PointingHandCursor);
    takeawayRadio->setCursor(Qt::PointingHandCursor);
    deliveryRadio->setCursor(Qt::PointingHandCursor);

    orderTypeGroup->addButton(dineInRadio, 1);
    orderTypeGroup->addButton(takeawayRadio, 2);
    orderTypeGroup->addButton(deliveryRadio, 3);

    radioRow->addWidget(dineInRadio);
    radioRow->addWidget(takeawayRadio);
    radioRow->addWidget(deliveryRadio);
    typeLayout->addLayout(radioRow);

    connect(orderTypeGroup, QOverload<int>::of(&QButtonGroup::idClicked), this, &NewOrderView::onOrderTypeChanged);
    layout->addLayout(typeLayout);

    // 4. Dynamic Table Number Container (Visible only for Dine-in)
    tableNumberContainer = new QWidget(panel);
    QHBoxLayout *tableLayout = new QHBoxLayout(tableNumberContainer);
    tableLayout->setContentsMargins(0, 0, 0, 0);
    tableLayout->setSpacing(12);

    QLabel *tableLabel = new QLabel("Table Number (1-100):", tableNumberContainer);
    tableLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #1E1E24;");

    tableNumberSpin = new QSpinBox(tableNumberContainer);
    tableNumberSpin->setRange(1, 100);
    tableNumberSpin->setValue(1);
    tableNumberSpin->setFixedHeight(38);
    tableNumberSpin->setFixedWidth(100);
    connect(tableNumberSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &NewOrderView::onTableNumberChanged);

    tableLayout->addWidget(tableLabel);
    tableLayout->addWidget(tableNumberSpin);
    tableLayout->addStretch();

    tableNumberContainer->setVisible(false);
    layout->addWidget(tableNumberContainer);

    // 5. Discount Card Option
    discountCardCheckBox = new QCheckBox("Customer possesses 5% Discount Card 💳", panel);
    discountCardCheckBox->setCursor(Qt::PointingHandCursor);
    discountCardCheckBox->setStyleSheet("font-size: 13px; font-weight: 600; color: #1E1E24;");
    connect(discountCardCheckBox, &QCheckBox::toggled, this, &NewOrderView::onDiscountCardToggled);
    layout->addWidget(discountCardCheckBox);

    layout->addStretch();

    // Cancel Order Button
    cancelOrderBtn = new QPushButton("🗑️ Cancel Order", panel);
    cancelOrderBtn->setObjectName("btnCancelOrder");
    cancelOrderBtn->setCursor(Qt::PointingHandCursor);
    cancelOrderBtn->setFixedHeight(44);
    connect(cancelOrderBtn, &QPushButton::clicked, this, &NewOrderView::onCancelOrderClicked);
    layout->addWidget(cancelOrderBtn);

    return panel;
}

QWidget* NewOrderView::createCartSummaryPanel() {
    QFrame *panel = new QFrame(this);
    panel->setObjectName("cartPanel");
    panel->setStyleSheet("QFrame#cartPanel { background-color: #FFFFFF; border: 1px solid #E4E4E7; border-radius: 12px; }");

    QVBoxLayout *layout = new QVBoxLayout(panel);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(14);

    QLabel *header = new QLabel("🛒 Active Order Cart", panel);
    header->setStyleSheet("font-size: 18px; font-weight: bold; color: #1E1E24;");
    layout->addWidget(header);

    // Dynamic Stacked State: Empty Cart vs Cart Content
    cartContentWidget = new QWidget(panel);
    QVBoxLayout *contentLayout = new QVBoxLayout(cartContentWidget);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(12);

    // Cart Items Table
    cartTable = new QTableWidget(0, 6, cartContentWidget);
    cartTable->setObjectName("cartTable");
    cartTable->setHorizontalHeaderLabels({"Item", "Details", "Qty", "Unit Price", "Total", "Action"});
    cartTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    cartTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    cartTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    cartTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    cartTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    cartTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    cartTable->verticalHeader()->setVisible(false);
    cartTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    cartTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    cartTable->setAlternatingRowColors(true);
    contentLayout->addWidget(cartTable, 1);

    // Empty Cart State Overlay Widget
    emptyCartStateWidget = new QWidget(panel);
    QVBoxLayout *emptyLayout = new QVBoxLayout(emptyCartStateWidget);
    emptyLayout->setContentsMargins(24, 32, 24, 32);
    emptyLayout->setSpacing(12);
    emptyLayout->setAlignment(Qt::AlignCenter);

    QLabel *emptyIcon = new QLabel("🛒", emptyCartStateWidget);
    emptyIcon->setStyleSheet("font-size: 48px; background-color: #F8F9FA; border-radius: 40px; padding: 12px;");
    emptyIcon->setAlignment(Qt::AlignCenter);

    QLabel *emptyTitle = new QLabel("Your order is empty", emptyCartStateWidget);
    emptyTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #1E1E24;");
    emptyTitle->setAlignment(Qt::AlignCenter);

    QLabel *emptySub = new QLabel("Add delicious items from the Menu to start an order.", emptyCartStateWidget);
    emptySub->setStyleSheet("font-size: 13px; color: #666666;");
    emptySub->setAlignment(Qt::AlignCenter);

    browseMenuBtn = new QPushButton("🍕 Browse Menu", emptyCartStateWidget);
    browseMenuBtn->setObjectName("btnBrowseMenu");
    browseMenuBtn->setCursor(Qt::PointingHandCursor);
    browseMenuBtn->setFixedHeight(42);
    browseMenuBtn->setFixedWidth(160);
    connect(browseMenuBtn, &QPushButton::clicked, [this]() {
        emit navigateToMenuRequested();
    });

    emptyLayout->addWidget(emptyIcon);
    emptyLayout->addWidget(emptyTitle);
    emptyLayout->addWidget(emptySub);
    emptyLayout->addWidget(browseMenuBtn, 0, Qt::AlignCenter);

    layout->addWidget(emptyCartStateWidget);
    layout->addWidget(cartContentWidget, 1);

    // Calculations Summary Card
    QFrame *calcFrame = new QFrame(panel);
    calcFrame->setStyleSheet("background-color: #F8F9FA; border: 1px solid #E5E7EB; border-radius: 8px; padding: 12px;");
    QVBoxLayout *calcLayout = new QVBoxLayout(calcFrame);
    calcLayout->setSpacing(6);

    // Subtotal Row
    QHBoxLayout *subRow = new QHBoxLayout();
    QLabel *subtotalTitle = new QLabel("Subtotal:", calcFrame);
    subtotalTitle->setObjectName("summaryLabel");
    subRow->addWidget(subtotalTitle);
    subtotalLabel = new QLabel("$0.00", calcFrame);
    subtotalLabel->setStyleSheet("font-weight: 600; color: #1E1E24;");
    subtotalLabel->setAlignment(Qt::AlignRight);
    subRow->addWidget(subtotalLabel);
    calcLayout->addLayout(subRow);

    // Discount Row
    QHBoxLayout *discRow = new QHBoxLayout();
    QLabel *discountTitle = new QLabel("Discount Offers:", calcFrame);
    discountTitle->setObjectName("summaryLabel");
    discRow->addWidget(discountTitle);
    discountLabel = new QLabel("-$0.00", calcFrame);
    discountLabel->setStyleSheet("color: #2E7D32; font-weight: bold;");
    discountLabel->setAlignment(Qt::AlignRight);
    discRow->addWidget(discountLabel);
    calcLayout->addLayout(discRow);

    // Delivery Fee Row
    deliveryFeeRowWidget = new QWidget(calcFrame);
    QHBoxLayout *delRow = new QHBoxLayout(deliveryFeeRowWidget);
    delRow->setContentsMargins(0, 0, 0, 0);
    QLabel *deliveryFeeTitle = new QLabel("Delivery Fee:", deliveryFeeRowWidget);
    deliveryFeeTitle->setObjectName("summaryLabel");
    delRow->addWidget(deliveryFeeTitle);
    deliveryFeeLabel = new QLabel("$0.00", deliveryFeeRowWidget);
    deliveryFeeLabel->setStyleSheet("font-weight: 600; color: #1E1E24;");
    deliveryFeeLabel->setAlignment(Qt::AlignRight);
    delRow->addWidget(deliveryFeeLabel);
    calcLayout->addWidget(deliveryFeeRowWidget);

    // Tax Row
    QHBoxLayout *taxRow = new QHBoxLayout();
    QLabel *taxTitle = new QLabel("Tax (14%):", calcFrame);
    taxTitle->setObjectName("summaryLabel");
    taxRow->addWidget(taxTitle);
    taxLabel = new QLabel("$0.00", calcFrame);
    taxLabel->setStyleSheet("color: #555555;");
    taxLabel->setAlignment(Qt::AlignRight);
    taxRow->addWidget(taxLabel);
    calcLayout->addLayout(taxRow);

    QFrame *line = new QFrame(calcFrame);
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("background-color: #E0E0E0; max-height: 1px; min-height: 1px;");
    calcLayout->addWidget(line);

    // Final Total Row
    QHBoxLayout *totRow = new QHBoxLayout();
    QLabel *totTitle = new QLabel("Final Total:", calcFrame);
    totTitle->setObjectName("summaryFinalLabel");
    finalTotalLabel = new QLabel("$0.00", calcFrame);
    finalTotalLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #D32F2F;");
    finalTotalLabel->setAlignment(Qt::AlignRight);
    totRow->addWidget(totTitle);
    totRow->addWidget(finalTotalLabel);
    calcLayout->addLayout(totRow);

    layout->addWidget(calcFrame);

    // Continue to Payment Button
    continuePaymentBtn = new QPushButton("💳 Continue to Payment", panel);
    continuePaymentBtn->setObjectName("btnContinuePayment");
    continuePaymentBtn->setCursor(Qt::PointingHandCursor);
    continuePaymentBtn->setFixedHeight(46);
    connect(continuePaymentBtn, &QPushButton::clicked, this, &NewOrderView::onContinueToPaymentClicked);
    layout->addWidget(continuePaymentBtn);

    return panel;
}

void NewOrderView::onNameChanged(const QString& text) {
    std::string val = text.trimmed().toStdString();
    m_order.customer.name = val;

    if (val.empty()) {
        nameEdit->setStyleSheet("border: 1px solid #D1D5DB; border-radius: 6px; padding-left: 10px; font-size: 14px;");
        nameValidationLabel->setText("");
    } else if (validName(val)) {
        nameEdit->setStyleSheet("border: 2px solid #2E7D32; border-radius: 6px; padding-left: 10px; font-size: 14px;");
        nameValidationLabel->setText("✓ Valid");
        nameValidationLabel->setStyleSheet("font-size: 11px; font-weight: 600; color: #2E7D32;");
    } else {
        nameEdit->setStyleSheet("border: 2px solid #C62828; border-radius: 6px; padding-left: 10px; font-size: 14px;");
        nameValidationLabel->setText("⚠ Letters only");
        nameValidationLabel->setStyleSheet("font-size: 11px; font-weight: 600; color: #C62828;");
    }
}

void NewOrderView::onPhoneChanged(const QString& text) {
    std::string val = text.trimmed().toStdString();
    m_order.customer.phone = val;

    if (val.empty()) {
        phoneEdit->setStyleSheet("border: 1px solid #D1D5DB; border-radius: 6px; padding-left: 10px; font-size: 14px;");
        phoneValidationLabel->setText("");
    } else if (validPhone(val)) {
        phoneEdit->setStyleSheet("border: 2px solid #2E7D32; border-radius: 6px; padding-left: 10px; font-size: 14px;");
        phoneValidationLabel->setText("✓ Valid");
        phoneValidationLabel->setStyleSheet("font-size: 11px; font-weight: 600; color: #2E7D32;");
    } else {
        phoneEdit->setStyleSheet("border: 2px solid #C62828; border-radius: 6px; padding-left: 10px; font-size: 14px;");
        phoneValidationLabel->setText("⚠ 11 digits (e.g. 010...)");
        phoneValidationLabel->setStyleSheet("font-size: 11px; font-weight: 600; color: #C62828;");
    }
}

void NewOrderView::onOrderTypeChanged(int id) {
    if (id == 1) m_order.customer.order_type = OrderType::DineIn;
    else if (id == 2) m_order.customer.order_type = OrderType::Takeaway;
    else if (id == 3) m_order.customer.order_type = OrderType::Delivery;

    tableNumberContainer->setVisible(id == 1);
    deliveryFeeRowWidget->setVisible(id == 3);

    calculate(m_order, m_data);
    refreshUi();
    emit orderUpdated();
}

void NewOrderView::onTableNumberChanged(int value) {
    m_order.customer.table_number = value;
}

void NewOrderView::onDiscountCardToggled(bool checked) {
    m_order.customer.discount_card = checked;
    calculate(m_order, m_data);
    refreshUi();
    emit orderUpdated();
}

void NewOrderView::onRemoveItemClicked(int index) {
    if (removeItem(m_order, m_data, index)) {
        refreshUi();
        emit orderUpdated();
        emit showStatusMessage("Item removed from cart.");
    }
}

void NewOrderView::onChangeItemQuantityClicked(int index, int delta) {
    if (changeItemQuantity(m_order, m_data, index, delta)) {
        refreshUi();
        emit orderUpdated();
    }
}

void NewOrderView::onCancelOrderClicked() {
    if (m_order.item_count == 0 && m_order.customer.name.empty() && m_order.customer.phone.empty()) {
        QMessageBox::information(this, "Notice", "Cart is already empty.");
        return;
    }

    auto res = QMessageBox::question(
        this, "Cancel Order",
        "Are you sure you want to cancel this order?\n\nAll cart items will be discarded.",
        QMessageBox::Yes | QMessageBox::No
    );

    if (res == QMessageBox::Yes) {
        // NOTE: No need to restore stock since it was never deducted from the cart
        m_order = Order();
        m_order.order_id = generateID();
        calculate(m_order, m_data);

        nameEdit->clear();
        phoneEdit->clear();
        takeawayRadio->setChecked(true);
        discountCardCheckBox->setChecked(false);
        tableNumberSpin->setValue(1);

        refreshUi();
        emit orderUpdated();
        emit showStatusMessage("Order cancelled.");
    }
}

void NewOrderView::onContinueToPaymentClicked() {
    if (!validName(m_order.customer.name)) {
        QMessageBox::warning(this, "Validation Error", "Please enter a valid customer name (letters and spaces only).");
        nameEdit->setFocus();
        return;
    }

    if (!validPhone(m_order.customer.phone)) {
        QMessageBox::warning(this, "Validation Error", "Please enter a valid 11-digit Egyptian phone number (e.g. 01012345678).");
        phoneEdit->setFocus();
        return;
    }

    if (m_order.item_count == 0) {
        QMessageBox::information(this, "Empty Cart", "Your cart is empty! Please add items from the Menu before proceeding to payment.");
        return;
    }

    if (!hasSufficientStock(m_order, m_data)) {
        QMessageBox::warning(
            this, "Insufficient Stock",
            "Not enough stock is available to fulfill this order. Please review the cart items and item stock before proceeding.");
        return;
    }

    // All validation passed — open the Payment dialog
    PaymentDialog payDlg(m_order, m_data, this);
    if (payDlg.exec() != QDialog::Accepted || !payDlg.paymentConfirmed()) {
        // User cancelled or payment was rejected — order unchanged
        return;
    }

    // Payment was finalized in core by PaymentDialog::onConfirmClicked()
    // Take a snapshot of the completed order for the invoice BEFORE resetting
    Order finalized = m_order;

    // Show the invoice
    InvoiceDialog invoice(finalized, this);
    invoice.exec();

    // Clear the active order for the next customer.
    // Stock was already consumed when items were added — do NOT restore it.
    m_order = Order();
    m_order.order_id = generateID();

    // Reset form fields
    nameEdit->clear();
    phoneEdit->clear();
    takeawayRadio->setChecked(true);
    discountCardCheckBox->setChecked(false);
    tableNumberSpin->setValue(1);

    refreshUi();
    emit orderUpdated();
    emit showStatusMessage(QString("Order #%1 completed successfully! 🎉").arg(finalized.order_id));
}

void NewOrderView::refreshUi() {
    // Synchronize Form Inputs with active order customer data
    if (nameEdit->text().toStdString() != m_order.customer.name) {
        nameEdit->setText(QString::fromStdString(m_order.customer.name));
    }
    if (phoneEdit->text().toStdString() != m_order.customer.phone) {
        phoneEdit->setText(QString::fromStdString(m_order.customer.phone));
    }

    if (m_order.customer.order_type == OrderType::DineIn) dineInRadio->setChecked(true);
    else if (m_order.customer.order_type == OrderType::Takeaway) takeawayRadio->setChecked(true);
    else if (m_order.customer.order_type == OrderType::Delivery) deliveryRadio->setChecked(true);

    tableNumberContainer->setVisible(m_order.customer.order_type == OrderType::DineIn);
    deliveryFeeRowWidget->setVisible(m_order.customer.order_type == OrderType::Delivery);

    discountCardCheckBox->setChecked(m_order.customer.discount_card);

    // Empty Cart State vs Cart Table Visibility
    bool isEmpty = (m_order.item_count == 0);
    emptyCartStateWidget->setVisible(isEmpty);
    cartContentWidget->setVisible(!isEmpty);

    // Populate Cart Table
    cartTable->setRowCount(0);
    QColor textColor("#1E1E24");
    for (int i = 0; i < m_order.item_count; i++) {
        const auto& item = m_order.items[i];
        int row = cartTable->rowCount();
        cartTable->insertRow(row);
        cartTable->setRowHeight(row, 42);

        auto* nameItem  = new QTableWidgetItem(QString::fromStdString(item.name));
        auto* detailItem= new QTableWidgetItem(QString::fromStdString(item.details));
        const bool hasPizzaSizeBreakdown =
            item.type == MealType::Pizza && item.details != "Different Sizes";
        auto* unitItem = new QTableWidgetItem(
            hasPizzaSizeBreakdown
                ? "See details"
                : QString("$%1").arg(item.unit_price, 0, 'f', 2));
        auto* totItem   = new QTableWidgetItem(QString("$%1").arg(item.total_price, 0, 'f', 2));

        // Explicit foreground guarantees text is visible regardless of stylesheet cascade
        nameItem->setForeground(textColor);
        detailItem->setForeground(textColor);
        unitItem->setForeground(textColor);
        totItem->setForeground(textColor);

        unitItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        totItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

        if (hasPizzaSizeBreakdown) {
            detailItem->setToolTip("Pizza size breakdown uses the selected menu prices. Item total includes any Core-applied offers.");
            unitItem->setToolTip("See the Details column for each size's actual unit price.");
        }

        cartTable->setItem(row, 0, nameItem);
        cartTable->setItem(row, 1, detailItem);
        QWidget *quantityControl = new QWidget(cartTable);
        QHBoxLayout *quantityLayout = new QHBoxLayout(quantityControl);
        quantityLayout->setContentsMargins(3, 3, 3, 3);
        quantityLayout->setSpacing(4);
        QPushButton *minusButton = new QPushButton("-", quantityControl);
        minusButton->setObjectName("btnQuantityMinus");
        minusButton->setFixedSize(28, 28);
        minusButton->setEnabled(item.quantity > 1);
        QLabel *quantityValue = new QLabel(QString::number(item.quantity), quantityControl);
        quantityValue->setObjectName("quantityValue");
        quantityValue->setAlignment(Qt::AlignCenter);
        quantityValue->setMinimumWidth(20);
        QPushButton *plusButton = new QPushButton("+", quantityControl);
        plusButton->setObjectName("btnQuantityPlus");
        plusButton->setFixedSize(28, 28);
        const int availableStock = item.type == MealType::Pizza ? m_data.pizza_stock
                                 : item.type == MealType::Burger ? m_data.burger_stock
                                 : item.type == MealType::Potatoes ? m_data.potatoes_stock
                                 : m_data.chicken_stock;
        plusButton->setEnabled(availableStock > 0);
        connect(minusButton, &QPushButton::clicked, [this, i]() { onChangeItemQuantityClicked(i, -1); });
        connect(plusButton, &QPushButton::clicked, [this, i]() { onChangeItemQuantityClicked(i, 1); });
        quantityLayout->addWidget(minusButton);
        quantityLayout->addWidget(quantityValue);
        quantityLayout->addWidget(plusButton);
        cartTable->setCellWidget(row, 2, quantityControl);
        cartTable->setItem(row, 3, unitItem);
        cartTable->setItem(row, 4, totItem);

        QPushButton *removeBtn = new QPushButton("❌ Remove", cartTable);
        removeBtn->setCursor(Qt::PointingHandCursor);
        removeBtn->setStyleSheet("background-color: #FFEBEE; color: #C62828; font-size: 11px; font-weight: bold; border-radius: 4px; border: 1px solid #FFCDD2; padding: 4px 8px;");

        connect(removeBtn, &QPushButton::clicked, [this, i]() {
            onRemoveItemClicked(i);
        });

        cartTable->setCellWidget(row, 5, removeBtn);
    }

    // Calculations Summary Display
    subtotalLabel->setText(QString("$%1").arg(m_order.subtotal, 0, 'f', 2));
    discountLabel->setText(QString("-$%1").arg(m_order.discount, 0, 'f', 2));
    deliveryFeeLabel->setText(QString("$%1").arg(m_order.delivery_fee, 0, 'f', 2));
    taxLabel->setText(QString("$%1").arg(m_order.tax, 0, 'f', 2));
    finalTotalLabel->setText(QString("$%1").arg(m_order.final_total, 0, 'f', 2));
}

void NewOrderView::applyViewStylesheet() {
    QString qss = R"(
        QLineEdit {
            background-color: #FFFFFF;
            color: #1E1E24;
            border: 1px solid #D1D5DB;
            border-radius: 6px;
            padding-left: 10px;
            font-size: 14px;
        }

        QLineEdit:focus {
            border: 2px solid #D32F2F;
        }

        QSpinBox {
            background-color: #FFFFFF;
            color: #1E1E24;
            border: 1px solid #D1D5DB;
            border-radius: 6px;
            padding: 4px 8px;
            font-size: 14px;
            font-weight: bold;
        }

        QRadioButton {
            color: #1E1E24;
            font-size: 13px;
            font-weight: 600;
            spacing: 6px;
        }

        QCheckBox {
            color: #1E1E24;
            font-size: 13px;
            font-weight: 600;
            spacing: 8px;
        }

        /* Explicit summary colors prevent a parent palette from making these
           labels unreadable on the light calculation card in every state. */
        QLabel#summaryLabel, QLabel#summaryLabel:hover, QLabel#summaryLabel:active {
            color: #1E1E24;
            font-size: 13px;
            font-weight: 600;
        }

        QLabel#summaryLabel:disabled {
            color: #4B5563;
        }

        QLabel#summaryFinalLabel, QLabel#summaryFinalLabel:hover, QLabel#summaryFinalLabel:active {
            color: #1E1E24;
            font-size: 15px;
            font-weight: bold;
        }

        QLabel#summaryFinalLabel:disabled {
            color: #4B5563;
        }

        #cartTable {
            background-color: #FFFFFF;
            border: 1px solid #E4E4E7;
            border-radius: 6px;
            font-size: 13px;
            gridline-color: #F0F0F0;
        }

        #cartTable::item {
            color: #1E1E24;
            background-color: #FFFFFF;
            padding: 4px 8px;
            border: none;
        }

        #cartTable::item:alternate {
            background-color: #F8F9FA;
        }

        #cartTable::item:selected {
            background-color: #FFEBEE;
            color: #D32F2F;
        }

        #cartTable QHeaderView::section {
            background-color: #F8F9FA;
            color: #4B5563;
            font-weight: bold;
            font-size: 12px;
            border: none;
            border-bottom: 1px solid #E4E4E7;
            padding: 6px;
        }

        #btnQuantityMinus, #btnQuantityPlus {
            background-color: #FEE2E2;
            color: #B91C1C;
            border: 1px solid #FCA5A5;
            border-radius: 5px;
            font-size: 18px;
            font-weight: bold;
        }

        #btnQuantityPlus {
            background-color: #DCFCE7;
            color: #166534;
            border-color: #86EFAC;
        }

        #btnQuantityMinus:hover { background-color: #FECACA; color: #991B1B; }
        #btnQuantityPlus:hover { background-color: #BBF7D0; color: #14532D; }
        #btnQuantityMinus:pressed { background-color: #FCA5A5; color: #7F1D1D; }
        #btnQuantityPlus:pressed { background-color: #86EFAC; color: #14532D; }
        #btnQuantityMinus:disabled, #btnQuantityPlus:disabled {
            background-color: #E5E7EB;
            color: #6B7280;
            border-color: #D1D5DB;
        }

        QLabel#quantityValue {
            color: #1E1E24;
            font-size: 13px;
            font-weight: bold;
        }

        #btnBrowseMenu {
            background-color: #D32F2F;
            color: #FFFFFF;
            font-weight: bold;
            font-size: 14px;
            border: none;
            border-radius: 6px;
        }

        #btnBrowseMenu:hover {
            background-color: #B71C1C;
        }

        #btnContinuePayment {
            background-color: #D32F2F;
            color: #FFFFFF;
            font-size: 15px;
            font-weight: bold;
            border: none;
            border-radius: 8px;
        }

        #btnContinuePayment:hover {
            background-color: #B71C1C;
        }

        #btnCancelOrder {
            background-color: #FFEBEE;
            color: #C62828;
            font-size: 14px;
            font-weight: bold;
            border: 1px solid #FFCDD2;
            border-radius: 8px;
        }

        #btnCancelOrder:hover {
            background-color: #FFCDD2;
            color: #B71C1C;
        }
    )";

    setStyleSheet(qss);
}
