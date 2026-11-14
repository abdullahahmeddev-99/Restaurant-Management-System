#ifndef PAYMENT_DIALOG_H
#define PAYMENT_DIALOG_H

#include <QDialog>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QPushButton>
#include <QWidget>

#include "../../core/models.h"

class PaymentDialog : public QDialog {
    Q_OBJECT

public:
    explicit PaymentDialog(Order& order, RestaurantData& data, QWidget *parent = nullptr);

    // Returns true if payment was successfully confirmed
    bool paymentConfirmed() const { return m_confirmed; }

private slots:
    void onPaymentMethodChanged(int id);
    void onPaidAmountChanged(double value);
    void onConfirmClicked();

private:
    void setupUi();
    void applyStylesheet();
    void updateChangeDisplay();
    bool processCreditCardPayment();

    Order&          m_order;
    RestaurantData& m_data;
    bool            m_confirmed = false;

    // Method selection
    QButtonGroup    *methodGroup;
    QRadioButton    *cashRadio;
    QRadioButton    *cardRadio;
    QRadioButton    *walletRadio;

    // Amount inputs / display
    QWidget         *cashInputWidget;
    QDoubleSpinBox  *paidAmountSpin;
    QLabel          *changeLabel;
    QLabel          *changeValueLabel;
    QLabel          *totalValueLabel;
    QLabel          *insufficientLabel;

    QPushButton     *confirmBtn;
    QPushButton     *cancelBtn;
};

#endif // PAYMENT_DIALOG_H
