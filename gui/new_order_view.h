#ifndef NEW_ORDER_VIEW_H
#define NEW_ORDER_VIEW_H

#include <QWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>

#include "../core/models.h"

class NewOrderView : public QWidget {
    Q_OBJECT

public:
    explicit NewOrderView(RestaurantData& data, Order& activeOrder, QWidget *parent = nullptr);
    void refreshUi();

signals:
    void orderUpdated();
    void showStatusMessage(const QString& message);
    void navigateToMenuRequested();

private slots:
    void onNameChanged(const QString& text);
    void onPhoneChanged(const QString& text);
    void onOrderTypeChanged(int id);
    void onTableNumberChanged(int value);
    void onDiscountCardToggled(bool checked);
    void onRemoveItemClicked(int index);
    void onChangeItemQuantityClicked(int index, int delta);
    void onCancelOrderClicked();
    void onContinueToPaymentClicked();

private:
    void setupUi();
    QWidget* createCustomerFormPanel();
    QWidget* createCartSummaryPanel();
    void applyViewStylesheet();

    RestaurantData& m_data;
    Order& m_order;

    // Customer Form Inputs
    QLineEdit *nameEdit;
    QLabel *nameValidationLabel;
    QLineEdit *phoneEdit;
    QLabel *phoneValidationLabel;

    QButtonGroup *orderTypeGroup;
    QRadioButton *dineInRadio;
    QRadioButton *takeawayRadio;
    QRadioButton *deliveryRadio;

    QWidget *tableNumberContainer;
    QSpinBox *tableNumberSpin;

    QCheckBox *discountCardCheckBox;

    // Cart Panel Widgets
    QTableWidget *cartTable;
    QWidget *emptyCartStateWidget;
    QWidget *cartContentWidget;

    // Summary Calculation Labels
    QLabel *subtotalLabel;
    QLabel *discountLabel;
    QWidget *deliveryFeeRowWidget;
    QLabel *deliveryFeeLabel;
    QLabel *taxLabel;
    QLabel *finalTotalLabel;

    // Action Buttons
    QPushButton *browseMenuBtn;
    QPushButton *continuePaymentBtn;
    QPushButton *cancelOrderBtn;
};

#endif // NEW_ORDER_VIEW_H
