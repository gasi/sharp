#include <stdio.h>
#include <vips/vips.h>

#include "composite.h"

const int STATS_SUM2_COLUMN = 3;

/*
  Compare images `actual` and `expected` and return mean squared error (MSE).
 */
int Compare(VipsObject *context, VipsImage *actual, VipsImage *expected, double *out) {
  if (actual->Bands != expected->Bands)
    return -1;

  if (actual->Type != expected->Type)
    return -1;

  VipsImage *actualPremultiplied;
  VipsImage *expectedPremultiplied;
  if (Premultiply(context, actual, &actualPremultiplied) ||
      Premultiply(context, expected, &expectedPremultiplied))
    return -1;

  vips_object_local(context, actualPremultiplied);
  vips_object_local(context, expectedPremultiplied);

  VipsImage *difference;
  VipsImage *stats;
  if (vips_subtract(expectedPremultiplied, actualPremultiplied, &difference, NULL) ||
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
