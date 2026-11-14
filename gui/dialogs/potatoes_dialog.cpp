#include "potatoes_dialog.h"
#include "quantity_spin_box.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFrame>

PotatoesDialog::PotatoesDialog(const RestaurantData& data, QWidget *parent)
    : QDialog(parent), m_data(data)
{
    setWindowTitle("Select Potatoes Portion");
    setMinimumWidth(480);
    setMinimumHeight(380);
    resize(500, 400);
    setupUi();
    applyDialogStylesheet();
}

void PotatoesDialog::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(24, 24, 24, 24);

    // Header Banner Frame
    QFrame *headerFrame = new QFrame(this);
    QVBoxLayout *headerLayout = new QVBoxLayout(headerFrame);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(4);

    QLabel *titleLabel = new QLabel("🍟 Select Potatoes", headerFrame);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #1E1E24;");

    stockLabel = new QLabel(QString("Available Stock: %1 portions").arg(m_data.potatoes_stock), headerFrame);
    if (m_data.potatoes_stock > 10) {
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

    // Product Info Card
    QFrame *infoCard = new QFrame(this);
    infoCard->setStyleSheet("background-color: #F9FAFB; border: 1px solid #E5E7EB; border-radius: 8px;");
    QHBoxLayout *infoLayout = new QHBoxLayout(infoCard);
    infoLayout->setContentsMargins(16, 14, 16, 14);

    QLabel *unitPriceText = new QLabel("French Fries Regular Portion:", infoCard);
    unitPriceText->setStyleSheet("font-size: 14px; font-weight: 600; color: #4B5563;");

    QLabel *unitPriceVal = new QLabel(QString("$%1").arg(m_data.potatoes, 0, 'f', 2), infoCard);
    unitPriceVal->setStyleSheet("font-size: 16px; font-weight: bold; color: #D32F2F;");
    unitPriceVal->setAlignment(Qt::AlignRight);

    infoLayout->addWidget(unitPriceText);
    infoLayout->addStretch();
    infoLayout->addWidget(unitPriceVal);
    mainLayout->addWidget(infoCard);

    // Quantity Selector Row
    QHBoxLayout *qtyLayout = new QHBoxLayout();
    qtyLayout->setSpacing(12);

    QLabel *qtyText = new QLabel("Quantity:", this);
    qtyText->setStyleSheet("font-size: 14px; font-weight: 600; color: #1E1E24;");

    quantitySpin = new QuantitySpinBox(this);
    quantitySpin->setRange(1, std::max(1, m_data.potatoes_stock));
    quantitySpin->setValue(1);
    quantitySpin->setFixedSize(122, 46);
    if (m_data.potatoes_stock <= 0) {
        quantitySpin->setEnabled(false);
    }

    qtyLayout->addWidget(qtyText);
    qtyLayout->addWidget(quantitySpin);
    qtyLayout->addStretch();
    mainLayout->addLayout(qtyLayout);

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
    if (m_data.potatoes_stock <= 0) {
        addBtn->setEnabled(false);
    }
    connect(addBtn, &QPushButton::clicked, this, &QDialog::accept);

    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(addBtn);
    mainLayout->addLayout(btnLayout);

    connect(quantitySpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &PotatoesDialog::updatePriceEstimate);

    updatePriceEstimate();
}

void PotatoesDialog::updatePriceEstimate() {
    double subtotal = m_data.potatoes * getSelectedQuantity();
    priceEstLabel->setText(QString("$%1").arg(subtotal, 0, 'f', 2));
}

int PotatoesDialog::getSelectedQuantity() const {
    return quantitySpin->value();
}

void PotatoesDialog::applyDialogStylesheet() {
    QString qss = R"(
        QDialog {
            background-color: #FFFFFF;
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
