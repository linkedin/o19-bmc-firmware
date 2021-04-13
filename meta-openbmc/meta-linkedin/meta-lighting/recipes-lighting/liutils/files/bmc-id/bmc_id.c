#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include <sys/mman.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <openbmc/kv.h>

/*
static int
bmc_key_check(char *key) {
  int i;

  i = 0;
  while(strcmp(key_list[i], LAST_KEY)) {

    // If Key is valid, return success
    if (!strcmp(key, key_list[i]))
      return 0;

    i++;
  }

  return -1;
}
*/

int
bmc_get_key_value(char *key, char *value) {

  // Check is key is defined and valid
//  if (bmc_key_check(key))
//    return -1;

  return kv_get(key, value);
}

int
bmc_set_key_value(char *key, char *value) {

  // Check is key is defined and valid
//  if (bmc_key_check(key))
//    return -1;

  return kv_set(key, value);
}

int
set_bmc_id(uint8_t id) {
  int i;
  char key[MAX_KEY_LEN] = {0};
  char str[MAX_VALUE_LEN] = {0};

  sprintf(key, "bmc_id");
  if (id == 1)
      return bmc_set_key_value(key, "bmc1");
  else
      return bmc_set_key_value(key, "bmc0");
}

int
get_bmc_id() {
  int ret;
  char key[MAX_KEY_LEN] = {0};
  char str[MAX_VALUE_LEN] = {0};

  sprintf(key, "bmc_id");

  ret = bmc_get_key_value(key, str);
  if (ret) {
    return ret;
  }

  if (!strcmp(str, "bmc1")) {
      return 1;
  }
  else if (!strcmp(str, "bmc0"))
  {
      return 0;
  }

  printf("\nNo id from flash,  default to 0\n");
  return -1;
}

/*
 * get or set the bmc_id
 */
int main(int argc, const char *argv[])
{
  char     *endptr = NULL;
  uint8_t   bmc_id = 0;
  uint8_t   bmc_id_max = 1;

  if ((argc == 1) || ((!strcmp(argv[1], "--get")) && (argc == 2))) {
      bmc_id = get_bmc_id();
  }
  else if (!strcmp(argv[1], "--set") && (argc == 3)) {
      /*
       * set bmc id
       */
      bmc_id = (uint8_t)strtol(argv[2], &endptr, 10);
      if (*endptr != '\0' || bmc_id > bmc_id_max) {
          printf("bmc_id should be 0 or 1\n");
          return -1;
      }

      set_bmc_id(bmc_id);
      bmc_id = get_bmc_id();
  } else {
      return -1;
  }

  if (bmc_id == 0)
      printf("bmc0\n");
  else if (bmc_id == 1) 
      printf("bmc1\n");
  else 
      printf("default - bmc0\n");

  return 0;
}
