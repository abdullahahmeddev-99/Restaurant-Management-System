#include "card_details_dialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QRegularExpression>
#include <QMessageBox>

CardDetailsDialog::CardDetailsDialog(double totalAmount, QWidget *parent)
    : QDialog(parent), m_totalAmount(totalAmount)
{
    setWindowTitle("Credit Card Details");
    setMinimumWidth(500);
    setMinimumHeight(450);
    resize(520, 480);
    setModal(true);
    setupUi();
    applyStylesheet();
}

void CardDetailsDialog::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(28, 24, 28, 24);
    mainLayout->setSpacing(18);

    // ── Header ──────────────────────────────────────────────────────────────
    QLabel *titleLbl = new QLabel("💳 Enter Card Details", this);
    titleLbl->setStyleSheet("font-size: 20px; font-weight: bold; color: #1E1E24;");
    mainLayout->addWidget(titleLbl);

    QLabel *amountLbl = new QLabel(QString("Amount: $%1").arg(m_totalAmount, 0, 'f', 2), this);
    amountLbl->setStyleSheet("font-size: 14px; color: #D32F2F; font-weight: bold;");
    mainLayout->addWidget(amountLbl);

    QFrame *divider = new QFrame(this);
    divider->setFrameShape(QFrame::HLine);
    divider->setStyleSheet("background-color: #E5E7EB; max-height: 1px;");
    mainLayout->addWidget(divider);

    // ── Card Number ──────────────────────────────────────────────────────────
    QLabel *cardNumLabel = new QLabel("Card Number (16 digits):", this);
    cardNumLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #1E1E24;");
    mainLayout->addWidget(cardNumLabel);

    cardNumberEdit = new QLineEdit(this);
    cardNumberEdit->setObjectName("cardNumberEdit");
    cardNumberEdit->setPlaceholderText("e.g. 4532123456789012");
    cardNumberEdit->setMaxLength(16);
    cardNumberEdit->setFixedHeight(40);
    connect(cardNumberEdit, &QLineEdit::textChanged, this, &CardDetailsDialog::onCardNumberChanged);
    mainLayout->addWidget(cardNumberEdit);

    // ── Cardholder Name ──────────────────────────────────────────────────────
    QLabel *nameLabel = new QLabel("Cardholder Name:", this);
    nameLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #1E1E24;");
    mainLayout->addWidget(nameLabel);

    cardholderNameEdit = new QLineEdit(this);
    cardholderNameEdit->setObjectName("cardholderNameEdit");
    cardholderNameEdit->setPlaceholderText("Full name as it appears on card");
    cardholderNameEdit->setFixedHeight(40);
    mainLayout->addWidget(cardholderNameEdit);

    // ── Expiry Date & CVV ────────────────────────────────────────────────────
    QHBoxLayout *expiryRow = new QHBoxLayout();
    expiryRow->setSpacing(12);

    QVBoxLayout *expiryCol = new QVBoxLayout();
    QLabel *expiryLabel = new QLabel("Expiry Date (MM/YY):", this);
    expiryLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #1E1E24;");
    expiryDateEdit = new QLineEdit(this);
    expiryDateEdit->setObjectName("expiryDateEdit");
    expiryDateEdit->setPlaceholderText("MM/YY");
    expiryDateEdit->setMaxLength(5);
    expiryDateEdit->setFixedHeight(40);
    connect(expiryDateEdit, &QLineEdit::textChanged, this, &CardDetailsDialog::onExpiryDateChanged);
    expiryCol->addWidget(expiryLabel);
    expiryCol->addWidget(expiryDateEdit);

    QVBoxLayout *cvvCol = new QVBoxLayout();
    QLabel *cvvLabel = new QLabel("CVV (3 digits):", this);
    cvvLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #1E1E24;");
    cvvEdit = new QLineEdit(this);
    cvvEdit->setObjectName("cvvEdit");
    cvvEdit->setPlaceholderText("CVV");
    cvvEdit->setMaxLength(3);
    cvvEdit->setEchoMode(QLineEdit::Password);
    cvvEdit->setFixedHeight(40);
    connect(cvvEdit, &QLineEdit::textChanged, this, &CardDetailsDialog::onCvvChanged);
    cvvCol->addWidget(cvvLabel);
    cvvCol->addWidget(cvvEdit);

    expiryRow->addLayout(expiryCol, 1);
    expiryRow->addLayout(cvvCol, 1);
    mainLayout->addLayout(expiryRow);

    // ── Validation Error Label ───────────────────────────────────────────────
    validationLabel = new QLabel(this);
    validationLabel->setStyleSheet("font-size: 12px; color: #C62828; font-weight: 600;");
    validationLabel->setWordWrap(true);
    validationLabel->setVisible(false);
    mainLayout->addWidget(validationLabel);

    mainLayout->addStretch();

    // ── Action Buttons ───────────────────────────────────────────────────────
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(14);

    cancelBtn = new QPushButton("✖  Cancel", this);
    cancelBtn->setObjectName("btnCardCancel");
    cancelBtn->setFixedHeight(46);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    confirmBtn = new QPushButton("✔  Verify Card", this);
    confirmBtn->setObjectName("btnCardConfirm");
    confirmBtn->setFixedHeight(46);
    confirmBtn->setCursor(Qt::PointingHandCursor);
    connect(confirmBtn, &QPushButton::clicked, this, &CardDetailsDialog::onConfirmClicked);

    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(confirmBtn, 1);
    mainLayout->addLayout(btnLayout);
}

