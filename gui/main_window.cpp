#include "main_window.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QFrame>
#include <QHeaderView>
#include <QMessageBox>
#include "../core/order_manager.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Flame & Feast - Restaurant Management System");
    setMinimumSize(1150, 720);
    resize(1280, 800);

    // Initialize active order ID from Core generateID()
    m_activeOrder.order_id = generateID();

    setupUi();
    applyStylesheet();

    statusBar()->showMessage("System Ready | Antigravity Core Engine Active", 5000);
}

void MainWindow::setupUi() {
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Left Sidebar
    QWidget *sidebar = createSidebar();
    mainLayout->addWidget(sidebar);

    // Right Main Container (Header + Stacked Pages)
    QWidget *rightContainer = new QWidget(this);
    rightContainer->setObjectName("rightContainer");
    QVBoxLayout *rightLayout = new QVBoxLayout(rightContainer);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    // Top Header
    QWidget *header = createHeader();
    rightLayout->addWidget(header);

    // Stacked Widget for Page Switching
    mainStackedWidget = new QStackedWidget(rightContainer);

    // Page 0: Dashboard Content in Scroll Area
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setObjectName("mainScrollArea");
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    dashboardWidget = createDashboardContent();
    scrollArea->setWidget(dashboardWidget);
    mainStackedWidget->addWidget(scrollArea);

    // Page 1: Real Menu Screen connected to Core
    menuView = new MenuView(m_restaurantData, m_activeOrder, this);
    mainStackedWidget->addWidget(menuView);

    // Page 2: Real New Order / Cart Screen connected to Core
    newOrderView = new NewOrderView(m_restaurantData, m_activeOrder, this);
    mainStackedWidget->addWidget(newOrderView);

    // Page 3: Sales Report View connected to Core
    salesView = new SalesView(m_restaurantData, this);
    mainStackedWidget->addWidget(salesView);

    // Connect View Signal / Slot Inter-Communication
    connect(menuView, &MenuView::showStatusMessage, [this](const QString& msg){
        statusBar()->showMessage(msg, 4000);
        updateDashboardMetrics();
    });
    connect(menuView, &MenuView::orderUpdated, [this](){
        if (newOrderView) newOrderView->refreshUi();
        updateDashboardMetrics();
    });

    connect(newOrderView, &NewOrderView::showStatusMessage, [this](const QString& msg){
        statusBar()->showMessage(msg, 4000);
        updateDashboardMetrics();
    });
    connect(newOrderView, &NewOrderView::orderUpdated, [this](){
        if (menuView) menuView->refreshUi();
        updateDashboardMetrics();
    });

    connect(newOrderView, &NewOrderView::navigateToMenuRequested, [this](){
        for (QPushButton *btn : sidebarButtons) {
            if (btn->objectName() == "btn_menu") {
                btn->click();
                return;
            }
        }
    });

    rightLayout->addWidget(mainStackedWidget);
    mainLayout->addWidget(rightContainer);
}

