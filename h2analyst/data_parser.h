#ifndef DATA_PARSER_H
#define DATA_PARSER_H

#include <QDebug>
#include <QThread>
#include <QtConcurrentRun>

#include <filesystem>
#include <string>
#include <fstream>
#include <cstdint>
#include <chrono>
#include <unordered_set>
#include <algorithm>

#include "datafile.h"

namespace DataParser
{

bool parseFile(const std::filesystem::path& filepath, H2A::datafile_ptr datafile, bool justProperties = false);
bool parseMatFile(const std::filesystem::path& filepath, H2A::datafile_ptr datafile, bool justProperties = false);

struct MatFile {
    std::vector<char> buffer;
    size_t cursor = 0;
    bool flip_bytes = false;
};

namespace MatFileFns {

struct MatDatasetProperties {
    uint16_t id;
};

struct DataElementTag {
    uint32_t type;
    uint32_t number_of_bytes;
};

struct MatrixTags {
    uint8_t matrix_type;
    std::string name;
    std::vector<uint32_t> dimensions;
};

std::string readHeader(MatFile& data);
void readDatasets(MatFile& data, const H2A::datafile_ptr& datafile);
bool readMessages(MatFile& data, const H2A::datafile_ptr& datafile);
template <typename T>
void readDataElement(MatFile& data, std::vector<T>& output);
void readDataElementTag(MatFile& data, DataElementTag& output, bool push_cursor = true);
template <typename T>
void readMatrix(MatFile& data, MatrixTags& tags, std::vector<T>& output);
template <typename T>
void readBytesToType(MatFile& data, T& output);
template <typename T>
T readSingleValue(MatFile& data);
char readByteDirectly(MatFile& data, size_t offset = 0);

void executePostLoadOperations(const H2A::datafile_ptr& datafile);
void removeDatasetsWithoutData(const H2A::datafile_ptr& datafile);
}  // MatFileFns namespace

};  // DataParser namespace

#endif // DATA_PARSER_H
