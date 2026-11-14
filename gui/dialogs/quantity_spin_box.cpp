#include "quantity_spin_box.h"

#include <QPainter>
#include <QStyle>
#include <QStyleOptionSpinBox>

void QuantitySpinBox::paintEvent(QPaintEvent *event) {
    QSpinBox::paintEvent(event);

    QStyleOptionSpinBox option;
    initStyleOption(&option);
    const auto enabledSteps = stepEnabled();

    QPainter painter(this);
    painter.setFont(QFont("Segoe UI", 16, QFont::Bold));

    const auto drawStepSymbol = [&](QStyle::SubControl control, const QString& symbol, bool enabled) {
        const QRect rect = style()->subControlRect(QStyle::CC_SpinBox, &option, control, this);
        painter.setPen(enabled ? QColor("#FFFFFF") : QColor("#4B5563"));
        painter.drawText(rect, Qt::AlignCenter, symbol);
    };

    drawStepSymbol(QStyle::SC_SpinBoxUp, "+", enabledSteps & QAbstractSpinBox::StepUpEnabled);
    drawStepSymbol(QStyle::SC_SpinBoxDown, "−", enabledSteps & QAbstractSpinBox::StepDownEnabled);
}
