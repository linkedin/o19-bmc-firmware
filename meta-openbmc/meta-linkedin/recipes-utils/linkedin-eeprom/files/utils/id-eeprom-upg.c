#include <facebook/linkedin_eeprom.h>

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <stdlib.h>
#include <arpa/inet.h>

#define LI_EE_V0_F_MAGIC 4
#define LI_EE_V0_F_VERSION 2
#define LI_EE_V0_F_PRODUCT_NAME 15
#define LI_EE_V0_F_PRODUCT_NUMBER 11
#define LI_EE_V0_F_ASSEMBLY_NUMBER 11
#define LI_EE_V0_F_PCBA_NUMBER 12
#define LI_EE_V0_F_PCB_NUMBER 12
#define LI_EE_V0_F_ODM_PCBA_NUMBER 13
#define LI_EE_V0_F_ODM_PCBA_SERIAL 13
#define LI_EE_V0_F_PRODUCT_STATE 1
#define LI_EE_V0_F_PRODUCT_VERSION 1
#define LI_EE_V0_F_PRODUCT_SUBVERSION 1
#define LI_EE_V0_F_PRODUCT_SERIAL 12
#define LI_EE_V0_F_PRODUCT_ASSET 13
#define LI_EE_V0_F_SYSTEM_MANUFACTURER 5
#define LI_EE_V0_F_SYSTEM_MANU_DATE 4
#define LI_EE_V0_F_PCB_MANUFACTURER 5
#define LI_EE_V0_F_ASSEMBLED 3
#define LI_EE_V0_F_LOCAL_MAC 12
#define LI_EE_V0_F_EXT_MAC_BASE 12
#define LI_EE_V0_F_EXT_MAC_SIZE  2
#define LI_EE_V0_F_RESERVED 8
#define LI_EE_V0_F_CRC8 1

#define LI_EE_V1_F_MAGIC 4
#define LI_EE_V1_F_VERSION 2
#define LI_EE_V1_F_PRODUCT_NAME 16
#define LI_EE_V1_F_PRODUCT_NUMBER 16
#define LI_EE_V1_F_ASSEMBLY_NUMBER 12
#define LI_EE_V1_F_PCBA_NUMBER 14
#define LI_EE_V1_F_PCB_NUMBER 14
#define LI_EE_V1_F_ODM_PCBA_NUMBER 14
#define LI_EE_V1_F_ODM_PCBA_SERIAL 24
#define LI_EE_V1_F_PRODUCT_STATE 2
#define LI_EE_V1_F_PRODUCT_VERSION 1
#define LI_EE_V1_F_PRODUCT_SUBVERSION 1
#define LI_EE_V1_F_PRODUCT_SERIAL 28
#define LI_EE_V1_F_PRODUCT_ASSET 12
#define LI_EE_V1_F_SYSTEM_MANUFACTURER 8
#define LI_EE_V1_F_SYSTEM_MANU_DATE 8
#define LI_EE_V1_F_PCB_MANUFACTURER 8
#define LI_EE_V1_F_ASSEMBLED 8
#define LI_EE_V1_F_LOCAL_MAC 12
#define LI_EE_V1_F_EXT_MAC_BASE 12
#define LI_EE_V1_F_EXT_MAC_SIZE  2
#define LI_EE_V1_F_RESERVED 8
#define LI_EE_V1_F_CRC8 1

#define BOARD_ID_EEPROM_TXT_FILE "/usr/local/bin/bmc-id-eeprom.example"
#define LI_EEPROM_V2_STR   "02"
#define LI_EEPROM_V1_STR   "01"

FILE *ftxt;

struct li_eeprom_v0 {
  /* version number of the eeprom. Must be the first element */
  char li_version[LI_EE_V0_F_VERSION + 1];

  /* Product Name: 16 characters XX-XXXXXX */
  char li_product_name[LI_EE_V0_F_PRODUCT_NAME + 1];

  /* Product Part Number: 16 characters XXXXXXXXXXXXXXXX */
  char li_product_number[LI_EE_V0_F_PRODUCT_NUMBER + 1];

  /* System Assembly Part Number: 12 characters XXXXXXXXXXXX */
  char li_assembly_number[LI_EE_V0_F_ASSEMBLY_NUMBER + 1];

  /* linkedin PCBA Part Number: 14 characters XXXXXXXXXXXXXX */
  char li_pcba_number[LI_EE_V0_F_PCBA_NUMBER + 1];

  /* Linkedin PCB Part Number: 14 characters XXXXXXXXXXXXXX */
  char li_pcb_number[LI_EE_V0_F_PCB_NUMBER + 1];

  /* ODM PCBA Part Number: 14 characters XXXXXXXXXXXXXX */
  char li_odm_pcba_number[LI_EE_V0_F_ODM_PCBA_NUMBER + 1];

  /* ODM PCBA Serial Number: 12 characters XXXXXXXXXXXX */
  char li_odm_pcba_serial[LI_EE_V0_F_ODM_PCBA_SERIAL + 1];

  /* Product Production State */
  char li_production_state[LI_EE_V0_F_PRODUCT_STATE + 1];

  /* Product Version */
  char li_product_version[LI_EE_V0_F_PRODUCT_VERSION + 1];

  /* Product Sub Version */
  char li_product_subversion[LI_EE_V0_F_PRODUCT_SUBVERSION + 1];

  /* Product Serial Number: 16 characters XXXXXXXXXXXXXXXX */
  char li_product_serial[LI_EE_V0_F_PRODUCT_SERIAL + 1];

