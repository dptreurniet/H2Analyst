#ifndef DATASET_H
#define DATASET_H

#include <QObject>
#include <QSharedPointer>
#include <QDebug>
#include <QVariant>

namespace H2A {

class Datafile;
class Dataset;

using datafile_ptr = QSharedPointer<H2A::Datafile>;
using datafile_vec = QVector<datafile_ptr>;
using dataset_ptr = QSharedPointer<H2A::Dataset>;
using dataset_vec = std::vector<dataset_ptr>;

enum DatasetType {
    mat,
    postpro
};

struct DatasetProperties {
    int id, uid;
    QString name, quantity, unit;

    std::map<QString, QVariant> toMap() const;
    static DatasetProperties fromMap(const std::map<QString, QString>& map);
    bool operator==(const DatasetProperties& o) const { return (id == o.id && uid == o.uid && name == o.name && quantity == o.quantity && unit == o.unit); }
};

struct DatasetMatProperties {
    quint8 length, byte_offset;
    quint32 datatype;
    float offset, scale;
    bool is_populated;
};

class Dataset {

    DatasetType type_;
    datafile_ptr datafile_;
    DatasetProperties properties_;
    DatasetMatProperties mat_properties_;

    QVector<double> time_vec_;
    QVector<double> data_vec_;

public:
    explicit Dataset(const datafile_ptr& datafile, const DatasetProperties& properties, const DatasetMatProperties& mat_properties);
    explicit Dataset(const datafile_ptr& datafile, const DatasetProperties& properties);

    QString stringPath() const;

    // Property getters
    inline DatasetType type() const { return type_; }
    inline const datafile_ptr datafile() const { return datafile_; }
    inline QString name() const { return properties_.name; }
    inline QString unit() const { return properties_.unit; }
    inline QString quantity() const { return properties_.quantity; }
    inline quint16 id() const { return properties_.id; }
    inline quint32 uid() const { return properties_.uid; }
    inline DatasetProperties properties() const { return properties_; }
    inline DatasetMatProperties matProperties() const { return mat_properties_; }
    inline quint32 datatype() const { return mat_properties_.datatype; }
    inline bool isPopulated() const { return mat_properties_.is_populated; }

    // Data accessors
    inline QVector<double>& getTimeVec() { return time_vec_; }
    inline QVector<double>& getDataVec() { return data_vec_; }

    // Matfile functions
    void setMatProperties(const DatasetMatProperties& properties) { mat_properties_ = properties; }
    void markAsPopulated() { mat_properties_.is_populated = true; }

};

}

Q_DECLARE_METATYPE(H2A::dataset_vec);
Q_DECLARE_METATYPE(H2A::dataset_ptr);
Q_DECLARE_METATYPE(H2A::DatasetProperties);

#endif // DATASET_H
