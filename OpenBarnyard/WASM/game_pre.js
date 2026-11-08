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

Module.szDataUrl = ( typeof location !== "undefined" ? new URLSearchParams( location.search ).get( "data" ) : null ) || "game/";