  /* Product Asset Tag: XXXXXXXX */
  char li_product_asset[LI_EE_V0_F_PRODUCT_ASSET + 1];

  /* System Manufacturer: XXXXXXXX */
  char li_system_manufacturer[LI_EE_V0_F_SYSTEM_MANUFACTURER + 1];

  /* System Manufacturing Date */
  char li_system_manufacturing_date[LI_EE_V0_F_SYSTEM_MANU_DATE + 1];

  /* PCB Manufacturer: XXXXXXXXX */
  char li_pcb_manufacturer[LI_EE_V1_F_PCB_MANUFACTURER + 1];

  /* Assembled At: XXXXXXXX */
  char li_assembled[LI_EE_V1_F_ASSEMBLED + 1];

  /* Local MAC Address */
  char li_local_mac[LI_EE_V0_F_LOCAL_MAC + 1];

  /* Extended MAC Address */
  char li_mac_base[LI_EE_V0_F_EXT_MAC_BASE + 1];

  /* Extended MAC Address Size */
  char li_mac_size[LI_EE_V0_F_EXT_MAC_SIZE + 1];

  /* Reserved field: 8 characters */
  char li_reserved[LI_EE_V0_F_RESERVED + 1];

  /* CRC8 */
  char li_crc8[LI_EE_V1_F_CRC8 + 1];
};

struct li_eeprom_v1 {
  /* version number of the eeprom */
  char li_version[LI_EE_V1_F_VERSION + 1];

  /* Product Name */
  char li_product_name[LI_EE_V1_F_PRODUCT_NAME + 1];

  /* Top Level 20 - Product Part Number: XX-XXXXXX */
  char li_product_number[LI_EE_V1_F_PRODUCT_NUMBER + 1];

  /* System Assembly Part Number XXX-XXXXXX-XX */
  char li_assembly_number[LI_EE_V1_F_ASSEMBLY_NUMBER + 1];

  /* Facebook PCBA Part Number: XXX-XXXXXXX-XX */
  char li_pcba_number[LI_EE_V1_F_PCBA_NUMBER + 1];

  /* Facebook PCB Part Number: XXX-XXXXXXX-XX */
  char li_pcb_number[LI_EE_V1_F_PCB_NUMBER + 1];

  /* ODM PCB Part Number: XXXXXXXXXXXX */
  char li_odm_pcba_number[LI_EE_V1_F_ODM_PCBA_NUMBER + 1];

  /* ODM PCB Serial Number: XXXXXXXXXXXX */
  char li_odm_pcba_serial[LI_EE_V1_F_ODM_PCBA_SERIAL + 1];

  /* Product Production State */
  char li_production_state[LI_EE_V1_F_PRODUCT_STATE + 1];

  /* Product Version */
  char li_product_version[LI_EE_V1_F_PRODUCT_VERSION + 1];

  /* Product Sub Version */
  char li_product_subversion[LI_EE_V1_F_PRODUCT_SUBVERSION + 1];

  /* Product Serial Number: XXXXXXXX */
  char li_product_serial[LI_EE_V1_F_PRODUCT_SERIAL + 1];

  /* Product Asset Tag: XXXXXXXX */
  char li_product_asset[LI_EE_V1_F_PRODUCT_ASSET + 1];

  /* System Manufacturer: XXXXXXXX */
  char li_system_manufacturer[LI_EE_V1_F_SYSTEM_MANUFACTURER + 1];

  /* System Manufacturing Date: mm-dd-yyyy */
  char li_system_manufacturing_date[LI_EE_V1_F_SYSTEM_MANU_DATE + 1];

  /* PCB Manufacturer: XXXXXXXXX */
  char li_pcb_manufacturer[LI_EE_V1_F_PCB_MANUFACTURER + 1];

  /* Assembled At: XXXXXXXX */
  char li_assembled[LI_EE_V1_F_ASSEMBLED + 1];

  /* Local MAC Address */
  char li_local_mac[LI_EE_V1_F_LOCAL_MAC + 1];

  /* Extended MAC Address */
  char li_mac_base[LI_EE_V1_F_EXT_MAC_BASE + 1];

  /* Extended MAC Address Size */
  char li_mac_size[LI_EE_V1_F_EXT_MAC_SIZE + 1];

  /* Location on Fabric: "LEFT"/"RIGHT", "WEDGE", "LC" */
  char li_reserved[LI_EE_V1_F_RESERVED + 1];

  /* CRC8 */
  char li_crc8[LI_EE_V1_F_CRC8 + 1];
};

static inline void li_copy_uint8(uint8_t *val, const uint8_t** src,
                                  int src_len)
{
  assert(src_len >= sizeof(*val));
  *val = **src;
  (*src) += src_len;
}

static inline void li_copy_uint16(uint16_t *val, const uint8_t** src,
                                  int src_len)
{
  assert(src_len >= sizeof(*val));
  *val = (**src) | ((*(*src + 1)) << 8);
  (*src) += src_len;
}

static inline void li_strcpy(char *dst, int dst_len,
                              const uint8_t **src, int src_len)
{
  assert(dst_len >= src_len + 1);    /* larger because of '\0' */
  strncpy(dst, (char *)*src, src_len);
  dst[src_len + 1] = '\0';
  (*src) += src_len;
}

