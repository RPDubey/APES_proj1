
/*******************************************************************************
   @Filename:tests.c
   @Brief:declarations for cmocka tests
   @Author:Ravi Dubey
   @Date:3/14/2018
 ******************************************************************************/

#include "/home/ravi/cmocka/include/cmocka.h"
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../sensors/adps9301Sensor.h"
#include "../sensors/tmp102Sensor.h"
#include "../socket_client/client_test.h"

void positive_temp_conv_test(void **state) {
  char buffer[2];

  // mock data
  buffer[0] = 0x19;
  buffer[1] = 0x00;

  float expected = 25.0;
  printf("Testing C,F,K conversion\n");
  float result = temperatureConv(CELCIUS, buffer);
  assert_int_equal((int)expected, (int)result);

  expected = 77.0;
  result = temperatureConv(FARENHEIT, buffer);
  assert_int_equal((int)expected, (int)result);

  expected = 298.15;
  result = temperatureConv(KELVIN, buffer);
  assert_int_equal((int)expected, (int)result);
}

void negative_temp_conv_test(void **state) {
  char buffer[2];

  // mock data
  buffer[0] = 0xE7;
  buffer[1] = 0x01;

  float expected = -25.0;
  printf("Testing C,F,K conversion\n");
  float result = temperatureConv(CELCIUS, buffer);
  assert_int_equal((int)expected, (int)result);

  expected = -13.0;
  result = temperatureConv(FARENHEIT, buffer);
  assert_int_equal((int)expected, (int)result);

  expected = 248.15;
  result = temperatureConv(KELVIN, buffer);
  assert_int_equal((int)expected, (int)result);
}

void light_conv_test(void **state) {
  uint16_t adc_data_ch0, adc_data_ch1;

  // mock data
  adc_data_ch0 = 9669;
  adc_data_ch1 = 973;

  float expected = 269;
  float result = reportLumen(adc_data_ch0, adc_data_ch1);
  assert_int_equal((int)expected, (int)result);
}

void socket_task_test(void **state) {
  //  int expected = 1;
  char buffer[4] = "Test";
  test_client_data(buffer, 1);
  assert_string_equal("TEMP", buffer);
}

int main(void) {
  const struct CMUnitTest tests[] = {cmocka_unit_test(positive_temp_conv_test),
                                     cmocka_unit_test(negative_temp_conv_test),
                                     cmocka_unit_test(light_conv_test),
                                     cmocka_unit_test(socket_task_test)

  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}
