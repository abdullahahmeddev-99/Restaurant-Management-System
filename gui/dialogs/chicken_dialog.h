#ifndef CHICKEN_DIALOG_H
#define CHICKEN_DIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QList>
#include <vector>
#include "../../core/models.h"

class ChickenDialog : public QDialog {
    Q_OBJECT

public:
    explicit ChickenDialog(const RestaurantData& data, QWidget *parent = nullptr);
    std::vector<int> getSelectedPortions() const; // 1 = Full, 2 = Half, 3 = Quarter

private slots:
    void onQuantityChanged(int count);
    void updatePriceEstimate();

private:
    void setupUi();
    void applyDialogStylesheet();
    void clearPortionsLayout();

    const RestaurantData& m_data;

    QSpinBox *quantitySpin;
    QWidget *portionsContainer;
    QVBoxLayout *portionsLayout;
    QList<QComboBox*> portionComboBoxes;
    QLabel *priceEstLabel;
    QLabel *stockLabel;
};

#endif // CHICKEN_DIALOG_H
