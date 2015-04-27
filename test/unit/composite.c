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

  VipsImage *image1Input;
  VipsImage *image1Colors;
  VipsImage *image1Alpha;
  VipsImage *image1AlphaNormalized;

  VipsImage *image2Input;
  VipsImage *image2Colors;
  VipsImage *image2Alpha;
  VipsImage *image2AlphaNormalized;

  VipsImage *out;
  VipsImage *outRGB;
  VipsImage *outAlpha;
  VipsImage *outAlphaNormalized;

  const int ALPHA_BAND_INDEX = 3;
  const int NUM_COLOR_BANDS = 3;

  if( VIPS_INIT( argv[0] ) )
    vips_error_exit( NULL );

  context = g_option_context_new( "composite in1 in2 out" );

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
    vips_error_exit( "usage: %s in1 in2 out", argv[0] );

  if( !(image1Input = vips_image_new_from_file( argv[1], NULL )) )
    vips_error_exit( NULL );

  if( !(image2Input = vips_image_new_from_file( argv[2], NULL )) )
    vips_error_exit( NULL );

  // Extract RGB bands for image 1
  if( vips_extract_band( image1Input, &image1Colors, 0, "n",
        NUM_COLOR_BANDS, NULL ) )
    vips_error_exit( NULL );

  // Extract RGB bands for image 2
  if( vips_extract_band( image2Input, &image2Colors, 0, "n",
        NUM_COLOR_BANDS, NULL ) )
    vips_error_exit( NULL );

  // Extract alpha band for image 1
  if( vips_extract_band( image1Input, &image1Alpha, ALPHA_BAND_INDEX, NULL ) )
    vips_error_exit( NULL );

  // Extract alpha band for image 2
  if( vips_extract_band( image2Input, &image2Alpha, ALPHA_BAND_INDEX, NULL ) )
    vips_error_exit( NULL );

  printf("Image 1 dimensions: %dx%d\n", vips_image_get_width(image1Input),
    vips_image_get_height(image1Input));
  printf("Image 2 dimensions: %dx%d\n", vips_image_get_width(image2Input),
    vips_image_get_height(image2Input));
  printf("image1Alpha->Bands: %d\n", image1Alpha->Bands);
  printf("image2Alpha->Bands: %d\n", image2Alpha->Bands);
  printf("image1Colors->Bands: %d\n", image1Colors->Bands);
  printf("image2Colors->Bands: %d\n", image2Colors->Bands);

  if( vips_linear1( image1Alpha, &image1AlphaNormalized,
        1.0 / 255.0, 0.0, NULL ) )
    vips_error_exit( NULL );

  if( vips_linear1( image2Alpha, &image2AlphaNormalized,
        1.0 / 255.0, 0.0, NULL ) )
    vips_error_exit( NULL );

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
  if( vips_linear1( image1AlphaNormalized, &t1, -1.0, 1.0, NULL ) ||
      vips_multiply( image2AlphaNormalized, t1, &t2, NULL ) ||
      vips_add( image1AlphaNormalized, t2, &outAlphaNormalized, NULL ) )
    vips_error_exit( NULL );

  // out_rgb = (src_rgb * src_a + dst_rgb * dst_a * (1 - src_a)) / out_a
  //            ^^^^^^^^^^^^^^^
  //                  t3
  //                              ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  //                                            t4
  //            ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  //                                  t5
  VipsImage *t3;
  VipsImage *t4;
  VipsImage *t5;
  if ( vips_multiply( image1Colors, image1AlphaNormalized, &t3, NULL ) ||
       vips_multiply( image2Colors, t2, &t4, NULL ) ||
       vips_add( t3, t4, &t5, NULL ) ||
       vips_divide( t5, outAlphaNormalized, &outRGB, NULL) )
    vips_error_exit( NULL );

  if( vips_linear1( outAlphaNormalized, &outAlpha, 255.0, 0.0, NULL ) )
    vips_error_exit( NULL );

  if( vips_bandjoin2( outRGB, outAlpha, &out, NULL ) )
    vips_error_exit( NULL );

  // Temporary variables
  g_object_unref( t1 );
  g_object_unref( t2 );
  g_object_unref( t3 );
  g_object_unref( t4 );
  g_object_unref( t5 );

  // Image 1
  g_object_unref( image1Input );
  g_object_unref( image1Colors );
  g_object_unref( image1Alpha );
  g_object_unref( image1AlphaNormalized );

  // Image 2
  g_object_unref( image2Input );
  g_object_unref( image2Colors );
  g_object_unref( image2Alpha );
  g_object_unref( image2AlphaNormalized );

  if( vips_image_write_to_file( out, argv[3], NULL ) )
    vips_error_exit( NULL );

  // Output
  g_object_unref( out );
  g_object_unref( outRGB );
  g_object_unref( outAlpha );
  g_object_unref( outAlphaNormalized );

  return( 0 );
}