QWidget* MainWindow::createSidebar() {
    QWidget *sidebar = new QWidget(this);
    sidebar->setObjectName("sidebar");
    sidebar->setFixedWidth(240);

    QVBoxLayout *layout = new QVBoxLayout(sidebar);
    layout->setContentsMargins(16, 24, 16, 20);
    layout->setSpacing(8);

    // Logo / Brand Header
    QHBoxLayout *logoLayout = new QHBoxLayout();
    logoLayout->setContentsMargins(8, 0, 8, 16);
    
    QLabel *logoIcon = new QLabel("🔥", sidebar);
    logoIcon->setStyleSheet("font-size: 26px;");
    
    QLabel *logoText = new QLabel("Flame & Feast", sidebar);
    logoText->setObjectName("logoText");
    logoText->setStyleSheet("font-size: 20px; font-weight: bold; color: #FFFFFF;");

    logoLayout->addWidget(logoIcon);
    logoLayout->addWidget(logoText);
    logoLayout->addStretch();
    layout->addLayout(logoLayout);

    QFrame *divider = new QFrame(sidebar);
    divider->setFrameShape(QFrame::HLine);
    divider->setStyleSheet("background-color: #2D2D35; border: none; min-height: 1px; max-height: 1px; margin-bottom: 12px;");
    layout->addWidget(divider);

    // Navigation Items
    struct NavItem { QString label; QString icon; QString objectName; };
    QList<NavItem> navItems = {
        {"Dashboard", "📊", "btn_dashboard"},
        {"Menu", "🍕", "btn_menu"},
        {"New Order", "🛒", "btn_new_order"},
        {"Inventory", "📦", "btn_inventory"},
        {"Sales", "📈", "btn_sales"},
        {"Customers", "👥", "btn_customers"},
        {"Settings", "⚙️", "btn_settings"}
    };

    for (const auto& item : navItems) {
        QPushButton *btn = new QPushButton(item.icon + "   " + item.label, sidebar);
        btn->setObjectName(item.objectName);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFixedHeight(44);
        
        connect(btn, &QPushButton::clicked, this, &MainWindow::onSidebarButtonClicked);
        sidebarButtons.append(btn);
        layout->addWidget(btn);

        if (item.objectName == "btn_dashboard") {
            activeSidebarButton = btn;
            btn->setProperty("active", true);
        } else {
            btn->setProperty("active", false);
        }
    }

    layout->addStretch();

    QLabel *footer = new QLabel("Antigravity POS v1.0\nQt 6 Core Architecture", sidebar);
    footer->setStyleSheet("color: #71717A; font-size: 11px; text-align: center; margin-top: 10px;");
    footer->setAlignment(Qt::AlignCenter);
    layout->addWidget(footer);

    return sidebar;
}

QWidget* MainWindow::createHeader() {
    QWidget *header = new QWidget(this);
    header->setObjectName("header");
    header->setFixedHeight(70);

    QHBoxLayout *layout = new QHBoxLayout(header);
    layout->setContentsMargins(24, 0, 24, 0);

    pageTitleLabel = new QLabel("Dashboard", header);
    pageTitleLabel->setObjectName("pageTitle");
    pageTitleLabel->setStyleSheet("font-size: 22px; font-weight: bold; color: #1E1E24;");
    layout->addWidget(pageTitleLabel);

    layout->addStretch();

    QPushButton *notifBtn = new QPushButton("🔔", header);
    notifBtn->setObjectName("headerBtn");
    notifBtn->setFixedSize(40, 40);
    notifBtn->setCursor(Qt::PointingHandCursor);
    notifBtn->setToolTip("Notifications");
    connect(notifBtn, &QPushButton::clicked, [this]() {
        statusBar()->showMessage("No new system notifications.", 3000);
    });
    layout->addWidget(notifBtn);

    QFrame *userFrame = new QFrame(header);
    userFrame->setObjectName("userFrame");
    QHBoxLayout *userLayout = new QHBoxLayout(userFrame);
    userLayout->setContentsMargins(10, 4, 12, 4);
    userLayout->setSpacing(10);

    QLabel *userAvatar = new QLabel("👤", userFrame);
    userAvatar->setStyleSheet("font-size: 20px;");

    QVBoxLayout *userInfoLayout = new QVBoxLayout();
    userInfoLayout->setSpacing(0);

    QLabel *userName = new QLabel("Manager Admin", userFrame);
    userName->setStyleSheet("font-size: 13px; font-weight: bold; color: #1E1E24;");

    QLabel *userRole = new QLabel("● Store Admin", userFrame);
    userRole->setStyleSheet("font-size: 11px; color: #2E7D32; font-weight: 500;");

    userInfoLayout->addWidget(userName);
    userInfoLayout->addWidget(userRole);

    userLayout->addWidget(userAvatar);
    userLayout->addLayout(userInfoLayout);

    layout->addWidget(userFrame);

    return header;
}

