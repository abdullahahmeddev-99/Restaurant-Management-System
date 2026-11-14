#include "menu_view.h"
#include "dialogs/pizza_dialog.h"
#include "dialogs/burger_dialog.h"
#include "dialogs/potatoes_dialog.h"
#include "dialogs/chicken_dialog.h"

#include "../core/meal_logic.h"
#include "../core/order_manager.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QHeaderView>
#include <QMessageBox>

MenuView::MenuView(RestaurantData& data, Order& activeOrder, QWidget *parent)
    : QWidget(parent), m_data(data), m_order(activeOrder)
{
    setupUi();
    refreshUi();
}

void MenuView::setupUi() {
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 24);
    mainLayout->setSpacing(24);

    // Left Container: Filters + Product Grid
    QVBoxLayout *leftLayout = new QVBoxLayout();
    leftLayout->setSpacing(16);

    // Filter Bar
    QWidget *filterBar = createFilterBar();
    leftLayout->addWidget(filterBar);

    // Scrollable Product Grid
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("background-color: transparent;");

    QWidget *gridWidget = createProductGrid();
    scrollArea->setWidget(gridWidget);
    leftLayout->addWidget(scrollArea);

    mainLayout->addLayout(leftLayout, 7);

    // Right Container: Active Order / Cart Preview Panel
    QWidget *cartPanel = createCartSummaryPanel();
    mainLayout->addWidget(cartPanel, 3);
}

QWidget* MenuView::createFilterBar() {
    QWidget *bar = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(bar);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);

    // Search Box
    searchEdit = new QLineEdit(bar);
    searchEdit->setPlaceholderText("🔍 Search menu items...");
    searchEdit->setFixedHeight(40);
    searchEdit->setStyleSheet("background-color: #FFFFFF; border: 1px solid #E4E4E7; border-radius: 8px; padding-left: 12px; font-size: 14px;");
    connect(searchEdit, &QLineEdit::textChanged, this, &MenuView::onSearchTextChanged);
    layout->addWidget(searchEdit, 4);

    // Category Filter Buttons
    QHBoxLayout *catLayout = new QHBoxLayout();
    catLayout->setSpacing(8);

    QStringList categories = {"All", "Pizza", "Burgers", "Sides", "Chicken"};
    for (const QString& cat : categories) {
        QPushButton *btn = new QPushButton(cat, bar);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFixedHeight(40);
        
        if (cat == "All") {
            btn->setStyleSheet("background-color: #D32F2F; color: #FFFFFF; font-weight: bold; border-radius: 8px; border: none; padding: 0px 16px;");
        } else {
            btn->setStyleSheet("background-color: #FFFFFF; color: #555555; font-weight: 600; border-radius: 8px; border: 1px solid #E4E4E7; padding: 0px 16px;");
        }

        connect(btn, &QPushButton::clicked, [this, btn, cat]() {
            filterCategory(cat);
            for (QPushButton *b : filterButtons) {
                if (b == btn) {
                    b->setStyleSheet("background-color: #D32F2F; color: #FFFFFF; font-weight: bold; border-radius: 8px; border: none; padding: 0px 16px;");
                } else {
                    b->setStyleSheet("background-color: #FFFFFF; color: #555555; font-weight: 600; border-radius: 8px; border: 1px solid #E4E4E7; padding: 0px 16px;");
                }
            }
        });

        filterButtons.append(btn);
        catLayout->addWidget(btn);
    }

    layout->addLayout(catLayout, 6);
    return bar;
}

