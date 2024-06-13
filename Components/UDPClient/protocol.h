/**
 * @file protocol.h
 * @author simakeng (simakeng@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-06-13
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <stdint.h>

#pragma pack(push, 1)

#define LECS_VERSION 100

enum lecp_type_e
{
    LECP_UNKNOWN = 0x00,
    LECP_DISCOVER = 0x01,
    LECP_PING = 0x02,
    LECP_SET = 0x03,
    LECP_GET = 0x04,
    LECP_RESPONSE = 0x05,
    LECP_EVENT = 0x06,

    LECP_EXTENDED = 0x80,
    LECP_ENCRYPTED = 0x8F
};

typedef struct lecp_pack_t
{
    uint32_t header = 0x4C454353; // "LECS"
    uint8_t type;
    uint8_t version;
    uint8_t packid;
    uint8_t size;
    uint16_t crc;
    uint8_t data[240]; // max 240 bytes, unless extended packet
};

typedef struct lecp_discover_data_t
{
    uint16_t device_category;
    uint8_t device_type;
    struct
    {
        uint8_t major;
        uint8_t minor;
        uint8_t patch;
    } version;

    uint8_t device_uuid[16];
    uint8_t device_name[64]; // max 64 bytes, terminated by null(\x00)
};

typedef struct lecp_field_id_t
{
    uint16_t category;
    uint8_t type;
    uint8_t sub_type;
};

typedef struct lecp_get_command_t
{
    uint8_t commend_version;
    uint8_t field_count;
    uint16_t request_id;
    lecp_field_id_t fields[58];  // max 58 fields
};

#pragma pack(pop)