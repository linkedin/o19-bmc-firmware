#define CHECKSUM1_ADDR         0x7
#define CHECKSUM2_ADDR         0x47
#define CHECKSUM3_ADDR         0x4B
#define CHECKSUM4_ADDR         0x4C
#define CHECKSUM5_ADDR         0x68
#define CHECKSUM6_ADDR         0x69
#define CHECKSUM7_ADDR         0x7A
#define CHECKSUM8_ADDR         0x7B

#define MANUFNAME_ADDR         0x0C
#define MANUFNAME_SIZE         6

#define PRODUCTNAME_ADDR       0x12
#define PRODUCTNAME_SIZE       15

#define MODELNO_ADDR           0x21
#define MODELNO_SIZE           12

#define PRODUCTVERSION_ADDR    0x2D
#define PRODUCTVERSION_SIZE    6

#define PRODUCTSERIAL_ADDR     0x33
#define PRODUCTSERIAL_SIZE     14

#define PSU_EEPROM_SIZE        256
#define CHECKSUM_TOTAL         8
#define FILENAME_SIZE          256

typedef struct {
    char checksum[CHECKSUM_TOTAL];
    char manuf_name[MANUFNAME_SIZE];
    char product_name[PRODUCTNAME_SIZE];
    char modelno[MODELNO_SIZE];
    char product_version[PRODUCTVERSION_SIZE];
    char product_serial[PRODUCTSERIAL_SIZE];
} psu_eeprom_t;

