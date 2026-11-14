#include "chicken_dialog.h"
#include "quantity_spin_box.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QFrame>
#include <QSizePolicy>

ChickenDialog::ChickenDialog(const RestaurantData& data, QWidget *parent)
    : QDialog(parent), m_data(data)
{
    setWindowTitle("Select Chicken Portion");
    setMinimumWidth(540);
    setMinimumHeight(480);
    resize(560, 520);
    setupUi();
    applyDialogStylesheet();
}

void ChickenDialog::clearPortionsLayout() {
    portionComboBoxes.clear();

    if (!portionsContainer) return;

    // Safely delete all direct row QWidgets of portionsContainer.
    // Qt parent-child ownership will automatically destroy child QLabels, QComboBoxes, and row QLayouts cleanly.
    const auto children = portionsContainer->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly);
    for (QWidget *child : children) {
        delete child;
    }
}

void ChickenDialog::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(24, 24, 24, 24);

    // Header Banner Frame
    QFrame *headerFrame = new QFrame(this);
    QVBoxLayout *headerLayout = new QVBoxLayout(headerFrame);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(4);

    QLabel *titleLabel = new QLabel("🍗 Select Chicken Portion", headerFrame);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #1E1E24;");

    stockLabel = new QLabel(QString("Available Stock: %1 chickens").arg(m_data.chicken_stock), headerFrame);
    if (m_data.chicken_stock > 10) {
        stockLabel->setStyleSheet("font-size: 13px; color: #2E7D32; font-weight: 600;");
    } else {
        stockLabel->setStyleSheet("font-size: 13px; color: #C62828; font-weight: 600;");
    }

    headerLayout->addWidget(titleLabel);
    headerLayout->addWidget(stockLabel);
    mainLayout->addWidget(headerFrame);

    QFrame *div = new QFrame(this);
    div->setFrameShape(QFrame::HLine);
    div->setStyleSheet("background-color: #E5E7EB; max-height: 1px; min-height: 1px;");
    mainLayout->addWidget(div);

    // Quantity Selector Row
    QHBoxLayout *qtyLayout = new QHBoxLayout();
    qtyLayout->setSpacing(12);

    QLabel *qtyText = new QLabel("Quantity:", this);
    qtyText->setStyleSheet("font-size: 14px; font-weight: 600; color: #1E1E24;");

    quantitySpin = new QuantitySpinBox(this);
    quantitySpin->setRange(1, std::max(1, m_data.chicken_stock));
    quantitySpin->setValue(1);
    quantitySpin->setFixedSize(122, 46);
    if (m_data.chicken_stock <= 0) {
        quantitySpin->setEnabled(false);
    }

    qtyLayout->addWidget(qtyText);
    qtyLayout->addWidget(quantitySpin);
    qtyLayout->addStretch();
    mainLayout->addLayout(qtyLayout);

    QLabel *portionTitle = new QLabel("Choose Portion for Each Item:", this);
    portionTitle->setStyleSheet("font-size: 13px; font-weight: 600; color: #4B5563; margin-top: 4px;");
    mainLayout->addWidget(portionTitle);

    // Scroll Area for dynamic portion rows
    QScrollArea *scroll = new QScrollArea(this);
    scroll->setObjectName("portionsScroll");
    scroll->setWidgetResizable(true);
    scroll->setMinimumHeight(150);
    scroll->setFrameShape(QFrame::NoFrame);

    portionsContainer = new QWidget(scroll);
    portionsContainer->setObjectName("portionsContainer");
    portionsLayout = new QVBoxLayout(portionsContainer);
    portionsLayout->setSpacing(12);
    portionsLayout->setContentsMargins(12, 12, 12, 12);
    scroll->setWidget(portionsContainer);

    mainLayout->addWidget(scroll, 1);

    // Price Estimate Card
    QFrame *priceCard = new QFrame(this);
    priceCard->setStyleSheet("background-color: #FEF2F2; border: 1px solid #FCA5A5; border-radius: 8px;");
    QHBoxLayout *priceLayout = new QHBoxLayout(priceCard);
    priceLayout->setContentsMargins(16, 12, 16, 12);

    QLabel *priceTitle = new QLabel("Estimated Base Subtotal:", priceCard);
    priceTitle->setStyleSheet("font-size: 14px; font-weight: 600; color: #991B1B;");

    priceEstLabel = new QLabel("$0.00", priceCard);
    priceEstLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #D32F2F;");
    priceEstLabel->setAlignment(Qt::AlignRight);

    priceLayout->addWidget(priceTitle);
    priceLayout->addWidget(priceEstLabel);
    mainLayout->addWidget(priceCard);

    // Dialog Buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(12);

    QPushButton *cancelBtn = new QPushButton("Cancel", this);
    cancelBtn->setObjectName("btnCancel");
    cancelBtn->setFixedHeight(42);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    QPushButton *addBtn = new QPushButton("Add to Order", this);
    addBtn->setObjectName("btnAdd");
    addBtn->setFixedHeight(42);
    addBtn->setCursor(Qt::PointingHandCursor);
    if (m_data.chicken_stock <= 0) {
        addBtn->setEnabled(false);
    }
    connect(addBtn, &QPushButton::clicked, this, &QDialog::accept);

    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(addBtn);
    mainLayout->addLayout(btnLayout);

    connect(quantitySpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &ChickenDialog::onQuantityChanged);
    onQuantityChanged(1);
}