static inline void li_copy_product_number(
    char *dst, int dst_len, const uint8_t **src, int src_len)
{
  int i;
  const uint8_t *cur = *src;
  /* 12 chars in the format of PAXXXXXXX-XXX, 2 additional chars */
  assert(dst_len >= src_len + 2);
  for (i = 0; i < 9; i++) {
    *dst++ = *cur++;
  }
  *dst++ = '-';
  for (i = 0; i < 3; i++) {
    *dst++ = *cur++;
  }
  *dst = '\0';
  (*src) += src_len;
}

static inline void li_copy_assembly_number(
    char *dst, int dst_len, const uint8_t **src, int src_len)
{
  int i;
  const uint8_t *cur = *src;
  /* 12 chars in the format of PAXXXXXXX-XXX, 2 additional chars */
  assert(dst_len >= src_len + 2);
  for (i = 0; i < 9; i++) {
    *dst++ = *cur++;
  }
  *dst++ = '-';
  for (i = 0; i < 3; i++) {
    *dst++ = *cur++;
  }
  *dst = '\0';
  (*src) += src_len;
}

static inline void li_copy_pcba_part(
    char *dst, int dst_len, const uint8_t **src, int src_len)
{
  int i;
  const uint8_t *cur = *src;
  /* 12 chars in the format of PBXXXXXXX-XXX, 2 additional chars */
  assert(dst_len >= src_len + 2);
  for (i = 0; i < 9; i++) {
    *dst++ = *cur++;
  }
  *dst++ = '-';
  for (i = 0; i < 3; i++) {
    *dst++ = *cur++;
  }
  *dst = '\0';
  (*src) += src_len;
}

static inline void li_copy_pcb_part(
    char *dst, int dst_len, const uint8_t **src, int src_len)
{
  int i;
  const uint8_t *cur = *src;
  /* 11 chars in the format of PAXX-XXXXX-XX, 3 additional chars */
  assert(dst_len >= src_len + 3);
  for (i = 0; i < 4; i++) {
    *dst++ = *cur++;
  }
  *dst++ = '-';
  for (i = 0; i < 5; i++) {
    *dst++ = *cur++;
  }
  *dst++ = '-';
  for (i = 0; i < 2; i++) {
    *dst++ = *cur++;
  }
  *dst = '\0';
  (*src) += src_len;
}

static inline void li_copy_date_v5(
    char *dst, int dst_len, const uint8_t **src, int src_len)
{
  int i;
  const uint8_t *cur = *src;
  /* 12 chars in the format Day Mon DayofMonth, Year, 5 additional chars */
  assert(dst_len >= src_len + 5);

  for (i = 0; i < 3; i++) {
    *dst++ = *cur++;
  }
  *dst++ = ' ';

  for (i = 0; i < 3; i++) {
    *dst++ = *cur++;
  }
  *dst++ = ' ';

  for (i = 0; i < 2; i++) {
    *dst++ = *cur++;
  }
  *dst++ = ',';
  *dst++ = ' ';

  for (i = 0; i < 4; i++) {
    *dst++ = *cur++;
  }

  *dst = '\0';

  (*src) += src_len;
}

static inline void li_copy_date_v2(
    char *dst, int dst_len, const uint8_t **src, int src_len)
{
  const uint8_t *cur = *src;
  uint16_t year;
  uint8_t month;
  uint8_t day;
  /* mm-dd-yy in output */
  assert(dst_len >= 9);
  /* input is 4 bytes YY YY MM DD */
  assert(src_len >= 4);
  li_copy_uint16(&year, &cur, 2);
  li_copy_uint8(&month, &cur, 1);
  li_copy_uint8(&day, &cur, 1);
  snprintf(dst, dst_len, "%02d-%02d-%02d", month % 13, day % 32, year % 100);
  (*src) += src_len;
}

static inline uint8_t _a2v(const uint8_t *a)
{
  uint8_t v = *a;
  if ('0' <= v && v <= '9') {
    return v - '0';
  }
  if ('a' <= v && v <= 'z') {
    return v - 'a' + 10;
  }
  if ('A' <= v && v <= 'Z') {
    return v - 'A' + 10;
  }
  return 0;
}

static inline void li_copy_mac(
    uint8_t* dst, int dst_len, const uint8_t **src, int src_len)
{
  int i;
  const uint8_t *cur = *src;

  assert(dst_len >= 6);
  assert(src_len >= 12);

  for (i = 0; i < 6; i++) {
    *dst = (_a2v(cur) << 4) | _a2v(cur + 1);
    dst++;
    cur +=2;
  }
  (*src) += src_len;
}

static int
li_v0_parse_buf(
             const uint8_t *buf,
             int len,
             struct li_eeprom_v0 *eeprom)
{
  int rc = 0;
  const uint8_t* cur = buf;
  char magics[LI_EE_V0_F_MAGIC + 1];
  int crc_len;
  uint8_t crc8;

  memset(eeprom, 0, sizeof(struct li_eeprom_v0));
  memset(magics, 0, sizeof(magics));

  cur = buf;

  /* Confirm the version number */
  li_strcpy(eeprom->li_version, sizeof(eeprom->li_version), &cur, LI_EE_V0_F_VERSION);
  syslog(LOG_NOTICE, "Read EEPROM version     : %s",
         eeprom->li_version);

  if (eeprom->li_version != LI_EEPROM_VERSION0) {
    syslog(LOG_WARNING, "Expected EEPROM version 0, exiting");
    rc = EEXIST;
    goto out;
  }

  li_strcpy(eeprom->li_product_name,
             sizeof(eeprom->li_product_name),
             &cur, LI_EE_V0_F_PRODUCT_NAME);
  syslog(LOG_NOTICE, "LI V0 EEPROM product name  : %s",
         eeprom->li_product_name);

