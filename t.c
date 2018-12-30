#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <modbus.h>
#include "gh.h"

int main() {
  modbus_t *ctx;
  uint16_t tab_reg[64];
  int rc0,rc100;
  int i;
  
  ctx = modbus_new_tcp("192.168.1.11", 502);
  if (modbus_connect(ctx) == -1) {
    fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
    modbus_free(ctx);
    return -1;
  }
  //////////////////////////////////////////////////////////
  rc0 = modbus_read_registers(ctx, 0, 1, tab_reg);
  if (rc0 == -1) {
    fprintf(stderr, "%s\n", modbus_strerror(errno));
    return -1;
}

  for (i=0; i < rc0; i++) {
    printf("rc0-->reg[%d]=%d (0x%X)\n", i, tab_reg[i], tab_reg[i]);
    printbitssimple(tab_reg[i]);
  }
  ////////////////////////////////////////////////////////////
  
  rc100 = modbus_read_registers(ctx, 100, 1, tab_reg);
  if (rc100 == -1) {
    fprintf(stderr, "%s\n", modbus_strerror(errno));
    return -1;
  }
  
  for (i=0; i < rc100; i++) {
    printf("rc100-->reg[%d]=%d (0x%X) (USCITE)\n", i, tab_reg[i], tab_reg[i]);
    printbitssimple(tab_reg[i]);
  }
  
  modbus_close(ctx);
  modbus_free(ctx);
  return 0;
}