QWidget* MenuView::createProductGrid() {
    QWidget *container = new QWidget(this);
    gridContainer = new QGridLayout(container);
    gridContainer->setContentsMargins(0, 0, 0, 0);
    gridContainer->setSpacing(16);

    // 1. Pizza Card
    QWidget *pizzaCard = createProductCard(
        "card_pizza", "Gourmet Pizza", "Pizza",
        "Delicious stone-baked pizza with rich toppings & mozzarella.",
        QString("From $%1").arg(m_data.pizza_s, 0, 'f', 2),
        m_data.pizza_stock, "🍕", SLOT(onAddPizzaClicked())
    );
    pizzaStockBadge = pizzaCard->findChild<QLabel*>("stockBadge");
    gridContainer->addWidget(pizzaCard, 0, 0);
    productCards.append(pizzaCard);

    // 2. Beef Burger Card
    QWidget *beefBurgerCard = createProductCard(
        "card_beef_burger", "Beef Burger", "Burgers",
        "Juicy grilled beef patty with fresh lettuce, cheese & special sauce.",
        QString("$%1").arg(m_data.burger_beef, 0, 'f', 2),
        m_data.burger_stock, "🍔", SLOT(onAddBeefBurgerClicked())
    );
    beefBurgerStockBadge = beefBurgerCard->findChild<QLabel*>("stockBadge");
    gridContainer->addWidget(beefBurgerCard, 0, 1);
    productCards.append(beefBurgerCard);

    // 3. Chicken Burger Card
    QWidget *chickenBurgerCard = createProductCard(
        "card_chicken_burger", "Chicken Burger", "Burgers",
        "Crispy fried chicken breast with mayo, pickles & toasted brioche bun.",
        QString("$%1").arg(m_data.burger_chicken, 0, 'f', 2),
        m_data.burger_stock, "🍔", SLOT(onAddChickenBurgerClicked())
    );
    chickenBurgerStockBadge = chickenBurgerCard->findChild<QLabel*>("stockBadge");
    gridContainer->addWidget(chickenBurgerCard, 1, 0);
    productCards.append(chickenBurgerCard);

    // 4. Potatoes Card
    QWidget *potatoesCard = createProductCard(
        "card_potatoes", "Golden French Fries", "Sides",
        "Crispy golden potatoes seasoned with signature herbs.",
        QString("$%1").arg(m_data.potatoes, 0, 'f', 2),
        m_data.potatoes_stock, "🍟", SLOT(onAddPotatoesClicked())
    );
    potatoesStockBadge = potatoesCard->findChild<QLabel*>("stockBadge");
    gridContainer->addWidget(potatoesCard, 1, 1);
    productCards.append(potatoesCard);

    // 5. Chicken Card
    QWidget *chickenCard = createProductCard(
        "card_chicken", "Roasted Chicken", "Chicken",
        "Tender seasoned chicken roasted to golden perfection.",
        QString("From $%1").arg(m_data.chicken / 4.0, 0, 'f', 2),
        m_data.chicken_stock, "🍗", SLOT(onAddChickenClicked())
    );
    chickenStockBadge = chickenCard->findChild<QLabel*>("stockBadge");
    gridContainer->addWidget(chickenCard, 2, 0);
    productCards.append(chickenCard);

    return container;
}

QWidget* MenuView::createProductCard(const QString& id, const QString& name, const QString& category, const QString& desc, const QString& priceText, int stock, const QString& icon, const char* slot) {
    QFrame *card = new QFrame(this);
    card->setObjectName("productCard");
    card->setProperty("category", category);
    card->setProperty("productName", name);
    card->setStyleSheet(
        "QFrame#productCard { background-color: #FFFFFF; border: 1px solid #E4E4E7; border-radius: 12px; }"
        "QFrame#productCard:hover { border: 1px solid #D32F2F; }"
    );

    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(8);

    // Header Row: Emoji Banner & Stock Badge
    QHBoxLayout *topRow = new QHBoxLayout();
    QLabel *iconLbl = new QLabel(icon, card);
    iconLbl->setStyleSheet("font-size: 32px; background-color: #F8F9FA; border-radius: 8px; padding: 6px;");

    QLabel *stockBadge = new QLabel(card);
    stockBadge->setObjectName("stockBadge");
    if (stock > 0) {
        stockBadge->setText(QString("Stock: %1").arg(stock));
        stockBadge->setStyleSheet("background-color: #E8F5E9; color: #2E7D32; font-size: 11px; font-weight: bold; border-radius: 4px; padding: 4px 8px;");
    } else {
        stockBadge->setText("Out of Stock");
        stockBadge->setStyleSheet("background-color: #FFEBEE; color: #C62828; font-size: 11px; font-weight: bold; border-radius: 4px; padding: 4px 8px;");
    }

    topRow->addWidget(iconLbl);
    topRow->addStretch();
    topRow->addWidget(stockBadge);
    layout->addLayout(topRow);

    // Product Title
    QLabel *titleLbl = new QLabel(name, card);
    titleLbl->setStyleSheet("font-size: 16px; font-weight: bold; color: #1E1E24;");
    layout->addWidget(titleLbl);

    // Description
    QLabel *descLbl = new QLabel(desc, card);
    descLbl->setWordWrap(true);
    descLbl->setStyleSheet("font-size: 12px; color: #666666; min-height: 32px;");
    layout->addWidget(descLbl);

    // Footer: Price & Add Button
    QHBoxLayout *bottomRow = new QHBoxLayout();
    QLabel *priceLbl = new QLabel(priceText, card);
    priceLbl->setStyleSheet("font-size: 16px; font-weight: bold; color: #D32F2F;");

    QPushButton *addBtn = new QPushButton("➕ Add", card);
    addBtn->setCursor(Qt::PointingHandCursor);
    addBtn->setFixedHeight(36);
    addBtn->setStyleSheet(
        "QPushButton { background-color: #D32F2F; color: #FFFFFF; font-weight: bold; border: none; border-radius: 6px; padding: 0px 14px; }"
        "QPushButton:hover { background-color: #B71C1C; }"
        "QPushButton:disabled { background-color: #CCCCCC; color: #666666; }"
    );
    if (stock <= 0) addBtn->setEnabled(false);

    connect(addBtn, SIGNAL(clicked()), this, slot);

    bottomRow->addWidget(priceLbl);
    bottomRow->addStretch();
    bottomRow->addWidget(addBtn);
    layout->addLayout(bottomRow);

    return card;
}