  li_strcpy(eeprom->li_product_number,
             sizeof(eeprom->li_product_number),
             &cur, LI_EE_V0_F_PRODUCT_NUMBER);
  syslog(LOG_NOTICE, "LI V0 EEPROM product num   : %s",
         eeprom->li_product_number);

  /* System Assembly Part Number */
  li_strcpy(eeprom->li_assembly_number,
             sizeof(eeprom->li_assembly_number),
             &cur, LI_EE_V0_F_ASSEMBLY_NUMBER);

  /* Partner PCBA Part Number: PAXX-XXXXX-XX */
  li_strcpy(eeprom->li_pcba_number,
            sizeof(eeprom->li_pcba_number),
            &cur, LI_EE_V0_F_PCBA_NUMBER);

  /* Partner PCB Part Number: PBXXXXXXX-XXX */
  li_strcpy(eeprom->li_pcb_number,
            sizeof(eeprom->li_pcb_number),
            &cur, LI_EE_V0_F_PCB_NUMBER);

  /* PCBA Part Number: PAXX-XXXXX-XX */
  li_strcpy(eeprom->li_odm_pcba_number,
            sizeof(eeprom->li_odm_pcba_number),
            &cur, LI_EE_V0_F_ODM_PCBA_NUMBER);

  /* PCBA Serial Number: XXXXXXXXXXXX */
  li_strcpy(eeprom->li_odm_pcba_serial,
             sizeof(eeprom->li_odm_pcba_serial),
             &cur, LI_EE_V0_F_ODM_PCBA_SERIAL);

  /* Product Production State */
  li_strcpy(eeprom->li_production_state,
             sizeof(eeprom->li_production_state),
             &cur, LI_EE_V0_F_PRODUCT_STATE);

  /* Product Version */
  li_strcpy(eeprom->li_product_version,
             sizeof(eeprom->li_product_version),
             &cur, LI_EE_V0_F_PRODUCT_VERSION);

  /* Product subversion */
  li_strcpy(eeprom->li_product_subversion,
             sizeof(eeprom->li_product_subversion),
             &cur, LI_EE_V0_F_PRODUCT_SUBVERSION);

  /* Product Serial Number: XXXXXXXX */
  li_strcpy(eeprom->li_product_serial,
             sizeof(eeprom->li_product_serial),
             &cur, LI_EE_V0_F_PRODUCT_SERIAL);
  syslog(LOG_NOTICE, "Li V0 EEPROM product ser   : %s",
         eeprom->li_product_serial);

  /* Product Asset Tag: XXXXXXXX */
  li_strcpy(eeprom->li_product_asset,
             sizeof(eeprom->li_product_asset),
             &cur, LI_EE_V0_F_PRODUCT_ASSET);
  syslog(LOG_NOTICE, "Li V0 EEPROM product asset : %s",
         eeprom->li_product_asset);

  /* System Manufacturer: XXXXXXXX */
  li_strcpy(eeprom->li_system_manufacturer,
             sizeof(eeprom->li_system_manufacturer),
             &cur, LI_EE_V0_F_SYSTEM_MANUFACTURER);

  /* System Manufacturing Date: mm-dd-yy */
  li_strcpy(eeprom->li_system_manufacturing_date,
                   sizeof(eeprom->li_system_manufacturing_date),
                   &cur, LI_EE_V0_F_SYSTEM_MANU_DATE);
  syslog(LOG_NOTICE, "Li V0 EEPROM Mfg Date      : %s",
         eeprom->li_system_manufacturing_date);

  /* PCB Manufacturer: XXXXXXXXX */
  li_strcpy(eeprom->li_pcb_manufacturer,
             sizeof(eeprom->li_pcb_manufacturer),
             &cur, LI_EE_V0_F_PCB_MANUFACTURER);

  /* Assembled At: XXXXXXXX */
  li_strcpy(eeprom->li_assembled,
             sizeof(eeprom->li_assembled),
             &cur, LI_EE_V0_F_ASSEMBLED);

  /* Local MAC Address */
  li_strcpy(eeprom->li_local_mac,
               sizeof(eeprom->li_local_mac),
               &cur, LI_EE_V0_F_LOCAL_MAC);

  /* Extended MAC Address */
  li_strcpy(eeprom->li_mac_base,
               sizeof(eeprom->li_mac_base),
               &cur, LI_EE_V0_F_EXT_MAC_BASE);

  /* Extended MAC Address Size */
  li_strcpy(eeprom->li_mac_size,
             sizeof(eeprom->li_mac_size),
                 &cur,LI_EE_V0_F_EXT_MAC_SIZE);

  /* Location on Fabric: "LEFT"/"RIGHT", "WEDGE", "LC" */
  li_strcpy(eeprom->li_reserved,
             sizeof(eeprom->li_reserved),
             &cur, LI_EE_V0_F_RESERVED);

  /* CRC8 */
  li_strcpy(eeprom->li_crc8,
             sizeof(eeprom->li_crc8),
                 &cur, LI_EE_V1_F_CRC8);
 out:
  return rc;
}

