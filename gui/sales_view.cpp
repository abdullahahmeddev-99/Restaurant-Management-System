#include "sales_view.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QScrollArea>

SalesView::SalesView(RestaurantData& data, QWidget *parent)
    : QWidget(parent), m_data(data)
{
    setupUi();
    applyStylesheet();
    refreshUi();
}

void SalesView::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(28, 24, 28, 28);
    mainLayout->setSpacing(24);

    // Header
    QLabel *titleLbl = new QLabel("📈 Sales Report", this);
    titleLbl->setStyleSheet("font-size: 22px; font-weight: bold; color: #1E1E24;");
    mainLayout->addWidget(titleLbl);

    QLabel *subtitleLbl = new QLabel("Comprehensive sales statistics and payment method breakdown.", this);
    subtitleLbl->setStyleSheet("font-size: 13px; color: #666666;");
    mainLayout->addWidget(subtitleLbl);

    // Stats Grid - 2 rows x 3 columns
    QHBoxLayout *row1 = new QHBoxLayout();
    row1->setSpacing(16);

    // Total Orders Card
    QFrame *ordersCard = new QFrame(this);
    ordersCard->setObjectName("statCard");
    QVBoxLayout *ordersLayout = new QVBoxLayout(ordersCard);
    ordersLayout->setContentsMargins(20, 18, 20, 18);
    ordersLayout->setSpacing(6);

    QLabel *ordersTitle = new QLabel("Total Orders", ordersCard);
    ordersTitle->setStyleSheet("font-size: 13px; font-weight: 600; color: #666666;");
    totalOrdersVal = new QLabel(ordersCard);
    totalOrdersVal->setObjectName("statVal");
    totalOrdersVal->setStyleSheet("font-size: 24px; font-weight: bold; color: #D32F2F;");
    QLabel *ordersSub = new QLabel("Completed orders", ordersCard);
    ordersSub->setStyleSheet("font-size: 11px; color: #888888;");

    ordersLayout->addWidget(ordersTitle);
    ordersLayout->addWidget(totalOrdersVal);
    ordersLayout->addWidget(ordersSub);

    // Total Sales Card
    QFrame *salesCard = new QFrame(this);
    salesCard->setObjectName("statCard");
    QVBoxLayout *salesLayout = new QVBoxLayout(salesCard);
    salesLayout->setContentsMargins(20, 18, 20, 18);
    salesLayout->setSpacing(6);

    QLabel *salesTitle = new QLabel("Total Sales", salesCard);
    salesTitle->setStyleSheet("font-size: 13px; font-weight: 600; color: #666666;");
    totalSalesVal = new QLabel(salesCard);
    totalSalesVal->setObjectName("statVal");
    totalSalesVal->setStyleSheet("font-size: 24px; font-weight: bold; color: #2E7D32;");
    QLabel *salesSub = new QLabel("Gross revenue", salesCard);
    salesSub->setStyleSheet("font-size: 11px; color: #888888;");

    salesLayout->addWidget(salesTitle);
    salesLayout->addWidget(totalSalesVal);
    salesLayout->addWidget(salesSub);

    // Average Order Card
    QFrame *avgCard = new QFrame(this);
    avgCard->setObjectName("statCard");
    QVBoxLayout *avgLayout = new QVBoxLayout(avgCard);
    avgLayout->setContentsMargins(20, 18, 20, 18);
    avgLayout->setSpacing(6);

    QLabel *avgTitle = new QLabel("Average Order", avgCard);
    avgTitle->setStyleSheet("font-size: 13px; font-weight: 600; color: #666666;");
    avgOrderVal = new QLabel(avgCard);
    avgOrderVal->setObjectName("statVal");
    avgOrderVal->setStyleSheet("font-size: 24px; font-weight: bold; color: #1976D2;");
    QLabel *avgSub = new QLabel("Average per order", avgCard);
    avgSub->setStyleSheet("font-size: 11px; color: #888888;");

    avgLayout->addWidget(avgTitle);
    avgLayout->addWidget(avgOrderVal);
    avgLayout->addWidget(avgSub);

    row1->addWidget(ordersCard);
    row1->addWidget(salesCard);
    row1->addWidget(avgCard);

    QHBoxLayout *row2 = new QHBoxLayout();
    row2->setSpacing(16);

    // Cash Orders Card
    QFrame *cashCard = new QFrame(this);
    cashCard->setObjectName("statCard");
    QVBoxLayout *cashLayout = new QVBoxLayout(cashCard);
    cashLayout->setContentsMargins(20, 18, 20, 18);
    cashLayout->setSpacing(6);

    QLabel *cashTitle = new QLabel("Cash Orders", cashCard);
    cashTitle->setStyleSheet("font-size: 13px; font-weight: 600; color: #666666;");
    cashOrdersVal = new QLabel(cashCard);
    cashOrdersVal->setObjectName("statVal");
    cashOrdersVal->setStyleSheet("font-size: 24px; font-weight: bold; color: #F57C00;");
    QLabel *cashSub = new QLabel("Cash payments", cashCard);
    cashSub->setStyleSheet("font-size: 11px; color: #888888;");

    cashLayout->addWidget(cashTitle);
    cashLayout->addWidget(cashOrdersVal);
    cashLayout->addWidget(cashSub);

    // Credit Card Orders Card
    QFrame *cardCard = new QFrame(this);
    cardCard->setObjectName("statCard");
    QVBoxLayout *cardLayout = new QVBoxLayout(cardCard);
    cardLayout->setContentsMargins(20, 18, 20, 18);
    cardLayout->setSpacing(6);

    QLabel *cardTitle = new QLabel("Credit Card Orders", cardCard);
    cardTitle->setStyleSheet("font-size: 13px; font-weight: 600; color: #666666;");
    cardOrdersVal = new QLabel(cardCard);
    cardOrdersVal->setObjectName("statVal");
    cardOrdersVal->setStyleSheet("font-size: 24px; font-weight: bold; color: #9C27B0;");
    QLabel *cardSub = new QLabel("Card payments", cardCard);
    cardSub->setStyleSheet("font-size: 11px; color: #888888;");

    cardLayout->addWidget(cardTitle);
    cardLayout->addWidget(cardOrdersVal);
    cardLayout->addWidget(cardSub);

    // Wallet Orders Card
    QFrame *walletCard = new QFrame(this);
    walletCard->setObjectName("statCard");
    QVBoxLayout *walletLayout = new QVBoxLayout(walletCard);
    walletLayout->setContentsMargins(20, 18, 20, 18);
    walletLayout->setSpacing(6);

    QLabel *walletTitle = new QLabel("Wallet Orders", walletCard);
    walletTitle->setStyleSheet("font-size: 13px; font-weight: 600; color: #666666;");
    walletOrdersVal = new QLabel(walletCard);
    walletOrdersVal->setObjectName("statVal");
    walletOrdersVal->setStyleSheet("font-size: 24px; font-weight: bold; color: #009688;");
    QLabel *walletSub = new QLabel("Wallet payments", walletCard);
    walletSub->setStyleSheet("font-size: 11px; color: #888888;");

    walletLayout->addWidget(walletTitle);
    walletLayout->addWidget(walletOrdersVal);
    walletLayout->addWidget(walletSub);

    row2->addWidget(cashCard);
    row2->addWidget(cardCard);
    row2->addWidget(walletCard);

    mainLayout->addLayout(row1);
    mainLayout->addLayout(row2);
    mainLayout->addStretch();
}

void SalesView::refreshUi() {
    totalOrdersVal->setText(QString::number(m_data.total_orders));
    totalSalesVal->setText(QString("$%1").arg(m_data.total_sales, 0, 'f', 2));
    cashOrdersVal->setText(QString::number(m_data.cash_orders));
    cardOrdersVal->setText(QString::number(m_data.card_orders));
    walletOrdersVal->setText(QString::number(m_data.wallet_orders));
    avgOrderVal->setText(QString("$%1").arg(getAverageOrderSales(m_data), 0, 'f', 2));
}

void SalesView::applyStylesheet() {
    setStyleSheet(R"(
        SalesView {
            background-color: #F4F5F8;
        }

        #statCard {
            background-color: #FFFFFF;
            border: 1px solid #E4E4E7;
            border-radius: 12px;
        }

        #statCard:hover {
            border: 1px solid #D32F2F;
        }
    )");
}
