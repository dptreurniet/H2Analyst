#ifndef PLOT_TIME_H
#define PLOT_TIME_H

#include <unordered_set>

#include "plot_base.h"

namespace H2A {

class PlotTime : public PlotBase
{

    QSharedPointer<QCPAxisTickerTime> ticker_x_;

protected:
    virtual void calculateBounds() override;

    virtual void setLabelY() override;

    // Key events
    virtual void keyPressEvent(QKeyEvent* event) override;

public:
    PlotTime(QWidget *parent = nullptr);
    PlotTime(PlotBase* base);

    virtual void plot(const H2A::dataset_vec& datasets) override;

};

}
#endif // PLOT_TIME_H
