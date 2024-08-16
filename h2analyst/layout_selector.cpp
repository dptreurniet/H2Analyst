#include "layout_selector.h"

LayoutSelector::LayoutSelector(QObject *parent)
    : QWidgetAction{parent},
    widget_(new QWidget),
    layout_(new QGridLayout),
    button_group_(new QButtonGroup)
{
    for (auto row_i = 0; row_i < kMaxRows_; row_i ++) {
        for (auto col_i = 0; col_i < kMaxCols_; col_i ++) {
            auto button = new LayoutSelectorButton(widget_);
            button_group_->addButton(button, (row_i * kMaxCols_) + col_i);
            layout_->addWidget(button, row_i, col_i);
            layout_->setSpacing(1);
        }
    }

    widget_->setLayout(layout_);
    setDefaultWidget(widget_);
    connect(button_group_, &QButtonGroup::idReleased, this, &LayoutSelector::buttonReleased);
}

void LayoutSelector::buttonReleased(int id) {
    emit layoutSelected((id / kMaxCols_) + 1, (id % kMaxCols_) + 1);
    trigger();
}

LayoutSelectorButton::LayoutSelectorButton(QWidget* parent)
    : QPushButton(parent)
{
    setFixedSize(QSize(20, 20));
}

void LayoutSelectorButton::paintEvent(QPaintEvent* event) {
    QStylePainter p(this);
    QStyleOptionButton option;
    initStyleOption(&option);
    p.drawControl(QStyle::CE_PushButton, option);
}

