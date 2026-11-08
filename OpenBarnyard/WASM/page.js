// The page side of the game: keeps the saves and sets up the module, gl_replay.js does the rendering

// ?log shows the log on the page. Errors are always shown, the game can't start without what they report
var bPageLog = location.search.indexOf( "log" ) >= 0;

// Quit in the game's menu: the Android app (?app) closes, a page shows that the game ended
var bGameQuit = false;

function OnGameQuit()
{
	bGameQuit = true;

	if ( /[?&]app(&|$)/.test( location.search ) )
	{
		location.href = "barnyard://quit";
		return;
	}

	document.body.innerHTML = '<div style="position:absolute;inset:0;display:flex;flex-direction:column;align-items:center;justify-content:center;gap:16px;color:#fff;font:20px sans-serif">' +
		'The game was closed<button style="font:18px sans-serif;padding:8px 20px" onclick="location.reload()">Play again</button></div>';
}

function PageLog( a_szText, a_bError )
{
	// The runtime reports the exit, the page already shows it
	if ( bGameQuit )
		return;

	if ( !bPageLog && !a_bError )
		return;

	var oLog = document.getElementById( "log" );

	if ( !oLog )
	{
		oLog    = document.createElement( "div" );
		oLog.id = "log";
		document.body.appendChild( oLog );
	}

	var oLine = document.createElement( "div" );
	oLine.textContent = a_szText;

	if ( a_bError )
		oLine.style.color = "#f88";

	oLog.appendChild( oLine );

	while ( oLog.childNodes.length > 40 )
		oLog.removeChild( oLog.firstChild );

	oLog.scrollTop = oLog.scrollHeight;
}

window.addEventListener( "error", function ( a_oEvent ) {
	PageLog( "error: " + a_oEvent.message + " (" + a_oEvent.filename + ":" + a_oEvent.lineno + ")", true );
} );

window.addEventListener( "unhandledrejection", function ( a_oEvent ) {
	PageLog( "rejected: " + a_oEvent.reason, true );
} );

( function ()
{
	var oGL = document.createElement( "canvas" ).getContext( "webgl2" );

	PageLog( navigator.userAgent );
	PageLog( "isolated " + window.crossOriginIsolated + ", shared memory " + ( typeof SharedArrayBuffer !== "undefined" ) + ", webgl2 " + !!oGL + ", worklet " + ( typeof AudioWorkletNode !== "undefined" ) );

	if ( oGL )
		PageLog( "s3tc " + !!oGL.getExtension( "WEBGL_compressed_texture_s3tc" ) + ", max texture " + oGL.getParameter( oGL.MAX_TEXTURE_SIZE ) );

	try
	{
		new WebAssembly.Memory( { initial : 6144, maximum : 16384, shared : true } );
		PageLog( "384 MB of shared memory with a 1 GB limit: ok" );
	}
	catch ( e )
	{
		PageLog( "384 MB of shared memory with a 1 GB limit: " + e, true );
	}
} )();

// The game's workers talk to the page directly, the runtime doesn't need to see these messages
( function ()
{
	var fnWorker = window.Worker;

	window.Worker = function ( a_szUrl, a_oOptions )
	{
		// Workers read the page's query (game arguments, data url) from their own location
		var oWorker = new fnWorker( String( a_szUrl ) + location.search, a_oOptions );

		oWorker.addEventListener( "error", function ( a_oEvent ) {
			PageLog( "worker error: " + a_oEvent.message + " (" + a_oEvent.filename + ":" + a_oEvent.lineno + ")", true );
		} );

		oWorker.addEventListener( "message", function ( a_oEvent ) {
			var oData = a_oEvent.data;

			if ( oData && oData.bGLStream )
				oGLReplay.Replay();
			else if ( oData && oData.szPageLog )
				PageLog( oData.szPageLog );
			else if ( oData && oData.bQuit )
				OnGameQuit();
			else if ( oData && oData.bSyncSaves )
				FS.syncfs( false, function ( a_oError ) { if ( a_oError ) console.warn( a_oError ); } );
			else
				return;

			a_oEvent.stopImmediatePropagation();
		} );

		return oWorker;
	};
} )();