QWidget* MainWindow::createDashboardContent() {
    QWidget *content = new QWidget(this);
    content->setObjectName("dashboardContent");

    QVBoxLayout *layout = new QVBoxLayout(content);
    layout->setContentsMargins(28, 24, 28, 28);
    layout->setSpacing(24);

    QFrame *welcomeCard = new QFrame(content);
    welcomeCard->setObjectName("welcomeCard");
    QVBoxLayout *welcomeLayout = new QVBoxLayout(welcomeCard);
    welcomeLayout->setContentsMargins(24, 20, 24, 20);

    QLabel *welcomeTitle = new QLabel("Welcome back, Manager! 👋", welcomeCard);
    welcomeTitle->setStyleSheet("font-size: 20px; font-weight: bold; color: #1E1E24;");

    QLabel *welcomeSub = new QLabel("Here is your daily operational summary and quick access to management tools.", welcomeCard);
    welcomeSub->setStyleSheet("font-size: 13px; color: #666666; margin-top: 4px;");

    welcomeLayout->addWidget(welcomeTitle);
    welcomeLayout->addWidget(welcomeSub);
    layout->addWidget(welcomeCard);

    QHBoxLayout *statsLayout = new QHBoxLayout();
    statsLayout->setSpacing(16);

    QWidget* card1 = createStatCard("Total Orders", QString::number(m_restaurantData.total_orders), "Today's completed orders", "🛍️", "#D32F2F");
    totalOrdersVal = card1->findChild<QLabel*>("statVal");

    QWidget* card2 = createStatCard("Total Sales", QString("$%1").arg(m_restaurantData.total_sales, 0, 'f', 2), "Gross revenue generated", "💰", "#2E7D32");
    totalSalesVal = card2->findChild<QLabel*>("statVal");

    QWidget* card3 = createStatCard("Active Menu", "4 Categories", "Pizza, Burger, Potatoes, Chicken", "🍕", "#1976D2");

    int lowStockCount = 0;
    if (m_restaurantData.pizza_stock < 10) lowStockCount++;
    if (m_restaurantData.burger_stock < 10) lowStockCount++;
    if (m_restaurantData.potatoes_stock < 10) lowStockCount++;
    if (m_restaurantData.chicken_stock < 10) lowStockCount++;

    QWidget* card4 = createStatCard("Stock Status", QString("%1 Low Stock").arg(lowStockCount), "Monitored inventory levels", "⚠️", "#F57C00");
    stockStatusVal = card4->findChild<QLabel*>("statVal");

    statsLayout->addWidget(card1);
    statsLayout->addWidget(card2);
    statsLayout->addWidget(card3);
    statsLayout->addWidget(card4);

    layout->addLayout(statsLayout);

    // Quick Actions Section
    QVBoxLayout *quickActionsSection = new QVBoxLayout();
    quickActionsSection->setSpacing(12);

    QLabel *quickActionsTitle = new QLabel("⚡ Quick Actions", content);
    quickActionsTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #1E1E24;");
    quickActionsSection->addWidget(quickActionsTitle);

    QHBoxLayout *quickActionsGrid = new QHBoxLayout();
    quickActionsGrid->setSpacing(16);

    quickActionsGrid->addWidget(createQuickActionButton("New Order", "🛒", true));
    quickActionsGrid->addWidget(createQuickActionButton("View Menu", "📖", false));
    quickActionsGrid->addWidget(createQuickActionButton("Manage Inventory", "📦", false));
    quickActionsGrid->addWidget(createQuickActionButton("Sales Report", "📈", false));

    quickActionsSection->addLayout(quickActionsGrid);
    layout->addLayout(quickActionsSection);

    // Recent Transactions Section
    QVBoxLayout *recentSection = new QVBoxLayout();
    recentSection->setSpacing(12);

    QLabel *recentTitle = new QLabel("📋 Recent Transactions", content);
    recentTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #1E1E24;");
    recentSection->addWidget(recentTitle);

    QTableWidget *table = new QTableWidget(0, 5, content);
    table->setObjectName("recentOrdersTable");
    table->setFixedHeight(220);
    table->setHorizontalHeaderLabels({"Order ID", "Customer", "Order Type", "Total Amount", "Payment Method"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);

    QFrame *tableContainer = new QFrame(content);
    tableContainer->setObjectName("tableContainer");
    QVBoxLayout *containerLayout = new QVBoxLayout(tableContainer);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->addWidget(table);

    QLabel *emptyStateLabel = new QLabel("No transactions recorded yet today. Click 'New Order' to start.", tableContainer);
    emptyStateLabel->setStyleSheet("color: #888888; font-size: 13px; font-style: italic; margin-top: 8px;");
    emptyStateLabel->setAlignment(Qt::AlignCenter);

    containerLayout->addWidget(emptyStateLabel);

    recentSection->addWidget(tableContainer);
    layout->addLayout(recentSection);

    return content;
}

QWidget* MainWindow::createStatCard(const QString& title, const QString& value, const QString& subtext, const QString& icon, const QString& accentColor) {
    QFrame *card = new QFrame(this);
    card->setObjectName("statCard");

    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setContentsMargins(20, 18, 20, 18);
    layout->setSpacing(6);

    QHBoxLayout *topRow = new QHBoxLayout();
    QLabel *titleLbl = new QLabel(title, card);
    titleLbl->setStyleSheet("font-size: 13px; font-weight: 600; color: #666666;");

    QLabel *iconLbl = new QLabel(icon, card);
    iconLbl->setStyleSheet(QString("font-size: 22px; color: %1;").arg(accentColor));

    topRow->addWidget(titleLbl);
    topRow->addStretch();
    topRow->addWidget(iconLbl);

    QLabel *valLbl = new QLabel(value, card);
    valLbl->setObjectName("statVal");
    valLbl->setStyleSheet(QString("font-size: 24px; font-weight: bold; color: %1; margin-top: 4px;").arg(accentColor));

    QLabel *subLbl = new QLabel(subtext, card);
    subLbl->setStyleSheet("font-size: 11px; color: #888888;");

    layout->addLayout(topRow);
    layout->addWidget(valLbl);
    layout->addWidget(subLbl);

    return card;
}

QPushButton* MainWindow::createQuickActionButton(const QString& text, const QString& icon, bool isPrimary) {
    QPushButton *btn = new QPushButton(icon + "  " + text, this);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setFixedHeight(48);

    if (isPrimary) {
        btn->setObjectName("quickActionPrimary");
    } else {
        btn->setObjectName("quickActionSecondary");
    }

    connect(btn, &QPushButton::clicked, this, &MainWindow::onQuickActionButtonClicked);
    return btn;
}

void MainWindow::updateDashboardMetrics() {
    if (totalOrdersVal) totalOrdersVal->setText(QString::number(m_restaurantData.total_orders));
    if (totalSalesVal) totalSalesVal->setText(QString("$%1").arg(m_restaurantData.total_sales, 0, 'f', 2));

    int lowStockCount = 0;
    if (m_restaurantData.pizza_stock < 10) lowStockCount++;
    if (m_restaurantData.burger_stock < 10) lowStockCount++;
    if (m_restaurantData.potatoes_stock < 10) lowStockCount++;
    if (m_restaurantData.chicken_stock < 10) lowStockCount++;

    if (stockStatusVal) stockStatusVal->setText(QString("%1 Low Stock").arg(lowStockCount));
}

void MainWindow::onSidebarButtonClicked() {
    QPushButton *clickedBtn = qobject_cast<QPushButton*>(sender());
    if (!clickedBtn) return;

    for (QPushButton *btn : sidebarButtons) {
        btn->setProperty("active", (btn == clickedBtn));
        btn->style()->unpolish(btn);
        btn->style()->polish(btn);
    }

    if (clickedBtn->objectName() == "btn_dashboard") {
        pageTitleLabel->setText("Dashboard");
        mainStackedWidget->setCurrentIndex(0);
        updateDashboardMetrics();
        statusBar()->showMessage("Viewing Dashboard Overview", 3000);
    } else if (clickedBtn->objectName() == "btn_menu") {
        pageTitleLabel->setText("Menu");
        mainStackedWidget->setCurrentIndex(1);
        menuView->refreshUi();
        statusBar()->showMessage("Viewing Restaurant Menu", 3000);
    } else if (clickedBtn->objectName() == "btn_new_order") {
        pageTitleLabel->setText("New Order");
        mainStackedWidget->setCurrentIndex(2);
        newOrderView->refreshUi();
        statusBar()->showMessage("Active Order Cart & Customer Details", 3000);
    } else if (clickedBtn->objectName() == "btn_sales") {
        pageTitleLabel->setText("Sales Report");
        mainStackedWidget->setCurrentIndex(3);
        salesView->refreshUi();
        statusBar()->showMessage("Viewing Sales Report", 3000);
    } else {
        QString moduleName = clickedBtn->text().trimmed();
        statusBar()->showMessage("Module '" + moduleName + "' will be implemented in the next step.", 4000);
        QMessageBox::information(this, "Module Upcoming", "The " + moduleName + " module will be implemented in the next step.");
    }
}

void MainWindow::onQuickActionButtonClicked() {
    QPushButton *clickedBtn = qobject_cast<QPushButton*>(sender());
    if (!clickedBtn) return;

    QString actionName = clickedBtn->text().trimmed();
    if (actionName.contains("View Menu")) {
        for (QPushButton *btn : sidebarButtons) {
            if (btn->objectName() == "btn_menu") {
                btn->click();
                return;
            }
        }
    } else if (actionName.contains("New Order")) {
        for (QPushButton *btn : sidebarButtons) {
            if (btn->objectName() == "btn_new_order") {
                btn->click();
                return;
            }
        }
    } else if (actionName.contains("Sales Report")) {
        for (QPushButton *btn : sidebarButtons) {
            if (btn->objectName() == "btn_sales") {
                btn->click();
                return;
            }
        }
    }

    statusBar()->showMessage("Action '" + actionName + "' will be enabled in the next step.", 4000);
    QMessageBox::information(this, "Feature Upcoming", "The action '" + actionName + "' will be connected to the core engine in the next step.");
}

void MainWindow::applyStylesheet() {
    QString qss = R"(
        QMainWindow {
            background-color: #F4F5F8;
        }

        #sidebar {
            background-color: #1E1E24;
        }

        #sidebar QPushButton {
            background-color: transparent;
            color: #A0A0AB;
            border: none;
            border-radius: 8px;
            text-align: left;
            padding-left: 14px;
            font-size: 14px;
            font-weight: 500;
        }

        #sidebar QPushButton:hover {
            background-color: rgba(255, 255, 255, 0.08);
            color: #FFFFFF;
        }

        #sidebar QPushButton[active="true"] {
            background-color: #D32F2F;
            color: #FFFFFF;
            font-weight: bold;
        }

        #header {
            background-color: #FFFFFF;
            border-bottom: 1px solid #E4E4E7;
        }

        #headerBtn {
            background-color: #F4F5F8;
            border: 1px solid #E4E4E7;
            border-radius: 20px;
            font-size: 16px;
        }

        #headerBtn:hover {
            background-color: #EAEBED;
        }

        #userFrame {
            background-color: #F8F9FA;
            border: 1px solid #E4E4E7;
            border-radius: 20px;
        }

        #rightContainer {
            background-color: #F4F5F8;
        }

        #mainScrollArea {
            background-color: #F4F5F8;
            border: none;
        }

        #dashboardContent {
            background-color: #F4F5F8;
        }

        #welcomeCard {
            background-color: #FFFFFF;
            border-radius: 12px;
            border-left: 5px solid #D32F2F;
            border-top: 1px solid #E4E4E7;
            border-right: 1px solid #E4E4E7;
            border-bottom: 1px solid #E4E4E7;
        }

        #statCard {
            background-color: #FFFFFF;
            border: 1px solid #E4E4E7;
            border-radius: 12px;
        }

        #statCard:hover {
            border: 1px solid #D32F2F;
        }

        #quickActionPrimary {
            background-color: #D32F2F;
            color: #FFFFFF;
            font-size: 14px;
            font-weight: bold;
            border: none;
            border-radius: 10px;
            padding: 0px 16px;
        }

        #quickActionPrimary:hover {
            background-color: #B71C1C;
        }

        #quickActionSecondary {
            background-color: #FFFFFF;
            color: #1E1E24;
            font-size: 14px;
            font-weight: 600;
            border: 1px solid #E4E4E7;
            border-radius: 10px;
            padding: 0px 16px;
        }

        #quickActionSecondary:hover {
            background-color: #F4F5F8;
            border-color: #D32F2F;
            color: #D32F2F;
        }

        #recentOrdersTable {
            background-color: #FFFFFF;
            border: 1px solid #E4E4E7;
            border-radius: 8px;
            gridline-color: #F0F0F0;
        }

        #recentOrdersTable QHeaderView::section {
            background-color: #F8F9FA;
            color: #666666;
            font-weight: bold;
            font-size: 12px;
            border: none;
            border-bottom: 1px solid #E4E4E7;
            padding: 8px;
        }

        QStatusBar {
            background-color: #FFFFFF;
            color: #666666;
            border-top: 1px solid #E4E4E7;
        }
    )";

    setStyleSheet(qss);
}
