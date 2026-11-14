#include "invoice_dialog.h"
#include "../../core/sales_manager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QHeaderView>
#include <QTableWidget>
#include <QDateTime>
#include <QSizePolicy>

InvoiceDialog::InvoiceDialog(const Order& order, QWidget *parent)
    : QDialog(parent), m_order(order)
{
    setWindowTitle(QString("Invoice — Order #%1").arg(m_order.order_id));
    setMinimumWidth(540);
    setMinimumHeight(600);
    resize(560, 680);
    setModal(true);
    setupUi();
    applyStylesheet();
}

void InvoiceDialog::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ── Scroll area wrapping the entire invoice ──────────────────────────────
    QScrollArea *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet("background-color: #FFFFFF;");

    QWidget *container = new QWidget(scroll);
    container->setStyleSheet("background-color: #FFFFFF;");
    QVBoxLayout *layout = new QVBoxLayout(container);
    layout->setContentsMargins(36, 32, 36, 32);
    layout->setSpacing(0);

    // ── Restaurant Header ────────────────────────────────────────────────────
    QLabel *logoLbl = new QLabel("🔥 FLAME & FEAST", container);
    logoLbl->setObjectName("invoiceRestaurantName");
    logoLbl->setAlignment(Qt::AlignCenter);

    QLabel *taglineLbl = new QLabel("Restaurant Management System", container);
    taglineLbl->setObjectName("invoiceTagline");
    taglineLbl->setAlignment(Qt::AlignCenter);

    layout->addWidget(logoLbl);
    layout->addWidget(taglineLbl);
    layout->addSpacing(10);

    auto addDivider = [&](const QString& color = "#D1D5DB") {
        QFrame *line = new QFrame(container);
        line->setFrameShape(QFrame::HLine);
        line->setStyleSheet(QString("background-color: %1; min-height: 1px; max-height: 1px;").arg(color));
        layout->addWidget(line);
    };

    addDivider("#D32F2F");
    layout->addSpacing(12);

    // ── Order Meta ──────────────────────────────────────────────────────────
    auto addMetaRow = [&](const QString& key, const QString& val) {
        QHBoxLayout *row = new QHBoxLayout();
        QLabel *kLbl = new QLabel(key + ":", container);
        kLbl->setObjectName("invoiceMetaKey");
        QLabel *vLbl = new QLabel(val, container);
        vLbl->setObjectName("invoiceMetaVal");
        vLbl->setAlignment(Qt::AlignRight);
        row->addWidget(kLbl);
        row->addStretch();
        row->addWidget(vLbl);
        layout->addLayout(row);
    };

    QDateTime now = QDateTime::currentDateTime();
    addMetaRow("Order ID",       QString("#%1").arg(m_order.order_id));
    addMetaRow("Date",           now.toString("dd/MM/yyyy"));
    addMetaRow("Time",           now.toString("hh:mm:ss AP"));
    if (!m_order.customer.name.empty())
        addMetaRow("Customer",   QString::fromStdString(m_order.customer.name));
    if (!m_order.customer.phone.empty())
        addMetaRow("Phone",      QString::fromStdString(m_order.customer.phone));

    QString orderTypeStr = "Takeaway";
    if (m_order.customer.order_type == OrderType::DineIn)
        orderTypeStr = QString("Dine-in (Table %1)").arg(m_order.customer.table_number);
    else if (m_order.customer.order_type == OrderType::Delivery)
        orderTypeStr = "Delivery";
    addMetaRow("Order Type", orderTypeStr);

    layout->addSpacing(14);
    addDivider();
    layout->addSpacing(12);

    // ── Items Table ──────────────────────────────────────────────────────────
    QLabel *itemsHeader = new QLabel("ITEMS", container);
    itemsHeader->setObjectName("invoiceSectionHeader");
    layout->addWidget(itemsHeader);
    layout->addSpacing(8);

    QTableWidget *itemTable = new QTableWidget(0, 3, container);
    itemTable->setObjectName("invoiceTable");
    itemTable->setHorizontalHeaderLabels({"Item", "Qty", "Price"});
    itemTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    itemTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    itemTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    itemTable->verticalHeader()->setVisible(false);
    itemTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    itemTable->setSelectionMode(QAbstractItemView::NoSelection);
    itemTable->setFocusPolicy(Qt::NoFocus);
    itemTable->setAlternatingRowColors(true);
    itemTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    itemTable->setStyleSheet(
        "QTableWidget { border: none; font-size: 13px; background-color: #FFFFFF; gridline-color: #F0F0F0; }"
        "QTableWidget::item { color: #1E1E24; background-color: #FFFFFF; padding: 6px 8px; border: none; }"
        "QTableWidget::item:alternate { background-color: #FEF2F2; }"
        "QHeaderView::section { background-color: #1E1E24; color: #FFFFFF; font-weight: bold; font-size: 12px; padding: 8px; border: none; }"
    );

    QColor textColor("#1E1E24");
    for (int i = 0; i < m_order.item_count; i++) {
        const auto& item = m_order.items[i];
        int row = itemTable->rowCount();
        itemTable->insertRow(row);
        itemTable->setRowHeight(row, 34);

        QString nameStr = QString::fromStdString(item.name);
        if (!item.details.empty())
            nameStr += "\n(" + QString::fromStdString(item.details) + ")";

        auto* n = new QTableWidgetItem(nameStr);
        auto* q = new QTableWidgetItem(QString::number(item.quantity));
        auto* p = new QTableWidgetItem(QString("$%1").arg(item.total_price, 0, 'f', 2));

        n->setForeground(textColor);
        q->setForeground(textColor);
        p->setForeground(textColor);
        q->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        p->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

        itemTable->setItem(row, 0, n);
        itemTable->setItem(row, 1, q);
        itemTable->setItem(row, 2, p);
    }

    itemTable->setMinimumHeight(itemTable->rowCount() * 36 + 38);
    layout->addWidget(itemTable);
    layout->addSpacing(14);
    addDivider();
    layout->addSpacing(10);

    // ── Financial Summary ────────────────────────────────────────────────────
    auto addFinRow = [&](const QString& label, const QString& val,
                         const QString& valColor = "#1E1E24",
                         bool bold = false) {
        QHBoxLayout *row = new QHBoxLayout();
        QLabel *lLbl = new QLabel(label, container);
        lLbl->setObjectName("invoiceFinLabel");
        if (bold) lLbl->setStyleSheet("font-size: 14px; font-weight: bold; color: #1E1E24;");

        QLabel *vLbl = new QLabel(val, container);
        vLbl->setAlignment(Qt::AlignRight);
        if (bold)
            vLbl->setStyleSheet(QString("font-size: 16px; font-weight: bold; color: %1;").arg(valColor));
        else
            vLbl->setStyleSheet(QString("font-size: 13px; color: %1;").arg(valColor));

        row->addWidget(lLbl);
        row->addStretch();
        row->addWidget(vLbl);
        layout->addLayout(row);
        layout->addSpacing(4);
    };

    addFinRow("Subtotal:",  QString("$%1").arg(m_order.subtotal,    0, 'f', 2));
    addFinRow("Discount:",  QString("-$%1").arg(m_order.discount,   0, 'f', 2), "#2E7D32");
    if (m_order.delivery_fee > 0.001)
        addFinRow("Delivery Fee:", QString("$%1").arg(m_order.delivery_fee, 0, 'f', 2));
    addFinRow("Tax (14%):", QString("$%1").arg(m_order.tax,         0, 'f', 2), "#555555");

    layout->addSpacing(4);
    addDivider("#D32F2F");
    layout->addSpacing(6);

    addFinRow("TOTAL:", QString("$%1").arg(m_order.final_total, 0, 'f', 2), "#D32F2F", true);

    layout->addSpacing(12);
    addDivider();
    layout->addSpacing(10);

    // ── Payment Details ──────────────────────────────────────────────────────
    QLabel *payHeader = new QLabel("PAYMENT", container);
    payHeader->setObjectName("invoiceSectionHeader");
    layout->addWidget(payHeader);
    layout->addSpacing(8);

    QString methodStr;
    switch (m_order.bill.payment) {
        case PaymentMethod::CreditCard: methodStr = "Credit Card"; break;
        case PaymentMethod::Wallet:     methodStr = "Wallet";      break;
        default:                        methodStr = "Cash";        break;
    }
    addFinRow("Payment Method:", methodStr);
    addFinRow("Paid Amount:",    QString("$%1").arg(m_order.bill.paid_amount, 0, 'f', 2));

    double change = m_order.bill.paid_amount - m_order.final_total;
    if (change > 0.001)
        addFinRow("Change:", QString("$%1").arg(change, 0, 'f', 2), "#2E7D32");

    layout->addSpacing(14);
    addDivider("#D32F2F");
    layout->addSpacing(14);

    // ── Footer ───────────────────────────────────────────────────────────────
    QLabel *thanks = new QLabel("Thank you for visiting!", container);
    thanks->setObjectName("invoiceFooter");
    thanks->setAlignment(Qt::AlignCenter);
    layout->addWidget(thanks);

    QLabel *footerBrand = new QLabel("🔥 FLAME & FEAST", container);
    footerBrand->setObjectName("invoiceFooterBrand");
    footerBrand->setAlignment(Qt::AlignCenter);
    layout->addWidget(footerBrand);

    layout->addStretch();

    scroll->setWidget(container);
    mainLayout->addWidget(scroll);

    // ── Close Button ─────────────────────────────────────────────────────────
    QWidget *btnBar = new QWidget(this);
    btnBar->setStyleSheet("background-color: #FFFFFF; border-top: 1px solid #E5E7EB; padding: 0px;");
    QHBoxLayout *btnLayout = new QHBoxLayout(btnBar);
    btnLayout->setContentsMargins(28, 14, 28, 14);

    QPushButton *closeBtn = new QPushButton("✓  Close & Start New Order", btnBar);
    closeBtn->setObjectName("btnInvoiceClose");
    closeBtn->setFixedHeight(46);
    closeBtn->setCursor(Qt::PointingHandCursor);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn, 2);
    btnLayout->addStretch();
    mainLayout->addWidget(btnBar);
}