void CardDetailsDialog::onCardNumberChanged(const QString& text) {
    // Only allow digits
    QString filtered;
    for (QChar c : text) {
        if (c.isDigit()) {
            filtered += c;
        }
    }
    if (filtered != text) {
        cardNumberEdit->blockSignals(true);
        cardNumberEdit->setText(filtered);
        cardNumberEdit->blockSignals(false);
    }
}

void CardDetailsDialog::onExpiryDateChanged(const QString& text) {
    // Format as MM/YY automatically
    QString filtered;
    for (QChar c : text) {
        if (c.isDigit() || c == '/') {
            filtered += c;
        }
    }

    if (filtered.length() == 2 && !filtered.contains('/')) {
        filtered += '/';
    }

    if (filtered != text) {
        expiryDateEdit->blockSignals(true);
        expiryDateEdit->setText(filtered);
        expiryDateEdit->blockSignals(false);
    }
}

void CardDetailsDialog::onCvvChanged(const QString& text) {
    // Only allow digits
    QString filtered;
    for (QChar c : text) {
        if (c.isDigit()) {
            filtered += c;
        }
    }
    if (filtered != text) {
        cvvEdit->blockSignals(true);
        cvvEdit->setText(filtered);
        cvvEdit->blockSignals(false);
    }
}

bool CardDetailsDialog::validateCardDetails() {
    clearValidationError();

    QString cardNum = cardNumberEdit->text().trimmed();
    QString cardName = cardholderNameEdit->text().trimmed();
    QString expiry = expiryDateEdit->text().trimmed();
    QString cvv = cvvEdit->text().trimmed();

    // Validate Card Number
    if (cardNum.isEmpty()) {
        showValidationError("⚠ Card Number is required.");
        cardNumberEdit->setFocus();
        return false;
    }
    if (cardNum.length() != 16) {
        showValidationError("⚠ Card Number must be exactly 16 digits.");
        cardNumberEdit->setFocus();
        return false;
    }
    if (!cardNum.at(0).isDigit()) {
        showValidationError("⚠ Card Number must contain only digits.");
        cardNumberEdit->setFocus();
        return false;
    }

    // Validate Cardholder Name
    if (cardName.isEmpty()) {
        showValidationError("⚠ Cardholder Name is required.");
        cardholderNameEdit->setFocus();
        return false;
    }
    if (cardName.length() < 3) {
        showValidationError("⚠ Cardholder Name must be at least 3 characters.");
        cardholderNameEdit->setFocus();
        return false;
    }

    // Validate Expiry Date
    if (expiry.isEmpty()) {
        showValidationError("⚠ Expiry Date is required.");
        expiryDateEdit->setFocus();
        return false;
    }
    QRegularExpression expiryRegex("^(0[1-9]|1[0-2])/\\d{2}$");
    if (!expiryRegex.match(expiry).hasMatch()) {
        showValidationError("⚠ Expiry Date must be in MM/YY format (e.g. 12/25).");
        expiryDateEdit->setFocus();
        return false;
    }

    // Validate CVV
    if (cvv.isEmpty()) {
        showValidationError("⚠ CVV is required.");
        cvvEdit->setFocus();
        return false;
    }
    if (cvv.length() != 3) {
        showValidationError("⚠ CVV must be exactly 3 digits.");
        cvvEdit->setFocus();
        return false;
    }

    return true;
}

void CardDetailsDialog::showValidationError(const QString& message) {
    validationLabel->setText(message);
    validationLabel->setVisible(true);
}

void CardDetailsDialog::clearValidationError() {
    validationLabel->setVisible(false);
}

void CardDetailsDialog::onConfirmClicked() {
    if (!validateCardDetails()) {
        return;
    }

    m_cardDetails.cardNumber = cardNumberEdit->text().trimmed();
    m_cardDetails.cardholderName = cardholderNameEdit->text().trimmed();
    m_cardDetails.expiryDate = expiryDateEdit->text().trimmed();
    m_cardDetails.cvv = cvvEdit->text().trimmed();

    accept();
}

void CardDetailsDialog::applyStylesheet() {
    setStyleSheet(R"(
        QDialog {
            background-color: #FFFFFF;
        }

        QLabel {
            color: #1E1E24;
            background-color: transparent;
        }

        QLineEdit {
            background-color: #FFFFFF;
            color: #1E1E24;
            border: 1px solid #D1D5DB;
            border-radius: 6px;
            padding: 8px 12px;
            font-size: 14px;
        }

        QLineEdit:focus {
            border: 2px solid #D32F2F;
        }

        #btnCardCancel {
            background-color: #F3F4F6;
            color: #374151;
            border: 1px solid #D1D5DB;
            border-radius: 8px;
            font-size: 14px;
            font-weight: 600;
        }

        #btnCardCancel:hover {
            background-color: #E5E7EB;
        }

        #btnCardConfirm {
            background-color: #D32F2F;
            color: #FFFFFF;
            border: none;
            border-radius: 8px;
            font-size: 14px;
            font-weight: 600;
        }

        #btnCardConfirm:hover {
            background-color: #B71C1C;
        }

        #btnCardConfirm:pressed {
            background-color: #9A1515;
        }
    )");
}
