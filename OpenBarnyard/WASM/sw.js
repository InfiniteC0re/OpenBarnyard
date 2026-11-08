// Threads need these headers and a plain static server can't send them, so they are added here
self.addEventListener( "install", function () {
	self.skipWaiting();
} );

self.addEventListener( "activate", function ( a_oEvent ) {
	a_oEvent.waitUntil( self.clients.claim() );
} );

self.addEventListener( "fetch", function ( a_oEvent ) {
	var oRequest = a_oEvent.request;

	if ( oRequest.cache == "only-if-cached" && oRequest.mode != "same-origin" )
		return;

	a_oEvent.respondWith( fetch( oRequest ).then( function ( a_oResponse ) {
		if ( a_oResponse.status == 0 )
			return a_oResponse;

		var oHeaders = new Headers( a_oResponse.headers );
		oHeaders.set( "Cross-Origin-Opener-Policy", "same-origin" );
		oHeaders.set( "Cross-Origin-Embedder-Policy", "require-corp" );
		oHeaders.set( "Cross-Origin-Resource-Policy", "same-origin" );

		return new Response( a_oResponse.body, { status : a_oResponse.status, statusText : a_oResponse.statusText, headers : oHeaders } );
	} ) );
} );
