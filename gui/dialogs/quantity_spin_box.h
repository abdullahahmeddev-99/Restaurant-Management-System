#ifndef QUANTITY_SPIN_BOX_H
#define QUANTITY_SPIN_BOX_H

#include <QSpinBox>

// Keeps Qt's native spinbox interaction while making its step indicators
// readable when a stylesheet replaces the platform arrows.
class QuantitySpinBox : public QSpinBox {
public:
    using QSpinBox::QSpinBox;

protected:
    void paintEvent(QPaintEvent *event) override;
};

#endif // QUANTITY_SPIN_BOX_H