QWidget* MenuView::createCartSummaryPanel() {
    QFrame *panel = new QFrame(this);
    panel->setObjectName("cartPanel");
    panel->setStyleSheet("QFrame#cartPanel { background-color: #FFFFFF; border: 1px solid #E4E4E7; border-radius: 12px; }");

    QVBoxLayout *layout = new QVBoxLayout(panel);
    layout->setContentsMargins(18, 18, 18, 18);
    layout->setSpacing(12);

    QLabel *header = new QLabel("🛒 Active Order Summary", panel);
    header->setStyleSheet("font-size: 16px; font-weight: bold; color: #1E1E24;");
    layout->addWidget(header);

    // Cart Items Table
    cartTable = new QTableWidget(0, 3, panel);
    cartTable->setHorizontalHeaderLabels({"Item", "Qty", "Price"});
    cartTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    cartTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    cartTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    cartTable->verticalHeader()->setVisible(false);
    cartTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    cartTable->setAlternatingRowColors(true);
    cartTable->setStyleSheet(
        "QTableWidget {"
        "  border: 1px solid #E0E0E0;"
        "  border-radius: 6px;"
        "  font-size: 13px;"
        "  background-color: #FFFFFF;"
        "  gridline-color: #F0F0F0;"
        "}"
        "QTableWidget::item {"
        "  color: #1E1E24;"
        "  background-color: #FFFFFF;"
        "  padding: 4px 8px;"
        "  border: none;"
        "}"
        "QTableWidget::item:alternate {"
        "  background-color: #F8F9FA;"
        "}"
        "QTableWidget::item:selected {"
        "  background-color: #FFEBEE;"
        "  color: #D32F2F;"
        "}"
        "QHeaderView::section {"
        "  background-color: #F4F5F8;"
        "  color: #555555;"
        "  font-weight: bold;"
        "  font-size: 12px;"
        "  padding: 6px 8px;"
        "  border: none;"
        "  border-bottom: 1px solid #E0E0E0;"
        "}"
    );
    layout->addWidget(cartTable);

    // Calculation Summary Block
    QFrame *calcFrame = new QFrame(panel);
    calcFrame->setStyleSheet("background-color: #F8F9FA; border-radius: 8px; padding: 8px;");
    QVBoxLayout *calcLayout = new QVBoxLayout(calcFrame);
    calcLayout->setSpacing(6);

    QHBoxLayout *subRow = new QHBoxLayout();
    QLabel *subtotalTitle = new QLabel("Subtotal:", calcFrame);
    subtotalTitle->setObjectName("menuSummaryLabel");
    subtotalTitle->setStyleSheet("color: #1E1E24; font-size: 13px; font-weight: 600;");
    subRow->addWidget(subtotalTitle);
    subtotalLabel = new QLabel("$0.00", calcFrame);
    subtotalLabel->setObjectName("menuSummaryDarkValue");
    subtotalLabel->setStyleSheet("color: #1E1E24; font-size: 13px; font-weight: 600;");
    subtotalLabel->setAlignment(Qt::AlignRight);
    subRow->addWidget(subtotalLabel);
    calcLayout->addLayout(subRow);

    QHBoxLayout *discRow = new QHBoxLayout();
    QLabel *discountTitle = new QLabel("Discount Offers:", calcFrame);
    discountTitle->setObjectName("menuSummaryLabel");
    discountTitle->setStyleSheet("color: #1E1E24; font-size: 13px; font-weight: 600;");
    discRow->addWidget(discountTitle);
    discountLabel = new QLabel("-$0.00", calcFrame);
    discountLabel->setObjectName("menuSummaryDiscountValue");
    discountLabel->setStyleSheet("color: #2E7D32; font-size: 13px; font-weight: bold;");
    discountLabel->setAlignment(Qt::AlignRight);
    discRow->addWidget(discountLabel);
    calcLayout->addLayout(discRow);

    QHBoxLayout *taxRow = new QHBoxLayout();
    QLabel *taxTitle = new QLabel("Tax (14%):", calcFrame);
    taxTitle->setObjectName("menuSummaryLabel");
    taxTitle->setStyleSheet("color: #1E1E24; font-size: 13px; font-weight: 600;");
    taxRow->addWidget(taxTitle);
    taxLabel = new QLabel("$0.00", calcFrame);
    taxLabel->setObjectName("menuSummaryDarkValue");
    taxLabel->setStyleSheet("color: #1E1E24; font-size: 13px; font-weight: 600;");
    taxLabel->setAlignment(Qt::AlignRight);
    taxRow->addWidget(taxLabel);
    calcLayout->addLayout(taxRow);

    QFrame *line = new QFrame(calcFrame);
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("background-color: #E0E0E0;");
    calcLayout->addWidget(line);

    QHBoxLayout *totalRow = new QHBoxLayout();
    QLabel *totTitle = new QLabel("Estimated Total:", calcFrame);
    totTitle->setObjectName("menuSummaryFinalLabel");
    totTitle->setStyleSheet("color: #1E1E24; font-size: 14px; font-weight: bold;");
    totalLabel = new QLabel("$0.00", calcFrame);
    totalLabel->setObjectName("menuSummaryTotalValue");
    totalLabel->setStyleSheet("color: #D32F2F; font-size: 16px; font-weight: bold;");
    totalLabel->setAlignment(Qt::AlignRight);
    totalRow->addWidget(totTitle);
    totalRow->addWidget(totalLabel);
    calcLayout->addLayout(totalRow);

    layout->addWidget(calcFrame);

    return panel;
}

