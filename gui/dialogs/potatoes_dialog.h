#ifndef POTATOES_DIALOG_H
#define POTATOES_DIALOG_H

#include <QDialog>
#include <QSpinBox>
#include <QLabel>
#include "../../core/models.h"

class PotatoesDialog : public QDialog {
    Q_OBJECT

public:
    explicit PotatoesDialog(const RestaurantData& data, QWidget *parent = nullptr);
    int getSelectedQuantity() const;

private slots:
    void updatePriceEstimate();

private:
    void setupUi();
    void applyDialogStylesheet();

    const RestaurantData& m_data;

    QSpinBox *quantitySpin;
    QLabel *priceEstLabel;
    QLabel *stockLabel;
};

#endif // POTATOES_DIALOG_H
