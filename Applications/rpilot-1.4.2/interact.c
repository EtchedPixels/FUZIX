// interact.c - handle interactive mode

#include "rpilot.h"

#ifndef NO_INTER

  #include "inter.h"

#endif

void interact()
{
  FILE *f;

  if( (f = fopen("interact.p", "r")) == NULL ) {
    if( (f = fopen(getenv(ENV_VAR), "r")) == NULL ) {
#ifndef NO_INTER
      inter();
#else
      puts( "Can't run interactive mode. RPilot was compiled with NO_INTER." );
      exit( 1 );
#endif
    } else {
      run( getenv(ENV_VAR) );
    }
  } else {
    run( "interact.p" );
  }
}


