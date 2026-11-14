#include "otp_verification_dialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QRandomGenerator>
#include <QTime>

OtpVerificationDialog::OtpVerificationDialog(double totalAmount, QWidget *parent)
    : QDialog(parent), m_totalAmount(totalAmount)
{
    setWindowTitle("OTP Verification");
    setMinimumWidth(450);
    setMinimumHeight(400);
    resize(480, 420);
    setModal(true);
    generateOtp();
    setupUi();
    applyStylesheet();
}

void OtpVerificationDialog::generateOtp() {
    // Generate a 6-digit OTP for testing
    quint32 otp = QRandomGenerator::global()->bounded(100000, 999999);
    m_generatedOtp = QString::number(otp);
}

void OtpVerificationDialog::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(28, 24, 28, 24);
    mainLayout->setSpacing(18);

    // ── Header ──────────────────────────────────────────────────────────────
    QLabel *titleLbl = new QLabel("🔐 Verify Payment", this);
    titleLbl->setStyleSheet("font-size: 20px; font-weight: bold; color: #1E1E24;");
    mainLayout->addWidget(titleLbl);

    QLabel *amountLbl = new QLabel(QString("Amount: $%1").arg(m_totalAmount, 0, 'f', 2), this);
    amountLbl->setStyleSheet("font-size: 14px; color: #D32F2F; font-weight: bold;");
    mainLayout->addWidget(amountLbl);

    QFrame *divider = new QFrame(this);
    divider->setFrameShape(QFrame::HLine);
    divider->setStyleSheet("background-color: #E5E7EB; max-height: 1px;");
    mainLayout->addWidget(divider);

    // ── Instruction ──────────────────────────────────────────────────────────
    instructionLabel = new QLabel(
        "A verification code has been sent to your registered mobile number.\n"
        "Enter the code below to complete the payment verification.",
        this
    );
    instructionLabel->setStyleSheet("font-size: 13px; color: #555555; line-height: 1.5;");
    instructionLabel->setWordWrap(true);
    mainLayout->addWidget(instructionLabel);

    // ── OTP Display (for testing only) ───────────────────────────────────────
    QFrame *otpDisplayFrame = new QFrame(this);
    otpDisplayFrame->setObjectName("otpDisplayFrame");
    otpDisplayFrame->setStyleSheet(R"(
        QFrame#otpDisplayFrame {
            background-color: #FEF3C7;
            border: 1px solid #FCD34D;
            border-radius: 8px;
            padding: 12px;
        }
    )");

    QVBoxLayout *otpFrameLayout = new QVBoxLayout(otpDisplayFrame);
    otpFrameLayout->setContentsMargins(12, 12, 12, 12);

    QLabel *testLabel = new QLabel("📝 Test OTP (for development):", otpDisplayFrame);
    testLabel->setStyleSheet("font-size: 12px; color: #92400E; font-weight: bold;");

    otpDisplayLabel = new QLabel(m_generatedOtp, otpDisplayFrame);
    otpDisplayLabel->setStyleSheet("font-size: 18px; color: #D97706; font-weight: bold; font-family: monospace;");
    otpDisplayLabel->setAlignment(Qt::AlignCenter);

    otpFrameLayout->addWidget(testLabel);
    otpFrameLayout->addWidget(otpDisplayLabel);
    mainLayout->addWidget(otpDisplayFrame);

    // ── OTP Input ────────────────────────────────────────────────────────────
    QLabel *otpLabel = new QLabel("Enter OTP (6 digits):", this);
    otpLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #1E1E24;");
    mainLayout->addWidget(otpLabel);

    otpInputEdit = new QLineEdit(this);
    otpInputEdit->setPlaceholderText("Enter 6-digit code");
    otpInputEdit->setMaxLength(6);
    otpInputEdit->setFixedHeight(40);
    otpInputEdit->setAlignment(Qt::AlignCenter);
    otpInputEdit->setStyleSheet("font-size: 16px; font-weight: bold; font-family: monospace; letter-spacing: 5px;");
    connect(otpInputEdit, &QLineEdit::textChanged, this, &OtpVerificationDialog::onOtpChanged);
    mainLayout->addWidget(otpInputEdit);

    // ── Error Label ──────────────────────────────────────────────────────────
    errorLabel = new QLabel(this);
    errorLabel->setStyleSheet("font-size: 12px; color: #C62828; font-weight: 600;");
    errorLabel->setWordWrap(true);
    errorLabel->setVisible(false);
    mainLayout->addWidget(errorLabel);

    mainLayout->addStretch();

    // ── Action Buttons ───────────────────────────────────────────────────────
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(14);

    cancelBtn = new QPushButton("✖  Cancel", this);
    cancelBtn->setObjectName("btnOtpCancel");
    cancelBtn->setFixedHeight(46);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    verifyBtn = new QPushButton("✔  Verify Payment", this);
    verifyBtn->setObjectName("btnOtpVerify");
    verifyBtn->setFixedHeight(46);
    verifyBtn->setCursor(Qt::PointingHandCursor);
    verifyBtn->setEnabled(false);
    connect(verifyBtn, &QPushButton::clicked, this, &OtpVerificationDialog::onVerifyClicked);

    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(verifyBtn, 1);
    mainLayout->addLayout(btnLayout);
}

