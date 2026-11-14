#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QList>
#include <QTableWidget>
#include <QStatusBar>
#include <QStackedWidget>

#include "../core/models.h"
#include "menu_view.h"
#include "new_order_view.h"
#include "sales_view.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void onSidebarButtonClicked();
    void onQuickActionButtonClicked();
    void updateDashboardMetrics();

private:
    void setupUi();
    QWidget* createSidebar();
    QWidget* createHeader();
    QWidget* createDashboardContent();
    QWidget* createStatCard(const QString& title, const QString& value, const QString& subtext, const QString& icon, const QString& accentColor);
    QPushButton* createQuickActionButton(const QString& text, const QString& icon, bool isPrimary);

    void applyStylesheet();

    // Data Engine Instances
    RestaurantData m_restaurantData;
    Order m_activeOrder;

    // Navigation & Layout
    QList<QPushButton*> sidebarButtons;
    QPushButton* activeSidebarButton = nullptr;
    QLabel* pageTitleLabel = nullptr;

    QStackedWidget *mainStackedWidget = nullptr;
    QWidget *dashboardWidget = nullptr;
    MenuView *menuView = nullptr;
    NewOrderView *newOrderView = nullptr;
    SalesView *salesView = nullptr;

    // Stat card labels to update dynamically
    QLabel *totalOrdersVal = nullptr;
    QLabel *totalSalesVal = nullptr;
    QLabel *stockStatusVal = nullptr;
};

#endif // MAIN_WINDOW_H
