#ifndef SALES_VIEW_H
#define SALES_VIEW_H

#include <QWidget>
#include <QLabel>

#include "../core/models.h"
#include "../core/sales_manager.h"

class SalesView : public QWidget {
    Q_OBJECT

public:
    explicit SalesView(RestaurantData& data, QWidget *parent = nullptr);
    void refreshUi();

private:
    void setupUi();
    void applyStylesheet();

    RestaurantData& m_data;

    QLabel *totalOrdersVal = nullptr;
    QLabel *totalSalesVal = nullptr;
    QLabel *cashOrdersVal = nullptr;
    QLabel *cardOrdersVal = nullptr;
    QLabel *walletOrdersVal = nullptr;
    QLabel *avgOrderVal = nullptr;
};

#endif // SALES_VIEW_H
