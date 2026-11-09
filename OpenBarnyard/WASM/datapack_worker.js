// Stores the game data pack (a .tar or .tar.gz of the Data folder) in the page's private storage and indexes it.
// Messages: { oFile } or { szUrl } to store one, { bDelete } to remove it
onmessage = function ( a_oEvent ) {
	Run( a_oEvent.data ).catch( function ( a_oError ) {
		postMessage( { szError : String( a_oError && a_oError.message || a_oError ) } );
	} );
};

async function Run( a_oMessage )
{
	var oRoot = await navigator.storage.getDirectory();

	// Without the index the pack isn't used, so it goes first
	for ( var szName of [ "data.idx", "data.tar" ] )
	{
		try
		{
			await oRoot.removeEntry( szName );
		}
		catch ( e )
		{
		}
	}

	if ( a_oMessage.bDelete )
	{
		postMessage( { bDone : true } );
		return;
	}

	var oStream, iTotal;

	if ( a_oMessage.oFile )
	{
		oStream = a_oMessage.oFile.stream();
		iTotal  = a_oMessage.oFile.size;
	}
	else
	{
		var oResponse = await fetch( a_oMessage.szUrl );

		if ( !oResponse.ok )
			throw new Error( "the server has no pack (" + oResponse.status + ")" );

		oStream = oResponse.body;
		iTotal  = +oResponse.headers.get( "Content-Length" ) || 0;
	}

	var oHandle = await ( await oRoot.getFileHandle( "data.tar", { create : true } ) ).createSyncAccessHandle();

	try
	{
		var iSize = await WritePack( oHandle, oStream, iTotal );
		var szIndex = MakeIndex( oHandle, iSize );

		oHandle.flush();
		oHandle.close();
		oHandle = null;

		var oIndex = await ( await oRoot.getFileHandle( "data.idx", { create : true } ) ).createSyncAccessHandle();
		oIndex.truncate( 0 );
		oIndex.write( new TextEncoder().encode( szIndex ), { at : 0 } );
		oIndex.flush();
		oIndex.close();

		postMessage( { bDone : true, iSize : iSize } );
	}
	finally
	{
		if ( oHandle )
			oHandle.close();
	}
}

// Copies the stream into the file, unpacking gzip, and returns the size
async function WritePack( a_oHandle, a_oStream, a_iTotal )
{
	var iRead   = 0;
	var oReader = a_oStream.getReader();
	var oFirst  = await oReader.read();

	var oCounted = new ReadableStream( {
		start : function ( a_oController ) {
			if ( !oFirst.done )
			{
				iRead += oFirst.value.byteLength;
				a_oController.enqueue( oFirst.value );
			}
		},
		pull : async function ( a_oController ) {
			var oChunk = await oReader.read();

			if ( oChunk.done )
			{
				a_oController.close();
				return;
			}

			iRead += oChunk.value.byteLength;
			a_oController.enqueue( oChunk.value );
		}
	} );

	var bGzip = !oFirst.done && oFirst.value[ 0 ] == 0x1f && oFirst.value[ 1 ] == 0x8b;
	var oData = ( bGzip ? oCounted.pipeThrough( new DecompressionStream( "gzip" ) ) : oCounted ).getReader();

	var iPos = 0, fLastReport = 0;
	a_oHandle.truncate( 0 );

	for ( ;; )
	{
		var oChunk = await oData.read();

		if ( oChunk.done )
			break;

		a_oHandle.write( oChunk.value, { at : iPos } );
		iPos += oChunk.value.byteLength;

		var fNow = performance.now();

		if ( fNow - fLastReport > 250 )
		{
			fLastReport = fNow;
			postMessage( { fProgress : a_iTotal ? Math.min( iRead / a_iTotal, 1 ) : -1, iWritten : iPos } );
		}
	}

	return iPos;
}

// Lines: d<tab>0<tab>0<tab>path or f<tab>size<tab>offset<tab>path
function MakeIndex( a_oHandle, a_iSize )
{
	var oHeader  = new Uint8Array( 512 );
	var oDecoder = new TextDecoder();
	var aFiles   = [];
	var szLong   = null;
	var iAt      = 0;

	function Text( a_iOffset, a_iLength )
	{
		var oPart = oHeader.subarray( a_iOffset, a_iOffset + a_iLength );
		var iEnd  = oPart.indexOf( 0 );
		return oDecoder.decode( iEnd < 0 ? oPart : oPart.subarray( 0, iEnd ) );
	}

	function ReadData( a_iOffset, a_iLength )
	{
		var oData = new Uint8Array( a_iLength );
		a_oHandle.read( oData, { at : a_iOffset } );
		return oDecoder.decode( oData );
	}

	while ( iAt + 512 <= a_iSize )
	{
		a_oHandle.read( oHeader, { at : iAt } );

		if ( oHeader[ 0 ] == 0 )
			break;

		var szName = Text( 0, 100 );
		var iSize  = parseInt( Text( 124, 12 ).trim() || "0", 8 );
		var cType  = String.fromCharCode( oHeader[ 156 ] );
		var iData  = iAt + 512;

		if ( Text( 257, 5 ) == "ustar" && oHeader[ 345 ] )
			szName = Text( 345, 155 ) + "/" + szName;

		if ( cType == "L" )
		{
			szLong = ReadData( iData, iSize ).replace( /\0+$/, "" );
		}
		else if ( cType == "x" )
		{
			var oPath = /(?:^|\n)\d+ path=([^\n]*)\n/.exec( ReadData( iData, iSize ) );

			if ( oPath )
				szLong = oPath[ 1 ];
		}
		else if ( cType != "g" )
		{
			if ( cType == "0" || cType == "\0" || cType == "7" )
				aFiles.push( [ szLong || szName, iSize, iData ] );

			szLong = null;
		}

		iAt = iData + Math.ceil( iSize / 512 ) * 512;
	}

	// Paths start with Data/ like the game asks for them, whatever folder the pack was made from
	for ( var aFile of aFiles )
		aFile[ 0 ] = aFile[ 0 ].replace( /\\/g, "/" ).replace( /^(\.\/|\/)+/, "" );

	if ( !aFiles.some( function ( a_aFile ) { return /^data\//i.test( a_aFile[ 0 ] ); } ) )
	{
		for ( var aFile of aFiles )
			aFile[ 0 ] = "Data/" + aFile[ 0 ];
	}

	aFiles = aFiles.filter( function ( a_aFile ) { return /^data\//i.test( a_aFile[ 0 ] ); } );

	if ( !aFiles.length )
		throw new Error( "this isn't a pack of the Data folder" );

	var oDirs  = new Set();
	var aLines = [];

	for ( var aFile of aFiles )
	{
		var aParts = aFile[ 0 ].split( "/" );

		for ( var i = 1; i < aParts.length; i++ )
		{
			var szDir = aParts.slice( 0, i ).join( "/" );

			if ( !oDirs.has( szDir ) )
			{
				oDirs.add( szDir );
				aLines.push( "d\t0\t0\t" + szDir );
			}
		}

		aLines.push( "f\t" + aFile[ 1 ] + "\t" + aFile[ 2 ] + "\t" + aFile[ 0 ] );
	}

	return aLines.join( "\n" );
}
