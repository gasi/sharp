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


/*
  Compare images `actual` and `expected` and return mean square error.
 */
int Compare(VipsObject *context, VipsImage *actual, VipsImage *expected, double *out) {
  if (actual->Bands != expected->Bands)
    return -1;

  if (actual->Type != expected->Type)
    return -1;

  VipsImage *difference;
  VipsImage *stats;
  if (vips_subtract(actual, expected, &difference, NULL) ||
      vips_stats(difference, &stats, NULL))
    return -1;

  vips_object_local(context, difference);

  printf("bands: %d\n", stats->Bands);
  printf("bandformat: %d\n", stats->BandFmt);
  printf("width: %d\n", stats->Xsize);
  printf("height: %d\n", stats->Ysize);
  printf("min: %f\n", (double) stats->data[0]);
  printf("max: %f\n", (double) stats->data[1]);
  printf("sum: %f\n", (double) stats->data[2]);
  printf("sum2: %f\n", (double) stats->data[3]);
  printf("avg: %f\n", (double) stats->data[4]);
  printf("sd: %f\n", (double) stats->data[5]);
  printf("xmin: %f\n", (double) stats->data[6]);
  printf("ymin: %f\n", (double) stats->data[7]);
  printf("xmax: %f\n", (double) stats->data[8]);
  printf("ymax: %f\n", (double) stats->data[9]);

  int numRows = stats->Bands + 1;
  int numColumns = 10;
  for (int row = 0; row < numRows; row++) {
    for (int column = 0; column < numColumns; column++) {
      printf("row %d column %d: %f\n", row, column, (double) stats->data[row * numColumns + column]);
    }
  }

  // Return a reference to the standard deviation:
  *out = (double) stats->data[5];

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

  printf("VIPS standard deviation: %f\n", out);

  g_object_unref(actualInput);
  g_object_unref(expectedInput);

  return 0;
}
