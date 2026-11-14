#ifndef CARD_DETAILS_DIALOG_H
#define CARD_DETAILS_DIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>

class CardDetailsDialog : public QDialog {
    Q_OBJECT

public:
    explicit CardDetailsDialog(double totalAmount, QWidget *parent = nullptr);

    // Returns the card details if accepted
    struct CardDetails {
        QString cardNumber;
        QString cardholderName;
        QString expiryDate;
        QString cvv;
    };

    CardDetails getCardDetails() const { return m_cardDetails; }

private slots:
    void onConfirmClicked();
    void onCardNumberChanged(const QString& text);
    void onExpiryDateChanged(const QString& text);
    void onCvvChanged(const QString& text);

private:
    void setupUi();
    void applyStylesheet();
    bool validateCardDetails();
    void showValidationError(const QString& message);
    void clearValidationError();

    double m_totalAmount;
    CardDetails m_cardDetails;

    // UI Elements
    QLineEdit *cardNumberEdit;
    QLineEdit *cardholderNameEdit;
    QLineEdit *expiryDateEdit;
    QLineEdit *cvvEdit;
    QLabel *validationLabel;
    QPushButton *confirmBtn;
    QPushButton *cancelBtn;
};

#endif // CARD_DETAILS_DIALOG_H
