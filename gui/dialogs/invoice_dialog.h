#ifndef INVOICE_DIALOG_H
#define INVOICE_DIALOG_H

#include <QDialog>
#include "../../core/models.h"

class InvoiceDialog : public QDialog {
    Q_OBJECT

public:
    explicit InvoiceDialog(const Order& order, QWidget *parent = nullptr);

private:
    void setupUi();
    void applyStylesheet();

    const Order& m_order;
};

#endif // INVOICE_DIALOG_H
