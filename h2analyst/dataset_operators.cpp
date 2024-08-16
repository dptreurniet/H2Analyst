#include "dataset_operators.h"


H2A::dataset_ptr H2A::resampleDataset(const H2A::dataset_ptr& source, const QVector<double>& time_vec) {

    source->datafile()->populateDataset(source);

    auto properties = DatasetProperties();
    properties.id = 0;
    properties.name = "Todo";
    properties.quantity = "Todo";
    properties.uid = 0;
    properties.unit = "Todo";

    H2A::dataset_ptr resampled_dataset = H2A::dataset_ptr::create(source->datafile(), properties);
    resampled_dataset->getTimeVec() = time_vec;
    resampled_dataset->getDataVec() = QVector<double>(time_vec.size());

    auto source_time_vec = source->getTimeVec();
    size_t source_time_i = 0;
    size_t resampled_time_i = 0;
    while (resampled_time_i < time_vec.size()) {
        if (source_time_i < source_time_vec.size() - 1) {
            if (time_vec[resampled_time_i] > source_time_vec[source_time_i + 1]) {
                source_time_i ++;
                continue;
            }
        }
        resampled_dataset->getDataVec()[resampled_time_i] = source->getDataVec()[source_time_i];
        resampled_time_i ++;
    }

    return resampled_dataset;
}
