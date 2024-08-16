#ifndef DATAFILE_H
#define DATAFILE_H

#include <QDebug>

#include <vector>
#include <cstdint>

#include "dataset.h"

namespace H2A {

class Datafile;
struct Dataset;

struct StartTime {
    uint16_t year, month, day, hour, minute, second, millisecond;
};

struct INTCANLOGData {
    std::vector<double> time;
    std::vector<uint16_t> ids;
    std::vector<uint8_t> data;
};

class Datafile {

    QString name_;
    StartTime start_time_;
    dataset_vec datasets_;
    size_t n_messages_;
    INTCANLOGData data_;
    bool ready_to_plot_;

public:

    explicit Datafile();

    void setStartTime(const std::vector<uint16_t>& start_time_vec);

    inline const QString name() const { return name_; }
    inline void setName(const QString& name) { name_ = name; }
    inline void setName(const std::string& name) { name_ = QString(name.c_str()); }

    void addDataset(dataset_ptr dataset);
    inline dataset_vec& datasets() { return datasets_; }

    inline void setNumberOfMessages(size_t n_messages) { n_messages_ = n_messages; }
    inline size_t numberOfMessages() { return n_messages_; }
    inline H2A::INTCANLOGData& data() { return data_; }

    inline void setReadyToPlot(bool ready) { ready_to_plot_ = ready; }
    inline bool isReadyToPlot() const { return ready_to_plot_; }

    void populateDataset(const dataset_ptr& dataset);
};

}


#endif // DATAFILE_H