var Module = {
	oCanvas : document.getElementById( "game" ),
	oGLReplay : oGLReplay,
	iFrames : 0,

	print : function ( a_szText ) { console.log( a_szText ); PageLog( a_szText ); },
	printErr : function ( a_szText ) { console.warn( a_szText ); PageLog( a_szText, true ); },

	setStatus : function ( a_szText )
	{
		document.getElementById( "status" ).textContent = a_szText;
	},

	// The frames shown so far, for the FPS display and the slow device check
	oStats : { iFrames : 0 },

	OnFramesReplayed : function ( a_iFrames )
	{
		Module.oStats.iFrames += a_iFrames;
		Module.iFrames += a_iFrames;

		if ( Module.iFrames == a_iFrames )
			document.getElementById( "status" ).textContent = "";
	},

	// The saves stay in IndexedDB
	preRun : [ function ()
	{
		FS.mkdir( "/Save" );
		FS.mount( IDBFS, {}, "/Save" );

		addRunDependency( "saves" );
		FS.syncfs( true, function ( a_oError ) {
			if ( a_oError ) console.warn( a_oError );
			removeRunDependency( "saves" );
		} );
	} ],

	onRuntimeInitialized : function ()
	{
		var pCounter = Module._WASM_GetFrameCounter() >> 2;
		var pPad     = Module._WASM_GetPadState();

		oGameInput.Start();

		// Paces the game's frames
		function Tick()
		{
			oGameInput.Update( pPad );

			Atomics.add( Module.HEAP32, pCounter, 1 );
			Atomics.notify( Module.HEAP32, pCounter );
			requestAnimationFrame( Tick );
		}

		requestAnimationFrame( Tick );

		StartAudio();

		var pSize = Module._WASM_GetPageSize() >> 2;

		// The game picks its resolution once, always in landscape. Phones have far more pixels than they can fill (settings.js)
		function UpdatePageSize()
		{
			var fScale  = window.devicePixelRatio || 1;
			var iWidth  = Math.max( window.innerWidth, window.innerHeight ) * fScale;
			var iHeight = Math.min( window.innerWidth, window.innerHeight ) * fScale;

			var iMaxHeight = oPageSettings.GetMaxHeight();

			if ( iMaxHeight && iHeight > iMaxHeight )
			{
				iWidth  = iWidth * iMaxHeight / iHeight;
				iHeight = iMaxHeight;
			}

			Module.HEAP32[ pSize + 0 ] = Math.round( iWidth );
			Module.HEAP32[ pSize + 1 ] = Math.round( iHeight );
		}

		function UpdatePageHidden()
		{
			Atomics.store( Module.HEAP32, pSize + 2, document.hidden ? 1 : 0 );
			Atomics.notify( Module.HEAP32, pCounter );
		}

		window.addEventListener( "resize", UpdatePageSize );
		document.addEventListener( "visibilitychange", UpdatePageHidden );
		UpdatePageSize();
		UpdatePageHidden();

		oPageSettings.ShowFps( oPageSettings.oValues.bShowFps );
		oPageSettings.WatchFrameRate();
	}
};

// Browsers keep sound off until the page is touched or a key is pressed
function StartAudio()
{
	var oContext = new AudioContext( { sampleRate : 44100, latencyHint : "interactive" } );

	oContext.audioWorklet.addModule( "audio_worklet.js" ).then( function () {
		var oNode = new AudioWorkletNode( oContext, "game-audio", {
			numberOfOutputs : 1,
			outputChannelCount : [ 2 ],
			processorOptions : {
				oMemory : Module.HEAPU8.buffer,
				pRing : Module._WASM_GetAudioRing(),
				pWrite : Module._WASM_GetAudioWrite(),
				iFrames : 8192
			}
		} );

		oNode.connect( oContext.destination );
	} );

	function Unlock()
	{
		if ( !document.hidden )
			oContext.resume();
	}

	document.addEventListener( "visibilitychange", function () {
		if ( document.hidden )
			oContext.suspend();
		else
			oContext.resume();
	} );

	for ( var szEvent of [ "pointerdown", "keydown", "touchend" ] )
		window.addEventListener( szEvent, Unlock );
}

if ( window.isSecureContext && bCanShareMemory )
{
	var oGameScript = document.createElement( "script" );
	oGameScript.src = "OpenBarnyard.js";
	document.body.appendChild( oGameScript );
}