void MenuView::filterCategory(const QString& category) {
    currentCategory = category;
    onSearchTextChanged(searchEdit->text());
}

void MenuView::onSearchTextChanged(const QString& text) {
    QString searchStr = text.trimmed().toLower();
    for (QWidget *card : productCards) {
        QString cat = card->property("category").toString();
        QString name = card->property("productName").toString().toLower();

        bool catMatch = (currentCategory == "All" || cat == currentCategory);
        bool searchMatch = searchStr.isEmpty() || name.contains(searchStr);

        card->setVisible(catMatch && searchMatch);
    }
}

void MenuView::onAddPizzaClicked() {
    PizzaDialog dlg(m_data, this);
    if (dlg.exec() == QDialog::Accepted) {
        std::vector<int> sizes = dlg.getSelectedSizes();
        if (addPizzaToOrder(m_order, m_data, sizes)) {
            calculate(m_order, m_data);
            refreshUi();
            emit orderUpdated();
            emit showStatusMessage("Pizza successfully added to order!");
        } else {
            QMessageBox::warning(this, "Order Error", "Could not add pizza. Check available stock.");
        }
    }
}

void MenuView::onAddBeefBurgerClicked() {
    BurgerDialog dlg(m_data, this);
    if (dlg.exec() == QDialog::Accepted) {
        int qty = dlg.getSelectedQuantity();
        if (addBurgerToOrder(m_order, m_data, qty, 1)) {
            calculate(m_order, m_data);
            refreshUi();
            emit orderUpdated();
            emit showStatusMessage("Beef Burger added to order!");
        } else {
            QMessageBox::warning(this, "Order Error", "Could not add burger. Check available stock.");
        }
    }
}

