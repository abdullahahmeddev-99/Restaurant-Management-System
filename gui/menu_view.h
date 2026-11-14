#ifndef MENU_VIEW_H
#define MENU_VIEW_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTableWidget>
#include <QList>
#include <QGridLayout>
#include "../core/models.h"

class MenuView : public QWidget {
    Q_OBJECT

public:
    explicit MenuView(RestaurantData& data, Order& activeOrder, QWidget *parent = nullptr);
    void refreshUi();

signals:
    void orderUpdated();
    void showStatusMessage(const QString& msg);

private slots:
    void filterCategory(const QString& category);
    void onSearchTextChanged(const QString& text);
    void onAddPizzaClicked();
    void onAddBeefBurgerClicked();
    void onAddChickenBurgerClicked();
    void onAddPotatoesClicked();
    void onAddChickenClicked();

private:
    void setupUi();
    QWidget* createFilterBar();
    QWidget* createProductGrid();
    QWidget* createCartSummaryPanel();
    QWidget* createProductCard(const QString& id, const QString& name, const QString& category, const QString& desc, const QString& priceText, int stock, const QString& icon, const char* slot);

    RestaurantData& m_data;
    Order& m_order;

    QLineEdit *searchEdit;
    QList<QPushButton*> filterButtons;
    QString currentCategory = "All";

    QGridLayout *gridContainer;
    QList<QWidget*> productCards;

    // Cart Panel Widgets
    QTableWidget *cartTable;
    QLabel *subtotalLabel;
    QLabel *discountLabel;
    QLabel *taxLabel;
    QLabel *totalLabel;

    // Stock Badges
    QLabel *pizzaStockBadge = nullptr;
    QLabel *beefBurgerStockBadge = nullptr;
    QLabel *chickenBurgerStockBadge = nullptr;
    QLabel *potatoesStockBadge = nullptr;
    QLabel *chickenStockBadge = nullptr;
};

#endif // MENU_VIEW_H
