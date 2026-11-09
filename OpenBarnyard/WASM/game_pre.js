// Runs on the page and on every worker of the game
if ( typeof Module === "undefined" ) var Module = {};

// ?log: the workers send their console to the page, phones have no other way to show it
if ( typeof importScripts === "function" && location.search.indexOf( "log" ) >= 0 )
{
	for ( const szName of [ "log", "warn", "error" ] )
	{
		const fnOriginal = console[ szName ];

		console[ szName ] = function ()
		{
			postMessage( { szPageLog : Array.prototype.join.call( arguments, " " ) } );
			fnOriginal.apply( console, arguments );
		};
	}
}

// The stored data pack (datapack.js) comes to each thread before the runtime loads
if ( typeof importScripts === "function" )
{
	self.addEventListener( "message", function ( a_oEvent ) {
		var oPack = a_oEvent.data && a_oEvent.data.oDataPack;

		if ( !oPack )
			return;

		var oOffsets = new Map();
		var aIndex   = [];

		for ( const szLine of oPack.szIndex.split( "\n" ) )
		{
			const aParts = szLine.split( "\t" );

			if ( aParts.length < 4 )
				continue;

			aIndex.push( aParts[ 0 ] + "\t" + aParts[ 1 ] + "\t" + aParts[ 3 ] );

			if ( aParts[ 0 ] == "f" )
				oOffsets.set( aParts[ 3 ], +aParts[ 2 ] );
		}

		Module.oDataPack = { oFile : oPack.oFile, szIndex : aIndex.join( "\n" ), oOffsets : oOffsets, oReader : new FileReaderSync() };
	} );
}

// A game script loaded from a blob (page.js) gets the data address from the page, a blob has no address of its own
if ( typeof g_szDataUrl !== "undefined" )
	Module.szDataUrl = g_szDataUrl;
else
	Module.szDataUrl = ( typeof location !== "undefined" ? new URLSearchParams( location.search ).get( "data" ) : null ) || "game/";
