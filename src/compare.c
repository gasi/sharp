/*
 * Compile with:
 *
 *      gcc -g -Wall compare.c `pkg-config vips --cflags --libs` -o compare
 *
 * Run with:
 *
 *      ./compare actual expected
 *
 * Examples:
 *
 *      ./compare actual.png expected.png
 */

#include <stdio.h>
#include <vips/vips.h>


const int STATS_SUM2_COLUMN = 3;

/*
  Compare images `actual` and `expected` and return mean square error.
 */
int Compare(VipsObject *context, VipsImage *actual, VipsImage *expected, double *out) {
  if (actual->Bands != expected->Bands)
    return -1;

  if (actual->Type != expected->Type)
    return -1;

  VipsImage *difference;
  VipsImage *absolute;
  VipsImage *stats;
  if (vips_subtract(actual, expected, &difference, NULL) ||
      vips_abs(difference, &absolute, NULL) ||
      vips_stats(absolute, &stats, NULL))
    return -1;

  double *statsData = (double*) stats->data;
  vips_object_local(context, difference);
  vips_object_local(context, absolute);
  vips_object_local(context, stats);

  printf("bands: %d\n", stats->Bands);
  printf("width: %d\n", stats->Xsize);
  printf("height: %d\n\n", stats->Ysize);
  printf("min: %f\n", statsData[0]);
  printf("max: %f\n", statsData[1]);
  printf("sum: %f\n", statsData[2]);
  printf("sum2: %f\n", statsData[3]);
  printf("avg: %f\n", statsData[4]);
  printf("sd: %f\n", statsData[5]);
  printf("xmin: %f\n", statsData[6]);
  printf("ymin: %f\n", statsData[7]);
  printf("xmax: %f\n", statsData[8]);
  printf("ymax: %f\n\n", statsData[9]);

  int numValues = actual->Xsize * actual->Ysize * actual->Bands;
  double sumOfSquares = statsData[STATS_SUM2_COLUMN];
  double meanSquaredError = sumOfSquares / numValues;

  // Return a reference to the mean squared error:
  *out = meanSquaredError;

  return 0;
}


int main(int argc, char **argv) {
  GOptionContext *context;
  GOptionGroup *main_group;
  GError *error = NULL;

  // Main
  char* programName = argv[0];
  char* actualPath = argv[1];
  char* expectedPath = argv[2];

  if (VIPS_INIT(programName))
    vips_error_exit(NULL);

  context = g_option_context_new("actual expected");

  main_group = g_option_group_new(NULL, NULL, NULL, NULL, NULL);
  g_option_context_set_main_group(context, main_group);
  // // TODO: Figure out how to compile this line without error:
  // vips_add_option_entries(main_group);

  if (!g_option_context_parse(context, &argc, &argv, &error)) {
    if (error) {
      fprintf(stderr, "%s\n", error->message);
      g_error_free(error);
    }

    vips_error_exit(NULL);
  }

  if (argc != 3)
    vips_error_exit("usage: %s actual expected", programName);

  // Load input images:
  VipsImage *actualInput;
  VipsImage *expectedInput;
  if (!(actualInput = vips_image_new_from_file(actualPath, NULL)) ||
      !(expectedInput = vips_image_new_from_file(expectedPath, NULL)))
    vips_error_exit(NULL);

  // the controlling object for this processing: all temp objects get unreffed
  // when this is unreffed
  VipsObject *handle = VIPS_OBJECT(vips_image_new());

  double out;
  if (Compare(handle, actualInput, expectedInput, &out)) {
    g_object_unref(handle);
    vips_error_exit(NULL);
  }
  g_object_unref(handle);

  printf("%f\n", out);

  g_object_unref(actualInput);
  g_object_unref(expectedInput);

  return 0;
}
