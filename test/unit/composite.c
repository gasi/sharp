/* compile with:
 *
 *      gcc -g -Wall composite.c `pkg-config vips --cflags --libs`
 */

#include <stdio.h>
#include <vips/vips.h>

int
main( int argc, char **argv )
{
  GOptionContext *context;
  GOptionGroup *main_group;
  GError *error = NULL;

  VipsImage *srcInput;
  VipsImage *srcRGB;
  VipsImage *srcAlpha;
  VipsImage *srcAlphaNormalized;

  VipsImage *dstInput;
  VipsImage *dstRGB;
  VipsImage *dstRGBPremultiplied;
  VipsImage *dstAlpha;
  VipsImage *dstAlphaNormalized;

  VipsImage *out;
  VipsImage *outRGB;
  VipsImage *outRGBPremultiplied;
  VipsImage *outAlpha;
  VipsImage *outAlphaNormalized;

  const int ALPHA_BAND_INDEX = 3;
  const int NUM_COLOR_BANDS = 3;

  // Main
  char* programName = argv[0];
  char* srcPath = argv[1];
  char* dstPath = argv[2];
  char* outPath = argv[3];

  if( VIPS_INIT( programName ) )
    vips_error_exit( NULL );

  context = g_option_context_new( "src dst out" );

  main_group = g_option_group_new( NULL, NULL, NULL, NULL, NULL );
  g_option_context_set_main_group( context, main_group );
  // // TODO: Figure out how to compile this line without error:
  // vips_add_option_entries( main_group );

  if( !g_option_context_parse( context, &argc, &argv, &error ) ) {
    if( error ) {
      fprintf( stderr, "%s\n", error->message );
      g_error_free( error );
    }

    vips_error_exit( NULL );
  }

  if( argc != 4 )
    vips_error_exit( "usage: %s src dst out", programName );

  // Load input images:
  if( !(srcInput = vips_image_new_from_file( srcPath, NULL )) )
    vips_error_exit( NULL );

  if( !(dstInput = vips_image_new_from_file( dstPath, NULL )) )
    vips_error_exit( NULL );

  // Extract RGB bands for source image
  if( vips_extract_band( srcInput, &srcRGB, 0, "n", NUM_COLOR_BANDS, NULL ) )
    vips_error_exit( NULL );

  // Extract RGB bands for destination image
  if( vips_extract_band( dstInput, &dstRGB, 0, "n", NUM_COLOR_BANDS, NULL ) )
    vips_error_exit( NULL );

  // Extract alpha band for source image
  if( vips_extract_band( srcInput, &srcAlpha, ALPHA_BAND_INDEX, NULL ) )
    vips_error_exit( NULL );

  // Extract alpha band for destination image
  if( vips_extract_band( dstInput, &dstAlpha, ALPHA_BAND_INDEX, NULL ) )
    vips_error_exit( NULL );

  // Compute normalized input alpha channels:
  if( vips_linear1( srcAlpha, &srcAlphaNormalized, 1.0 / 255.0, 0.0, NULL ) )
    vips_error_exit( NULL );

  if( vips_linear1( dstAlpha, &dstAlphaNormalized, 1.0 / 255.0, 0.0, NULL ) )
    vips_error_exit( NULL );

  //
  // Compute normalized output alpha channel:
  //
  // References:
  // - http://en.wikipedia.org/wiki/Alpha_compositing#Alpha_blending
  // - https://github.com/jcupitt/ruby-vips/issues/28#issuecomment-9014826
  //
  // out_a = src_a + dst_a * (1 - src_a)
  //                         ^^^^^^^^^^^
  //                             t1
  //                 ^^^^^^^^^^^^^^^^^^^
  //                         t2
  VipsImage *t1;
  VipsImage *t2;
  if( vips_linear1( srcAlphaNormalized, &t1, -1.0, 1.0, NULL ) ||
      vips_multiply( dstAlphaNormalized, t1, &t2, NULL ) ||
      vips_add( srcAlphaNormalized, t2, &outAlphaNormalized, NULL ) )
    vips_error_exit( NULL );

  g_object_unref( t1 );
  g_object_unref( t2 );

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
  if( vips_multiply( dstRGB, dstAlphaNormalized, &dstRGBPremultiplied, NULL ) )
    vips_error_exit( NULL );

  if( vips_ifthenelse( srcAlpha, srcRGB, dstRGBPremultiplied,
      &outRGBPremultiplied, "blend", TRUE, NULL ) )
    vips_error_exit( NULL );

  // Unpremultiply RGB channels:
  if( vips_divide( outRGBPremultiplied, outAlphaNormalized, &outRGB, NULL ) )
    vips_error_exit( NULL );

  if( vips_linear1( outAlphaNormalized, &outAlpha, 255.0, 0.0, NULL ) )
    vips_error_exit( NULL );

  if( vips_bandjoin2( outRGB, outAlpha, &out, NULL ) )
    vips_error_exit( NULL );

  // Source
  g_object_unref( srcInput );
  g_object_unref( srcRGB );
  g_object_unref( srcAlpha );
  g_object_unref( srcAlphaNormalized );

  // Destination
  g_object_unref( dstInput );
  g_object_unref( dstRGB );
  g_object_unref( dstRGBPremultiplied );
  g_object_unref( dstAlpha );
  g_object_unref( dstAlphaNormalized );

  // Output
  if( vips_image_write_to_file( out, outPath, NULL ) )
    vips_error_exit( NULL );

  // Output
  g_object_unref( out );
  g_object_unref( outRGB );
  g_object_unref( outRGBPremultiplied );
  g_object_unref( outAlpha );
  g_object_unref( outAlphaNormalized );

  return( 0 );
}
