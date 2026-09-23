#include <algorithm>
#include <cmath>

#include <QApplication>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMainWindow>
#include <QPainter>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include "hmi/dashboard_view_model.hpp"

namespace {

QString alarm_text(const domain::AlarmSeverity alarm) {
    switch (alarm) {
    case domain::AlarmSeverity::low_low: return "LOW LOW";
    case domain::AlarmSeverity::high_high: return "HIGH HIGH";
    case domain::AlarmSeverity::communication: return "COMMUNICATION";
    case domain::AlarmSeverity::none: return "NORMAL";
    }
    return "UNKNOWN";
}

class TrendWidget final : public QWidget {
public:
    explicit TrendWidget(hmi::DashboardViewModel& model, QWidget* parent = nullptr)
        : QWidget(parent), model_(model) {
        setMinimumHeight(210);
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        painter.fillRect(rect(), QColor("#10212b"));
        const QRectF chart = rect().adjusted(42, 16, -16, -30);
        painter.setPen(QColor("#38505e"));
        for (int tick = 0; tick <= 4; ++tick) {
            const auto y = chart.bottom() - chart.height() * tick / 4.0;
            painter.drawLine(QPointF(chart.left(), y), QPointF(chart.right(), y));
            painter.drawText(QRectF(2, y - 9, 35, 18), Qt::AlignRight | Qt::AlignVCenter, QString::number(tick * 25));
        }
        const auto samples = model_.recent_trend();
        if (samples.empty()) return;
        auto point_for = [&](std::size_t i, double value) {
            const double x = chart.left() + chart.width() * i / std::max<std::size_t>(samples.size() - 1, 1U);
            const double y = chart.bottom() - chart.height() * std::clamp(value, 0.0, 100.0) / 100.0;
            return QPointF(x, y);
        };
        QPainterPath level_path;
        QPainterPath setpoint_path;
        for (std::size_t i = 0; i < samples.size(); ++i) {
            (i == 0 ? level_path.moveTo(point_for(i, samples[i].level_percent))
                    : level_path.lineTo(point_for(i, samples[i].level_percent)));
            (i == 0 ? setpoint_path.moveTo(point_for(i, samples[i].setpoint_percent))
                    : setpoint_path.lineTo(point_for(i, samples[i].setpoint_percent)));
        }
        painter.setPen(QPen(QColor("#35c6d8"), 2));
        painter.drawPath(level_path);
        painter.setPen(QPen(QColor("#f5c451"), 2, Qt::DashLine));
        painter.drawPath(setpoint_path);
        painter.setPen(Qt::white);
        painter.drawText(chart.left(), height() - 7, "Level");
        painter.setPen(QColor("#f5c451"));
        painter.drawText(chart.left() + 52, height() - 7, "Setpoint");
    }

private:
    hmi::DashboardViewModel& model_;
};

class MainWindow final : public QMainWindow {
public:
    MainWindow() {
        setWindowTitle("Industrial Control HMI Simulator");
        resize(920, 620);
        auto* root = new QWidget(this);
        auto* layout = new QVBoxLayout(root);
        auto* overview = new QGridLayout;
        level_ = make_value("Level");
        setpoint_ = make_value("Setpoint");
        inlet_ = make_value("Inlet command");
        alarm_ = make_value("Alarm");
        connection_ = make_value("Connection");
        overview->addWidget(level_, 0, 0);
        overview->addWidget(setpoint_, 0, 1);
        overview->addWidget(inlet_, 0, 2);
        overview->addWidget(alarm_, 0, 3);
        overview->addWidget(connection_, 0, 4);
        layout->addLayout(overview);
        layout->addWidget(new TrendWidget(model_));

        auto* controls = new QGroupBox("Operator controls", root);
        auto* form = new QFormLayout(controls);
        mode_ = new QComboBox(controls);
        mode_->addItems({"Auto", "Manual"});
        desired_setpoint_ = new QDoubleSpinBox(controls);
        desired_setpoint_->setRange(0.0, 100.0);
        desired_setpoint_->setValue(65.0);
        desired_setpoint_->setSuffix(" %");
        auto* apply = new QPushButton("Apply command", controls);
        form->addRow("Mode", mode_);
        form->addRow("Level setpoint", desired_setpoint_);
        form->addRow(apply);
        layout->addWidget(controls);
        setCentralWidget(root);

        connect(apply, &QPushButton::clicked, this, [this] {
            model_.set_operator_command({desired_setpoint_->value(), mode_->currentIndex() == 0});
        });
        model_.set_connection_status({true, "Simulator connected"});
        model_.set_operator_command({65.0, true});
        connect(&timer_, &QTimer::timeout, this, [this] { publish_simulated_sample(); });
        timer_.start(500);
        publish_simulated_sample();
    }

private:
    static QLabel* make_value(const QString& title) {
        auto* value = new QLabel(title + "\n--", nullptr);
        value->setFrameShape(QFrame::StyledPanel);
        value->setMinimumWidth(145);
        value->setAlignment(Qt::AlignCenter);
        value->setStyleSheet("QLabel { padding: 10px; font-weight: 600; background: #edf5f7; }");
        return value;
    }

    void publish_simulated_sample() {
        const auto command = model_.operator_command();
        const double target = command.automatic_mode ? command.setpoint_percent : manual_level_;
        manual_level_ += (target - manual_level_) * 0.13;
        const auto alarm = manual_level_ < 10.0 ? domain::AlarmSeverity::low_low :
                           manual_level_ > 90.0 ? domain::AlarmSeverity::high_high : domain::AlarmSeverity::none;
        model_.publish_tags({std::chrono::steady_clock::now(), manual_level_, command.setpoint_percent,
                             std::clamp((command.setpoint_percent - manual_level_) * 2.0 + 50.0, 0.0, 100.0),
                             domain::Quality::good, alarm});
        const auto tags = model_.current_tags();
        level_->setText(QString("Level\n%1 %").arg(tags.level_percent, 0, 'f', 1));
        setpoint_->setText(QString("Setpoint\n%1 %").arg(tags.setpoint_percent, 0, 'f', 1));
        inlet_->setText(QString("Inlet command\n%1 %").arg(tags.inlet_command_percent, 0, 'f', 1));
        alarm_->setText("Alarm\n" + alarm_text(tags.alarm));
        alarm_->setStyleSheet(tags.alarm == domain::AlarmSeverity::none
                                  ? "QLabel { padding: 10px; font-weight: 600; background: #dff5e1; }"
                                  : "QLabel { padding: 10px; font-weight: 600; background: #ffc9c4; }");
        connection_->setText("Connection\n" + QString::fromStdString(model_.connection_status().detail));
        centralWidget()->findChild<TrendWidget*>()->update();
    }

    hmi::DashboardViewModel model_;
    QTimer timer_;
    double manual_level_{20.0};
    QLabel *level_{}, *setpoint_{}, *inlet_{}, *alarm_{}, *connection_{};
    QComboBox* mode_{};
    QDoubleSpinBox* desired_setpoint_{};
};

}  // namespace

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    MainWindow window;
    window.show();
    return app.exec();
}