int
li_v0_parse(struct li_eeprom_v0 *v0)
{
  int rc = 0;
  const char *fn = LI_EEPROM_FILE;
  uint32_t len;
  FILE *fin = NULL;
  char buf[LI_EEPROM_SIZE];

  if (!v0) {
    return -EINVAL;
  }

  fin = fopen(fn, "r");
  if (fin == NULL) {
    rc = errno;
    syslog(LOG_WARNING, "Failed to open %s", LI_EEPROM_FILE);
    goto out;
  }

  /* check the file size */
  rc = fseek(fin, 0, SEEK_END);
  if (rc) {
    rc = errno;
    syslog(LOG_WARNING, "Failed to seek to the end of %s", LI_EEPROM_FILE);
    goto out;
  }

  len = ftell(fin);
  if (len < LI_EEPROM_SIZE) {
    rc = ENOSPC;
    syslog(LOG_WARNING, "File '%s' is too small (%u < %u)", LI_EEPROM_FILE,
            len, LI_EEPROM_SIZE);
    goto out;
  }

  /* go back to the beginning of the file */
  rewind(fin);

  rc = fread(buf, 1, sizeof(buf), fin);
  if (rc < sizeof(buf)) {
    syslog(LOG_WARNING, "Failed to complete the read. Only got %d", rc);
    rc = ENOSPC;
    goto out;
  }

  rc = li_v0_parse_buf((const uint8_t *)buf, sizeof(buf), v0);
  if (rc) {
    goto out;
  }

 out:
  syslog(LOG_NOTICE, "wedge_eeprom_parse: return code 0x%x", rc);

  if (fin) {
    fclose(fin);
  }

  return rc;
}

static int
li_v1_parse_buf(
             const uint8_t *buf,
             int len,
             struct li_eeprom_v1 *eeprom)
{
  int rc = 0;
  const uint8_t* cur = buf;
  char magics[LI_EE_V1_F_MAGIC + 1];
  int crc_len;
  uint8_t crc8;

  memset(eeprom, 0, sizeof(struct li_eeprom_v1));
  memset(magics, 0, sizeof(magics));

  /* Verify the magic number */
  cur = buf;
  li_strcpy(magics, sizeof(magics), &cur, LI_EE_V1_F_MAGIC);
  syslog(LOG_NOTICE, "li_v1_parse_buf(): Detected magic number   : %s", magics);
  if (!strcmp(LI_EEPROM_MAGIC_NUM, magics) == 0) {
    rc = EFAULT;
    syslog(LOG_WARNING, "Unexpected magic word %s", magics);
    goto out;
  }

  /* Confirm the version number, should be 01 */
  li_strcpy(eeprom->li_version, sizeof(eeprom->li_version), &cur, LI_EE_V1_F_VERSION);
  syslog(LOG_NOTICE, "Read EEPROM version     : %s",
         eeprom->li_version);
  if (strcmp(eeprom->li_version, LI_EEPROM_V1_STR) != 0) {
    syslog(LOG_WARNING, "Expected EEPROM version 1, exiting");
    rc = EEXIST;
    goto out;
  }

  li_strcpy(eeprom->li_product_name,
             sizeof(eeprom->li_product_name),
             &cur, LI_EE_V1_F_PRODUCT_NAME);
  syslog(LOG_NOTICE, "LI V1 EEPROM product name  : %s",
         eeprom->li_product_name);

  li_strcpy(eeprom->li_product_number,
             sizeof(eeprom->li_product_number),
             &cur, LI_EE_V1_F_PRODUCT_NUMBER);
  syslog(LOG_NOTICE, "LI V1 EEPROM product num   : %s",
         eeprom->li_product_number);

  /* System Assembly Part Number */
  li_strcpy(eeprom->li_assembly_number,
             sizeof(eeprom->li_assembly_number),
             &cur, LI_EE_V1_F_ASSEMBLY_NUMBER);

  /* Partner PCBA Part Number: PAXX-XXXXX-XX */
  li_strcpy(eeprom->li_pcba_number,
                              sizeof(eeprom->li_pcba_number),
                              &cur, LI_EE_V1_F_PCBA_NUMBER);

  /* Partner PCB Part Number: PBXXXXXXX-XXX */
  li_strcpy(eeprom->li_pcb_number,
                             sizeof(eeprom->li_pcb_number),
                             &cur, LI_EE_V1_F_PCB_NUMBER);

  /* PCBA Part Number: PAXX-XXXXX-XX */
  li_strcpy(eeprom->li_odm_pcba_number,
                              sizeof(eeprom->li_odm_pcba_number),
                              &cur, LI_EE_V1_F_ODM_PCBA_NUMBER);

  /* PCBA Serial Number: XXXXXXXXXXXX */
  li_strcpy(eeprom->li_odm_pcba_serial,
             sizeof(eeprom->li_odm_pcba_serial),
             &cur, LI_EE_V1_F_ODM_PCBA_SERIAL);

  /* Product Production State */
  li_strcpy(eeprom->li_production_state,
             sizeof(eeprom->li_production_state),
             &cur, LI_EE_V1_F_PRODUCT_STATE);

  /* Product Version */
  li_strcpy(eeprom->li_product_version,
             sizeof(eeprom->li_product_version),
             &cur, LI_EE_V1_F_PRODUCT_VERSION);

  /* Product subversion */
  li_strcpy(eeprom->li_product_subversion,
             sizeof(eeprom->li_product_subversion),
             &cur, LI_EE_V1_F_PRODUCT_SUBVERSION);

  /* Product Serial Number: XXXXXXXX */
  li_strcpy(eeprom->li_product_serial,
             sizeof(eeprom->li_product_serial),
             &cur, LI_EE_V1_F_PRODUCT_SERIAL);
  syslog(LOG_NOTICE, "Li V1 EEPROM product ser   : %s",
         eeprom->li_product_serial);

