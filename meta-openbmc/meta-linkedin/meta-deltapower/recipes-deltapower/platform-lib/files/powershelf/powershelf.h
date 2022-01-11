#include <openbmc/obmc-i2c.h>

/* ****************************************
 *                COMMON                  *
 ******************************************/

#define   MAX_I2C_BUS_NUM                 12
#define   MIN_I2C_BUS_NUM                 0
#define   MAX_STR_SIZE                    32
#define   MAX_RETRIES                     10
#define   DEVICE_FILENAME_SIZE            128
#define   MMAP_FILEPATH                   "/tmp/mmap.bin"

#define   DEVICE_OPERATION_ON             0x80
#define   DEVICE_OPERATION_OFF            0x0

/*
 * Register addresses
 */
#define   OPERATION_REG                   0x1
#define   CLEAR_FAULTS_REG                0x3
#define   STATUS_BYTE_ADDR                0x78
#define   STATUS_WORD_ADDR                0x79
#define   STATUS_VOUT_ADDR                0x7a
#define   STATUS_IOUT_ADDR                0x7b
#define   STATUS_INPUT_ADDR               0x7c
#define   STATUS_TEMPERATURE_ADDR         0x7d
#define   STATUS_MFR_SPECIFIC             0x80
#define   STATUS_FAN_ADDR                 0x81

#define   STATUS_POWER_GOOD_MASK         0x800
#define   STATUS_INPUT_MASK              0x2000

/*
 * Status bits
 */
#define   BIT_POWER_GOOD_NEGATED         1 << 11

/*
 * STATUS_TEMPERATURE bits
 */
#define   BIT_TEMP_OT_FAULT              1 << 7    /* Overtemperature Fault */
#define   BIT_TEMP_OT_WARN               1 << 6    /* Overtemperature Warning */

/*
 * STATUS_WORD bits
 */
#define   BIT_CML_FAULT                  1 << 1
#define   BIT_TEMPERATURE_WARN_FAULT     1 << 2
#define   BIT_UNIT_OFF                   1 << 6
#define   BIT_MFR_SPECIFIC_ERROR         1 << 12
#define   BIT_VIN_IIN_FAULT              1 << 13
#define   BIT_VOUT_FAULT_WARN            1 << 15

/*
 * STATUS_VOUT bits
 */
#define   BIT_VOUT_UV_WARN               1 << 5

/*
 * STATUS_INPUT bits
 */
#define   BIT_VIN_OV_FAULT               1 << 7
#define   BIT_VIN_OV_WARN                1 << 6
#define   BIT_VIN_UV_FAULT               1 << 5
#define   BIT_VIN_UV_WARN                1 << 4
#define   BIT_IIN_OC_FAULT               1 << 2
#define   BIT_IIN_OC_WARN                1 << 1
#define   BIT_PIN_OP_WARN                1 << 0

/*
 * Status Vout  0x7A
 */
#define BIT_VOUT_OV_FAULT               1 << 7
#define BIT_VOUT_UV_FAULT               1 << 4

/*
 * Status Iout  0x7B
 */
#define BIT_IOUT_OC_FAULT                1 << 7
#define BIT_IOUT_OC_WARN                 1 << 5

/*
 * Status Input 0x7C
 */
#define BIT_VIN_LOW_OFF                 1 << 3

/*
 * Status_Fans_1_2 bits
 */
#define   BIT_FAN1_FAULT                 1 << 7    /* fan 1 fault */
#define   BIT_FAN1_OVER_RIDE             1 << 3    /* Over-ride by system */

/*
 * STATUS_MFR_SPECIFIC 0x80
 */
#define   BIT_CIRCUIT_BREAKER_FAULT      1 << 7
#define   BIT_MOSFET_FAULT               1 << 6

typedef enum {
    FILE_EFUSE = 0,
    FILE_PSU   = 1,
    FILE_FAN   = 2,
} device_type;

typedef enum {
    OP_NO_ERROR      = 0,
    OP_ON_ERROR      = 1,
    OP_OFF_ERROR     = 2,
    OP_RESET_ERROR   = 3,
    OP_INVALID       = 4,
} op_error;

typedef struct status_info {
    uint8_t        status;
    uint8_t        status_i2c_addr;
    uint16_t       bitmask;
    char           status_desc[MAX_STR_SIZE];
} status_info_t;


/* ****************************************
 *                EFUSE                   *
 ******************************************/
#define   MAX_EFUSE_NUM                 50

/*
 * efuse bus: 0 annd 4
 * efuse bus i2c master selector address
 */
