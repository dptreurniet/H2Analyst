#ifndef DATASET_OPERATORS_H
#define DATASET_OPERATORS_H

#include "dataset.h"
#include "datafile.h"

namespace H2A {

dataset_ptr resampleDataset(const H2A::dataset_ptr& source, const QVector<double>& time_vec);

}


#endif // DATASET_OPERATORS_H