void MenuView::onAddChickenBurgerClicked() {
    BurgerDialog dlg(m_data, this);
    if (dlg.exec() == QDialog::Accepted) {
        int qty = dlg.getSelectedQuantity();
        if (addBurgerToOrder(m_order, m_data, qty, 2)) {
            calculate(m_order, m_data);
            refreshUi();
            emit orderUpdated();
            emit showStatusMessage("Chicken Burger added to order!");
        } else {
            QMessageBox::warning(this, "Order Error", "Could not add burger. Check available stock.");
        }
    }
}

void MenuView::onAddPotatoesClicked() {
    PotatoesDialog dlg(m_data, this);
    if (dlg.exec() == QDialog::Accepted) {
        int qty = dlg.getSelectedQuantity();
        if (addPotatoesToOrder(m_order, m_data, qty)) {
            calculate(m_order, m_data);
            refreshUi();
            emit orderUpdated();
            emit showStatusMessage("Potatoes added to order!");
        } else {
            QMessageBox::warning(this, "Order Error", "Could not add potatoes. Check available stock.");
        }
    }
}

void MenuView::onAddChickenClicked() {
    ChickenDialog dlg(m_data, this);
    if (dlg.exec() == QDialog::Accepted) {
        std::vector<int> portions = dlg.getSelectedPortions();
        if (addChickenToOrder(m_order, m_data, portions)) {
            calculate(m_order, m_data);
            refreshUi();
            emit orderUpdated();
            emit showStatusMessage("Chicken added to order!");
        } else {
            QMessageBox::warning(this, "Order Error", "Could not add chicken. Check available stock.");
        }
    }
}

void MenuView::refreshUi() {
    if (pizzaStockBadge) pizzaStockBadge->setText(QString("Stock: %1").arg(m_data.pizza_stock));
    if (beefBurgerStockBadge) beefBurgerStockBadge->setText(QString("Stock: %1").arg(m_data.burger_stock));
    if (chickenBurgerStockBadge) chickenBurgerStockBadge->setText(QString("Stock: %1").arg(m_data.burger_stock));
    if (potatoesStockBadge) potatoesStockBadge->setText(QString("Stock: %1").arg(m_data.potatoes_stock));
    if (chickenStockBadge) chickenStockBadge->setText(QString("Stock: %1").arg(m_data.chicken_stock));

    cartTable->setRowCount(0);
    for (int i = 0; i < m_order.item_count; i++) {
        const auto& item = m_order.items[i];
        int row = cartTable->rowCount();
        cartTable->insertRow(row);
        cartTable->setRowHeight(row, 32);

        QString nameText = QString::fromStdString(item.name);
        if (!item.details.empty()) {
            nameText += " (" + QString::fromStdString(item.details) + ")";
        }

        auto* nameItem  = new QTableWidgetItem(nameText);
        auto* qtyItem   = new QTableWidgetItem(QString::number(item.quantity));
        auto* priceItem = new QTableWidgetItem(QString("$%1").arg(item.total_price, 0, 'f', 2));

        // Explicit foreground ensures text is visible regardless of stylesheet cascade
        QColor textColor("#1E1E24");
        nameItem->setForeground(textColor);
        qtyItem->setForeground(textColor);
        priceItem->setForeground(textColor);

        qtyItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        priceItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

        cartTable->setItem(row, 0, nameItem);
        cartTable->setItem(row, 1, qtyItem);
        cartTable->setItem(row, 2, priceItem);
    }

    subtotalLabel->setText(QString("$%1").arg(m_order.subtotal, 0, 'f', 2));
    discountLabel->setText(QString("-$%1").arg(m_order.discount, 0, 'f', 2));
    taxLabel->setText(QString("$%1").arg(m_order.tax, 0, 'f', 2));
    totalLabel->setText(QString("$%1").arg(m_order.final_total, 0, 'f', 2));
}
