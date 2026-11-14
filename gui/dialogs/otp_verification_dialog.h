#ifndef OTP_VERIFICATION_DIALOG_H
#define OTP_VERIFICATION_DIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>

class OtpVerificationDialog : public QDialog {
    Q_OBJECT

public:
    explicit OtpVerificationDialog(double totalAmount, QWidget *parent = nullptr);

    // Returns true if OTP was verified successfully
    bool otpVerified() const { return m_verified; }

    // Get the generated OTP (for testing)
    QString getGeneratedOtp() const { return m_generatedOtp; }

private slots:
    void onVerifyClicked();
    void onOtpChanged(const QString& text);

private:
    void setupUi();
    void applyStylesheet();
    void generateOtp();
    void showError(const QString& message);
    void clearError();

    double m_totalAmount;
    QString m_generatedOtp;
    bool m_verified = false;

    // UI Elements
    QLabel *otpDisplayLabel;
    QLineEdit *otpInputEdit;
    QLabel *instructionLabel;
    QLabel *errorLabel;
    QPushButton *verifyBtn;
    QPushButton *cancelBtn;
};

#endif // OTP_VERIFICATION_DIALOG_H
