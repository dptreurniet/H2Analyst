#ifndef LAYOUT_SELECTOR_H
#define LAYOUT_SELECTOR_H

#include <QWidgetAction>
#include <QWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QButtonGroup>
#include <QStylePainter>
#include <QStyleOptionButton>

class LayoutSelector;  // Forward-declaration

class LayoutSelectorButton : public QPushButton
{
    void paintEvent(QPaintEvent *) override;

public:
    explicit LayoutSelectorButton(QWidget* parent = nullptr);
};

class LayoutSelector : public QWidgetAction
{
    Q_OBJECT

    const uint8_t kMaxCols_ = 4;
    const uint8_t kMaxRows_ = 4;

    QWidget* widget_;
    QGridLayout* layout_;
    QButtonGroup* button_group_;

public:
    explicit LayoutSelector(QObject *parent = nullptr);

private slots:
    void buttonReleased(int id);

signals:
    void layoutSelected(uint8_t n_rows, uint8_t n_cols);

};

#endif // LAYOUT_SELECTOR_H
