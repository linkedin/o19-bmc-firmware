#define   MAX_PSU_NUM              6
#define   MAX_PSU_NUM_ASIDE        3
#define   PSU_STATUS_POWER_GOOD    0x800
#define   PSU_STATUS_INPUT         0x2000

#define   MAX_I2C_BUS_NUM          12
#define   MIN_I2C_BUS_NUM          0

#define   DEVICE_FILENAME_SIZE     256

int  psu_to_i2cbus (int psu);
int psu_is_present(uint16_t gpio);
void psu_usage ();
int psu_read(int fd, void *buf, size_t size, char *msg);
int psu_read_device(const char *device, int *value);
int psu_write_device(const char *device, int value);
float psu_get_realvalue(int input);
float psu_get_realvalue_vout(int mode, int vout);
