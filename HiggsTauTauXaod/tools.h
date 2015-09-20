#include "EventLoop/StatusCode.h"

// XAOD ROOT ACCESS
#include "xAODRootAccess/tools/ReturnCheck.h"
#include "xAODRootAccess/tools/Message.h"

/// Helper macro for checking xAOD::TReturnCode return values
#define EL_RETURN_CHECK( CONTEXT, EXP )                     \
  do {                                                     \
  if( ! EXP.isSuccess() ) {                             \
  Error( CONTEXT,                                    \
    XAOD_MESSAGE( "Failed to execute: %s" ),    \
	 #EXP );                                     \
  return EL::StatusCode::FAILURE;                    \
  }                                                     \
  } while( false )