void ChickenDialog::onQuantityChanged(int count) {
    clearPortionsLayout();

    for (int i = 0; i < count; i++) {
        QWidget *rowWidget = new QWidget(portionsContainer);
        QHBoxLayout *rowLayout = new QHBoxLayout(rowWidget);
        rowLayout->setContentsMargins(0, 0, 0, 0);
        rowLayout->setSpacing(12);

        QLabel *lbl = new QLabel(QString("Item #%1:").arg(i + 1), rowWidget);
        lbl->setStyleSheet("font-size: 13px; font-weight: bold; color: #1E1E24;");
        lbl->setFixedWidth(75);

        QComboBox *combo = new QComboBox(rowWidget);
        combo->addItem(QString("Full Chicken     ($%1)").arg(m_data.chicken, 0, 'f', 2), 1);
        combo->addItem(QString("Half Chicken     ($%1)").arg(m_data.chicken / 2.0, 0, 'f', 2), 2);
        combo->addItem(QString("Quarter Chicken  ($%1)").arg(m_data.chicken / 4.0, 0, 'f', 2), 3);
        combo->setFixedHeight(38);
        combo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ChickenDialog::updatePriceEstimate);

        rowLayout->addWidget(lbl);
        rowLayout->addWidget(combo, 1);

        portionsLayout->addWidget(rowWidget);
        portionComboBoxes.append(combo);
    }

    updatePriceEstimate();
}

void ChickenDialog::updatePriceEstimate() {
    double estTotal = 0.0;
    for (QComboBox *combo : portionComboBoxes) {
        int portion = combo->currentData().toInt();
        if (portion == 1) estTotal += m_data.chicken;
        else if (portion == 2) estTotal += m_data.chicken / 2.0;
        else estTotal += m_data.chicken / 4.0;
    }
    priceEstLabel->setText(QString("$%1").arg(estTotal, 0, 'f', 2));
}

std::vector<int> ChickenDialog::getSelectedPortions() const {
    std::vector<int> portions;
    for (QComboBox *combo : portionComboBoxes) {
        portions.push_back(combo->currentData().toInt());
    }
    return portions;
}

void ChickenDialog::applyDialogStylesheet() {
    QString qss = R"(
        QDialog {
            background-color: #FFFFFF;
        }

        QWidget#portionsContainer {
            background-color: #F9FAFB;
        }

        QLabel {
            color: #1E1E24;
            font-family: 'Segoe UI', sans-serif;
            background-color: transparent;
        }

        QSpinBox {
            background-color: #FFFFFF;
            color: #1E1E24;
            border: 1px solid #D1D5DB;
            border-radius: 6px;
            padding: 4px 30px 4px 10px;
            font-size: 14px;
            font-weight: bold;
        }

        QSpinBox:focus {
            border: 2px solid #D32F2F;
        }

        QSpinBox::up-button, QSpinBox::down-button {
            subcontrol-origin: border;
            width: 28px;
            border-left: 1px solid #FFFFFF;
        }
        QSpinBox::up-button { subcontrol-position: top right; background-color: #16A34A; border-bottom: 1px solid #FFFFFF; border-top-right-radius: 5px; }
        QSpinBox::down-button { subcontrol-position: bottom right; background-color: #DC2626; border-top: 1px solid #FFFFFF; border-bottom-right-radius: 5px; }
        QSpinBox::up-button:hover { background-color: #15803D; }
        QSpinBox::down-button:hover { background-color: #B91C1C; }
        QSpinBox::up-button:pressed { background-color: #166534; }
        QSpinBox::down-button:pressed { background-color: #991B1B; }
        QSpinBox::up-button:disabled, QSpinBox::down-button:disabled { background-color: #E5E7EB; border-color: #D1D5DB; }
        QSpinBox::up-arrow, QSpinBox::down-arrow { image: none; width: 0; height: 0; }

        QComboBox {
            background-color: #FFFFFF;
            color: #1E1E24;
            border: 1px solid #D1D5DB;
            border-radius: 6px;
            padding: 4px 12px;
            font-size: 13px;
            font-weight: 500;
        }

        QComboBox:focus, QComboBox:on {
            border: 2px solid #D32F2F;
        }

        QComboBox QAbstractItemView {
            background-color: #FFFFFF;
            color: #1E1E24;
            border: 1px solid #D1D5DB;
            selection-background-color: #D32F2F;
            selection-color: #FFFFFF;
            outline: 0;
            padding: 4px;
        }

        #portionsScroll {
            background-color: #F9FAFB;
            border: 1px solid #E5E7EB;
            border-radius: 8px;
        }

        #btnCancel {
            background-color: #F3F4F6;
            color: #374151;
            border: 1px solid #D1D5DB;
            border-radius: 6px;
            font-weight: 600;
            font-size: 14px;
        }

        #btnCancel:hover {
            background-color: #E5E7EB;
            color: #111827;
        }

        #btnAdd {
            background-color: #D32F2F;
            color: #FFFFFF;
            border: none;
            border-radius: 6px;
            font-weight: bold;
            font-size: 14px;
        }

        #btnAdd:hover {
            background-color: #B71C1C;
        }

        #btnAdd:disabled {
            background-color: #E5E7EB;
            color: #9CA3AF;
        }
    )";

    setStyleSheet(qss);
}
