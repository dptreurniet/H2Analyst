#include "data_parser.h"

bool DataParser::parseFile(const std::filesystem::path& path, H2A::datafile_ptr datafile, bool justProperties) {

    auto start_time = std::chrono::high_resolution_clock::now();

    datafile->setName(path.stem());

    if (!std::filesystem::exists(path)) {
        qInfo() << "Failed to parse file, selected file does not exist.";
        return false;
    }

    bool success = false;
    if (path.extension() == ".mat")
        success = DataParser::parseMatFile(path, datafile, justProperties);

    // Insert other parsers here

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time);
    if (success) {
        qInfo() << "File load took" << duration.count() << "ms";
        return true;
    }

    qInfo() << "Failed to parse file, no parser defined for the selected filetype: " << path.extension().c_str();
    return false;
}


bool DataParser::parseMatFile(const std::filesystem::path& path, H2A::datafile_ptr datafile, bool justProperties) {
    qInfo() << "Starting parsing of" << path.filename().c_str();

    std::fstream file(path.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    MatFile data;

    data.buffer = std::vector<char>(size);
    if (!file.read(data.buffer.data(), size)) {
        qInfo() << "Failed to parse file, unable to read file contents.";
    }

    // FILE HEADER - Standard and required part of MAT-file (page 1-4)
    // ReadHeader() also checks if byte_swap is necessary (endianness)
    std::string header_text = MatFileFns::readHeader(data);

    // START TIME - Implemented as a 1x7 uint16_t vector
    std::vector<uint16_t> start_time;
    MatFileFns::readDataElement(data, start_time);
    datafile->setStartTime(start_time);

    // DATASETS -
    std::vector<H2A::Dataset> datasets;
    MatFileFns::readDatasets(data, datafile);
    qInfo() << "- datasets:" << datafile->datasets().size() << "loaded";

    // MESSAGES
    if (!justProperties) {
        if(!MatFileFns::readMessages(data, datafile))
            return false;
        qInfo() << "- messages:" << datafile->data().data.size() / 12 << "loaded";

        auto n_datasets = datafile->datasets().size();
        MatFileFns::removeDatasetsWithoutData(datafile);
        qInfo() << "- removed" << n_datasets - datafile->datasets().size() << "empty datasets";

        QFuture<void> _ = QtConcurrent::run(DataParser::MatFileFns::executePostLoadOperations, datafile);
    }

    return true;
}

std::string DataParser::MatFileFns::readHeader(MatFile& data) {
    /*
     * Refer to MAT-File reference PDF, page 1-4.
    */
    std::string text = std::string(data.buffer.begin() + data.cursor, data.buffer.begin() + data.cursor + 116);
    data.flip_bytes = (data.buffer[data.cursor + 126] == 'M');
    data.cursor += 128;
    return text;
}

void DataParser::MatFileFns::readDatasets(MatFile& data, const H2A::datafile_ptr& datafile) {
    // Datasets are implemented as a struct with the following fields:
    // - id             [uint16_t]
    // - name           [char]
    // - uid            [uint32_t]
    // - quantity       [char]
    // - unit           [char]
    // - length         [uint8_t]
    // - byte_offset    [uint8_t]
    // - datatype       [uint32_t]
    // - offset         [float]
    // - scale          [float]

    // STRUCT FIELDS
    DataElementTag tag;
    readDataElementTag(data, tag);

    std::vector<uint32_t> flags;
    readDataElement(data, flags);

    std::vector<int32_t> dimensions;
    readDataElement(data, dimensions);
    int32_t n_datasets = dimensions[1];

    std::vector<char> name;
    readDataElement(data, name);

    std::vector<int32_t> field_name_length;
    readDataElement(data, field_name_length);

    std::vector<char> field_names;
    readDataElement(data, field_names);

    std::vector<uint8_t> uint8_vec;
    std::vector<uint16_t> uint16_vec;
    std::vector<uint32_t> uint32_vec;
    std::vector<char> char_vec;

    for (auto ds_i = 0; ds_i < n_datasets; ds_i ++) {
        H2A::DatasetProperties properties;
        H2A::DatasetMatProperties mat_properties;

        properties.id = readSingleValue<uint16_t>(data);

        readDataElement(data, char_vec);
        properties.name = QString(std::string(char_vec.begin(), char_vec.end()).c_str());

        properties.uid = readSingleValue<uint32_t>(data);

        readDataElement(data, char_vec);
        properties.quantity = QString(std::string(char_vec.begin(), char_vec.end()).c_str());

        readDataElement(data, char_vec);
        properties.unit = QString(std::string(char_vec.begin(), char_vec.end()).c_str());

        mat_properties.length = readSingleValue<uint8_t>(data);
        mat_properties.byte_offset = readSingleValue<uint8_t>(data);
        mat_properties.datatype = readSingleValue<uint32_t>(data);
        mat_properties.offset = readSingleValue<float>(data);
        mat_properties.scale = readSingleValue<float>(data);

        auto dataset = QSharedPointer<H2A::Dataset>::create(datafile, properties, mat_properties);
        datafile->addDataset(dataset);
    }
}

bool DataParser::MatFileFns::readMessages(MatFile& data, const H2A::datafile_ptr& datafile) {

    /*
     * Messages are stored in a 12xN matrix of uint8_t.
     * They could be read with the readMatrix function, but this function will copy every individual value, leading to longer load time.
     * Instead, this function checks if the messages matrix is as expected and if true, copy the whole thing in a single operation.
     */

    DataElementTag tag;
    readDataElementTag(data, tag);
    size_t element_end = data.cursor + tag.number_of_bytes;

    MatrixTags tags;

    data.cursor += 16;  // Push cursor 16 bytes to account for unused flags (page 1-10)

    // DIMENSIONS
    readDataElement(data, tags.dimensions);
    if (tags.dimensions[0] != 12) {
        qInfo() << "Unable to load data: Expected message matrix with 12 rows, but it has" << tags.dimensions[0];
        return false;
    }
    size_t n_messages = tags.dimensions[0] * tags.dimensions[1];

    // NAME
    std::vector<char> name_vec;
    readDataElement(data, name_vec);
    tags.name = std::string(name_vec.begin(), name_vec.end());
    if (tags.name != "messages") {
        qInfo() << "Unable to load data. Expected messages matrix, but found" << tags.name;
        return false;
    }

    // DATA TAG
    data.cursor += 8;  // Push cursor 8 bytes to account for unused data tag

    datafile->data().data.clear();
    datafile->data().data = std::vector<uint8_t>(data.buffer.begin() + data.cursor, data.buffer.begin() + data.cursor + n_messages);
    datafile->setNumberOfMessages(tags.dimensions[1]);

    data.cursor = element_end;
    return true;
}

template <typename T>
void DataParser::MatFileFns::readDataElement(MatFile& data, std::vector<T>& output) {
    DataElementTag tag;
    readDataElementTag(data, tag);
    size_t element_end = data.cursor + tag.number_of_bytes;

    if (tag.type == 14) {  // miMATRIX
        MatrixTags matrix_tags;
        readMatrix(data, matrix_tags, output);
    }
    else {
        output = std::vector<T>(tag.number_of_bytes / sizeof(T));
        //output.reserve(tag.number_of_bytes / sizeof(T));
        for (auto i = 0; i < tag.number_of_bytes / sizeof(T); i ++) {
            readBytesToType(data, output[i]);
        }
        if (data.cursor % 8 != 0) data.cursor += (8 - (data.cursor % 8));
    }
}

void DataParser::MatFileFns::readDataElementTag(MatFile& data, DataElementTag& output, bool push_cursor) {
    /*
     * Refer to MAT-File reference, page 1-5
     */
    auto cursor_at_start = data.cursor;

    // Check if this element is in Small Element Data Format (page 1-7)
    if ((int)(readByteDirectly(data, data.flip_bytes ? 0 : 3) | readByteDirectly(data, data.flip_bytes ? 1 : 2)) != 0) {
        uint16_t small_type;
        readBytesToType(data, small_type);
        output.type = (uint32_t) small_type;

        uint16_t small_number_of_bytes;
        readBytesToType(data, small_number_of_bytes);
        output.number_of_bytes = (uint32_t) small_number_of_bytes;

    } else {
        readBytesToType(data, output.type);
        readBytesToType(data, output.number_of_bytes);
    }

    if (!push_cursor) data.cursor = cursor_at_start;
}

template <typename T>
void DataParser::MatFileFns::readMatrix(MatFile& data, MatrixTags& tags, std::vector<T>& output) {
    // FLAGS - not used for anything, only read to push cursor
    std::vector<uint32_t> flags;
    readDataElement(data, flags);
    tags.matrix_type = (flags[0] & 0xFF);

    // DIMENSIONS
    readDataElement(data, tags.dimensions);

    // NAME
    std::vector<char> name_vec;
    readDataElement(data, name_vec);
    tags.name = std::string(name_vec.begin(), name_vec.end());

    if (tags.matrix_type != 1 &&
        tags.matrix_type != 2 &&
        tags.matrix_type != 3 &&
        tags.matrix_type != 5) {  // Most matrix types do not have other tags (page 1-10)
        readDataElement(data, output);

    } else if (tags.matrix_type == 2) {
        // Structure, has field name length, field names (page 1-20)

        // FIELD NAME LENGTH
        std::vector<uint32_t> field_name_length_vec;
        readDataElement(data, field_name_length_vec);
        uint32_t field_name_length = field_name_length_vec.front();

        // FIELD NAMES
        std::vector<char> field_names_vec;
        readDataElement(data, field_names_vec);
        size_t n_names = field_names_vec.size() / field_name_length;
        std::vector<std::string> field_names(n_names);
        for (auto i = 0; i < n_names; i ++) {
            field_names[i] = std::string(field_names_vec.begin() + i * field_name_length,
                                         field_names_vec.begin() + (i + 1) * field_name_length);
        }

        readDataElement(data, output);
    }
}

template <typename T>
void DataParser::MatFileFns::readBytesToType(MatFile& data, T& output) {
    size_t n_bytes = sizeof(T);

    if (!data.flip_bytes) {
        std::memcpy(&output, &data.buffer[data.cursor], n_bytes);
    }
    else {
        char bytes[n_bytes];
        for (auto i = n_bytes - 1; i >= 0; i--) {
            bytes[i] = data.buffer[data.cursor + i];
        }
        std::memcpy(&output, &bytes, n_bytes);
    }
    data.cursor += n_bytes;
}

char DataParser::MatFileFns::readByteDirectly(MatFile& data, size_t offset) {
    return data.buffer[data.cursor + offset];
}

template <typename T>
T DataParser::MatFileFns::readSingleValue(MatFile& data) {
    std::vector<T> temp;
    readDataElement(data, temp);
    return temp.front();
}

void DataParser::MatFileFns::executePostLoadOperations(const H2A::datafile_ptr& datafile) {
    qInfo() << "Starting post-load operations";
    auto n_messages = datafile->numberOfMessages();

    // Create ids and dts vector from messages
    datafile->data().ids = std::vector<uint16_t>(n_messages);
    std::vector<uint16_t> dts(n_messages);
    datafile->data().time = std::vector<double>(n_messages);
    for (auto i = 0; i < n_messages; i ++) {
        memcpy(&datafile->data().ids[i], &datafile->data().data[i*12], sizeof(uint16_t));
        memcpy(&dts[i], &datafile->data().data[(i*12)+2], sizeof(uint16_t));
    }

    auto time_vec = datafile->data().time;
    for (auto i = 1; i < n_messages; i ++) {
        datafile->data().time[i] = datafile->data().time[i - 1] + ((double) dts[i]) * 1E-5;
    }
    qInfo() << "Post-load operations finished";

    datafile->setReadyToPlot(true);
}

void DataParser::MatFileFns::removeDatasetsWithoutData(const H2A::datafile_ptr& datafile) {
    /*
     * Because INTCANLOG logs all messages and any dataset is logged at at least 10 Hz or so,
     * every dataset will be in the first second of the logfile.
     * Hence no more needs to be scanned, as scanning the complete datafile can take multiple seconds.
     */

    std::unordered_set<uint16_t> ids_present;
    uint16_t scanned_time(0), dt, id;
    for (auto i = 1; i < datafile->numberOfMessages(); i ++) {
        memcpy(&id, &datafile->data().data[i*12], sizeof(uint16_t));
        ids_present.insert(id);

        memcpy(&dt, &datafile->data().data[(i*12)+2], sizeof(uint16_t));
        scanned_time += dt;

        if (scanned_time > 1000) break;
    }

    // Remove datasets not found in the first second.
    // Keep datasets with type 10 (boolean, used by EMCY datasets), as these are only logged when they are triggered.
    datafile->datasets().erase(
        std::remove_if(datafile->datasets().begin(), datafile->datasets().end(),
                       [ids_present](const H2A::dataset_ptr& dataset){ return ids_present.count(dataset->id()) == 0 && dataset->datatype() != 10; }),
        datafile->datasets().end());
}



