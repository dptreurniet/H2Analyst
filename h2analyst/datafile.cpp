#include "datafile.h"

H2A::Datafile::Datafile() :
    name_(""),
    ready_to_plot_(false)
{
}

void H2A::Datafile::setStartTime(const std::vector<uint16_t>& start_time_vec) {
    if (start_time_vec.size() != 7) {
        qWarning() << "Tried to set start time with ill-shaped vector";
        return;
    }
    start_time_.year = start_time_vec[0];
    start_time_.month = start_time_vec[1];
    start_time_.day = start_time_vec[2];
    start_time_.hour = start_time_vec[3];
    start_time_.minute = start_time_vec[4];
    start_time_.second = start_time_vec[5];
    start_time_.millisecond = start_time_vec[6];
}

void H2A::Datafile::addDataset(dataset_ptr dataset) {
    datasets_.push_back(dataset);
}

void H2A::Datafile::populateDataset(const dataset_ptr& dataset) {

    if (dataset->type() != DatasetType::mat) return;
    if (dataset->isPopulated()) return;

    while (!isReadyToPlot()) {}  // Wait for id and dt vectors to be generated

    auto n_messages = std::count(data_.ids.begin(), data_.ids.end(), dataset->id());
    dataset->getTimeVec().assign(n_messages, 0);
    dataset->getDataVec().assign(n_messages, 0);

    auto id = dataset->id();
    auto mat_properties = dataset->matProperties();
    auto byte_offset = mat_properties.byte_offset;
    auto length = mat_properties.length;
    size_t message_i = 0;

    /*
     * 0: uint8
     * 1: int8
     * 2: uint16
     * 3: int16
     * 4: uint32
     * 5: int32
     * 6: uint64
     * 7: int64
     * 8: float
     * 9: double
     * 10: bool
    */

    quint64 temp_int = 0;
    float temp_float = 0;
    double double_val = 0;

    for (auto i = 0; i < data_.ids.size(); i ++) {
        if (data_.ids[i] != id) continue;
        dataset->getTimeVec()[message_i] = data_.time[i];

        std::vector<uint8_t> frame(data_.data.begin() + i * 12, data_.data.begin() + i * 12 + 12);
        /*
        switch (dataset->datatype()) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
            memcpy(&temp_val, &data_.data[i * 12 + 4 + byte_offset], length);
            break;
        }
        */
        if (dataset->datatype() == 8) {  // Float datatype
            memcpy(&temp_float, &data_.data[i * 12 + 4 + byte_offset], length);
            double_val = static_cast<double>(temp_float);
        } else if (dataset->datatype() == 9) {  // Double datatype (untested! no datasets in F8 data with this type)
            memcpy(&double_val, &data_.data[i * 12 + 4 + byte_offset], length);
        } else {  // All (u)int datatypes
            memcpy(&temp_int, &data_.data[i * 12 + 4 + byte_offset], length);
            double_val = static_cast<double>(temp_int);
        }
        dataset->getDataVec()[message_i] = mat_properties.scale * double_val + mat_properties.offset;
        message_i ++;
    }

    dataset->markAsPopulated();
}
