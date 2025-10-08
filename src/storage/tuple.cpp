#include "ds.h"

std :: array<char, ROW_SIZE> Row::serialize() const  {
    std::array<char, ROW_SIZE> buffer{};

    std::memcpy(buffer.data() + ID_OFFSET, &id, sizeof(id));
    std::memcpy(buffer.data() + TEXT_SIZE_OFFSET, &text_size, sizeof(text_size));
    std::memcpy(buffer.data() + TEXT_OFFSET, text.data(), text_size);
    return buffer;
}

Row Row::deserialize(const std::array<char, ROW_SIZE>& buffer) {
    Row row;
    std::memcpy(&row.id, buffer.data() + ID_OFFSET, sizeof(row.id));
    std::memcpy(&row.text_size, buffer.data() + TEXT_SIZE_OFFSET, sizeof(row.text_size));
    std::memcpy(row.text.data(), buffer.data() + TEXT_OFFSET, row.text_size);

    return row;
}