#define   I2C_MASTER_EFUSE_BUS_0        0
#define   I2C_MASTER_EFUSE_BUS_4        4
#define   I2C_MASTER_SELETOR_ADDR_0     0x79
#define   I2C_MASTER_SELETOR_ADDR_4     0x7a


/* ****************************************
 *                FAN                     *
 ******************************************/

/* Fan status word
 *    Bit 2: Temperature warning
 *    Bit 11: Fan warning
 *    Bit 15: VOUT fault
 */
#define    FAN_BIT_TEMP_WARNING           1<<2
#define    FAN_BIT_VIN_UV_FAULT           1<<3

/*    Fan1_2 and Fan3_4 status:
 *       HI BYTE(Fan 1 or fan3) / LO BYTE(Fan 2 or Fan 4)
 *          Bit 0: FAN FAIL
 *          Bit 1: FAN SPEED WARNING
 */
#define    FAN_BIT_SPEED_WARNING    1<<9
#define    FAN_BIT_SPEED_FAIL       1<<8

#define    MAX_FAN_NUM             4
/*
 * number of Temperature sensors
 */
#define    MAX_TEMP_NUM            4

#define    FAN_BUS                 8
#define    FAN_MUX_ADDR            0x7c
#define    FAN_CTRL_ADDR           0x30
#define    FAN_DIR                 "/sys/class/i2c-adapter/i2c-%d/%d-0030/"

/* ****************************************
 *                PSU                     *
 ******************************************/

#define   MAX_PSU_NUM              6
#define   MAX_PSU_NUM_ASIDE        3
#define   PSU_FAN_NUM              3

/* ****************************************
 *         Structures for eFuse           *
 ******************************************/
/*
 * Status
 */
typedef enum {
    /*
     * EFUSE_STATUS_WORD_ADDR 0x79
     */
    EFUSE_CML_FAULT                = 0,
    EFUSE_TEMPERATURE_WARN_FAULT   = 1,
    EFUSE_UNIT_OFF                 = 2,
    EFUSE_POWER_GOOD_NEGATED       = 3,
    EFUSE_MFR_SPECIFIC_ERROR       = 4,
    EFUSE_VIN_IIN_FAULT            = 5,
    EFUSE_VOUT_FAULT_WARN          = 6,

    /*
     * EFUSE_STATUS_VOUT_ADDR 0x7a
     */
    EFUSE_VOUT_UV_WARN             = 7,

    /*
     * EFUSE_STATUS_INPUT_ADDR 0x7c
     */
    EFUSE_VIN_OV_FAULT             = 8,
    EFUSE_VIN_OV_WARN              = 9,
    EFUSE_VIN_UV_FAULT             = 10,
    EFUSE_VIN_UV_WARN              = 11,
    EFUSE_IIN_OC_FAULT             = 12,
    EFUSE_IIN_OC_WARN              = 13,
    EFUSE_PIN_OP_WARN              = 14,

    /*
     * EFUSE_STATUS_TEMPERATURE_ADDR 0x7d
     */
    EFUSE_TEMP_OT_FAULT           = 15,
    EFUSE_TEMP_OT_WARN            = 16,

    /*
     * EFUSE_STATUS_MFR_SPECIFIC 0x80
     */
    EFUSE_CIRCUIT_BREAKER_FAULT   = 17,
    EFUSE_MOSFET_FAULT            = 18,
} efuse_status_t;

typedef enum {
  EFUSE_ON          = 0x01,
  EFUSE_OFF         = 0x02,
  EFUSE_UNKNOWN     = 0x03,
} efuse_operation_state_t;

/* ****************************************
 *         Structures for Fans            *
 ******************************************/
/*
 * Status
 */
typedef enum {
  FAN_VIN_UV_FAULT           = 0,
  FAN_OVER_TEMP_WARN         = 1,  /* temperature Warning */
  FAN_FAN1_FAULT             = 2,  /* fan 1 fault */
  FAN_FAN1_WARN              = 3,  /* fan 1 warning */
} fan_status_t;

typedef struct fan_info {
    int      i2c_bus;
    int      i2c_addr;
    uint32_t fan_status;
    int      fan_speed[MAX_FAN_NUM];
    float    volt_input;
    float    temperature[MAX_TEMP_NUM];
} fan_info_t;

typedef struct efuse_info_ {
    uint8_t    efuse_num;
    uint8_t    i2c_bus;
    uint8_t    i2c_addr;
    efuse_operation_state_t   state;
    uint32_t   efuse_status;
    float      volt_input;
    float      current_input;
    float      volt_out;
    float      power_input;
    int        fan_speed;
    int        temperature;
    uint8_t    efuse_tripped;
} efuse_info_t;

