#ifndef PIZZA_DIALOG_H
#define PIZZA_DIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QList>
#include <vector>
#include "../../core/models.h"

class PizzaDialog : public QDialog {
    Q_OBJECT

public:
    explicit PizzaDialog(const RestaurantData& data, QWidget *parent = nullptr);
    std::vector<int> getSelectedSizes() const;

private slots:
    void onQuantityChanged(int count);
    void updatePriceEstimate();

private:
    void setupUi();
    void applyDialogStylesheet();
    void clearSizesLayout();

    const RestaurantData& m_data;

    QSpinBox *quantitySpin;
    QWidget *sizesContainer;
    QVBoxLayout *sizesLayout;
    QList<QComboBox*> sizeComboBoxes;
    QLabel *priceEstLabel;
    QLabel *stockLabel;
};

#endif // PIZZA_DIALOG_H
