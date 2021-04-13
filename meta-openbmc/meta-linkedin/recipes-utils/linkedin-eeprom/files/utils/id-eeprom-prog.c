/*
 * Copyright 2018-present Linkedin. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <facebook/linkedin_eeprom.h>

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <stdlib.h>
#include <arpa/inet.h>

#define LI_ID_EEPROM_TXT   "/usr/local/bin/bmc-id-eeprom.txt"

int main (int argc, char **argv)
{
    int        rc = 0;
    const char *fn = LI_EEPROM_FILE;
    char       buf[LI_EEPROM_SIZE];
    struct     li_eeprom_gn eeprom;
    FILE       *ftxt, *fin;

    memset(&eeprom, 0, sizeof(struct li_eeprom_gn));

    ftxt = fopen(LI_ID_EEPROM_TXT, "r");
    if (ftxt == NULL) {
        rc = errno;
        fprintf(stderr, "Failed to open %s\n", argv[1]);
        goto out;
    }

    fin = fopen(fn, "r+");
    if (fin == NULL) {
        rc = errno;
        fprintf(stderr, "Failed to open %s\n", LI_EEPROM_FILE);
        goto out;
    }

    rc = fread(buf, 1, sizeof buf, fin);
    if (rc < sizeof buf) {
        fprintf(stderr, "Failed to complete read.  Only got %d bytes\n", rc);
        rc = ENOSPC;
        goto out;
    }
    memcpy ((char*)&eeprom, (char*)buf + LI_EEPROM_F_MAGIC, sizeof(struct li_eeprom_gn));

    /* go back to the beginning of the file */
    rewind(fin);

    copy_data (LI_EEPROM_MAGIC_NUM, buf, LI_EEPROM_F_MAGIC);
    syslog(LOG_NOTICE, "Writing MAGIC: %c%c%c%c", buf[0], buf[1],
           buf[2], buf[3]);

    rc = fwrite(buf, 1, LI_EEPROM_F_MAGIC, fin);
    if (rc < LI_EEPROM_F_MAGIC) {
        fprintf(stderr, "Failed to write magic number");
        goto out;
    }

    rc = li_fill_struct_from_file (ftxt, &eeprom);
    if (rc) {
        goto out;
    } else {
        uint8_t crc_len = LI_EEPROM_V1_SIZE;

        /* calculate based on all bytes but crc */
        /* validate on all bytes, resulting in crc value 0 */
        eeprom.li_crc8 = li_crc8((void *) &eeprom, crc_len - 1) ^ LI_EEPROM_CRC8_CSUM;

        /* DBG ONLY */
        syslog(LOG_NOTICE, "li_eeprom_gn sz : %d bytes",
               sizeof (struct li_eeprom_gn));
        syslog(LOG_NOTICE, "crc_len            : %d", crc_len);
        syslog(LOG_NOTICE, "EEPROM CRC8        : %X", eeprom.li_crc8);

        ssize_t sz = sizeof eeprom;
        rc = fwrite(&eeprom, 1, sz, fin);
        if (rc < sz) {
            fprintf(stderr, "Failed to write entire EEPROM (%d bytes).  Wrote %d bytes",
                    sz, rc);
            syslog(LOG_WARNING, "Failed to write entire EEPROM (%d bytes).  Wrote %d bytes",
                    sz, rc);

            goto out;
        }
        syslog(LOG_NOTICE, "Wrote all %d bytes to EEPROM", rc);
        rc = 0;
    }

    out:
    if (fin) {
        fclose(fin);
    }

    if (ftxt) {
        fclose(ftxt);
    }

    return rc;
}
