/* compile with:
 *
 *      gcc -g -Wall composite.c `pkg-config vips --cflags --libs`
 */

#include <stdio.h>
#include <vips/vips.h>

/* Composite images `src` and `dst`.
 */
int Composite(VipsObject *context, VipsImage *src, VipsImage *dst,
  VipsImage **out) {
  const int ALPHA_BAND_INDEX = 3;
  const int NUM_COLOR_BANDS = 3;

  // These images will all be unreffed when context is unreffed:
  VipsImage **t = (VipsImage **) vips_object_local_array(context, 3);

  // Extract RGB bands:
  VipsImage *srcRGB;
  VipsImage *dstRGB;
  if (vips_extract_band(src, &srcRGB, 0, "n", NUM_COLOR_BANDS, NULL) ||
      vips_extract_band(dst, &dstRGB, 0, "n", NUM_COLOR_BANDS, NULL))
    return -1;

  vips_object_local(context, srcRGB);
  vips_object_local(context, dstRGB);

  // Extract alpha bands:
  VipsImage *srcAlpha;
  VipsImage *dstAlpha;
  if (vips_extract_band(src, &srcAlpha, ALPHA_BAND_INDEX, NULL) ||
      vips_extract_band(dst, &dstAlpha, ALPHA_BAND_INDEX, NULL))
    return -1;

  vips_object_local(context, srcAlpha);
  vips_object_local(context, dstAlpha);

  // Compute normalized input alpha channels:
  VipsImage *srcAlphaNormalized;
  VipsImage *dstAlphaNormalized;
  if (vips_linear1(srcAlpha, &srcAlphaNormalized, 1.0 / 255.0, 0.0, NULL) ||
      vips_linear1(dstAlpha, &dstAlphaNormalized, 1.0 / 255.0, 0.0, NULL))
    return -1;

  vips_object_local(context, srcAlphaNormalized);
  vips_object_local(context, dstAlphaNormalized);

  //
  // Compute normalized output alpha channel:
  //
  // References:
  // - http://en.wikipedia.org/wiki/Alpha_compositing#Alpha_blending
  // - https://github.com/jcupitt/ruby-vips/issues/28#issuecomment-9014826
  //
  // out_a = src_a + dst_a * (1 - src_a)
  //                         ^^^^^^^^^^^
  //                            t[0]
  //                 ^^^^^^^^^^^^^^^^^^^
  //                        t[1]
  VipsImage *outAlphaNormalized;
  if (vips_linear1(srcAlphaNormalized, &t[0], -1.0, 1.0, NULL) ||
      vips_multiply(dstAlphaNormalized, t[0], &t[1], NULL) ||
      vips_add(srcAlphaNormalized, t[1], &outAlphaNormalized, NULL))
    return -1;

  vips_object_local(context, outAlphaNormalized);

  //
  // Compute output RGB channels:
  //
  // Wikipedia:
  // out_rgb = (src_rgb * src_a + dst_rgb * dst_a * (1 - src_a)) / out_a
  //
  // `vips_ifthenelse` with `blend=TRUE`: http://bit.ly/1KoSsga
  // out = (cond / 255) * in1 + (1 - cond / 255) * in2
  //
  // Substitutions:
  //
  //     cond --> src_a
  //     in1 --> src_rgb
  //     in2 --> dst_rgb * dst_a (premultiplied destination RGB)
  //
  // Finally, manually divide by `out_a` to unpremultiply the RGB channels.
  // Failing to do so results in darker than expected output with low
  // opacity images.
  //
  VipsImage *dstRGBPremultiplied;
  if (vips_multiply(dstRGB, dstAlphaNormalized, &dstRGBPremultiplied, NULL))
    return -1;

  vips_object_local(context, dstRGBPremultiplied);

  VipsImage *outRGBPremultiplied;
  if (vips_ifthenelse(srcAlpha, srcRGB, dstRGBPremultiplied,
      &outRGBPremultiplied, "blend", TRUE, NULL))
    return -1;

  vips_object_local(context, outRGBPremultiplied);

  // Unpremultiply RGB channels:
  VipsImage *outRGB;
  if (vips_divide(outRGBPremultiplied, outAlphaNormalized, &outRGB, NULL))
    return -1;

  vips_object_local(context, outRGB);

  // Denormalize output alpha channel:
  VipsImage *outAlpha;
  if (vips_linear1(outAlphaNormalized, &outAlpha, 255.0, 0.0, NULL))
    return -1;

  vips_object_local(context, outAlpha);

  // Combine RGB and alpha channel into output image:
  if (vips_bandjoin2(outRGB, outAlpha, &t[2], NULL))
    return -1;

  // Return a reference to the output image:
  g_object_ref(t[2]);
  *out = t[2];

  return 0;
}

int main(int argc, char **argv) {
  GOptionContext *context;
  GOptionGroup *main_group;
  GError *error = NULL;

  // Main
  char* programName = argv[0];
  char* srcPath = argv[1];
  char* dstPath = argv[2];
  char* outPath = argv[3];

  if (VIPS_INIT(programName))
    vips_error_exit(NULL);

  context = g_option_context_new("src dst out");

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
    vips_error_exit("usage: %s src dst out", programName);

  // Load input images:
  VipsImage *srcInput;
  VipsImage *dstInput;
  if (!(srcInput = vips_image_new_from_file(srcPath, NULL)) ||
      !(dstInput = vips_image_new_from_file(dstPath, NULL)))
    vips_error_exit(NULL);

  // the controlling object for this processing: all temp objects get unreffed
  // when this is unreffed
  VipsObject *handle = VIPS_OBJECT(vips_image_new());

  VipsImage *out;
  if (Composite(handle, srcInput, dstInput, &out)) {
    g_object_unref(handle);
    vips_error_exit(NULL);
  }
  g_object_unref(handle);

  // Output
  if (vips_image_write_to_file(out, outPath, NULL))
    vips_error_exit(NULL);

  g_object_unref(out);
  g_object_unref(srcInput);
  g_object_unref(dstInput);

  return 0;
}
