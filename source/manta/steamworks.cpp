#include <manta/steamworks.hpp>

#include <core/debug.hpp>

#if COMPILE_STEAMWORKS
#include <vendor/steamworks/steam_api.h>

#include <manta/engine.hpp>
#include <manta/profiler.hpp>
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Steamworks::init()
{
#if COMPILE_STEAMWORKS

	#if STEAMWORKS_DISTRIBUTE
		// NOTE: For distribute builds using Steam API, Steam must be running and the application must be
		// launched by Steam. The following function starts the Steam client and relaunches the application.
		if( SteamAPI_RestartAppIfNecessary( static_cast<u32>( STEAMWORKS_APP_ID ) ) ) { Engine::exit( 1 ); }
	#endif

	// Initialize Steam
	SteamErrMsg errorMsg = { 0 };
	if( SteamAPI_InitEx( &errorMsg ) != k_ESteamAPIInitResult_OK )
	{
		DEBUG( ErrorReturnMsg( false, "Steam API initialization failed: %s", errorMsg ) );
		return false;
	}

	// Verify Login
	if( !SteamUser()->BLoggedOn() )
	{
		DEBUG( ErrorReturnMsg( false, "Steam API: User is not logged in!" ) );
		return false;
	}

	// Verify Ownership
	if( !SteamApps()->BIsSubscribedApp( SteamUtils()->GetAppID() ) )
	{
		DEBUG( ErrorReturnMsg( false, "Steam API: User does not own the AppID!" ) );
		return false;
	}

	// Steam Overlay Position
	SteamUtils()->SetOverlayNotificationPosition( k_EPositionTopRight );

#endif

	return true;
}


bool Steamworks::free()
{
#if COMPILE_STEAMWORKS
	SteamAPI_Shutdown();
#endif

	return true;
}


void Steamworks::callbacks()
{
#if COMPILE_STEAMWORKS
	PROFILE_SCOPE( "Steamworks Callbacks" )
	SteamAPI_RunCallbacks();
#endif
};


u64 Steamworks::get_userid_u64()
{
#if COMPILE_STEAMWORKS
	return static_cast<u64>( SteamUser()->GetSteamID().ConvertToUint64() );
#else
	return 0LLU;
#endif
};


bool Steamworks::get_username( char *buffer, usize size )
{
#if COMPILE_STEAMWORKS
	const char *persona = SteamFriends()->GetPersonaName();
	if( !persona ) { buffer[0] = '\0'; return false; }
	strncpy( buffer, persona, size - 1 );
	buffer[size - 1] = '\0';
	return true;
#else
	buffer[0] = '\0';
	return false;
#endif
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////