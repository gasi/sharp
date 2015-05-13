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
  VipsImage *stats;
  if (vips_subtract(expected, actual, &difference, NULL) ||
      vips_stats(difference, &stats, NULL))
    return -1;

  vips_object_local(context, difference);
  vips_object_local(context, stats);

  double *statsData = (double*) stats->data;
  int numValues = actual->Xsize * actual->Ysize * actual->Bands;
  double sumOfSquares = statsData[STATS_SUM2_COLUMN];
  double meanSquaredError = sumOfSquares / numValues;

  // Return a reference to the mean squared error:
  *out = meanSquaredError;

  return 0;
}
