#include "dataset.h"
#include "datafile.h"

H2A::Dataset::Dataset(const datafile_ptr& datafile, const H2A::DatasetProperties& properties, const H2A::DatasetMatProperties& mat_properties) :
    type_(DatasetType::mat),
    datafile_(datafile),
    properties_(properties),
    mat_properties_(mat_properties)
{
    mat_properties_.is_populated = false;
}

H2A::Dataset::Dataset(const datafile_ptr& datafile, const H2A::DatasetProperties& properties) :
    Dataset(datafile, properties, H2A::DatasetMatProperties())
{
    type_ = DatasetType::postpro;
}

QString H2A::Dataset::stringPath() const {
    return datafile_->name() + "/" + name();
}

std::map<QString, QVariant> H2A::DatasetProperties::toMap() const {
    std::map<QString, QVariant> map;

    QVariant idVar;
    idVar.setValue(id);
    map["id"] = idVar;

    QVariant uidVar;
    uidVar.setValue(uid);
    map["uid"] = uidVar;

    QVariant nameVar;
    nameVar.setValue(name);
    map["name"] = nameVar;

    QVariant quantityVar;
    quantityVar.setValue(quantity);
    map["quantity"] = quantityVar;

    QVariant unitVar;
    unitVar.setValue(unit);
    map["unit"] = unitVar;

    return map;
}

H2A::DatasetProperties H2A::DatasetProperties::fromMap(const std::map<QString, QString>& map) {
    H2A::DatasetProperties properties;
    properties.id = map.at("id").toInt();
    properties.uid = map.at("uid").toInt();
    properties.name = map.at("name");
    properties.quantity = map.at("quantity");
    properties.unit = map.at("unit");
    return properties;
}
