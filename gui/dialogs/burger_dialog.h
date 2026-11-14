#ifndef BURGER_DIALOG_H
#define BURGER_DIALOG_H

#include <QDialog>
#include <QRadioButton>
#include <QSpinBox>
#include <QLabel>
#include <QButtonGroup>
#include "../../core/models.h"

class BurgerDialog : public QDialog {
    Q_OBJECT

public:
    explicit BurgerDialog(const RestaurantData& data, QWidget *parent = nullptr);
    int getSelectedQuantity() const;
    int getSelectedType() const; // 1 = Beef, 2 = Chicken

private slots:
    void updatePriceEstimate();

private:
    void setupUi();
    void applyDialogStylesheet();

    const RestaurantData& m_data;

    QRadioButton *beefRadio;
    QRadioButton *chickenRadio;
    QButtonGroup *typeGroup;

    QSpinBox *quantitySpin;
    QLabel *priceEstLabel;
    QLabel *stockLabel;
};

#endif // BURGER_DIALOG_H