/* ****************************************
 *         Structures for PSUs            *
 ******************************************/
/*
 * Status
 */
typedef enum {
  PSU_POWER_NO_GOOD          = 0,
  PSU_VOUT_OV_FAULT          = 1,
  PSU_VOUT_UV_FAULT          = 2,
  PSU_IOUT_OC_FAULT          = 3,
  PSU_IOUT_OC_WARN           = 4,
  PSU_VIN_OV_FAULT           = 5,
  PSU_VIN_UV_FAULT           = 6,
  PSU_VIN_LOW_OFF            = 7,

  /*
   * STATUS_TEMPERATURE bits
   */
  PSU_OVER_TEMP_FAULT        = 8,    /* Overtemperature Fault */
  PSU_OVER_TEMP_WARN         = 9,   /* Overtemperature Warning */

  /*
   * Status_Fans_1_2 bits
   */
  PSU_FAN1_FAULT             = 10,    /* fan 1 fault */
  PSU_FAN1_OVER_RIDE         = 11,    /* Over-ride by system */
  PSU_STATUS_MAX             = 12,
} psu_status_t;

typedef enum {
  PSU_NOT_PRESENT = 0x00,
  PSU_ON          = 0x01,
  PSU_OFF         = 0x02,
  PSU_NO_AC_INPUT = 0x03,
  PSU_OTHER       = 0x04,
} psu_operation_state_t;

typedef struct psu_aggregate_info {
    float  total_power;
    float  total_current;
    float  side_b_power;
    float  side_a_power;
} psu_aggregate_info_t;

typedef struct psu_clear_status_info {
    uint8_t i2c_addr;
    uint8_t reg;
} psu_clear_status_info_t;

typedef struct psu_info {
    uint8_t  psu_num;
    int      i2c_bus;
    int      i2c_addr;
    psu_operation_state_t   operation_state;
    float  volt_input;
    float  current_input;
    float  volt_out;
    float  power_input;
    int    fan_speed;
    uint8_t status_cntr[PSU_STATUS_MAX];
    int    temperature[PSU_FAN_NUM];
} psu_info_t;


/* ****************************************
 *         Common Functions               *
 ******************************************/
uint8_t crc8(const void* vptr, int len);
int i2c_master_selector_ctrl(uint8_t ctrl_reg);
int i2c_master_selector_access(uint8_t bus, uint8_t addr);
int i2c_open(uint8_t bus, uint8_t addr);
int i2c_write_device(const char *device, int value);
int i2c_read_device(const char *device, int *value);
int i2c_read_device_float(char* dir, char *device, float *value);
int get_file_offset(device_type type);
int get_mmap_info (int* cache, device_type type, size_t len);
int convert2compl_to_decimal (int data, int bits);
int read_value_linear (char* dir, char *device, float *value);

/* ****************************************
 *         Functions for eFuse            *
 ******************************************/
int   efuse_to_i2cbus (uint8_t efuse);
int   efuse_is_tripped(uint8_t gpio);
int   efuse_get_info(efuse_info_t *efuse_info);
int   efuse_poll_info(efuse_info_t *efuse_info);
void  efuse_get_mmap_info();
int   efuse_get_status(efuse_info_t *efuse_info);
char* efuse_get_operation_state(efuse_operation_state_t state);
int   efuse_i2c_selector_addr(uint8_t bus);
int   efuse_reset(uint8_t efuse);
void  efuse_show_status(efuse_info_t *efuse_info);
int   efuse_operation (uint8_t efuse_num, uint8_t op);

/* ****************************************
 *         Functions for PSU              *
 ******************************************/
void  reset_stats (psu_info_t *psu_info);
int   psu_poll_info(psu_info_t *psu_info);
int   psu_get_status(psu_info_t *psu_info);
int   psu_clear_status(psu_info_t *psu_info);
char* psu_get_operation_state(psu_operation_state_t state);
int   psu_aggregate_info(psu_aggregate_info_t *psu_aggr_info);
int   reset_psu_mmap_stats (int psu_num, status_info_t *status_info, int num_status);
int   psu_get_mmap_info();

/* ****************************************
 *         Functions for FAN              *
 ******************************************/
int   fan_get_mmap_info();
int   fan_get_info(fan_info_t *fan_info);
int   fan_poll_info(fan_info_t *fan_info);
int   fan_get_status(fan_info_t *fan_info);
