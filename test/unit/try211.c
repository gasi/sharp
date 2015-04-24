/* compile with:
 *
 *      gcc -g -Wall try211.c `pkg-config vips --cflags --libs`
 */

#include <stdio.h>
#include <vips/vips.h>

int
main( int argc, char **argv )
{
  GOptionContext *context;
  GOptionGroup *main_group;
  GError *error = NULL;
  VipsImage *in;
  double mean;
  VipsImage *out;

  if( VIPS_INIT( argv[0] ) )
    vips_error_exit( NULL );

  context = g_option_context_new( "hello infile outfile - VIPS demo" );

  main_group = g_option_group_new( NULL, NULL, NULL, NULL, NULL );
  g_option_context_set_main_group( context, main_group );
  vips_add_option_entries( main_group );

  if( !g_option_context_parse( context, &argc, &argv, &error ) ) {
    if( error ) {
      fprintf( stderr, "%s\n", error->message );
      g_error_free( error );
    }

    vips_error_exit( NULL );
  }

  if( argc != 3 )
    vips_error_exit( "usage: %s infile outfile", argv[0] );

  if( !(in = vips_image_new_from_file( argv[1], NULL )) )
    vips_error_exit( NULL );

  printf( "image width = %d\n", vips_image_get_width( in ) );

  if( vips_avg( in, &mean, NULL ) )
    vips_error_exit( NULL );

  printf( "mean pixel value = %g\n", mean );

  if( vips_invert( in, &out, NULL ) )
    vips_error_exit( NULL );

  g_object_unref( in );

  if( vips_image_write_to_file( out, argv[2], NULL ) )
    vips_error_exit( NULL );

  g_object_unref( out );

  return( 0 );
}