  /* Product Asset Tag: XXXXXXXX */
  li_strcpy(eeprom->li_product_asset,
             sizeof(eeprom->li_product_asset),
             &cur, LI_EE_V1_F_PRODUCT_ASSET);
  syslog(LOG_NOTICE, "Li V1 EEPROM product asset : %s",
         eeprom->li_product_asset);

  /* System Manufacturer: XXXXXXXX */
  li_strcpy(eeprom->li_system_manufacturer,
             sizeof(eeprom->li_system_manufacturer),
             &cur, LI_EE_V1_F_SYSTEM_MANUFACTURER);

  /* System Manufacturing Date: mm-dd-yy */
  li_strcpy(eeprom->li_system_manufacturing_date,
                   sizeof(eeprom->li_system_manufacturing_date),
                   &cur, LI_EE_V1_F_SYSTEM_MANU_DATE);
  syslog(LOG_NOTICE, "Li V1 EEPROM Mfg Date      : %s",
         eeprom->li_system_manufacturing_date);


  /* PCB Manufacturer: XXXXXXXXX */
  li_strcpy(eeprom->li_pcb_manufacturer,
             sizeof(eeprom->li_pcb_manufacturer),
             &cur, LI_EE_V1_F_PCB_MANUFACTURER);

  /* Assembled At: XXXXXXXX */
  li_strcpy(eeprom->li_assembled,
             sizeof(eeprom->li_assembled),
             &cur, LI_EE_V1_F_ASSEMBLED);

  /* Local MAC Address */
  li_strcpy(eeprom->li_local_mac,
               sizeof(eeprom->li_local_mac),
               &cur, LI_EE_V1_F_LOCAL_MAC);

   syslog(LOG_NOTICE, "Li V1 EEPROM local mac      : %s",
          eeprom->li_local_mac);

  /* Extended MAC Address */
  li_strcpy(eeprom->li_mac_base,
               sizeof(eeprom->li_mac_base),
               &cur, LI_EE_V1_F_EXT_MAC_BASE);

  /* Extended MAC Address Size */
  li_strcpy(eeprom->li_mac_size,
             sizeof(eeprom->li_mac_size),
                 &cur,LI_EE_V1_F_EXT_MAC_SIZE);

  /* Location on Fabric: "LEFT"/"RIGHT", "WEDGE", "LC" */
  li_strcpy(eeprom->li_reserved,
             sizeof(eeprom->li_reserved),
             &cur, LI_EE_V1_F_RESERVED);

  /* CRC8 */
  li_strcpy(eeprom->li_crc8,
             sizeof(eeprom->li_crc8),
                 &cur, LI_EE_V1_F_CRC8);
 out:
  return rc;
}

int
li_v1_parse(struct li_eeprom_v1 *v1)
{
  int rc = 0;
  const char *fn = LI_EEPROM_FILE;
  uint32_t len;
  FILE *fin;
  char buf[LI_EEPROM_SIZE];

  if (!v1) {
    return -EINVAL;
  }

  fin = fopen(fn, "r");
  if (fin == NULL) {
    rc = errno;
    syslog(LOG_WARNING, "Failed to open %s", LI_EEPROM_FILE);
    goto out;
  }

  /* check the file size */
  rc = fseek(fin, 0, SEEK_END);
  if (rc) {
    rc = errno;
    syslog(LOG_WARNING, "Failed to seek to the end of %s", LI_EEPROM_FILE);
    goto out;
  }

  len = ftell(fin);
  if (len < LI_EEPROM_SIZE) {
    rc = ENOSPC;
    syslog(LOG_WARNING, "File '%s' is too small (%u < %u)", LI_EEPROM_FILE,
            len, LI_EEPROM_SIZE);
    goto out;
  }

  /* go back to the beginning of the file */
  rewind(fin);

  rc = fread(buf, 1, sizeof(buf), fin);
  if (rc < sizeof(buf)) {
    syslog(LOG_WARNING, "Failed to complete the read. Only got %d", rc);
    rc = ENOSPC;
    goto out;
  }

  rc = li_v1_parse_buf((const uint8_t *)buf, sizeof(buf), v1);
  if (rc) {
    goto out;
  }

 out:
  syslog(LOG_NOTICE, "wedge_eeprom_parse: return code 0x%x", rc);

  if (fin) {
    fclose(fin);
  }

  return rc;
}

