/*
 * Compile with:
 *
 *      gcc -g -Wall resize-alpha.c `pkg-config vips --cflags --libs` -o resize-alpha
 *
 * Run with:
 *
 *      ./resize-alpha in out interpolation
 *
 * Examples:
 *
 *      ./resize-alpha input.png output-bilinear.png 'bilinear'
 *      ./resize-alpha input.png output-bicubic.png 'bicubic'
 */

#include <stdio.h>
#include <vips/vips.h>

/*
  Resize `image`.
 */
int Resize(VipsObject *context, VipsImage *image, VipsImage **out,
    double scale, char* interpolation, int premultiply) {
  if (image->Bands != 4)
    return -1;

  // Create interpolator:
  VipsInterpolate *interpolator = vips_interpolate_new(interpolation);
  if (interpolator == NULL) {
    return -1;
  }
  vips_object_local(context, interpolator);

  if (!premultiply) {
    VipsImage *affined;
    if (vips_affine(image, &affined, scale, 0.0, 0.0, scale, "interpolate", interpolator, NULL)) {
      return -1;
    }
    *out = affined;
    return 0;
  }

  // Premultiply image before transformation:
  VipsImage *imageRGB;
  VipsImage *imageAlpha;
  VipsImage *imageAlphaNormalized;
  VipsImage *imageRGBPremultiplied;
  VipsImage *imagePremultiplied;
  if (vips_extract_band(image, &imageRGB, 0, "n", 3, NULL) ||
      vips_extract_band(image, &imageAlpha, 3, "n", 1, NULL) ||
      vips_linear1(imageAlpha, &imageAlphaNormalized, 1.0 / 255.0, 0.0, NULL) ||
      vips_multiply(imageRGB, imageAlphaNormalized, &imageRGBPremultiplied, NULL) ||
      vips_bandjoin2(imageRGBPremultiplied, imageAlpha, &imagePremultiplied, NULL)) {
     return -1;
  }
  vips_object_local(context, imageRGB);
  vips_object_local(context, imageAlpha);
  vips_object_local(context, imageAlphaNormalized);
  vips_object_local(context, imageRGBPremultiplied);
  vips_object_local(context, imagePremultiplied);

  // Perform affine transformation:
  VipsImage *imagePremultipliedTransformed;
  if (vips_affine(imagePremultiplied, &imagePremultipliedTransformed,
      scale, 0.0, 0.0, scale, "interpolate", interpolator, NULL)) {
    return -1;
  }
  vips_object_local(context, imagePremultipliedTransformed);

  // Unpremultiply image after transformation:
  VipsImage *imageRGBPremultipliedTransformed;
  VipsImage *imageAlphaTransformed;
  VipsImage *imageAlphaNormalizedTransformed;
  VipsImage *imageRGBUnpremultipliedTransformed;
  VipsImage *imageUnpremultiplied;
  if (vips_extract_band(imagePremultipliedTransformed, &imageRGBPremultipliedTransformed, 0, "n", 3, NULL) ||
      vips_extract_band(imagePremultipliedTransformed, &imageAlphaTransformed, 3, "n", 1, NULL) ||
      vips_linear1(imageAlphaTransformed, &imageAlphaNormalizedTransformed, 1.0 / 255.0, 0.0, NULL) ||
      vips_divide(imageRGBPremultipliedTransformed, imageAlphaNormalizedTransformed, &imageRGBUnpremultipliedTransformed, NULL) ||
      vips_bandjoin2(imageRGBUnpremultipliedTransformed, imageAlphaTransformed, &imageUnpremultiplied, NULL)) {
     return -1;
  }
  vips_object_local(context, imageRGBPremultipliedTransformed);
  vips_object_local(context, imageAlphaTransformed);
  vips_object_local(context, imageAlphaNormalizedTransformed);
  vips_object_local(context, imageRGBUnpremultipliedTransformed);
  // vips_object_local(context, imageUnpremultiplied);

  // Return a reference to the output image:
  *out = imageUnpremultiplied;

  return 0;
}


int main(int argc, char **argv) {
  GOptionContext *context;
  GOptionGroup *main_group;
  GError *error = NULL;

  // Main
  char* programName = argv[0];
  char* inPath = argv[1];
  char* outPath = argv[2];
  char* interpolation = argv[3];

  if (VIPS_INIT(programName))
    vips_error_exit(NULL);

  context = g_option_context_new("in out interpolation");

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

  if (argc != 4)
    vips_error_exit("usage: %s in out interpolation", programName);

  // Load input images:
  VipsImage *in;
  if (!(in = vips_image_new_from_file(inPath, NULL)))
    vips_error_exit(NULL);

  // the controlling object for this processing: all temp objects get unreffed
  // when this is unreffed
  VipsObject *handle = VIPS_OBJECT(vips_image_new());

  VipsImage *out;
  double scale = 2.0;
  int premultiply = TRUE;
  if (Resize(handle, in, &out, scale, interpolation, premultiply)) {
    g_object_unref(handle);
    vips_error_exit(NULL);
  }
  g_object_unref(handle);

  // Output
  if (vips_image_write_to_file(out, outPath, NULL))
    vips_error_exit(NULL);

  g_object_unref(out);
  g_object_unref(in);

  return 0;
}