void OtpVerificationDialog::onOtpChanged(const QString& text) {
    // Only allow digits
    QString filtered;
    for (QChar c : text) {
        if (c.isDigit()) {
            filtered += c;
        }
    }
    if (filtered != text) {
        otpInputEdit->blockSignals(true);
        otpInputEdit->setText(filtered);
        otpInputEdit->blockSignals(false);
    }

    // Enable verify button only if 6 digits are entered
    verifyBtn->setEnabled(otpInputEdit->text().length() == 6);
}

void OtpVerificationDialog::onVerifyClicked() {
    clearError();

    QString enteredOtp = otpInputEdit->text().trimmed();

    if (enteredOtp.isEmpty() || enteredOtp.length() != 6) {
        showError("⚠ Please enter a valid 6-digit OTP.");
        return;
    }

    if (enteredOtp != m_generatedOtp) {
        showError("⚠ Incorrect OTP. Please try again.");
        otpInputEdit->clear();
        otpInputEdit->setFocus();
        verifyBtn->setEnabled(false);
        return;
    }

    // OTP verified
    m_verified = true;
    accept();
}

void OtpVerificationDialog::showError(const QString& message) {
    errorLabel->setText(message);
    errorLabel->setVisible(true);
}

void OtpVerificationDialog::clearError() {
    errorLabel->setVisible(false);
}

void OtpVerificationDialog::applyStylesheet() {
    setStyleSheet(R"(
        QDialog {
            background-color: #FFFFFF;
        }

        QLabel {
            color: #1E1E24;
            background-color: transparent;
        }

        QLineEdit {
            background-color: #FFFFFF;
            color: #1E1E24;
            border: 1px solid #D1D5DB;
            border-radius: 6px;
            padding: 8px 12px;
            font-size: 14px;
        }

        QLineEdit:focus {
            border: 2px solid #D32F2F;
        }

        #btnOtpCancel {
            background-color: #F3F4F6;
            color: #374151;
            border: 1px solid #D1D5DB;
            border-radius: 8px;
            font-size: 14px;
            font-weight: 600;
        }

        #btnOtpCancel:hover {
            background-color: #E5E7EB;
        }

        #btnOtpVerify {
            background-color: #D32F2F;
            color: #FFFFFF;
            border: none;
            border-radius: 8px;
            font-size: 14px;
            font-weight: 600;
        }

        #btnOtpVerify:hover:!disabled {
            background-color: #B71C1C;
        }

        #btnOtpVerify:pressed {
            background-color: #9A1515;
        }

        #btnOtpVerify:disabled {
            background-color: #CCCCCC;
            color: #999999;
        }
    )");
}