static int
get_version(uint8_t *eep_ver)
{
  int rc = 0;
  const char *fn = LI_EEPROM_FILE;
  char buf[LI_EEPROM_SIZE];
  FILE *fin;
  uint16_t magic;
  uint8_t vers = 0;
  uint32_t len;
  const uint8_t *cur = buf;
  char version[LI_EE_V1_F_VERSION + 1];
  char magics[LI_EE_V1_F_MAGIC + 1];

  fin = fopen(fn, "r");
  if (fin == NULL) {
    rc = errno;
    syslog(LOG_WARNING, "Failed to open %s", LI_EEPROM_FILE);
    goto out;
  }

  /* check the file size */
  rc = fseek(fin, 0, SEEK_END);
  if (rc) {
    rc = errno;
    syslog(LOG_WARNING, "Failed to seek to the end of %s", LI_EEPROM_FILE);
    goto out;
  }

  len = ftell(fin);
  if (len < LI_EEPROM_SIZE) {
    rc = ENOSPC;
    syslog(LOG_WARNING, "File '%s' is too small (%u < %u)", LI_EEPROM_FILE,
            len, LI_EEPROM_SIZE);
    goto out;
  }

  syslog(LOG_NOTICE, "EEPROM file size        : %d", len);
  /* go back to the beginning of the file */
  rewind(fin);

  rc = fread(buf, 1, sizeof(buf), fin);
  if (rc < sizeof(buf)) {
    syslog(LOG_WARNING, "Failed to complete the read. Only got %d", rc);
    rc = ENOSPC;
    goto out;
  }

  rc = 0;
  /* Get the magic number */
  cur = buf;

  memset(magics, 0, sizeof(magics));
  memset(version, 0, sizeof(version));

  li_strcpy(magics, sizeof(magics), &cur, LI_EEPROM_F_MAGIC);
  if (strcmp(LI_EEPROM_MAGIC_NUM, magics) == 0) {
     syslog(LOG_NOTICE, "LI Detected magic number   : %s", magics);
     if (strcmp(LI_EEPROM_MAGIC_NUM, magics) == 0) {
         li_strcpy(version, sizeof(version), &cur, LI_EE_V1_F_VERSION);
         syslog(LOG_NOTICE, "LI MAGIC WORD, Detected EEPROM version : 0x%s", version);

         if (strcmp(version, LI_EEPROM_V1_STR) == 0) {
          *eep_ver = LI_EEPROM_VERSION1;
         }
         else if (strcmp(version, LI_EEPROM_V2_STR) == 0) {
             /* version: 02 */
             syslog(LOG_NOTICE, "LI MAGIC WORD, Detected EEPROM version 2");
             *eep_ver = LI_EEPROM_VERSION2;
         }
         else
            *eep_ver = -1;
            syslog(LOG_NOTICE, "LI MAGIC WORD, Unsupported version: %s\n", version);
        }
  }
  else {
      /*
       * No Linkedin Magic word
       */
      cur = buf;
      li_strcpy(version, sizeof(version), &cur, LI_EE_V1_F_VERSION);
      if (strncmp(version, LI_EEPROM_V1_STR, LI_EE_V1_F_VERSION) == 0) {
         syslog(LOG_NOTICE, "Detected EEPROM version : 0x%s", version);
         *eep_ver = LI_EEPROM_VERSION0;
      }
      else {
          *eep_ver = -1;
          syslog(LOG_NOTICE, "get_version(): Unsupported version: %s\n", version);
          goto out;
      }
  }

 out:
  if (fin) {
    fclose(fin);
  }

  syslog(LOG_NOTICE, "get_version returns version: 0x%X", *eep_ver);
  return rc;
}