void InvoiceDialog::applyStylesheet() {
    setStyleSheet(R"(
        QDialog {
            background-color: #FFFFFF;
        }

        QLabel {
            background-color: transparent;
            color: #1E1E24;
        }

        #invoiceRestaurantName {
            font-size: 22px;
            font-weight: bold;
            color: #D32F2F;
            letter-spacing: 2px;
        }

        #invoiceTagline {
            font-size: 12px;
            color: #666666;
        }

        #invoiceMetaKey {
            font-size: 13px;
            color: #555555;
            font-weight: 500;
        }

        #invoiceMetaVal {
            font-size: 13px;
            color: #1E1E24;
            font-weight: 600;
        }

        #invoiceSectionHeader {
            font-size: 11px;
            font-weight: bold;
            color: #888888;
            letter-spacing: 2px;
        }

        #invoiceFinLabel {
            font-size: 13px;
            color: #555555;
        }

        #invoiceFooter {
            font-size: 14px;
            color: #666666;
            font-style: italic;
        }

        #invoiceFooterBrand {
            font-size: 16px;
            font-weight: bold;
            color: #D32F2F;
        }

        #btnInvoiceClose {
            background-color: #D32F2F;
            color: #FFFFFF;
            font-size: 15px;
            font-weight: bold;
            border: none;
            border-radius: 8px;
        }

        #btnInvoiceClose:hover {
            background-color: #B71C1C;
        }
    )");
}
