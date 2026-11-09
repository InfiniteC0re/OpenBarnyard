// The game data as one pack in the page's storage: the game reads it from there instead of asking the server for every file
var oDataPack = {
	oPack : null, // { oFile, szIndex } of the stored pack
	oWorker : null,

	IsSupported : function ()
	{
		return !!( window.isSecureContext && navigator.storage && navigator.storage.getDirectory ) && !/[?&]app(&|$)/.test( location.search );
	},

	// Finds a stored pack, the game starts after this
	Open : async function ()
	{
		if ( !oDataPack.IsSupported() || location.search.indexOf( "nopack" ) >= 0 )
			return;

		try
		{
			var oRoot   = await navigator.storage.getDirectory();
			var szIndex = await ( await ( await oRoot.getFileHandle( "data.idx" ) ).getFile() ).text();
			var oFile   = await ( await oRoot.getFileHandle( "data.tar" ) ).getFile();

			oDataPack.oPack = { oFile : oFile, szIndex : szIndex };
		}
		catch ( e )
		{
		}
	},

	// a_oMessage: { oFile }, { szUrl } or { bDelete }. a_fnProgress gets 0..1 (-1 when the size is unknown) and the bytes written
	Store : function ( a_oMessage, a_fnProgress )
	{
		if ( navigator.storage.persist )
			navigator.storage.persist().catch( function () {} );

		return new Promise( function ( a_fnResolve, a_fnReject ) {
			var oWorker = new Worker( "datapack_worker.js" );
			oDataPack.oWorker = oWorker;

			oWorker.onmessage = function ( a_oEvent ) {
				var oData = a_oEvent.data;

				if ( oData.fProgress !== undefined )
				{
					a_fnProgress( oData.fProgress, oData.iWritten );
					return;
				}

				oWorker.terminate();
				oDataPack.oWorker = null;

				if ( oData.szError )
					a_fnReject( new Error( oData.szError ) );
				else
					a_fnResolve( oData );
			};

			oWorker.onerror = function ( a_oEvent ) {
				oWorker.terminate();
				oDataPack.oWorker = null;
				a_fnReject( new Error( a_oEvent.message || "the storage worker failed" ) );
			};

			oWorker.postMessage( a_oMessage );
		} );
	},

	GetPackUrl : function ()
	{
		return ( new URLSearchParams( location.search ).get( "data" ) || "game/" ) + "Data.tar.gz";
	}
};

// The game's threads are workers made by the runtime. Each one gets the pack before its load message (cmd 1)
( function () {
	var fnPost = Worker.prototype.postMessage;

	Worker.prototype.postMessage = function ( a_oMessage ) {
		if ( oDataPack.oPack && a_oMessage && a_oMessage.cmd === 1 && !this.bHasPack )
		{
			this.bHasPack = true;
			fnPost.call( this, { oDataPack : oDataPack.oPack } );
		}

		return fnPost.apply( this, arguments );
	};
} )();