int main (int argc, char **argv)
{
  int rc = 0;
  const char *fn = LI_EEPROM_FILE;
  char buf[LI_EEPROM_SIZE];
  struct li_eeprom_gn eeprom;
  struct li_eeprom_v1 li_v1_ee;
  struct li_eeprom_v0 li_v0_ee;
  FILE *fin;
  uint8_t crc_len;
  uint8_t eep_version;
  char tmp_buf[2];
  int i;

  /* First get the current eeprom version */
  rc = get_version(&eep_version);
  if (rc) {
    syslog(LOG_WARNING, "Failed to read current EEPROM version");
    goto out;
  }

  switch (eep_version) {
  case LI_EEPROM_VERSION0:
    printf("\nLinkedIn V0 eeprom upgrade\n");
    rc = li_v0_parse(&li_v0_ee);
    if (rc) {
      syslog(LOG_WARNING, "Failed to parse LI V0 EEPROM");
      printf("Failed to parse LI V0 EEPROM\n");
      goto out;
    }
    syslog(LOG_NOTICE, "Storing these EEPROM linked v0 parameters\n");
    syslog(LOG_NOTICE, "%-24s : %s\n", "PCBA Serial Number", li_v0_ee.li_odm_pcba_serial);
    syslog(LOG_NOTICE, "%-24s : %s\n", "System Serial Number", li_v0_ee.li_product_serial);
    syslog(LOG_NOTICE, "%-24s : %s\n", "BMC MAC", li_v0_ee.li_local_mac);
    break;
  case LI_EEPROM_VERSION1:
    printf("\nLinkedIn V1 eeprom upgrade\n");
    rc = li_v1_parse(&li_v1_ee);
    if (rc) {
      syslog(LOG_WARNING, "Failed to parse LI V1 EEPROM");
      printf("Failed to parse LI V1 EEPROM\n");
      goto out;
    }
    syslog(LOG_NOTICE, "Storing these EEPROM linked v1 parameters\n");
    syslog(LOG_NOTICE, "%-24s : %s\n", "PCBA Serial Number", li_v1_ee.li_odm_pcba_serial);
    syslog(LOG_NOTICE, "%-24s : %s\n", "System Serial Number", li_v1_ee.li_product_serial);
    syslog(LOG_NOTICE, "%-24s : %s\n", "BMC MAC", li_v1_ee.li_local_mac);
    syslog(LOG_NOTICE, "%-24s : %02X:%02X:%02X:%02X:%02X:%02X\n", "BMC MAC",
           li_v1_ee.li_local_mac[0], li_v1_ee.li_local_mac[1],
           li_v1_ee.li_local_mac[2], li_v1_ee.li_local_mac[3],
           li_v1_ee.li_local_mac[4], li_v1_ee.li_local_mac[5]);
    break;

  case LI_EEPROM_VERSION2:
    printf("EEPROM version 2 is current, no need to upgrade\n");
    syslog(LOG_NOTICE, "EEPROM version 2 is current, no need to upgrade");
    return 0;

  default:
    syslog(LOG_WARNING, "Invalid EEPROM version");
    return -1;
  }

  ftxt = fopen(BOARD_ID_EEPROM_TXT_FILE, "r+");
  if (ftxt == NULL) {
    rc = errno;
    syslog(LOG_WARNING, "Failed to open %s", BOARD_ID_EEPROM_TXT_FILE);
    printf("Failed to open %s\n", BOARD_ID_EEPROM_TXT_FILE);

    goto out;
  }

  fin = fopen(fn, "r+");
  if (fin == NULL) {
    rc = errno;
    syslog(LOG_WARNING, "Failed to open %s", LI_EEPROM_FILE);
    goto out;
  }

  rc = fread(buf, 1, sizeof buf, fin);
  if (rc < sizeof buf) {
    syslog(LOG_WARNING, "Failed to complete read, only got %d bytes", rc);
    printf("Failed to complete read, only got %d bytes\n");
    rc = ENOSPC;
    goto out;
  }
  memcpy ((char*)&eeprom, (char*)buf+LI_EEPROM_F_MAGIC, sizeof(eeprom));

  /* go back to the beginning of the file */
  rewind(fin);

  copy_data (LI_EEPROM_MAGIC_NUM, buf, LI_EEPROM_F_MAGIC);
  rc = fwrite(buf, 1, LI_EEPROM_F_MAGIC, fin);
  if (rc < 2) {
    syslog(LOG_WARNING, "Failed to write magic number");
    printf("Failed to write magic number\n");
    goto out;
  }

  if (li_fill_struct_from_file (ftxt, &eeprom)) {
    goto out;
  } else {
    crc_len = LI_EEPROM_V1_SIZE;
  }

  /* Fill in the archived fields */
  syslog(LOG_NOTICE, "BEFORE RESTORING ARCHIVED ITEMS:");
  syslog(LOG_NOTICE, "Product Asset Tag     : %s",
         eeprom.li_product_asset);
  syslog(LOG_NOTICE, "Product Serial Num    : %s",
         eeprom.li_product_serial);
  syslog(LOG_NOTICE, "Manufacturing date    : %s",
         eeprom.li_system_manufacturing_date);
  syslog(LOG_NOTICE, "Fabric Location       : %s",
         eeprom.li_reserved);

  char product_asset[LI_EEPROM_F_PRODUCT_ASSET];
  copy_data((char*)eeprom.li_product_asset,
            (char*)product_asset,
            sizeof eeprom.li_product_asset);

  switch (eep_version) {
  case LI_EEPROM_VERSION0:
    copy_data((char*)li_v0_ee.li_product_serial,
              (char*)eeprom.li_product_serial,
               sizeof li_v0_ee.li_product_serial);

    copy_data((char*)li_v0_ee.li_odm_pcba_serial,
              (char*)eeprom.li_odm_pcba_serial,
              sizeof li_v0_ee.li_odm_pcba_serial);

    /* loacl MAC */
    copy_data((char*)li_v0_ee.li_local_mac,
              (char*)eeprom.li_local_mac,
               sizeof(eeprom.li_local_mac));

   /* Extended MAC Address */
     copy_data((char*)li_v0_ee.li_mac_base,
               (char*)eeprom.li_mac_base,
                sizeof(eeprom.li_mac_base));
    break;
  case LI_EEPROM_VERSION1:
    copy_data((char*)li_v1_ee.li_product_serial,
              (char*)eeprom.li_product_serial,
              sizeof li_v1_ee.li_product_serial);
    copy_data((char*)li_v1_ee.li_odm_pcba_serial,
              (char*)eeprom.li_odm_pcba_serial,
              sizeof li_v1_ee.li_odm_pcba_serial);
    copy_data((char*)li_v1_ee.li_local_mac,
              (char*)eeprom.li_local_mac,
              sizeof(eeprom.li_local_mac));
    copy_data((char*)li_v1_ee.li_mac_base,
              (char*)eeprom.li_mac_base,
               sizeof(eeprom.li_mac_base));
    break;
  default:
    syslog(LOG_NOTICE, "Invalid EEPROM version %d", eep_version);
    printf("Invalid EEPROM version %d\n", eep_version);
    goto out;
  break;
  }

  syslog(LOG_NOTICE, "AFTER RESTORING ARCHIVED ITEMS:");
  syslog(LOG_NOTICE, "Product Asset Tag     : %s",
         eeprom.li_product_asset);
  syslog(LOG_NOTICE, "Product Serial Num    : %s",
         eeprom.li_product_serial);
  syslog(LOG_NOTICE, "Manufacturing date    : %s",
         eeprom.li_system_manufacturing_date);
  syslog(LOG_NOTICE, "Fabric Location       : %s",
         eeprom.li_reserved);

  /* calculate based on all bytes but crc */
  /* validate on all bytes, resulting in crc value 0 */
  eeprom.li_crc8 = li_crc8((void *) &eeprom, crc_len - 1) ^ LI_EEPROM_CRC8_CSUM;
  ssize_t sz = sizeof eeprom;
  rc = fwrite(&eeprom, 1, sz, fin);
  if (rc < sz) {
      syslog(LOG_WARNING,
             "Failed to write entire EEPROM (%d bytes).  Wrote %d bytes",
             sz, rc);
      printf("Failed to write entire EEPROM (%d bytes).  Wrote %d bytes\n",
             sz, rc);

      goto out;
  }
  syslog(LOG_NOTICE, "Wrote all %d bytes to EEPROM", rc);
  rc = 0;

out:
  if (fin) {
    fclose(fin);
  }

  if (ftxt) {
    fclose(ftxt);
  }

  return rc;
}
