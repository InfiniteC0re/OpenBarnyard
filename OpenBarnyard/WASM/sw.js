// Keeps the page's own files for offline starts, and adds the headers threads need when the server can't send them.
// The game data (game/) isn't kept here, a stored data pack (datapack.js) is used offline.
var aFiles = [
	"./", "index.html", "page.js", "gl_replay.js", "input.js", "settings.js", "datapack.js", "datapack_worker.js",
	"audio_worklet.js", "OpenBarnyard.js", "OpenBarnyard.wasm", "manifest.json", "icon.png", "icon512.png", "favicon.png"
];

var szCache = "barnyard-" + self.registration.scope;

self.addEventListener( "install", function ( a_oEvent ) {
	self.skipWaiting();

	// Some files may be missing on older builds, each one is fetched on its own
	a_oEvent.waitUntil( caches.open( szCache ).then( function ( a_oCache ) {
		return Promise.all( aFiles.map( function ( a_szFile ) {
			return a_oCache.add( a_szFile ).catch( function () {} );
		} ) );
	} ) );
} );

self.addEventListener( "activate", function ( a_oEvent ) {
	a_oEvent.waitUntil( self.clients.claim() );
} );

function AddHeaders( a_oResponse )
{
	if ( a_oResponse.status == 0 || a_oResponse.headers.get( "Cross-Origin-Embedder-Policy" ) )
		return a_oResponse;

	var oHeaders = new Headers( a_oResponse.headers );
	oHeaders.set( "Cross-Origin-Opener-Policy", "same-origin" );
	oHeaders.set( "Cross-Origin-Embedder-Policy", "require-corp" );
	oHeaders.set( "Cross-Origin-Resource-Policy", "same-origin" );

	return new Response( a_oResponse.body, { status : a_oResponse.status, statusText : a_oResponse.statusText, headers : oHeaders } );
}

self.addEventListener( "fetch", function ( a_oEvent ) {
	var oRequest = a_oEvent.request;

	if ( oRequest.cache == "only-if-cached" && oRequest.mode != "same-origin" )
		return;

	var oUrl = new URL( oRequest.url );

	// Game data goes straight to the server
	if ( oRequest.method != "GET" || oUrl.origin != location.origin || oUrl.pathname.indexOf( "/game/" ) >= 0 )
		return;

	a_oEvent.respondWith( Respond( oRequest ) );
} );

// The last page was opened from the kept copy: its files come from it too, without waiting on the network
var bOfflinePage = false;

// The kept copy of a file. The page itself is kept as ./ and index.html, any query (?app, ?log) gets it too
async function FromCache( a_oRequest )
{
	var oCache  = await caches.open( szCache );
	var oCached = await oCache.match( a_oRequest, { ignoreSearch : true, ignoreVary : true } );

	if ( !oCached && a_oRequest.mode == "navigate" )
		oCached = await oCache.match( new URL( "./", self.registration.scope ).href, { ignoreVary : true } ) || await oCache.match( new URL( "index.html", self.registration.scope ).href, { ignoreVary : true } );

	return oCached ? AddHeaders( oCached ) : null;
}

function Timeout( a_iMilliseconds )
{
	return new Promise( function ( a_fnResolve, a_fnReject ) {
		setTimeout( function () { a_fnReject( new Error( "timeout" ) ); }, a_iMilliseconds );
	} );
}

// The newest files when online, the kept ones when not. iOS can wait a long time on a request
// it can't send instead of failing it, so nothing waits on the network forever when a copy is kept
async function Respond( a_oRequest )
{
	var bNavigate = a_oRequest.mode == "navigate";

	if ( !self.navigator.onLine || ( !bNavigate && bOfflinePage ) )
	{
		var oOffline = await FromCache( a_oRequest );

		if ( oOffline )
		{
			if ( bNavigate )
				bOfflinePage = true;

			return oOffline;
		}
	}

	var oNetwork = fetch( a_oRequest ).then( function ( a_oResponse ) {
		if ( a_oResponse.ok && a_oResponse.status == 200 )
		{
			var oCopy = a_oResponse.clone();
			caches.open( szCache ).then( function ( a_oCache ) {
				a_oCache.put( a_oRequest, oCopy );
			} );
		}

		return AddHeaders( a_oResponse );
	} );

	try
	{
		// The page decides for its files: 4 seconds for it, then a long wait for the big files of a slow connection
		var oResponse = await Promise.race( [ oNetwork, Timeout( bNavigate ? 4000 : 30000 ) ] );

		if ( bNavigate )
			bOfflinePage = false;

		return oResponse;
	}
	catch ( e )
	{
		var oCached = await FromCache( a_oRequest );

		if ( oCached )
		{
			if ( bNavigate )
				bOfflinePage = true;

			return oCached;
		}

		// A slow network is still better than nothing
		return oNetwork.catch( function () { return Response.error(); } );
	}
}
